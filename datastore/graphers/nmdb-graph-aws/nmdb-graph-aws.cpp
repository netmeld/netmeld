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

#include <netmeld/datastore/tools/AbstractGraphTool.hpp>

#include "GraphHelper.hpp"

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
       "AWS resources",    // help blurb, prefixed with:
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
      opts.addRequiredOption("device-id", std::make_tuple(
            "device-id",
            po::value<std::string>()->required(),
            "Name of device")
          );

//      opts.addOptionalOption("all", std::make_tuple(
//            "all",
//            NULL_SEMANTIC,
//            "Display all rules, regardless of known application state")
//          );
    }

    // Overriden from AbstractGraphTool
    int
    runTool() override
    {
      pqxx::connection db {getDbConnectString()};
      nmdu::dbPrepareCommon(db);

      db.prepare("select_aws_eni_vertices" , R"(
          SELECT DISTINCT
              interface_id , mac_address , ip_address
          FROM  aws_network_interface_mac_ips
          ORDER BY 1,2,3
          )"
        );
      db.prepare("select_aws_eni_vertex_egress_rules", R"(
          SELECT DISTINCT
              interface_id , protocol , ports , cidr_block
          FROM aws_eni_security_group_rules_full
          WHERE egress = 't'
          ORDER BY 1,2,3,4
          )"
        );
      db.prepare("select_aws_eni_vertex_ingress_rules", R"(
          SELECT DISTINCT
              interface_id , protocol , ports , cidr_block
          FROM aws_eni_security_group_rules_full
          WHERE egress = 'f'
          ORDER BY 1,2,3,4
          )"
        );
      db.prepare("select_aws_subnet_vertices" , R"(
          SELECT DISTINCT
              subnet_id , cidr_block
          FROM raw_aws_subnet_cidr_blocks
          )"
        );
      db.prepare("select_aws_subnet_vertex_egress_rules" , R"(
          SELECT DISTINCT
              net_id , rule_number , action
            , nacl_dest , nacl_protocol , nacl_ports
          FROM aws_eni_egress_t4_cleaned
          ORDER BY net_id, rule_number, nacl_dest
          )"
        );
      db.prepare("select_aws_subnet_vertex_ingress_rules" , R"(
          SELECT DISTINCT
              net_id , rule_number , action
            , nacl_src , nacl_protocol, nacl_ports
          FROM aws_eni_ingress_t4_cleaned
          ORDER BY net_id, rule_number, nacl_src
          )"
        );
      db.prepare("select_aws_router_vertices" , R"(
          SELECT DISTINCT
              next_hop_id , next_hop , state
          FROM aws_route_table_routes
          ORDER BY 1,2
          )"
        );
      db.prepare("select_aws_vpc_vertices" ,R"(
          SELECT DISTINCT
              vpc_id , cidr_block
          FROM raw_aws_vpc_cidr_blocks
          WHERE state = 'available'
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
              destination_id, cidr_block
          FROM raw_aws_route_table_routes_cidr
          WHERE cidr_block = '0/0'
          ORDER BY 1,2
          )"
        );


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
      // select * from aws_eni_egress_t4_cleaned;

      // add eni vertex
      {
        pqxx::result vRows =
          t.exec_prepared("select_aws_eni_vertices");

        for (const auto& vRow : vRows) {
          std::string id;
          vRow.at("interface_id").to(id);
          std::string mac;
          vRow.at("mac_address").to(mac);
          std::string ip;
          vRow.at("ip_address").to(ip);

          std::ostringstream oss;
          if (!vertexLookup.count(id)) {
            oss << id << R"(\l)";
          }
          oss << " - "
              << mac << " : "
              << ip
              << R"(\l)"
              ;

          addVertex("box", id, oss.str());
        }
      }
      // add sg rules to eni vertex
      {
        { // egress
          pqxx::result vRows =
            t.exec_prepared("select_aws_eni_vertex_egress_rules");

          std::string lastId {""};
          for (const auto& vRow : vRows) {
            std::string id;
            vRow.at("interface_id").to(id);
            std::string protocol;
            vRow.at("protocol").to(protocol);
            std::string ports;
            vRow.at("ports").to(ports);
            std::string dest;
            vRow.at("cidr_block").to(dest);

            std::ostringstream oss;
            if (lastId != id) {
              oss << R"(\nAllowed Egress (SG)\l)"
                  << R"( - any stateful\l)"
                  ;
            }
            if (!dest.empty()) {
              oss << " - "
                  << dest << " "
                  << protocol << " "
                  << ports << " "
                  << R"(\l)"
                  ;
            }

            addVertex("box", id, oss.str());
            lastId = id;
          }
        }
        { // ingress
          pqxx::result vRows =
            t.exec_prepared("select_aws_eni_vertex_ingress_rules");

          std::string lastId {""};
          for (const auto& vRow : vRows) {
            std::string id;
            vRow.at("interface_id").to(id);
            std::string protocol;
            vRow.at("protocol").to(protocol);
            std::string ports;
            vRow.at("ports").to(ports);
            std::string src;
            vRow.at("cidr_block").to(src);

            std::ostringstream oss;
            if (lastId != id) {
              oss << R"(\nAllowed Ingress (SG)\l)"
                  << R"( - any stateful\l)";
            }
            if (!src.empty()) {
              oss << " - "
                  << src << " "
                  << protocol << " "
                  << ports << " "
                  << R"(\l)"
                  ;
            }

            addVertex("box", id, oss.str());
            lastId = id;
          }
        }
      }
      // add subnet vertex
      {
        pqxx::result vRows =
          t.exec_prepared("select_aws_subnet_vertices");

        for (const auto& vRow : vRows) {
          std::string id;
          vRow.at("subnet_id").to(id);
          std::string subnet;
          vRow.at("cidr_block").to(subnet);

          std::ostringstream oss;
          oss << id << R"(\l)"
              << subnet << R"(\l)"
              ;
          addVertex("oval", id, oss.str());
        }
      }
      // add nacl rules to subnet vertex
      {
        { // egress
          pqxx::result vRows =
            t.exec_prepared("select_aws_subnet_vertex_egress_rules");

          std::string lastId {""};
          for (const auto& vRow : vRows) {
            std::string id;
            vRow.at("net_id").to(id);
            std::string ruleNumber;
            vRow.at("rule_number").to(ruleNumber);
            std::string action;
            vRow.at("action").to(action);
            std::string target;
            vRow.at("nacl_dest").to(target);
            std::string protocol;
            vRow.at("nacl_protocol").to(protocol);
            std::string ports;
            vRow.at("nacl_ports").to(ports);

            std::ostringstream oss;
            if (lastId != id) {
              oss << R"(\nAllowed Egress (NACL)\l)"
                  ;
            }
            if (!target.empty()) {
              oss << " - "
                  << ruleNumber << " "
                  << action << " "
                  << target << " "
                  << protocol << " "
                  << ports << " "
                  << R"(\l)"
                  ;
            }

            addVertex("oval", id, oss.str());
            lastId = id;
          }
        }
        { // ingress
          pqxx::result vRows =
            t.exec_prepared("select_aws_subnet_vertex_ingress_rules");

          std::string lastId {""};
          for (const auto& vRow : vRows) {
            std::string id;
            vRow.at("net_id").to(id);
            std::string ruleNumber;
            vRow.at("rule_number").to(ruleNumber);
            std::string action;
            vRow.at("action").to(action);
            std::string target;
            vRow.at("nacl_src").to(target);
            std::string protocol;
            vRow.at("nacl_protocol").to(protocol);
            std::string ports;
            vRow.at("nacl_ports").to(ports);

            std::ostringstream oss;
            if (lastId != id) {
              oss << R"(\nAllowed Ingress (NACL)\l)"
                  ;
            }
            if (!target.empty()) {
              oss << " - "
                  << ruleNumber << " "
                  << action << " "
                  << target << " "
                  << protocol << " "
                  << ports << " "
                  << R"(\l)"
                  ;
            }

            addVertex("oval", id, oss.str());
            lastId = id;
          }
        }
      }
      // add router vertex
      {
        pqxx::result vRows =
          t.exec_prepared("select_aws_router_vertices");

        for (const auto& vRow : vRows) {
          std::string id;
          vRow.at("next_hop_id").to(id);
          std::string nextHop;
          vRow.at("next_hop").to(nextHop);
          std::string state;
          vRow.at("state").to(state);

          if ("local" == id) {
            continue;
          }

          std::ostringstream oss;
          if (!vertexLookup.count(id)) {
            oss << id << R"(\n)";
          }
          oss << " - " << nextHop << " (" << state << R"()\l)";

          addVertex("box", id, oss.str());
        }
        for (const auto& vRow : vRows) {
          std::string id;
          vRow.at("next_hop_id").to(id);
          std::string nextHop;
          vRow.at("next_hop").to(nextHop);
          std::string state;
          vRow.at("state").to(state);

          std::ostringstream oss;
          if ("local" != id) {
            continue;
          }
          std::string nId {id+nextHop};

          if (!vertexLookup.count(nId)) {
            oss << id
                << R"( (routing)\n)"
                ;
          }
          oss << " - " << nextHop << " (" << state << R"()\l)";

          addVertex("box", nId, oss.str());
        }
      }
      // add vpcs
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
          }
          oss << " - " << cidrs << R"(\l)";

          addVertex("oval", id, oss.str());
        }
      }
      // TODO add final hops? rtr/next hop not known in DB?
    }

    void
    addVertex( const std::string shape
             , const std::string id
             , const std::string label=""
             )
    {
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
      // eni -> subnet
      {
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
      // subnet -> local router
      {
        pqxx::result eRows =
          t.exec_prepared("select_aws_subnet_to_local_router_edges");

        for (const auto& eRow : eRows) {
          std::string src;
          eRow.at("subnet_id").to(src);
          std::string dst;
          eRow.at("lr_id").to(dst);

          LOG_DEBUG << "s--lr: " << src << " -- " << dst << std::endl;
          addEdge(src, dst);
        }
      }
      // local router -> vpc
      {
        pqxx::result eRows =
          t.exec_prepared("select_aws_local_router_to_vpc_edges");

        for (const auto& eRow : eRows) {
          std::string src;
          eRow.at("lr_id").to(src);
          std::string dst;
          eRow.at("vpc_id").to(dst);

          LOG_DEBUG << "lr--vpc: " << src << " -- " << dst << std::endl;
          addEdge(src, dst);
        }
      }
      // vpc -> tgw/igw/pcx/etc
      {
        pqxx::result eRows =
          t.exec_prepared("select_aws_vpc_to_router_edges");

        for (const auto& eRow : eRows) {
          std::string src;
          eRow.at("vpc_id").to(src);
          std::string dst;
          eRow.at("next_hop_id").to(dst);

          LOG_DEBUG << "vpc--rr: " << src << " -- " << dst << std::endl;
          addEdge(src, dst);
        }
      }
      // tgw/igw/pcx/etc -> outside
      {
        pqxx::result eRows =
          t.exec_prepared("select_aws_router_to_destination_edges");

        for (const auto& eRow : eRows) {
          std::string src;
          eRow.at("destination_id").to(src);
          std::string dst;
          eRow.at("cidr_block").to(dst);

          LOG_DEBUG << "rr--ext: " << src << " -- " << dst << std::endl;
          if (!vertexLookup.count(dst)) {
            addVertex("box", dst, dst);
          }
          addEdge(src, dst);
        }
      }
    }

    void
    addEdge(const std::string& src, const std::string& dst)
    {
      try {
        const auto u {vertexLookup.at(src)};
        const auto v {vertexLookup.at(dst)};

      if (edgeLookup.count(src) && edgeLookup[src].count(dst)) {
        const auto e {edgeLookup.at(src).at(dst)};
        //graph[e].label += oss.str();
      } else {
        Edge e;
        bool inserted;
        tie(e, inserted) = boost::add_edge(u, v, graph);
        edgeLookup[src][dst] = e;
        //graph[e].label = "";
      }
      } catch (const std::exception& e) {
        LOG_ERROR << "Failed: " << e.what() << std::endl;
        LOG_ERROR << "- Probably not '" << src << "' or '" << dst << "'" << std::endl;
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
