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
    const nmcu::FileManager& nmfm {nmcu::FileManager::getInstance()};

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
       "AWS VPC related resources", // help blurb, prefixed with:
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
      opts.addOptionalOption("icons-folder", std::make_tuple(
          "icons-folder",
          po::value<std::string>()->required()->
            default_value(nmfm.getConfPath()/"images"),
          "Folder to search in for icons. ")
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
          ORDER BY 1,2,7,8,3,4,5,6
          )"
        );
      db.prepare("select_aws_eni_vertex_sg_rules_empty", R"(
          SELECT DISTINCT
              interface_id
          FROM aws_eni_security_group_rules_full_machine
          WHERE egress IS NULL
            AND interface_id NOT IN (
              SELECT DISTINCT
                  interface_id
              FROM aws_eni_security_group_rules_full_machine
              WHERE egress IS NOT NULL
            )
          ORDER BY 1
          )"
        );

      db.prepare("select_aws_subnet_vertices", R"(
          select distinct
              t1.vpc_id , t1.subnet_id , t2.cidr_block
          from raw_aws_vpc_subnets as t1
          left join raw_aws_subnet_cidr_blocks as t2
            on t1.subnet_id = t2.subnet_id
          order by 1,2,3
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
          select distinct
              route_table_id
          from raw_aws_route_tables
          order by 1
          )"
        );
      db.prepare("select_aws_router_vertices_cidrs", R"(
          select distinct
              route_table_id , cidr_block , destination_id , state
          from raw_aws_route_table_routes_cidr
          order by 1,2,3,4
          )"
        );
      db.prepare("select_aws_router_vertices_non_cidrs", R"(
          select distinct
              route_table_id , destination , destination_id , state
          from raw_aws_route_table_routes_non_cidr
          order by 1,2,3,4
          )"
        );


      db.prepare("select_aws_vpc_vertex", R"(
          SELECT DISTINCT
              vpc_id , owner_id , cidr_block
          FROM aws_vpc_cidr_blocks
          WHERE $1 = vpc_id
          ORDER BY 1,2,3
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
      db.prepare("select_aws_eni_to_subnet_edge" , R"(
          SELECT DISTINCT
              interface_id , subnet_id
          FROM raw_aws_network_interface_vpc_subnet
          WHERE $1 = interface_id
          ORDER BY 1,2
          )"
        );

      db.prepare("select_aws_subnet_to_router_edges" , R"(
          select distinct
              t1.subnet_id
            , coalesce(t2.route_table_id , t3.route_table_id) as route_table_id
          from raw_aws_vpc_subnets as t1
          left join raw_aws_route_table_associations as t2
            on t1.subnet_id = t2.association_id
          left join aws_vpc_route_table_defaults as t3
            on t1.vpc_id = t3.vpc_id
          order by 1,2
          )"
        );

      db.prepare("select_aws_igw_to_internet", R"(
          SELECT DISTINCT
              destination_id , 'internet' AS next_hop
          FROM raw_aws_route_table_routes_cidr
          WHERE destination_id LIKE 'igw-%'
            AND state = 'active'
          )"
        );
      db.prepare("select_aws_pcx_to_external", R"(
          WITH cte1 AS (
            SELECT DISTINCT
                pcx_id , accepter_vpc_id AS vpc_id
            FROM raw_aws_vpc_peering_connection_peers
          ), cte2 AS (
            SELECT DISTINCT
                pcx_id , requester_vpc_id AS vpc_id
            FROM raw_aws_vpc_peering_connection_peers
          ), cte3 AS (
            SELECT
                vpc_id
            FROM aws_vpc_cidr_blocks
            WHERE state IS NULL
          ) SELECT DISTINCT
              pcx_id , vpc_id
          FROM cte1
          WHERE vpc_id IN (SELECT * FROM cte3)
          UNION
          SELECT DISTINCT
              pcx_id , vpc_id
          FROM cte2
          WHERE vpc_id IN (SELECT * FROM cte3)
          ORDER BY 1,2
          )"
        );
      db.prepare("select_aws_vpce_to_external", R"(
          SELECT DISTINCT
              destination_id , destination
          FROM raw_aws_route_table_routes_non_cidr
          WHERE destination_id LIKE 'vpce-%'
            AND state = 'active'
          ORDER BY 1,2
          )"
        );
      db.prepare("select_aws_nat_to_external", R"(
          SELECT DISTINCT
              destination_id , 'internet' AS next_hop
          FROM raw_aws_route_table_routes_cidr
          WHERE destination_id LIKE 'nat-%'
            AND state = 'active'
          ORDER BY 1,2
          )"
        );
      db.prepare("select_aws_route_eni_to_subnet", R"(
          SELECT DISTINCT
              t1.destination_id, t2.subnet_id
          FROM (
            SELECT DISTINCT
                destination_id
            FROM raw_aws_route_table_routes_cidr
            WHERE state = 'active'
              AND destination_id LIKE 'eni-%'
            UNION
            SELECT DISTINCT
                destination_id
            FROM raw_aws_route_table_routes_non_cidr
            WHERE state = 'active'
              AND destination_id LIKE 'eni-%'
          ) AS t1
          LEFT JOIN raw_aws_network_interface_vpc_subnet AS t2
            ON t1.destination_id = t2.interface_id
            ORDER BY 1
          )"
        );
      db.prepare("select_aws_routes_to_blackhole", R"(
          SELECT DISTINCT
              destination_id , 'blackhole' AS next_hop
          FROM raw_aws_route_table_routes_cidr
          WHERE state = 'blackhole'
          UNION
          SELECT DISTINCT
              destination_id , 'blackhole' AS next_hop
          FROM raw_aws_route_table_routes_non_cidr
          WHERE state = 'blackhole'
          ORDER BY 1
          )"
        );
      db.prepare("select_aws_tgw_to_resource", R"(
          SELECT DISTINCT
              tgw_id , resource_type , resource_id , resource_owner_id
          FROM raw_aws_transit_gateway_attachment_details
          WHERE association_state = 'associated'
            AND resource_id NOT IN (SELECT vpc_id FROM raw_aws_vpcs)
          ORDER BY 1,2,3,4
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
      pqxx::work t {db};

      addAwsVertices(t);
      addAwsEdges(t);

      t.commit();

      //boost::remove_edge_if(IsRedundantEdge(graph), graph);

      finalizeVertices();
    }

    void
    finalizeVertices()
    {
      for (const auto& [_, v] : vertexLookup) {
        graph[v].label += R"(</font></td></tr></table>> + ")";
      }
    }

    void
    addAwsVertices(pqxx::transaction_base& t)
    {
      addInstances(t);
      addNetworkInterfaces(t);
      addSubnets(t);
      addRoutes(t);
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

        if (!noDetails) {
          oss << R"(Type: )" << instanceType
                << R"(<br align="left"/>)"
              << R"(Image: )" << imageId
                << R"(<br align="left"/>)"
              << R"(Platform: )" << platformDetails
                << R"(<br align="left"/>)"
              << R"(Architecture: )" << architecture
                << R"(<br align="left"/>)"
              << R"(<br/>)"
              << R"(State: )" << stateCode << " (" << stateName << R"())"
                << R"(<br align="left"/>)"
              << R"(Launched: )" << launchTime
                << R"(<br align="left"/>)"
              << R"(Availability Zone: )" << availabilityZone
                << R"(<br align="left"/>)"
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

          addVertex("box", id);
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
          oss << R"(Type: )" << type << R"(<br align="left"/>)"
              << R"(Description: )" << description << R"(<br align="left"/>)"
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
            oss << R"(<br/>Mac -- IP pairs:<br align="left"/>)";
          }

          oss << " - " << mac << " -- " << ip << R"(<br align="left"/>)"
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
              << R"(<br align="left"/>)"
              ;

          if (egress) {
            sgMap.at(id).at(sgEgress) = oss.str();
          } else {
            sgMap.at(id).at(sgIngress) = oss.str();
          }
        }

        for (const auto& [id, sgFlows] : sgMap) {
          std::ostringstream oss;
          oss << R"(<br/>SG allowed egress<br align="left"/>)"
              << R"( - any stateful<br align="left"/>)"
              << sgFlows.at(sgEgress)
              << R"(<br/>SG allowed ingress<br align="left"/>)"
              << R"( - any stateful<br align="left"/>)"
              << sgFlows.at(sgIngress)
              ;

          addVertex("box", id, oss.str());
        }
      }
      {
        pqxx::result vRows =
          t.exec_prepared("select_aws_eni_vertex_sg_rules_empty");

        for (const auto& vRow : vRows) {
          std::string id;
          vRow.at("interface_id").to(id);

          std::ostringstream oss;
          oss << R"(<br/>SG allowed egress<br align="left"/>)"
              << R"( - none<br align="left"/>)"
              << R"(<br/>SG allowed ingress<br align="left"/>)"
              << R"( - none<br align="left"/>)"
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
        if ("-1" == t) {
          oss << "any";
        } else {
          oss << t;
        }
        oss << ':';
        if ("-1" == c) {
          oss << "any";
        } else {
          oss << c;
        }
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
          oss << '(' << subnet << R"()<br/>)";

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
              << R"(<br align="left"/>)"
              ;

          if (egress) {
            naclMap.at(id).at(naclEgress) = oss.str();
          } else {
            naclMap.at(id).at(naclIngress) = oss.str();
          }
        }

        for (const auto& [id, naclFlows] : naclMap) {
          std::ostringstream oss;
          oss << R"(</font></td></tr>)"
              << R"(<tr>)" << getIconString("acl-")
              << R"(<td><font point-size="11">)"
              << R"(Intra-subnet<br align="left"/>)"
              << R"(- allow any any<br align="left"/>)"
              << R"(<br/>)"
              << R"(Inter-subnet; NACL egress<br align="left"/>)"
              << naclFlows.at(naclEgress)
              << R"(<br/>)"
              << R"(Inter-subnet; NACL ingress<br align="left"/>)"
              << naclFlows.at(naclIngress)
              ;

          addVertex("oval", id, oss.str());
        }
      }
    }

    void
    addRoutes(pqxx::transaction_base& t)
    {
      pqxx::result routerTables =
        t.exec_prepared("select_aws_router_vertices");

      for (const auto& vRow : routerTables) {
        std::string rtbId;
        vRow.at("route_table_id").to(rtbId);

        std::ostringstream oss;
        if (!noDetails) {
          oss << R"(Route(s)<br align="left"/>)";
        }

        addVertex("box", rtbId, oss.str());
      }

      pqxx::result routeTableCidrs =
        t.exec_prepared("select_aws_router_vertices_cidrs");
      for (const auto& vRow : routeTableCidrs) {
        std::string rtbId;
        vRow.at("route_table_id").to(rtbId);
        std::string nextHop;
        vRow.at("cidr_block").to(nextHop);
        std::string nextHopId;
        vRow.at("destination_id").to(nextHopId);
        std::string state;
        vRow.at("state").to(state);

        std::ostringstream oss;
        oss << " - " << nextHop
              << " (" << state << ')'
              << " via " << nextHopId
              << R"(<br align="left"/>)"
            ;

        if (!noDetails) {
          addVertex("box", rtbId, oss.str());
        }

        if ("local" != nextHopId) {
          addVertex("box", nextHopId);
        }
      }

      pqxx::result routeTableNonCidrs =
        t.exec_prepared("select_aws_router_vertices_non_cidrs");
      for (const auto& vRow : routeTableNonCidrs) {
        std::string rtbId;
        vRow.at("route_table_id").to(rtbId);
        std::string nextHop;
        vRow.at("destination").to(nextHop);
        std::string nextHopId;
        vRow.at("destination_id").to(nextHopId);
        std::string state;
        vRow.at("state").to(state);

        std::ostringstream oss;
        oss << " - " << nextHop
              << " (" << state << ')'
              << " via " << nextHopId
              << R"(<br align="left"/>)"
            ;

        if (!noDetails) {
          addVertex("box", rtbId, oss.str());
        }

        if ("local" != nextHopId) {
          addVertex("box", nextHopId);
        }
      }
    }

    void
    addVpc(pqxx::transaction_base& t, const std::string& vpcId)
    {
      pqxx::result vRows =
        t.exec_prepared("select_aws_vpc_vertex", vpcId);

      for (const auto& vRow : vRows) {
        std::string id;
        vRow.at("vpc_id").to(id);
        std::string ownerId;
        vRow.at("owner_id").to(ownerId);
        std::string cidr;
        vRow.at("cidr_block").to(cidr);

        std::ostringstream oss;

        if (!vertexLookup.count(id)) {
          oss << R"((Owner: )" << ownerId << R"()<br/>)";
        }
        if (!noDetails) {
          if (!vertexLookup.count(id)) {
            oss << R"(Subnet(s)<br align="left"/>)";
          }
          oss << " - " << cidr << R"(<br align="left"/>)";
        }

        addVertex("oval", id, oss.str());
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
        if (id.starts_with("local")) {
          mLabel = getLabelHeader("VPC intra-routing") + label;
        } else {
          mLabel = getLabelHeader(id) + label;
        }
      }

      const auto v {vertexLookup[id]};
      graph[v].name = id;
      if (!mLabel.empty()) {
        graph[v].label += mLabel;
      }
      graph[v].shape = "rectangle";
      graph[v].style = "rounded";
    }

    std::string
    getLabelHeader(const std::string& id) {
      std::ostringstream oss;

      oss << R"(" + <<table border="0" cellborder="0"><tr>)"
          << getIconString(id)
          << R"(<td><font point-size="11">)" << id << R"(<br/>)"
          ;

      return oss.str();
    }

    std::string
    getIconString(const std::string& id)
    {
      std::string iconPath;

      std::size_t pos {id.find_first_of("-")};
      LOG_DEBUG << "Postition of '-' (" << id << "):" << pos << '\n';
      if (  pos != std::string::npos
         || "internet" == id
         )
      {
        std::string type {"aws-" + id.substr(0, pos) + ".svg"};
        LOG_DEBUG << "Looking for icon name: " << type << '\n';

        sfs::path iPath {opts.getValue("icons-folder")};
        for (const auto& pathIter : sfs::recursive_directory_iterator(iPath)) {
          std::string fileName {pathIter.path().filename()};

          LOG_DEBUG << " - " << fileName << '\n';
          if (std::equal(type.begin(), type.end(),
                         fileName.begin(), fileName.end(),
                         [](auto a, auto b) {
                            return std::tolower(a) == std::tolower(b);
                         }))
          {
            iconPath = pathIter.path();
            break;
          }
        }
      }

      LOG_DEBUG << "Icon path: " << iconPath << '\n';
      std::ostringstream oss;
      if (iconPath.empty()) {
        oss << R"(<td></td>)";
      } else {
        oss << R"(<td width="60" height="60" fixedsize="true">)"
            << R"(<img src=")" << iconPath << R"(" scale="true"/>)"
            << R"(</td>)"
            ;
      }

      return oss.str();
    }

    void
    addIcon(const std::string& id, const std::string& icon)
    {
      if (!vertexLookup.count(id)) {
        return;
      }
      LOG_DEBUG << "Vertex: " << id << " found; updating to: " << icon;

      const auto v {vertexLookup[id]};
      std::string curLabel {graph[v].label};

      std::regex re {R"(<td></td>)"};
      std::string newText {getIconString(icon)};
      std::ostringstream oss;
      oss << std::regex_replace(curLabel, re, newText);

      graph[v].label = oss.str();
    }

    void
    addAwsEdges(pqxx::transaction_base& t)
    {
      addInstanceToNetworkInterface(t);
      addNetworkInterfaceToSubnet(t);
      addSubnetsToRouteTable(t);
      addRouterToNextHop(t);
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
    addSubnetsToRouteTable(pqxx::transaction_base& t)
    {
      pqxx::result eRows =
        t.exec_prepared("select_aws_subnet_to_router_edges");

      for (const auto& eRow : eRows) {
        std::string src;
        eRow.at("subnet_id").to(src);
        std::string dst;
        eRow.at("route_table_id").to(dst);

        addEdge(src, dst);
      }
    }

    void
    addRouterToNextHop(pqxx::transaction_base& t)
    {
      {
        pqxx::result eRows =
          t.exec_prepared("select_aws_router_vertices_cidrs");

        for (const auto& eRow : eRows) {
          std::string src;
          eRow.at("route_table_id").to(src);
          std::string dst;
          eRow.at("destination_id").to(dst);

          if ("local" != dst) {
            addEdge(src, dst);
          }
        }
      }
      {
        pqxx::result eRows =
          t.exec_prepared("select_aws_router_vertices_non_cidrs");

        for (const auto& eRow : eRows) {
          std::string src;
          eRow.at("route_table_id").to(src);
          std::string dst;
          eRow.at("destination_id").to(dst);

          if ("local" != dst) {
            addEdge(src, dst);
          }
        }
      }
    }

    void
    addExternalRouteToDestination(pqxx::transaction_base& t)
    {
      {
        pqxx::result eRows =
          t.exec_prepared("select_aws_igw_to_internet");

        for (const auto& eRow : eRows) {
          std::string src;
          eRow.at("destination_id").to(src);
          std::string dst;
          eRow.at("next_hop").to(dst);

          if (!vertexLookup.count(dst)) {
            addVertex("oval", dst);
          }

          addEdge(src, dst);
        }
      }
      {
        pqxx::result eRows =
          t.exec_prepared("select_aws_pcx_to_external");

        for (const auto& eRow : eRows) {
          std::string src;
          eRow.at("pcx_id").to(src);
          std::string dst;
          eRow.at("vpc_id").to(dst);

          if (!vertexLookup.count(src)) {
            addVertex("oval", src);
          }
          if (!vertexLookup.count(dst)) {
            addVpc(t, dst);
          }

          addEdge(src, dst);
        }
      }
      {
        pqxx::result eRows =
          t.exec_prepared("select_aws_vpce_to_external");

        for (const auto& eRow : eRows) {
          std::string src;
          eRow.at("destination_id").to(src);
          std::string dst;
          eRow.at("destination").to(dst);

          if (!vertexLookup.count(dst)) {
            addVertex("oval", dst);
          }

          addEdge(src, dst);
        }
      }
      {
        pqxx::result eRows =
          t.exec_prepared("select_aws_nat_to_external");

        for (const auto& eRow : eRows) {
          std::string src;
          eRow.at("destination_id").to(src);
          std::string dst;
          eRow.at("next_hop").to(dst);

          if (!vertexLookup.count(dst)) {
            addVertex("oval", dst);
          }

          addEdge(src, dst);
        }
      }
      {
        pqxx::result eRows =
          t.exec_prepared("select_aws_route_eni_to_subnet");

        for (const auto& eRow : eRows) {
          std::string src;
          eRow.at("destination_id").to(src);
          std::string dst;
          eRow.at("subnet_id").to(dst);

          addEdge(src, dst);
        }
      }
      {
        pqxx::result eRows =
          t.exec_prepared("select_aws_tgw_to_resource");

        for (const auto& eRow : eRows) {
          std::string src;
          eRow.at("tgw_id").to(src);
          std::string dst;
          eRow.at("resource_id").to(dst);
          std::string type;
          eRow.at("resource_type").to(type);
          std::string owner;
          eRow.at("resource_owner_id").to(owner);

          if (!vertexLookup.count(dst)) {
            std::ostringstream oss;
            oss << R"((Owner: )" << owner << R"()<br/>)";
            addVertex("oval", dst, oss.str());
            addIcon(dst, "dcg-");
          }

          addEdge(src, dst);
        }
      }
      {
        pqxx::result eRows =
          t.exec_prepared("select_aws_routes_to_blackhole");

        for (const auto& eRow : eRows) {
          std::string src;
          eRow.at("destination_id").to(src);
          std::string dst;
          eRow.at("next_hop").to(dst);

          if (!vertexLookup.count(dst)) {
            addVertex("oval", dst);
          }

          addEdge(src, dst);
        }
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
        } else if (edgeLookup.count(dst) && edgeLookup[dst].count(src)) {
          const auto e {edgeLookup.at(dst).at(src)};
        } else {
          Edge e;
          bool inserted;
          std::tie(e, inserted) = boost::add_edge(u, v, graph);
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
