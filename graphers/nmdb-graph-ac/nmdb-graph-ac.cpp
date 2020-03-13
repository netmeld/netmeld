// =============================================================================
// Copyright 2017 National Technology & Engineering Solutions of Sandia, LLC
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

#include <netmeld/core/tools/AbstractGraphTool.hpp>

#include "GraphHelper.hpp"

namespace nmct = netmeld::core::tools;
namespace nmcu = netmeld::core::utils;


// =============================================================================
// Graph tool definition
// =============================================================================
class Tool : public nmct::AbstractGraphTool
{
  // ===========================================================================
  // Variables
  // ===========================================================================
  private: // Variables should generally be private
    AcGraph graph;

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
    Tool() : nmct::AbstractGraphTool
      (
       "network based access control rules",    // help blurb, prefixed with:
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
    void modifyToolOptions() override
    {
      opts.addRequiredOption("device-id", std::make_tuple(
            "device-id",
            po::value<std::string>()->required(),
            "Name of device")
          );
    }

    // Overriden from AbstractGraphTool
    int
    runTool() override
    {
      const auto& dbName  {getDbName()};
      const auto& dbArgs  {opts.getValue("db-args")};
      pqxx::connection db {std::string("dbname=") + dbName + " " + dbArgs};
      nmcu::dbPrepareCommon(db);

      db.prepare
        ("select_device_ac_sets",
         "SELECT DISTINCT *"
         " FROM ("
         "   SELECT"
         "     src_net_set_id  AS id,"
         "     src_net_set     AS name,"
         "     src_iface       AS iface"
         "    FROM device_ac_rules"
         "    WHERE ($1 = device_id)"
         "   UNION"
         "   SELECT"
         "     dst_net_set_id  AS id,"
         "     dst_net_set     AS name,"
         "     dst_iface       AS iface"
         "    FROM device_ac_rules"
         "    WHERE ($1 = device_id)"
         " ) AS data"
         " ORDER BY 1,3,2"
         "");

      db.prepare
        ("select_device_ac_nets",
         "SELECT"
         "  net_set_data  AS data"
         " FROM device_ac_nets"
         " WHERE ($1 = device_id)"
         "   AND ($2 = net_set_id)"
         "   AND ($3 = net_set)"
         "");

      db.prepare
        ("select_device_ac_edges",
         "SELECT"
         "  CONCAT(dar.src_net_set_id,':',dar.src_net_set,':',dar.src_iface)"
         "                   AS src,"
         "  CONCAT(dar.dst_net_set_id,':',dar.dst_net_set,':',dar.dst_iface)"
         "                   AS dst,"
         "  dar.ac_id        AS id,"
         "  dar.action       AS action,"
         "  dar.description  AS description,"
         "  STRING_AGG("
         "    COALESCE(das.service_set_data,dar.service_set),"
         "    ',' ORDER BY das.service_set_data"
         "    )              AS services"
         " FROM device_ac_rules AS dar"
         " FULL JOIN device_ac_services as das"
         "   ON (dar.device_id = das.device_id)"
         "  AND (dar.service_set = das.service_set)"
         " WHERE ($1 = dar.device_id)"
         "   AND (dar.enabled)"
         " GROUP BY src, dst, id, action, description"
         "");


      std::string const deviceId {nmcu::toLower(opts.getValue("device-id"))};

      buildAcGraph(db, deviceId);

      boost::write_graphviz
        (std::cout, graph,
         LabelWriter(graph),   // VertexPropertyWriter
         LabelWriter(graph),   // EdgePropertyWriter
         GraphWriter(),        // GraphPropertyWriter
         boost::get(&VertexProperties::name, graph));  // VertexID

      return nmcu::Exit::SUCCESS;
    }

    void
    buildAcGraph(pqxx::connection& db, const std::string& deviceId)
    {
      pqxx::work t{db};

      addVertices(t, deviceId);
      addEdges(t, deviceId);

      t.commit();
    }

    void
    addVertices(pqxx::transaction_base& t, const std::string& deviceId)
    {
      pqxx::result vRows =
        t.exec_prepared("select_device_ac_sets", deviceId);
      for (const auto& vRow : vRows) {
        std::string id;
        vRow.at("id").to(id);
        std::string name;
        vRow.at("name").to(name);
        std::string iface;
        vRow.at("iface").to(iface);

        std::ostringstream oss;
        oss << id << ":" << name << ":" << iface;
        std::string vName {oss.str()};
        oss.str("");
        oss << "(" << id << ") "
            << name
            << " (via " << iface << ")"
            << "\\n"
            ;

        pqxx::result targetedNetRows =
          t.exec_prepared("select_device_ac_nets",
              deviceId,
              id,
              name);
        if (targetedNetRows.size() > 0) {
          for (const auto& targetedNetRow : targetedNetRows) {
            std::string data;
            targetedNetRow.at("data").to(data);
            if (!data.empty()) {
              oss << "  " << data << "\\l";
            }
          }
        } else {
          pqxx::result globalNetRows =
            t.exec_prepared("select_device_ac_nets",
                deviceId,
                "global",
                name);
          for (const auto& globalNetRow : globalNetRows) {
            std::string data;
            globalNetRow.at("data").to(data);
            if (!data.empty()) {
              oss << "  " << data << "\\l";
            }
          }
        }

        Vertex v {boost::add_vertex(graph)};
        vertexLookup[vName] = v;

        graph[v].name  = vName;
        graph[v].label = oss.str();
        graph[v].shape = "box";

      }
    }

    void
    addEdges(pqxx::transaction_base& t, const std::string& deviceId)
    {
      pqxx::result eRows =
        t.exec_prepared("select_device_ac_edges", deviceId);

      for (const auto& eRow : eRows) {
        std::string src;
        eRow.at("src").to(src);
        std::string dst;
        eRow.at("dst").to(dst);
        std::string id;
        eRow.at("id").to(id);
        std::string description;
        eRow.at("description").to(description);
        std::string action;
        eRow.at("action").to(action);
        std::string services;
        eRow.at("services").to(services);

        std::regex quote {"\""};
        action = std::regex_replace(action, quote, "\\\"");
        services = std::regex_replace(services, quote, "\\\"");

        std::ostringstream oss;
        oss << "(" << id;
        if (!description.empty()) {
          oss << " - " << description;
        }
        oss << ") " << action << "->" << services << "\\l";

        const auto u {vertexLookup.at(src)};
        const auto v {vertexLookup.at(dst)};

        if (edgeLookup.count(src) && edgeLookup[src].count(dst)) {
          const auto e {edgeLookup.at(src).at(dst)};
          graph[e].label += oss.str();
        } else {
          Edge e;
          bool inserted;
          tie(e, inserted) = boost::add_edge(u, v, graph);
          edgeLookup[src][dst] = e;
          graph[e].label = oss.str();
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
