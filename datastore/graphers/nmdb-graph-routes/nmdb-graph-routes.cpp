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
            po::value<std::string>()->required()
              ->default_value("10.0/16"),
            "Source host or network for routes")
          );
      opts.addRequiredOption("destination", std::make_tuple(
            "destination,d",
            po::value<std::string>()->required()
              ->default_value("8.8.8.8"),
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
      //db.prepare("", R"(
      //    )"
      //  );

      db.prepare("select_initial_hops", R"(
          SELECT DISTINCT
              device_id
            , vrf_id
            , interface_name
            , ip_addr
            , ip_net
          FROM device_vrfs_ip_addrs
          WHERE ip_net  && $1
            AND ip_addr != $1
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
            , outgoing_interface_name
            , protocol
            , administrative_distance
            , metric
            , description
          FROM device_ip_routes
          WHERE is_active
            AND $1 <<= dst_ip_net
            AND $2 = device_id
            AND $3 = vrf_id
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


//      db.prepare("select_next_hop", R"(
//          SELECT DISTINCT
//            --, dst_ip_net
//              device_id
//            , vrf_id
//            --, table_id
//            --, is_active
//            --, protocol
//            --, administrative_distance
//            --, metric
//            --, incoming_interface_name
//            --, incoming_ip_addr
//            , incoming_ip_net
//            --, outgoing_interface_name
//            --, outgoing_ip_addr
//            --, outgoing_ip_net
//            --, next_hop_device_id
//            --, next_hop_vrf_id
//            --, next_hop_incoming_interface_name
//            --, next_hop_incoming_ip_addr
//            --, next_hop_incoming_ip_net
//          FROM device_ip_route_connections
//          WHERE (is_active = true)
//            AND ( (   (incoming_interface_name NOT LIKE 'management%')
//                  AND (incoming_interface_name NOT LIKE 'mgmt%')
//                  )
//                OR (incoming_interface_name IS NULL)
//                )
//            AND ( (   (outgoing_interface_name NOT LIKE 'management%')
//                  AND (outgoing_interface_name NOT LIKE 'mgmt%')
//                  )
//                OR (outgoing_interface_name IS NULL)
//                )
//            AND ($1 && incoming_ip_net)
//          )"
//        );

//      db.prepare("select_next_hops", R"(
//          SELECT DISTINCT
//              dst_ip_net
//            , device_id
//            , vrf_id
//            --, table_id
//            --, is_active
//            --, protocol
//            --, administrative_distance
//            --, metric
//            , incoming_interface_name
//            , incoming_ip_addr
//            , incoming_ip_net
//            , outgoing_interface_name
//            , outgoing_ip_addr
//            , outgoing_ip_net
//            --, next_hop_device_id
//            --, next_hop_vrf_id
//            --, next_hop_incoming_interface_name
//            --, next_hop_incoming_ip_addr
//            --, next_hop_incoming_ip_net
//          FROM device_ip_route_connections
//          WHERE (is_active = true)
//            AND ( (   (incoming_interface_name NOT LIKE 'management%')
//                  AND (incoming_interface_name NOT LIKE 'mgmt%')
//                  )
//                OR (incoming_interface_name IS NULL)
//                )
//            AND ( (   (outgoing_interface_name NOT LIKE 'management%')
//                  AND (outgoing_interface_name NOT LIKE 'mgmt%')
//                  )
//                OR (outgoing_interface_name IS NULL)
//                )
//          )"
//        );

//      db.prepare("select_hop_routes", R"(
//          SELECT DISTINCT
//              dst_ip_net
//            , device_id
//            , vrf_id
//            --, table_id
//            --, is_active
//            --, protocol
//            --, administrative_distance
//            --, metric
//            --, incoming_interface_name
//            --, incoming_ip_addr
//            , incoming_ip_net
//            , outgoing_interface_name
//            , outgoing_ip_addr
//            , outgoing_ip_net
//            --, next_hop_device_id
//            --, next_hop_vrf_id
//            --, next_hop_incoming_interface_name
//            --, next_hop_incoming_ip_addr
//            --, next_hop_incoming_ip_net
//          FROM device_ip_route_connections
//          WHERE (is_active = true)
//            AND ( (   (incoming_interface_name NOT LIKE 'management%')
//                  AND (incoming_interface_name NOT LIKE 'mgmt%')
//                  )
//                OR (incoming_interface_name IS NULL)
//                )
//            AND ( (   (outgoing_interface_name NOT LIKE 'management%')
//                  AND (outgoing_interface_name NOT LIKE 'mgmt%')
//                  )
//                OR (outgoing_interface_name IS NULL)
//                )
//            AND (   $1 = device_id
//                AND $2 = vrf_id
//                AND $3 <<= dst_ip_net
//                )
//          ORDER BY dst_ip_net desc
//          LIMIT 1
//          )"
//        );

      // TODO
    }

    void
    buildRouteGraph(pqxx::connection& db)
    {
      pqxx::work t {db};

      // TODO
      findRoutesBetweenPoints(t);

      t.abort();
      finalizeVertices();
    }

    /*
      findRoutes()
        for each nextHop(source)
          getPossibleRoutes()

      getPossibleRoutes()
        rExists = false

        get node route where target is reachable
        if route == default
          rExists = true
        if route == target
          rExists = true
        otherwise
          get node nextHops via outIp
          for each nextHop(outIp)
            if nextHop exists
              rExists = getPossibleRoutes()
            else
              rExists = true

        if rExists
          add data
          
        return rExists
          
    */
    void
    findRoutesBetweenPoints(pqxx::transaction_base& t)
    {
      // add source and destination nodes
      addVertex("box", testStartIp);
      addVertex("box", testDestIp);

      pqxx::result nextHops =
        t.exec_prepared("select_initial_hops", testStartIp);

      for (const auto& nextHop : nextHops) {
        std::string deviceId;
        nextHop.at("device_id").to(deviceId);
        std::string vrfId;
        nextHop.at("vrf_id").to(vrfId);
        std::string ipNet;
        nextHop.at("ip_net").to(ipNet);

        const std::string id {vrfId + "::" + deviceId};
        addVertex("oval", ipNet);
        addEdge(testStartIp, ipNet);

        getPossibleRoutes(t, deviceId, vrfId);
        if (vertexLookup.count(id)) {
          LOG_ERROR << "Route was found starting at: "
                    << ipNet << "->" << id
                    << std::endl;
          addEdge(ipNet, id);
        }
      }
    }
//    void
//    findRoutesBetweenPoints(pqxx::transaction_base& t)
//    {
//      // add source and destination nodes
//      addVertex("oval", testStartIp);
//      addVertex("oval", testDestIp);
//
//      pqxx::result nextHops =
//        t.exec_prepared("select_next_hop", testStartIp);
//
//      for (const auto& nextHop : nextHops) {
//        std::string deviceId;
//        nextHop.at("device_id").to(deviceId);
//        std::string vrfId;
//        nextHop.at("vrf_id").to(vrfId);
//        std::string inIpNet;
//        nextHop.at("incoming_ip_net").to(inIpNet);
//
//        if (getPossibleRoutes(t, deviceId, vrfId)) {
//          std::string id {vrfId + "::" + deviceId};
//          addEdge(inIpNet, id);
//        }
//      }
//    }

    void
    getPossibleRoutes( pqxx::transaction_base& t
                     , const std::string& deviceId
                     , const std::string& vrfId
                     )
    {
      std::string id {vrfId + "::" + deviceId};

      // process hop route(s) where destination is reachable
      // - empty if no routes, otherwise a route is known
      pqxx::result routes =
        t.exec_prepared("select_hop_routes", testDestIp, deviceId, vrfId);
      for (const auto& route : routes) {
        std::string nextHopIpAddr;
        route.at("next_hop_ip_addr").to(nextHopIpAddr);
        
        // found if node out IP is destination; else inspect further
        if ((testDestIp == nextHopIpAddr)) {
          LOG_ERROR << "Route found ending at: "
                    << id << "->" << testDestIp
                    << std::endl;
          addVertex("box", id);
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
            addVertex("box", id);
            addEdge(id, testDestIp);
          } else {
            for (const auto& nextHop : nextHops) {
              std::string nextDeviceId;
              nextHop.at("device_id").to(nextDeviceId);
              std::string nextVrfId;
              nextHop.at("vrf_id").to(nextVrfId);
              std::string ipNet;
              nextHop.at("ip_net").to(ipNet);

              const std::string nextId {nextVrfId + "::" + nextDeviceId};

              if (!visited.contains(nextId)) {
                visited.insert(nextId);
                LOG_ERROR << "Visiting: "
                          << nextId
                          << std::endl;

                // continue along path until can't
                getPossibleRoutes(t, nextDeviceId, nextVrfId);
              }

              if (vertexLookup.contains(nextId)) {
                LOG_ERROR << "NextHop was viable: "
                          << ipNet << "->" << nextId
                          << std::endl;
                addVertex("oval", ipNet);
                addEdge(ipNet, nextId);
                addVertex("box", id);
                addEdge(id, ipNet);
              }
            }
          }
        }
      }
    }

//    bool
//    getPossibleRoutes( pqxx::transaction_base& t
//                     , const std::string& deviceId
//                     , const std::string& vrfId
//                     )
//    {
//      bool        routeExists {false};
//      std::string id          {vrfId + "::" + deviceId};
//
//      std::string dstIpNet;
//      std::string outIpAddr;
//      std::string outIpNet;
//      std::string inIpNet;
//     
//      LOG_ERROR << "visiting: "
//                << deviceId << "::" << vrfId << "--" << testDestIp
//                << std::endl;
//
//      // process hop route(s) where destination is reachable
//      // - empty if no routes, otherwise a route is known
//      pqxx::result routes =
//        t.exec_prepared("select_hop_routes", deviceId, vrfId, testDestIp);
//      for (const auto& route : routes) {
//        route.at("dst_ip_net").to(dstIpNet);
//        route.at("outgoing_ip_addr").to(outIpAddr);
//        route.at("outgoing_ip_net").to(outIpNet);
//        route.at("incoming_ip_net").to(inIpNet);
//        
//        LOG_ERROR << "Route: " << dstIpNet << " via " << outIpAddr << std::endl;
//
//        // found if node out IP is destination or next hop is default
//        if ((testDestIp == outIpAddr)) {
//          LOG_ERROR << "Route found" << std::endl;
//          routeExists = true;
//        }
//        // further inspect route
//        else {
//          // can be multiple next hops (e.g. failover)
//          pqxx::result nextHops =
//            t.exec_prepared("select_next_hop", outIpAddr);
//          // route entry for destination, but next hop not known
//          if (0 == nextHops.size()) {
//            LOG_ERROR << "Viable route" << std::endl;
//            routeExists = true;
//          }
//          else {
//            for (const auto& nextHop : nextHops) {
//              std::string nextDeviceId;
//              nextHop.at("device_id").to(nextDeviceId);
//              std::string nextVrfId;
//              nextHop.at("vrf_id").to(nextVrfId);
//
//              const std::string visitId {nextDeviceId + nextVrfId};
//              if (!visited.contains(visitId)) {
//                visited.insert(visitId);
//                LOG_ERROR << "Visiting: "
//                          << nextDeviceId << "::" << nextVrfId
//                          << std::endl;
//
//                // continue along path until can't
//                if (getPossibleRoutes(t, nextDeviceId, nextVrfId)) {
//                  LOG_ERROR << "Viable nextHop" << std::endl;
//                  routeExists = true;
//                }
//              } else {
//                if ("0.0.0.0/0" == dstIpNet) {
//                  LOG_ERROR << "Default routed" << std::endl;
//                  routeExists = true;
//                }
//              }
//            }
//          }
//        }
//
//        if (routeExists) {
//          // add subnet nodes
//          addVertex("oval", inIpNet);
//          addVertex("oval", outIpNet);
//
//          // add host node
//          // TODO? list viable host routes
//          addVertex("box", id);
//
//          // add edges between host and subnets
//          addEdge(inIpNet, id);
//          addEdge(id, outIpNet);
//        }
//      }
//
//      return routeExists;
//    }

    bool
    doesNetContainIp( pqxx::transaction_base& t
                    , const std::string& ipNet, const std::string& ip)
    {
      pqxx::result rows =
        t.exec_prepared("select_net_contains_ip", ipNet, ip);

      // TODO clean up
      for (const auto& row : rows) {
        bool contains;
        row.at("contains").to(contains);

        if (contains) {
          return true;
        }
      }
      return false;
    }

    void
    getNextHops(pqxx::transaction_base& t, const std::string& startHop)
    {
      pqxx::result rows =
        t.exec_prepared("select_next_hop", startHop);

      for (const auto& row : rows) {
        std::string inIpNet;
        row.at("incoming_ip_net").to(inIpNet);
        std::string inIpAddr;
        row.at("incoming_ip_addr").to(inIpAddr);
        std::string inIfaceName;
        row.at("incoming_interface_name").to(inIfaceName);
        std::string deviceId;
        row.at("device_id").to(deviceId);
        std::string vrfId;
        row.at("vrf_id").to(vrfId);
        std::string outIfaceName;
        row.at("outgoing_interface_name").to(outIfaceName);
        std::string outIpNet;
        row.at("outgoing_ip_net").to(outIpNet);
        std::string outIpAddr;
        row.at("outgoing_ip_addr").to(outIpAddr);

        std::string id {vrfId + "::" + deviceId};
        bool doRecurse {
               !vertexLookup.contains(inIpNet)
            || !vertexLookup.contains(outIpNet)
            || !vertexLookup.contains(id)
          };

        // add subnet nodes
        addVertex("oval", inIpNet);
        addVertex("oval", outIpNet);

        // add host node
        std::ostringstream oss;

        oss << inIpAddr
            << "--" << inIfaceName
            << " --- "
            << outIfaceName
            << "--" << outIpAddr
            << R"(<br align="left"/>)"
            ;

        addVertex("box", id, oss.str());

        // add edges between host and subnets
        addEdge(inIpNet, id);
        addEdge(id, outIpNet);

        // recurse
        if (doRecurse) {
          getNextHops(t, outIpAddr);
        }
      }
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
    finalizeVertices()
    {
      for (const auto& [_, v] : vertexLookup) {
        graph[v].label += R"(</font></td></tr></table>> + ")";
      }
    }

    void
    addEdge(const std::string& src, const std::string& dst)
    {
      LOG_DEBUG << "Edge: " << src << " <-> " << dst << std::endl;

      try {
        const auto u {vertexLookup.at(src)};
        const auto v {vertexLookup.at(dst)};

        if (edgeLookup.count(src) && edgeLookup[src].count(dst)) {
          const auto e {edgeLookup.at(src).at(dst)};
        } else if (edgeLookup.count(dst) && edgeLookup[dst].count(src)) {
          const auto e {edgeLookup.at(dst).at(src)};
        } else {
          Edge e;
          bool inserted;
          tie(e, inserted) = boost::add_edge(u, v, graph);
          edgeLookup[src][dst] = e;
        }
      } catch (const std::exception& e) {
        LOG_ERROR << "Failed to create edge: " << e.what()
                  << std::endl;
        for (const auto& i : {src,dst}) {
          if (!vertexLookup.count(i)) {
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
