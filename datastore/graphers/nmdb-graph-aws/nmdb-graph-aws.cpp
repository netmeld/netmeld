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

#include <regex>

#include <netmeld/datastore/objects/PortRange.hpp>
#include <netmeld/datastore/tools/AbstractGraphTool.hpp>

#include "GraphHelper.hpp"

namespace nmdo = netmeld::datastore::objects;
namespace nmdt = netmeld::datastore::tools;
namespace nmdu = netmeld::datastore::utils;
namespace nmcu = netmeld::core::utils;


class IsRedundantEdge
{
  private:
    const AwsGraph& g_;
  public:
    explicit IsRedundantEdge(const AwsGraph& g) : g_(g) { }

    bool operator()(Edge const& e) const
    {
      const Vertex s {boost::source(e, g_)};
      const Vertex t {boost::target(e, g_)};

      return (get<1>(boost::edge(t, s, g_))
          && (   (g_[s].distance > g_[t].distance)
              || ((s > t) && (g_[s].distance >= g_[t].distance))
             ));
    }
};


// =============================================================================
// Graph tool definition
// =============================================================================
class Tool : public nmdt::AbstractGraphTool
{
  // ===========================================================================
  // Variables
  // ===========================================================================
  private: // Variables should generally be private
    AwsGraph graph;

    std::map<std::string, Vertex>
      vertexLookup;
    std::map<std::string, std::map<std::string, Edge>>
      edgeLookup;

    bool noDetails            {false};
    bool noNetworkInterfaces  {false};
    bool graphInstances       {false};

  protected: // Variables intended for internal/subclass API
    // Inhertied from AbstractTool at this scope
      // std::string            helpBlurb;
      // std::string            programName;
      // std::string            version;
      // ProgramOptions         opts;
  public: // Variables should rarely appear at this scope


  // ===========================================================================
  // Constructors
  // ===========================================================================
  private: // Constructors should rarely appear at this scope
  protected: // Constructors intended for internal/subclass API
  public: // Constructors should generally be public
    Tool() : nmdt::AbstractGraphTool
      (
       "AWS resources", // help blurb, prefixed with:
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
    void addToolOptions() override
    {
      opts.addOptionalOption("no-details", std::make_tuple(
            "no-details",
            NULL_SEMANTIC,
            "Remove details, just show connectivity")
          );
      opts.addOptionalOption("no-network-interfaces", std::make_tuple(
            "no-network-interfaces",
            NULL_SEMANTIC,
            "Remove network interface vertices as start points."
            " Will still graph if intermediate hop (e.g., routing).")
          );
      opts.addOptionalOption("graph-instances", std::make_tuple(
            "graph-instances",
            NULL_SEMANTIC,
            "Add vertices for usable instances"
            " and connect to network interfaces.")
          );
    }

    // Overriden from AbstractGraphTool
    int
    runTool() override
    {
      pqxx::connection db {getDbConnectString()};
      nmdu::dbPrepareCommon(db);

      db.prepare("select_aws_instance_vertices", R"(
          SELECT DISTINCT
              instance_id , instance_type , image_id , architecture
            , platform_details , launch_time , availability_zone
            , state_code , state_name
          FROM aws_active_instance_details
          ORDER BY 1,2
          )"
        );

      db.prepare("select_aws_eni_vertices", R"(
          SELECT DISTINCT
            interface_id
          FROM raw_aws_network_interfaces
          ORDER BY 1
          )"
        );

      db.prepare("select_aws_eni_details", R"(
          SELECT DISTINCT
              interface_id , interface_type , description
          FROM raw_aws_network_interface_details
          WHERE interface_id IN (
            SELECT DISTINCT interface_id FROM aws_network_interface_mac_ips
          )
          ORDER BY 1
          )"
        );

      db.prepare("select_aws_eni_mac_ips", R"(
          SELECT DISTINCT
              interface_id , mac_address , ip_address
          FROM aws_network_interface_mac_ips
          ORDER BY 1,2,3
          )"
        );

      db.prepare("select_aws_eni_vertex_sg_rules", R"(
          SELECT DISTINCT
              interface_id , egress , protocol , ports , type , code
            , cidr_block , target
          FROM aws_eni_security_group_rules_full_machine
          WHERE egress IS NOT NULL
          ORDER BY 1,2,3,4,5
          )"
        );

      db.prepare("select_aws_subnet_vertices", R"(
          SELECT DISTINCT
              subnet_id , cidr_block
          FROM raw_aws_subnet_cidr_blocks
          ORDER BY 1,2
          )"
        );

      db.prepare("select_aws_subnet_vertex_nacl_rules", R"(
          SELECT DISTINCT
              subnet_id , egress , rule_number , action , protocol 
            , cidr_block , ports , type , code
          FROM aws_subnet_network_acl_rules_full_machine
          WHERE egress IS NOT NULL
          ORDER BY 1,2,3
          )"
        );

      db.prepare("select_aws_router_vertices", R"(
          SELECT DISTINCT
              next_hop_id , next_hop , state
          FROM aws_route_table_routes
          ORDER BY 1,2
          )"
        );

      db.prepare("select_aws_vpc_vertices", R"(
          SELECT DISTINCT
              vpc_id , cidr_block
          FROM raw_aws_vpc_cidr_blocks
          WHERE state = 'available'
          ORDER BY 1,2
          )"
        );

      db.prepare("select_aws_router_destinations_unknown", R"(
          SELECT DISTINCT
            next_hop_id , next_hop
          FROM aws_route_table_next_hops_unknown_in_db
          ORDER BY 1,2
          )"
        );

      db.prepare("select_aws_instance_to_eni_edges", R"(
          SELECT DISTINCT
              instance_id , interface_id
          FROM raw_aws_instance_network_interfaces
          ORDER BY 1,2
          )"
        );

      db.prepare("select_aws_eni_to_subnet_edges" , R"(
          SELECT DISTINCT
              interface_id , subnet_id
          FROM raw_aws_network_interface_vpc_subnet
          ORDER BY 1,2
          )"
        );

      db.prepare("select_aws_subnet_to_local_router_edges" , R"(
          SELECT DISTINCT
              subnet_id
            , next_hop_id || next_hop AS lr_id
          FROM aws_subnet_route_table_next_hops
          WHERE next_hop_id = 'local'
          ORDER BY 1,2
          )"
        );

      db.prepare("select_aws_local_router_to_vpc_edges" , R"(
          SELECT DISTINCT
              next_hop_id || next_hop AS lr_id
            , vpc_id
          FROM aws_subnet_route_table_next_hops
          WHERE next_hop_id = 'local'
          ORDER BY 1,2
          )"
        );

      db.prepare("select_aws_vpc_to_router_edges" , R"(
          SELECT DISTINCT
              vpc_id , next_hop_id
          FROM aws_subnet_route_table_next_hops
          WHERE next_hop_id != 'local'
          ORDER BY 1,2
          )"
        );

      db.prepare("select_aws_router_to_destination_edges" , R"(
          SELECT DISTINCT
              next_hop_id , next_hop
          FROM aws_subnet_route_table_next_hops
          WHERE next_hop_id != 'local'
            AND next_hop NOT IN (
              SELECT DISTINCT
                  cidr_block::TEXT
              FROM raw_aws_route_table_routes_cidr
              WHERE destination_id = 'local'
            )
          ORDER BY 1,2
          )"
        );


      // control flags
      noDetails           = opts.exists("no-details");
      noNetworkInterfaces = opts.exists("no-network-interfaces");
      graphInstances      = opts.exists("graph-instances");


      buildAwsGraph(db);

      boost::write_graphviz
        (std::cout, graph,
         LabelWriter(graph),   // VertexPropertyWriter
         LabelWriter(graph),   // EdgePropertyWriter
         GraphWriter(),        // GraphPropertyWriter
         boost::get(&VertexProperties::name, graph));  // VertexID

      return nmcu::Exit::SUCCESS;
    }

    void
    buildAwsGraph(pqxx::connection& db)
    {
      pqxx::work t  {db};

      addAwsVertices(t);
      addAwsEdges(t);

      t.commit();

      //boost::remove_edge_if(IsRedundantEdge(graph), graph);
    }

    void
    addAwsVertices(pqxx::transaction_base& t)
    {
      addInstances(t);
      addNetworkInterfaces(t);
      addSubnets(t);
      /* NOTE:
        Merging 'local' route(s) with the VPC vertex could result in
        incorrect visual representation in the case a VPC has multiple
        local routes defined between the subnets it contains.
      */
      addRoutes(t);
      addVpcs(t);
      addDestinations(t);
    }

    void
    addInstances(pqxx::transaction_base& t)
    {
      if (!graphInstances) { return; }

      pqxx::result vRows =
        t.exec_prepared("select_aws_instance_vertices");

      for (const auto& vRow : vRows) {
        std::string id;
        vRow.at("instance_id").to(id);
        std::string instanceType;
        vRow.at("instance_type").to(instanceType);
        std::string imageId;
        vRow.at("image_id").to(imageId);
        std::string architecture;
        vRow.at("architecture").to(architecture);
        std::string platformDetails;
        vRow.at("platform_details").to(platformDetails);
        std::string launchTime;
        vRow.at("launch_time").to(launchTime);
        std::string availabilityZone;
        vRow.at("availability_zone").to(availabilityZone);
        std::string stateCode;
        vRow.at("state_code").to(stateCode);
        std::string stateName;
        vRow.at("state_name").to(stateName);

        std::ostringstream oss;
        if (!vertexLookup.count(id)) {
          oss << id << R"(\n)";
        }

        if (!noDetails) {
          oss << R"(State: )" << stateCode
                << " (" << stateName << R"()\l)"
              << R"(Type: )" << instanceType << R"(\l)"
              << R"(Image: )" << imageId
                << " (" << platformDetails << " " << architecture << R"()\l)"
              << R"(Launched: )" << launchTime
                << " (" << availabilityZone << R"()\l)"
              ;
        }

        addVertex("box", id, oss.str());
      }
    }

    void
    addNetworkInterfaces(pqxx::transaction_base& t)
    {
      if (noNetworkInterfaces) { return; }

      {
        pqxx::result vRows =
          t.exec_prepared("select_aws_eni_vertices");

        for (const auto& vRow : vRows) {
          std::string id;
          vRow.at("interface_id").to(id);

          std::ostringstream oss;
          if (!vertexLookup.count(id)) {
            oss << id << R"(\n)";
          }

          addVertex("box", id, oss.str());
        }
      }

      if (noDetails) { return; }

      // add interface details
      {
        pqxx::result vRows =
          t.exec_prepared("select_aws_eni_details");

        for (const auto& vRow : vRows) {
          std::string id;
          vRow.at("interface_id").to(id);
          std::string type;
          vRow.at("interface_type").to(type);
          std::string description;
          vRow.at("description").to(description);

          std::ostringstream oss;
          oss << R"(Type: )" << type << R"(\l)"
              << R"(Description: )" << description << R"(\l)"
              ;

          addVertex("box", id, oss.str());
        }
      }
      {
        pqxx::result vRows =
          t.exec_prepared("select_aws_eni_mac_ips");

        std::string lastId {""};
        for (const auto& vRow : vRows) {
          std::string id;
          vRow.at("interface_id").to(id);
          std::string mac;
          vRow.at("mac_address").to(mac);
          std::string ip;
          vRow.at("ip_address").to(ip);

          std::ostringstream oss;
          if (lastId != id) {
            oss << R"(\nMac -- IP pairs:\l)";
          }

          oss << " - " << mac << " -- " << ip << R"(\l)"
              ;

          addVertex("box", id, oss.str());
          lastId = id;
        }
      }

      // add sg rules to eni vertex
      {
        pqxx::result vRows =
          t.exec_prepared("select_aws_eni_vertex_sg_rules");

        std::map<std::string, std::map<std::string, std::string>>
          sgMap;
        const std::string sgEgress  {"egress"};
        const std::string sgIngress {"ingress"};
        for (const auto& vRow : vRows) {
          std::string id;
          vRow.at("interface_id").to(id);
          bool egress;
          vRow.at("egress").to(egress);
          std::int32_t protocol;
          vRow.at("protocol").to(protocol);
          std::string ports;
          vRow.at("ports").to(ports);
          std::string type;
          vRow.at("type").to(type);
          std::string code;
          vRow.at("code").to(code);
          std::string cidrBlock;
          vRow.at("cidr_block").to(cidrBlock);
          std::string target;
          vRow.at("target").to(target);

          std::ostringstream oss;
          if (!sgMap.count(id)) {
            sgMap[id][sgEgress] = "";
            sgMap[id][sgIngress] = "";
          }

          if (egress) {
            oss << sgMap.at(id).at(sgEgress);
          } else {
            oss << sgMap.at(id).at(sgIngress);
          }

          oss << " - "
              << getDest(cidrBlock, target) << " "
              << getHumanProtocol(protocol) << " "
              << getPorts(ports, type, code) << " "
              << R"(\l)"
              ;

          if (egress) {
            sgMap.at(id).at(sgEgress) = oss.str();
          } else {
            sgMap.at(id).at(sgIngress) = oss.str();
          }
        }

        for (const auto& [id, sgFlows] : sgMap) {
          std::ostringstream oss;
          oss << R"(\nSG allowed egress\l)"
              << R"( - any stateful\l)"
              << sgFlows.at(sgEgress)
              << R"(\nSG allowed ingress\l)"
              << R"( - any stateful\l)"
              << sgFlows.at(sgIngress)
              ;

          addVertex("box", id, oss.str());
        }
      }
    }

    std::string
    getDest(const std::string& s1, const std::string& s2)
    {
      return s1.empty() ? s2 : s1;
    }
    std::string
    getHumanProtocol(std::int32_t p)
    {
      std::ostringstream oss;
      switch (p) {
        case -1:  oss << "any";   break;
        case 1:   oss << "icmp";  break;
        case 6:   oss << "tcp";   break;
        case 17:  oss << "udp";   break;
        default:  oss << p;       break;
      }
      return oss.str();
    }
    std::string
    getPorts(const std::string& p, const std::string& t, const std::string& c)
    {
      std::ostringstream oss;

      if (p.empty()) {
        oss << t << ':' << c;
      } else if ("[0,65535]" == p) {
        oss << "any";
      } else {
        oss << nmdo::PortRange(p).toHumanString();
      }
      return oss.str();
    }

    void
    addSubnets(pqxx::transaction_base& t)
    {
      {
        pqxx::result vRows =
          t.exec_prepared("select_aws_subnet_vertices");

        for (const auto& vRow : vRows) {
          std::string id;
          vRow.at("subnet_id").to(id);
          std::string subnet;
          vRow.at("cidr_block").to(subnet);

          std::ostringstream oss;
          oss << id << R"(\n)"
              << subnet << R"(\n)"
              ;

          addVertex("oval", id, oss.str());
        }
      }

      if (noDetails) { return; }

      // add nacl rules to subnet vertex
      {
        pqxx::result vRows =
          t.exec_prepared("select_aws_subnet_vertex_nacl_rules");

        std::map<std::string, std::map<std::string, std::string>>
          naclMap;
        const std::string naclEgress  {"egress"};
        const std::string naclIngress {"ingress"};
        for (const auto& vRow : vRows) {
          std::string id;
          vRow.at("subnet_id").to(id);
          bool egress;
          vRow.at("egress").to(egress);
          std::int32_t ruleNumber;
          vRow.at("rule_number").to(ruleNumber);
          std::string action;
          vRow.at("action").to(action);
          std::int32_t protocol;
          vRow.at("protocol").to(protocol);
          std::string cidrBlock;
          vRow.at("cidr_block").to(cidrBlock);
          std::string ports;
          vRow.at("ports").to(ports);
          std::string type;
          vRow.at("type").to(type);
          std::string code;
          vRow.at("code").to(code);

          std::ostringstream oss;
          if (!naclMap.count(id)) {
            naclMap[id][naclEgress] = "";
            naclMap[id][naclIngress] = "";
          }

          if (egress) {
            oss << naclMap.at(id).at(naclEgress);
          } else {
            oss << naclMap.at(id).at(naclIngress);
          }

          oss << " - "
              << ruleNumber << " "
              << action << " "
              << cidrBlock << " "
              << getHumanProtocol(protocol) << " "
              << getPorts(ports, type, code) << " "
              << R"(\l)"
              ;

          if (egress) {
            naclMap.at(id).at(naclEgress) = oss.str();
          } else {
            naclMap.at(id).at(naclIngress) = oss.str();
          }
        }

        for (const auto& [id, naclFlows] : naclMap) {
          std::ostringstream oss;
          oss << R"(Subnet internal\l- allow any any\l)"
              << R"(\nNACL allowed egress\l)"
              << naclFlows.at(naclEgress)
              << R"(\nNACL allowed ingress\l)"
              << naclFlows.at(naclIngress)
              ;

          addVertex("oval", id, oss.str());
        }
      }
    }

    void
    addRoutes(pqxx::transaction_base& t)
    {
      pqxx::result vRows =
        t.exec_prepared("select_aws_router_vertices");

      // local
      for (const auto& vRow : vRows) {
        std::string id;
        vRow.at("next_hop_id").to(id);
        std::string nextHop;
        vRow.at("next_hop").to(nextHop);
        std::string state;
        vRow.at("state").to(state);

        std::ostringstream oss;

        std::string nId {("local" == id) ? id+nextHop : id};

        if (!vertexLookup.count(nId)) {
          oss << id << R"(\n)";
          if (!noDetails) {
            oss << R"(Route(s)\l)";
          }
        }

        if (!noDetails) {
          oss << " - " << nextHop << " (" << state << R"()\l)";
        }

        addVertex("box", nId, oss.str());
      }
    }

    void
    addVpcs(pqxx::transaction_base& t)
    {
      pqxx::result vRows =
        t.exec_prepared("select_aws_vpc_vertices");

      for (const auto& vRow : vRows) {
        std::string id;
        vRow.at("vpc_id").to(id);
        std::string cidrs;
        vRow.at("cidr_block").to(cidrs);

        std::ostringstream oss;
        if (!vertexLookup.count(id)) {
          oss << id << R"(\n)";
          if (!noDetails) {
            oss << R"(Subnet\l)";
          }
        }
        if (!noDetails) {
          oss << " - " << cidrs << R"(\l)";
        }

        addVertex("oval", id, oss.str());
      }
    }

    void
    addDestinations(pqxx::transaction_base& t)
    {
      pqxx::result vRows =
        t.exec_prepared("select_aws_router_destinations_unknown");

      for (const auto& vRow : vRows) {
        std::string id;
        vRow.at("next_hop").to(id);

        std::ostringstream oss;
        if (!vertexLookup.count(id)) {
          oss << id << R"(\n)";
        }

        addVertex("oval", id, oss.str());
      }
    }

    void
    addVertex( const std::string shape
             , const std::string id
             , const std::string label=""
             )
    {
      LOG_DEBUG << "Vertex: " << id << " (" << shape << ") -- " << label
                << std::endl;

      if (!vertexLookup.count(id)) {
        Vertex v {boost::add_vertex(graph)};
        vertexLookup[id] = v;
      }

      const auto v {vertexLookup[id]};
      graph[v].name = id;
      if (!label.empty()) {
        graph[v].label += label;
      }
      graph[v].shape = shape;
    }

    void
    addAwsEdges(pqxx::transaction_base& t)
    {
      addInstanceToNetworkInterface(t);
      addNetworkInterfaceToSubnet(t);
      addSubnetToLocalRoute(t);
      addLocalRouteToVpc(t);
      addVpcToExternalRoute(t);
      addExternalRouteToDestination(t);
    }

    void
    addInstanceToNetworkInterface(pqxx::transaction_base& t)
    {
      if (!(graphInstances && !noNetworkInterfaces)) { return; }

      pqxx::result eRows =
        t.exec_prepared("select_aws_instance_to_eni_edges");

      for (const auto& eRow : eRows) {
        std::string src;
        eRow.at("instance_id").to(src);
        std::string dst;
        eRow.at("interface_id").to(dst);

        addEdge(src, dst);
      }
    }

    void
    addNetworkInterfaceToSubnet(pqxx::transaction_base& t)
    {
      if (noNetworkInterfaces) { return; }

      pqxx::result eRows =
        t.exec_prepared("select_aws_eni_to_subnet_edges");

      for (const auto& eRow : eRows) {
        std::string src;
        eRow.at("interface_id").to(src);
        std::string dst;
        eRow.at("subnet_id").to(dst);

        addEdge(src, dst);
      }
    }

    void
    addSubnetToLocalRoute(pqxx::transaction_base& t)
    {
      pqxx::result eRows =
        t.exec_prepared("select_aws_subnet_to_local_router_edges");

      for (const auto& eRow : eRows) {
        std::string src;
        eRow.at("subnet_id").to(src);
        std::string dst;
        eRow.at("lr_id").to(dst);

        addEdge(src, dst);
      }
    }

    void
    addLocalRouteToVpc(pqxx::transaction_base& t)
    {
      pqxx::result eRows =
        t.exec_prepared("select_aws_local_router_to_vpc_edges");

      for (const auto& eRow : eRows) {
        std::string src;
        eRow.at("lr_id").to(src);
        std::string dst;
        eRow.at("vpc_id").to(dst);

        addEdge(src, dst);
      }
    }

    void
    addVpcToExternalRoute(pqxx::transaction_base& t)
    {
      pqxx::result eRows =
        t.exec_prepared("select_aws_vpc_to_router_edges");

      for (const auto& eRow : eRows) {
        std::string src;
        eRow.at("vpc_id").to(src);
        std::string dst;
        eRow.at("next_hop_id").to(dst);

        addEdge(src, dst);
      }
    }

    void
    addExternalRouteToDestination(pqxx::transaction_base& t)
    {
      pqxx::result eRows =
        t.exec_prepared("select_aws_router_to_destination_edges");

      for (const auto& eRow : eRows) {
        std::string src;
        eRow.at("next_hop_id").to(src);
        std::string dst;
        eRow.at("next_hop").to(dst);

        //if (!vertexLookup.count(dst)) {
        //  addVertex("box", dst, dst);
        //}
        addEdge(src, dst);
      }
    }

    void
    addEdge(const std::string& src, const std::string& dst)
    {
      LOG_DEBUG << "Edge: "
                << src << " <-> " << dst
                << std::endl;

      try {
        const auto u {vertexLookup.at(src)};
        const auto v {vertexLookup.at(dst)};

        if (edgeLookup.count(src) && edgeLookup[src].count(dst)) {
          const auto e {edgeLookup.at(src).at(dst)};
        } else {
          Edge e;
          bool inserted;
          tie(e, inserted) = boost::add_edge(u, v, graph);
          edgeLookup[src][dst] = e;
        }
      } catch (const std::exception& e) {
        LOG_ERROR << "Failed to create edge: " << e.what()
                  << std::endl;
        LOG_ERROR << "- One of '" << src << "'" << " or '" << dst << "'"
                  << " is a non-existant graph vertex"
                  << std::endl;
      }
    }

  protected: // Methods part of subclass API
    // Inherited from AbstractTool at this scope
      // std::string const getDbName() const;
      // virtual void printVersion() const;
    // Inherited from AbstractGraphTool at this scope
      // virtual void printHelp() const;
  public: // Methods part of public API
    // Inherited from AbstractTool, don't override as primary tool entry point
      // int start(int, char**) noexcept;
};


// =============================================================================
// Program entry point
// =============================================================================
int
main(int argc, char** argv)
{
  Tool tool;
  return tool.start(argc, argv);
}
