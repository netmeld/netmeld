// =============================================================================
// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// =============================================================================
// Maintained by Sandia National Laboratories <Netmeld@sandia.gov>
// =============================================================================

#include <netmeld/datastore/tools/AbstractGraphTool.hpp>

#include "GraphHelper.hpp"

namespace nmdt = netmeld::datastore::tools;
namespace nmdu = netmeld::datastore::utils;
namespace nmcu = netmeld::core::utils;


// =============================================================================
// Graph tool definition
// =============================================================================
class Tool : public nmdt::AbstractGraphTool
{
  // ===========================================================================
  // Variables
  // ===========================================================================
  private: // Variables should generally be private
    RouteGraph                                          graph;
    std::map<std::string, Vertex>                       vertexLookup;
    std::map<std::string, std::map<std::string, Edge>>  edgeLookup;

    std::set<std::string> visited;

    // TODO
    std::string testStartIp;
    std::string testDestIp;

    bool noRouteDetails {false};

    std::map<std::string, std::set<std::string>> vertexDetails;

    double maxHostCount {0}; // PostgreSQL BIGINT is 8 bytes

  protected: // Variables intended for internal/subclass API
  public: // Variables should rarely appear at this scope


  // ===========================================================================
  // Constructors
  // ===========================================================================
  private: // Constructors should rarely appear at this scope
  protected: // Constructors intended for internal/subclass API
  public: // Constructors should generally be public
    Tool() : nmdt::AbstractGraphTool
      (
       "help blurb",    // help blurb, prefixed with:
                        //   "Create dot formatted graph of "
       PROGRAM_NAME,    // program name (set in CMakeLists.txt)
       PROGRAM_VERSION  // program version (set in CMakeLists.txt)
      )
    {}


  // ===========================================================================
  // Methods
  // ===========================================================================
  private: // Methods part of internal API
    // Overriden from AbstractGraphTool
    void
    addToolOptions() override
    {
      opts.addRequiredOption("source", std::make_tuple(
            "source,s",
            po::value<std::string>()->required(),
            "Route(s) start at this IP/CIDR")
          );
      opts.addRequiredOption("destination", std::make_tuple(
            "destination,d",
            po::value<std::string>()->required(),
            "Route(s) end at this IP/CIDR")
          );

      opts.addOptionalOption("no-route-details", std::make_tuple(
            "no-route-details",
            NULL_SEMANTIC,
            "Do not show route details on hops")
          );
    }

    // Overriden from AbstractGraphTool
    int
    runTool() override
    {
      processOptions();

      pqxx::connection db {getDbConnectString()};
      nmdu::dbPrepareCommon(db);

      dbPrepareToolSpecific(db);

      // TODO control flags

      buildRouteGraph(db);

      boost::write_graphviz
        (std::cout, graph,
         LabelWriter(graph),   // VertexPropertyWriter
         LabelWriter(graph),   // EdgePropertyWriter
         GraphWriter(),        // GraphPropertyWriter
         boost::get(&VertexProperties::name, graph));  // VertexID

      return nmcu::Exit::SUCCESS;
    }

    void
    processOptions()
    {
      testStartIp = opts.getValue("source");
      testDestIp  = opts.getValue("destination");

      noRouteDetails = opts.exists("no-route-details");
    }

    void
    dbPrepareToolSpecific(pqxx::connection& db)
    {
      db.prepare("select_initial_hops", R"(
          SELECT DISTINCT
              device_id
            , vrf_id
            , interface_name
            , ip_addr
            , ip_net
          FROM device_vrfs_ip_addrs
          WHERE ip_net && $1
          ORDER BY device_id, vrf_id
          )"
        );
      db.prepare("select_hop_routes", R"(
          SELECT DISTINCT
              device_id
            , vrf_id
            , table_id
--            , is_active
            , dst_ip_net
            , next_vrf_id
            , next_table_id
            , next_hop_ip_addr
            , max(outgoing_interface_name) AS outgoing_interface_name
--            , protocol
--            , administrative_distance
--            , metric
--            , description
            , CASE
                WHEN family(dst_ip_net) = 4 THEN
                  (2^(32 - MASKLEN(dst_ip_net)))
                ELSE
                  (2^(128 - MASKLEN(dst_ip_net)))
              END AS count
            , CASE
                WHEN (COALESCE(MAX(outgoing_interface_name), '')
                      IN ('discard', 'null0', 'reject')
                     ) THEN
                  true
                ELSE
                  false
              END AS is_blackhole
          FROM device_ip_routes
          WHERE is_active
--            AND $1 <<= dst_ip_net
            AND $1 && dst_ip_net
            AND $2 = device_id
            AND $3 = COALESCE(vrf_id, '')
--            AND NOT (COALESCE(outgoing_interface_name, '')
--                     IN ('discard', 'null0', 'reject')
--                    )
          GROUP BY  device_id
                  , vrf_id
                  , table_id
--                  , is_active
                  , dst_ip_net
                  , next_vrf_id
                  , next_table_id
                  , next_hop_ip_addr
          ORDER BY dst_ip_net DESC
          )"
        );
      db.prepare("select_next_hops", R"(
          SELECT DISTINCT
              device_id
            , vrf_id
            , interface_name
            , ip_addr
            , ip_net
          FROM device_vrfs_ip_addrs
          WHERE ip_addr = $1
          ORDER BY device_id, vrf_id
          )"
        );
      db.prepare("count_max_hosts", R"(
          SELECT
              CASE
                WHEN family($1) = 4 THEN
                  (2^(32 - MASKLEN($1)))
                ELSE
                  (2^(128 - MASKLEN($1)))
              END AS max_host_count
          )"
        );
    }

    void
    buildRouteGraph(pqxx::connection& db)
    {
      pqxx::read_transaction rt {db};

      maxHostCount = (rt.exec_prepared1("count_max_hosts", testDestIp)
                     )[0].as<double>()
        ;

      // TODO
      findRoutesBetweenPoints(rt);
      addVertexDetails();

      finalizeVertices();
    }

    void
    findRoutesBetweenPoints(pqxx::read_transaction& rt)
    {
      // add source and destination nodes
      addVertex("box", testStartIp);
      addVertex("box", testDestIp);

      for ( const auto& nextHop
          : rt.exec_prepared("select_initial_hops", testStartIp)
          )
      {
        const std::string deviceId    {nextHop.at("device_id").c_str()};
        const std::string vrfId       {nextHop.at("vrf_id").c_str()};
        const std::string ipNet       {nextHop.at("ip_net").c_str()};
        const std::string inIfaceName {nextHop.at("interface_name").c_str()};

        const std::string id {getLookupId(vrfId, deviceId)};
        getPossibleRoutes(rt, deviceId, vrfId, inIfaceName);
        if (vertexLookup.count(id)) {
          addVertex("oval", ipNet);
          addEdge(testStartIp, ipNet);

          LOG_DEBUG << "Route was found starting at: "
                    << ipNet << "->" << id
                    << std::endl;
          addEdge(ipNet, id);
        }
      }
    }

    // This behaves as a DFS type lookup for routes
    void
    getPossibleRoutes( pqxx::read_transaction& rt
                     , const std::string& deviceId
                     , const std::string& vrfId
                     , const std::string& inIfaceName
                     )
    {
      const std::string id {getLookupId(vrfId, deviceId)};

      // process hop route(s) where destination is reachable
      // - empty if no routes, otherwise a route is known
      auto hostsToCover {maxHostCount};
      for ( const auto& route
          : rt.exec_prepared("select_hop_routes", testDestIp, deviceId, vrfId)
          )
      {
        const std::string nextHopIpAddr {route.at("next_hop_ip_addr").c_str()};
        const std::string dstIpNet      {route.at("dst_ip_net").c_str()};
        const double hostCount          {route.at("count").as<double>()};
        bool isBlackhole                {route.at("is_blackhole").as<bool>()};

        // Except for default route, routes can not overlap
        LOG_DEBUG << std::format("Route: On {} -- to {} via {} ({})\n"
                                , id
                                , dstIpNet
                                , nextHopIpAddr
                                , route.at("outgoing_interface_name").c_str()
                                )
                  ;
        if (hostsToCover <= 0) { // known paths cover all hosts
          LOG_DEBUG << "Skipping un-reachable route\n";
          continue;
        } else { // paths left to cover
          LOG_DEBUG << std::format("Route covers {} of {} hosts left\n"
                                  , hostCount
                                  , hostsToCover
                                  )
                    ;
          hostsToCover -= hostCount;
        }

        // last hop found; IP/CIDR is destination
        if (testDestIp == nextHopIpAddr || testDestIp == dstIpNet) {
          LOG_DEBUG << "Final hop found\n";
          addVertexRoute(route, inIfaceName);
          addEdge(id, testDestIp);
        }
        // last hop found; route ultimately discards packets
        else if (isBlackhole) {
          LOG_DEBUG << "Blackhole route found\n";
          addVertexRoute(route, inIfaceName);
          addVertex("octagon", "Blackhole");
          addEdge(id, "Blackhole");
        }
        // route via interface, possible unknown next hop (i.e., no arp data)
        else if (nextHopIpAddr.empty()) {
          LOG_DEBUG << "Routing via interface\n";
          /* TODO
            - if next hop device known, use it
            - else connect to endpoint with dotted line

            NOTE: probably leverage nextHops and adjust logic to combine
            above/below.
          */
        }
        // route via IP
        else {
          LOG_DEBUG << "Routing via IP\n";
          // can be multiple next hops (e.g. failover)
          pqxx::result nextHops {
              rt.exec_prepared("select_next_hops", nextHopIpAddr)
            };
          // route entry for destination, but next hop not known
          if (0 == nextHops.size()) {
            LOG_DEBUG << "Possible final hop found\n";
            addVertexRoute(route, inIfaceName);
            addEdge(id, testDestIp, "dashed");
          } else {
            LOG_DEBUG << "More valid hops found\n";
            for (const auto& nextHop : nextHops) {
              const std::string nextDeviceId  {nextHop.at("device_id").c_str()};
              const std::string nextVrfId     {nextHop.at("vrf_id").c_str()};

              const std::string nextId {getLookupId(nextVrfId, nextDeviceId)};

              LOG_DEBUG << std::format("From {}, examining {}\n", id, nextId);
              if (!visited.contains(nextId)) {
                LOG_DEBUG << "Checking unvisitied next hop\n";
                visited.insert(nextId);
                // continue along path until can't
                const std::string nextInIfaceName
                  {nextHop.at("interface_name").c_str()};
                getPossibleRoutes(rt, nextDeviceId, nextVrfId, nextInIfaceName);
              }

              if (vertexLookup.contains(nextId)) {
                LOG_DEBUG << std::format("Connecting viable next hop:"
                                         "{} -> {}\n"
                                        , id
                                        , nextId
                                        )
                          ;
                const std::string ipNet {nextHop.at("ip_net").c_str()};
                addVertex("oval", ipNet);
                addEdge(ipNet, nextId);
                addVertexRoute(route, inIfaceName);
                addEdge(id, ipNet);
              }
            }
          }
        }
      }
    }

    void
    addVertexRoute( const auto& route
                  , const std::string& inIfaceName
                  )
    {
      const std::string id {
          getLookupId( route.at("vrf_id").c_str()
                     , route.at("device_id").c_str()
                     )
        };

      if (!vertexDetails.contains(id)) {
        vertexDetails.emplace(id, std::set<std::string>());
        addVertex("box", id);
      }

      if (!noRouteDetails) {
        vertexDetails.at(id).insert(
            std::format(R"(From {} to {} via {} ({})<br align="left"/>)"
                       , inIfaceName
                       , route.at("dst_ip_net").c_str()
                       , route.at("next_hop_ip_addr").c_str()
                       , route.at("outgoing_interface_name").c_str()
                       )
          );
      }
    }

    void
    addVertex( const std::string& shape
             , const std::string& id
             , const std::string& label=""
             )
    {
      LOG_DEBUG << std::format("Vertex: {} () -- {}\n", id, shape, label);

      std::string mLabel {label};
      if (!vertexLookup.count(id)) {
        Vertex v {boost::add_vertex(graph)};
        vertexLookup[id] = v;

        mLabel = getLabelHeader(id) + label;
      }

      const auto v {vertexLookup[id]};
      graph[v].name = id;
      if (!mLabel.empty()) {
        graph[v].label += mLabel;
      }
      graph[v].shape = shape;
      graph[v].style = "rounded";
    }

    std::string
    getLabelHeader(const std::string& id)
    {
      return std::format(R"(" + <<table border="0" cellborder="0"><tr>)"
                         R"(<td></td>)" // empty icon
                         R"(<td><font point-size="11">{}<br/>)"
                        , id
                        )
        ;
    }

    void
    addVertexDetails()
    {
      for (const auto& [vertex, dataSet] : vertexDetails) {
        for (const auto& data : dataSet) {
          addVertex("box", vertex, data);
        }
      }
    }

    void
    finalizeVertices()
    {
      for (const auto& [_, v] : vertexLookup) {
        graph[v].label += R"(</font></td></tr></table>> + ")";
      }
    }

    void
    addEdge( const std::string& src
           , const std::string& dst
           , const std::string& style=""
           )
    {
      LOG_DEBUG << std::format("Edge: {} <-> {}\n", src, dst);

      try {
        const auto u {vertexLookup.at(src)};
        const auto v {vertexLookup.at(dst)};

        Edge e;
        if (edgeLookup.contains(src) && edgeLookup[src].contains(dst)) {
          e = edgeLookup.at(src).at(dst);
        } else if (edgeLookup.contains(dst) && edgeLookup[dst].contains(src)) {
          e = edgeLookup.at(dst).at(src);
        } else {
          bool inserted;
          std::tie(e, inserted) = boost::add_edge(u, v, graph);
          edgeLookup[src][dst] = e;
        }
        if (!style.empty()) {
          graph[e].style = style;
        }
      } catch (const std::exception& e) {
        LOG_ERROR << "Failed to create/update edge: " << e.what()
                  << std::endl;
        for (const auto& i : {src,dst}) {
          if (!vertexLookup.contains(i)) {
            LOG_ERROR << " - Non-existant vertex: " << i << std::endl;
          }
        }
      }
    }

  std::string
  getLookupId(const std::string& v1, const std::string& v2)
  {
    return std::format("{}::{}", v1, v2);
  }


  protected: // Methods part of subclass API
  public: // Methods part of public API
};


// =============================================================================
// Program entry point
// =============================================================================
int main(int argc, char** argv) {
  Tool tool;
  return tool.start(argc, argv);
}
