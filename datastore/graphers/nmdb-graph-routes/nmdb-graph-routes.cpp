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

    std::map<std::string, std::set<std::string>> vertexDetails;

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
            "Source host or network for routes")
          );
      opts.addRequiredOption("destination", std::make_tuple(
            "destination,d",
            po::value<std::string>()->required(),
            "Destination host or network for routes")
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
          )"
        );
      db.prepare("select_hop_routes", R"(
          SELECT DISTINCT
              device_id
            , vrf_id
            , table_id
            , is_active
            , dst_ip_net
            , next_vrf_id
            , next_table_id
            , next_hop_ip_addr
            , max(outgoing_interface_name) AS outgoing_interface_name
--            , protocol
--            , administrative_distance
--            , metric
--            , description
          FROM device_ip_routes
          WHERE is_active
            --AND $1 <<= dst_ip_net
            AND $1 && dst_ip_net
            AND $2 = device_id
            AND $3 = vrf_id
            AND NOT (  outgoing_interface_name = 'discard'
                    OR outgoing_interface_name = 'null0'
                    )
          GROUP BY  device_id
                  , vrf_id
                  , table_id
                  , is_active
                  , dst_ip_net
                  , next_vrf_id
                  , next_table_id
                  , next_hop_ip_addr
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
          )"
        );
    }

    void
    buildRouteGraph(pqxx::connection& db)
    {
      pqxx::work t {db};

      // TODO
      findRoutesBetweenPoints(t);
      addVertexDetails();

      t.abort();
      finalizeVertices();
    }

    void
    findRoutesBetweenPoints(pqxx::transaction_base& t)
    {
      // add source and destination nodes
      addVertex("box", testStartIp);
      addVertex("box", testDestIp);

      pqxx::result nextHops =
        t.exec_prepared("select_initial_hops", testStartIp);

      for (const auto& nextHop : nextHops) {
        const std::string deviceId    {nextHop.at("device_id").c_str()};
        const std::string vrfId       {nextHop.at("vrf_id").c_str()};
        const std::string ipNet       {nextHop.at("ip_net").c_str()};
        const std::string inIfaceName {nextHop.at("interface_name").c_str()};

        const std::string id {vrfId + "::" + deviceId};
        getPossibleRoutes(t, deviceId, vrfId, inIfaceName);
        if (vertexLookup.count(id)) {
          addVertex("oval", ipNet);
          addEdge(testStartIp, ipNet);

          LOG_ERROR << "Route was found starting at: "
                    << ipNet << "->" << id
                    << std::endl;
          addEdge(ipNet, id);
        }
      }
    }

    // This behaves as a DFS type lookup for routes
    void
    getPossibleRoutes( pqxx::transaction_base& t
                     , const std::string& deviceId
                     , const std::string& vrfId
                     , const std::string& inIfaceName
                     )
    {
      const std::string id {vrfId + "::" + deviceId};

      // process hop route(s) where destination is reachable
      // - empty if no routes, otherwise a route is known
      pqxx::result routes =
        t.exec_prepared("select_hop_routes", testDestIp, deviceId, vrfId);
      for (const auto& route : routes) {
        const std::string nextHopIpAddr {route.at("next_hop_ip_addr").c_str()};
        
        // found if node out IP is destination; else inspect further
        if ((testDestIp == nextHopIpAddr)) {
          LOG_ERROR << "Route found ending at: "
                    << id << "->" << testDestIp
                    << std::endl;
          //addVertex("box", id);
          addVertexRoute(route, inIfaceName);
          addEdge(id, testDestIp);
        } else {
          // can be multiple next hops (e.g. failover)
          pqxx::result nextHops =
            t.exec_prepared("select_next_hops", nextHopIpAddr);
          // route entry for destination, but next hop not known
          if (0 == nextHops.size()) {
            LOG_ERROR << "Possible route: "
                      << id << "->" << testDestIp
                      << std::endl;
            //addVertex("box", id);
            addVertexRoute(route, inIfaceName);
            addEdge(id, testDestIp, "dashed");
          } else {
            for (const auto& nextHop : nextHops) {
              const std::string nextDeviceId  {nextHop.at("device_id").c_str()};
              const std::string nextVrfId     {nextHop.at("vrf_id").c_str()};

              const std::string nextId {nextVrfId + "::" + nextDeviceId};

              if (!visited.contains(nextId)) {
                visited.insert(nextId);
                LOG_ERROR << "Visiting: "
                          << nextId
                          << std::endl;

                // continue along path until can't
                const std::string nextInIfaceName
                  {nextHop.at("interface_name").c_str()};
                getPossibleRoutes(t, nextDeviceId, nextVrfId, nextInIfaceName);
              }

              if (vertexLookup.contains(nextId)) {
                const std::string ipNet {nextHop.at("ip_net").c_str()};
                LOG_ERROR << "NextHop was viable: "
                          << ipNet << "->" << nextId
                          << std::endl;
                addVertex("oval", ipNet);
                addEdge(ipNet, nextId);
                addVertexRoute(route, inIfaceName);
                //addVertex("box", id);
                addEdge(id, ipNet);
              }
            }
          }
        }
      }
    }

    void
    addVertexRoute(const auto& route
                  , const std::string& inIfaceName
                  )
    {
      const std::string vrfId     {route.at("vrf_id").c_str()};
      const std::string deviceId  {route.at("device_id").c_str()};

      const std::string id {vrfId + "::" + deviceId};

      std::ostringstream oss;
      oss << "From " << inIfaceName
          << " to "
          << route.at("dst_ip_net")
          << " via "
          << route.at("outgoing_interface_name")
          << " / "
          << route.at("next_hop_ip_addr")
          << R"(<br align="left"/>)"
          ;

      if (!vertexDetails.contains(id)) {
        vertexDetails.emplace(id, std::set<std::string>());
      }
      vertexDetails.at(id).insert(oss.str());

      addVertex("box", id);
    }

    void
    addVertex( const std::string& shape
             , const std::string& id
             , const std::string& label=""
             )
    {
      LOG_DEBUG << "Vertex: " << id << " (" << shape << ") -- " << label
                << std::endl;

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
      graph[v].shape = shape;//"rectangle";
      graph[v].style = "rounded";
    }

    std::string
    getLabelHeader(const std::string& id) {
      std::ostringstream oss;

      oss << R"(" + <<table border="0" cellborder="0"><tr>)"
//          << getIconString(id)
          << R"(<td></td>)" // TODO empty icon
          << R"(<td><font point-size="11">)" << id << R"(<br/>)"
          ;

      return oss.str();
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
      LOG_DEBUG << "Edge: " << src << " <-> " << dst << std::endl;

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
          tie(e, inserted) = boost::add_edge(u, v, graph);
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
