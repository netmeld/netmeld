// =============================================================================
// Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC
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

#include <boost/graph/dijkstra_shortest_paths.hpp>

#include <netmeld/datastore/tools/AbstractGraphTool.hpp>

#include "GraphHelper.hpp"

namespace nmdt = netmeld::datastore::tools;
namespace nmcu = netmeld::core::utils;


// ============================================================================
// Main code
// ============================================================================
class Tool : public nmdt::AbstractGraphTool
{
  private:
    std::map<std::string, Vertex> vertexLookup;

    NetworkGraph graph;

    const std::string nodeShape {"box"};
    const std::string netShape  {"oval"};
    // naively colorblind safe:
    // - (use paul tol) https://davidmathlogic.com/colorblind/
    const std::string green {"#117733"};

    const nmcu::FileManager& nmfm {nmcu::FileManager::getInstance()};

    bool useIcons           {false};
    bool hideUnknown        {false};
    bool removeEmptySubnets {false};
    bool showTracerouteHops {false};

    std::string respondingState;
    bool        passRespondingState {false};

  public:
    Tool() : nmdt::AbstractGraphTool
      ("targeted network layer information",
       PROGRAM_NAME, PROGRAM_VERSION)
    {}

    void addToolOptions() override
    {
      opts.addRequiredOption("layer", std::make_tuple(
          "layer,L",
          po::value<unsigned int>()->required()->default_value(3),
          "Layer (2,3) of the network stack to graph")
        );
      opts.addRequiredOption("device-id", std::make_tuple(
          "device-id",
          po::value<std::string>()->required(),
          "Device ID or subnet CIDR address to use as graph's root node")
        );

      opts.addOptionalOption("icons", std::make_tuple(
          "icons",
          NULL_SEMANTIC,
          "Enable device icons in graph")
        );
      opts.addOptionalOption("icons-folder", std::make_tuple(
          "icons-folder",
          po::value<std::string>()->required()->
            default_value(nmfm.getConfPath()/"images"),
          "Folder to search in for icons. ")
        );
      opts.addOptionalOption("no-unknown", std::make_tuple(
          "no-unknown",
          NULL_SEMANTIC,
          "Omit 'Unknown Device' graph nodes")
        );
      opts.addOptionalOption("no-empty-subnets", std::make_tuple(
          "no-empty-subnets",
          NULL_SEMANTIC,
          "Omit empty subnet graph nodes")
        );
      opts.addOptionalOption("show-traceroute-hops", std::make_tuple(
          "show-traceroute-hops",
          NULL_SEMANTIC,
          "Show hops found in traceroutes for devices")
        );
      opts.addOptionalOption("responding-state", std::make_tuple(
          "responding-state",
          po::value<std::string>()->required()->default_value("any"),
          "Only graph devices whose IP or MAC responding state is:"
          " any, true, or false")
        );
    }

    int
    runTool() override
    {
      pqxx::connection db {getDbConnectString()};

      // Layer 2
      db.prepare("select_device_connections", R"(
          SELECT DISTINCT
              self_device_id, self_interface_name
            , peer_device_id, peer_interface_name
          FROM device_connections
          )"
        );

      // Layer 3
      db.prepare ("select_ip_nets_extra_weights", R"(
          SELECT DISTINCT
              n.ip_net                      AS ip_net
            , COALESCE(w.extra_weight, 0.0) AS extra_weight
          FROM ip_nets AS n
          LEFT OUTER JOIN ip_nets_extra_weights AS w
            ON (n.ip_net = w.ip_net)
          WHERE NOT (n.ip_net <<= '169.254.0.0/16')
            AND NOT (n.ip_net <<= 'fe80::/10')
          ORDER BY ip_net
          )"
        );

      db.prepare("count_network_devices", R"(
          SELECT
            COUNT(dia.device_id)
          FROM device_ip_addrs AS dia
          JOIN ip_nets AS n
            ON (dia.ip_addr <<= n.ip_net)
          JOIN ip_addrs AS ia
            ON (dia.ip_addr = ia.ip_addr)
          WHERE ($1 = n.ip_net)
            AND (ia.is_responding = ANY($2))
          )"
        );

      db.prepare("select_network_vlan", R"(
          SELECT DISTINCT
              vlan AS vlan
          FROM (
            SELECT vlan, ip_net FROM vlans_ip_nets
            UNION
            SELECT vlan, ip_net FROM device_vlans_ip_nets
          ) AS foo
          WHERE ($1 = ip_net)
          )"
        );

      db.prepare("select_network_descriptions", R"(
          SELECT DISTINCT
              (LOWER(TRIM(description))) AS description
          FROM (
            SELECT DISTINCT ip_net, description FROM ip_nets
            UNION
            SELECT DISTINCT ip_net, description FROM vlans_summaries
            UNION
            SELECT DISTINCT ip_net, description FROM device_vlans_summaries
          ) AS foo
          WHERE ($1 = ip_net) AND (description IS NOT NULL)
          )"
        );

      db.prepare("select_network_connections", R"(
          SELECT DISTINCT
              ia.ip_addr    AS ip_addr
            , dia.device_id AS device_id
          FROM ip_addrs AS ia
          LEFT OUTER JOIN device_ip_addrs AS dia
            ON (ia.ip_addr = dia.ip_addr)
          JOIN ip_nets AS n
            ON (ia.ip_addr <<= n.ip_net)
          WHERE ($1 = n.ip_net) AND (ia.is_responding = ANY($2))
          ORDER BY ia.ip_addr
          )"
        );

      // Common
      db.prepare("select_devices", R"(
          SELECT DISTINCT
              d.device_id               AS device_id
            , dc.color                  AS device_color
            , dhi.device_type           AS device_type
            , dhi.vendor                AS vendor
            , COALESCE(dhi.model, '?')  AS model
          FROM devices AS d
          LEFT OUTER JOIN device_colors AS dc
            ON (d.device_id = dc.device_id)
          LEFT OUTER JOIN device_hardware_information AS dhi
            ON (d.device_id = dhi.device_id)
          ORDER BY d.device_id
          )"
        );

      db.prepare("select_device_interfaces", R"(
          SELECT DISTINCT
              dia.interface_name  AS interface_name
            , dia.ip_addr         AS ip_addr
            , ia.is_responding    AS ip_responding
            , dmaip.mac_addr      AS mac_addr
            , ma.is_responding    AS mac_responding
          FROM device_ip_addrs AS dia
          LEFT OUTER JOIN device_mac_addrs_ip_addrs AS dmaip
            ON (dia.device_id = dmaip.device_id)
            AND (dia.interface_name = dmaip.interface_name)
          LEFT OUTER JOIN ip_addrs AS ia
            ON (ia.ip_addr = dia.ip_addr)
          LEFT OUTER JOIN mac_addrs AS ma
            ON (ma.mac_addr = dmaip.mac_addr)
          WHERE ($1 = dia.device_id)
            AND (ia.is_responding = ANY($2) OR ma.is_responding = ANY($2))
          ORDER BY dia.ip_addr
          )"
        );

      db.prepare("select_mac_addrs_without_devices", R"(
          SELECT DISTINCT
              maia.ip_addr    AS ip_addr
            , mawd.mac_addr   AS mac_addr
            , mav.vendor_name AS vendor_name
          FROM mac_addrs_without_devices AS mawd
          LEFT OUTER JOIN mac_addrs_ip_addrs AS maia
            ON (mawd.mac_addr = maia.mac_addr)
          LEFT OUTER JOIN mac_addrs_vendors AS mav
            ON (mawd.mac_addr = mav.mac_addr)
          WHERE (mawd.is_responding = ANY($1))
          ORDER BY mac_addr
          )"
        );

      db.prepare("select_ip_addrs_without_devices", R"(
          SELECT DISTINCT
              iawd.ip_addr    AS ip_addr
            , maia.mac_addr   AS mac_addr
            , mav.vendor_name AS vendor_name
          FROM ip_addrs_without_devices AS iawd
          LEFT OUTER JOIN mac_addrs_ip_addrs AS maia
            ON (iawd.ip_addr = maia.ip_addr)
          LEFT OUTER JOIN mac_addrs_vendors AS mav
            ON (maia.mac_addr = mav.mac_addr)
          WHERE (iawd.is_responding = ANY($1))
          ORDER BY ip_addr
          )"
        );

      db.prepare("select_hostnames_by_ip_addr", R"(
          SELECT DISTINCT
              hostname
          FROM hostnames
          WHERE (ip_addr = host(($1)::INET)::INET)
          ORDER BY hostname
          )"
        );

      db.prepare("select_device_virtualizations", R"(
          SELECT DISTINCT
              host_device_id, guest_device_id
          FROM device_virtualizations
          )"
        );

      db.prepare("select_traceroutes", R"(
          SELECT DISTINCT
              origin   , last_hop
            , hop_count, next_hop_ip_addr
          FROM ip_traceroutes
          )"
        );

      useIcons = opts.exists("icons");
      hideUnknown = opts.exists("no-unknown");
      removeEmptySubnets = opts.exists("no-empty-subnets");
      showTracerouteHops = opts.exists("show-traceroute-hops");

      std::string state {opts.getValue("responding-state")};
      if ("0" == state || "true" == state) {
        respondingState = "{t}";
        passRespondingState = false;
      } else if ("1" == state || "false" == state) {
        respondingState = "{f}";
        passRespondingState = false;
      } else {
        respondingState = "{t,f,NULL}";
        passRespondingState = true;
      }

      unsigned int layer {opts.getValueAs<unsigned int>("layer")};
      switch (layer) {
        case 2:
          buildLayer2Graph(db);
          break;
        case 3:
          buildLayer3Graph(db);
          break;
        default:
          break;
      }

      const std::string deviceId {nmcu::toLower(opts.getValue("device-id"))};
      if (!vertexLookup.count(deviceId)) {
        LOG_ERROR << "Specified device-id ("
                  << deviceId << ") not found in datastore"
                  << std::endl;
        std::exit(nmcu::Exit::FAILURE);
      }

      boost::dijkstra_shortest_paths(
          graph, vertexLookup.at(deviceId),
          weight_map(boost::get(&EdgeProperties::weight, graph)).
          distance_map(boost::get(&VertexProperties::distance, graph))
        );

      // Remove all the "wrong direction" and redundant edges.
      boost::remove_edge_if(IsRedundantEdge(graph), graph);

      buildVirtualizationGraph(db);
      buildTracerouteGraph(db);

      boost::write_graphviz(
          std::cout, graph,
          LabelWriter(graph),   // VertexPropertyWriter
          LabelWriter(graph),   // EdgePropertyWriter
          GraphWriter(),        // GraphPropertyWriter
          boost::get(&VertexProperties::name, graph)  // VertexID
        );

      return nmcu::Exit::SUCCESS;
    }

  // ==========================================================================
  // Helpers
  // ==========================================================================
  private:
    void
    buildLayer2Graph(pqxx::connection& db)
    {
      pqxx::read_transaction rt {db};

      // Create a graph vertex (and label)
      // for each device.
      createVertexForAssociated(rt);
      // for each MAC address that is not yet associated with a device.
      createVertexForUnassociated(rt, "mac_addr");

      // Create graph edges that connect two physical ports.
      for ( const auto& physConnRow
          : rt.exec_prepared("select_device_connections")
          )
      {
        std::string selfDeviceName {physConnRow.at("self_device_id").c_str()};
        std::string peerDeviceName {physConnRow.at("peer_device_id").c_str()};

        addBidirectionalEdge(selfDeviceName, peerDeviceName);
      }
    }

    void
    buildLayer3Graph(pqxx::connection& db)
    {
      pqxx::read_transaction rt {db};

      // Create a graph vertex (and label)
      // for each device.
      createVertexForAssociated(rt);
      // for each MAC address that is not yet associated with a device.
      createVertexForUnassociated(rt, "ip_addr");

      // Create a graph vertex for each IP network.
      for ( const auto& ipNetRow
          : rt.exec_prepared("select_ip_nets_extra_weights")
          )
      {
        std::string ipNet   {ipNetRow.at("ip_net").c_str()};
        double extraWeight  {ipNetRow.at("extra_weight").as<double>()};

        // Skip empty subnets if requested
        if (removeEmptySubnets) {
          pqxx::row count {
              rt.exec_prepared1("count_network_devices"
                               , ipNet, respondingState
                               )
            };
          if (count[0].as<unsigned int>() <= 1) {
            continue;
          }
        }

        std::ostringstream oss;
        oss << ipNet << R"(\n)";

        // Add any VLAN tag information
        pqxx::result vlanRows {rt.exec_prepared("select_network_vlan", ipNet)};
        if (vlanRows.size()) {
          oss << "VLAN:";
          for (const auto& vlanRow : vlanRows) {
            oss << " " << vlanRow.at("vlan").c_str();
          }
          oss << R"(\n)";
        }

        // Add any IP net description
        for ( const auto& descRow
            : rt.exec_prepared("select_network_descriptions", ipNet)
            )
        {
          oss << descRow.at("description").c_str() << R"(\n)";
        }

        addNetVertex(ipNet, oss.str(), extraWeight);

        // only connect responding IPs to subnets; unless want specific state
        std::string state {"{t}"};
        if (!passRespondingState) {
          state = respondingState;
        }

        // Create graph edges that connect the IP network to the devices and IP
        // addresses in that network.
        for ( const auto& ipAddrRow
            : rt.exec_prepared("select_network_connections", ipNet, state)
            )
        {
          std::string ipAddr      {ipAddrRow.at("ip_addr").c_str()};
          std::string deviceName  {ipAddrRow.at("device_id").c_str()};

          const std::string vertexName {
              deviceName.size() ? (deviceName) : (ipAddr)
            };

          addBidirectionalEdge(ipNet, vertexName);
        }
      }
    }

    // Create graph edges that connect VM host and guest devices.
    void
    buildVirtualizationGraph(pqxx::connection& db)
    {
      pqxx::read_transaction rt {db};

      for ( const auto& vmRow
          : rt.exec_prepared("select_device_virtualizations")
          )
      {
        std::string hostDeviceName  {vmRow.at("host_device_id").c_str()};
        std::string guestDeviceName {vmRow.at("guest_device_id").c_str()};

        if (!verticesExist(guestDeviceName, hostDeviceName)) {
          LOG_DEBUG << "(__FUNCTION__) Missing a vertex for edge: "
                    << guestDeviceName << " -- " << hostDeviceName
                    << std::endl;
          continue;
        }

        const Vertex u {vertexLookup.at(guestDeviceName)};
        const Vertex v {vertexLookup.at(hostDeviceName)};

        Edge e;
        bool inserted;

        std::tie(e, inserted) = boost::add_edge(u, v, graph);
        if (inserted) {
          graph[e].style = "dashed";
          graph[e].direction = "forward";
        }
      }
    }

    // Create graph edges that represent hops found in traceroutes
    void
    buildTracerouteGraph(pqxx::connection& db)
    {
      if (!showTracerouteHops) {
        return;
      }

      pqxx::read_transaction rt {db};

      for ( const auto& tracerouteRow
          : rt.exec_prepared("select_traceroutes")
          )
      {
        std::string origin    {tracerouteRow.at("origin").c_str()};
        std::string lastHop   {tracerouteRow.at("last_hop").c_str()};
        std::string hopNumber {tracerouteRow.at("hop_count").c_str()};
        std::string nextHop   {tracerouteRow.at("next_hop_ip_addr").c_str()};

        if (!verticesExist(origin, nextHop)) {
          LOG_DEBUG << "(__FUCTION__) Missing a vertex for edge: "
                    << origin << " -- " << nextHop
                    << std::endl;
          continue;
        }

        const Vertex u {vertexLookup.at(origin)};
        const Vertex v {vertexLookup.at(nextHop)};

        Edge e;
        bool inserted;

        std::tie(e, inserted) = boost::add_edge(u, v, graph);
        if (inserted) {
          graph[e].style = "dashed";
          graph[e].direction = "forward";

          std::ostringstream oss(graph[e].label, std::ios_base::ate);
          if (oss.tellp() > 0) {
            oss << '\n';            
          }
          oss << std::format("hop {} to {}", hopNumber, lastHop);
          graph[e].label = oss.str();

          graph[e].weight = 2.0;
        }
      }
    }

    void
    createVertexForAssociated(pqxx::read_transaction& rt)
    {
      for ( const auto& deviceRow
          : rt.exec_prepared("select_devices")
          )
      {
        std::string deviceName  {deviceRow.at("device_id").c_str()};
        std::string deviceColor {deviceRow.at("device_color").c_str()};
        std::string deviceType  {deviceRow.at("device_type").c_str()};
        std::string vendor      {deviceRow.at("vendor").c_str()};
        std::string model       {deviceRow.at("model").c_str()};

        std::ostringstream oss;

        oss << initVertexLabel(deviceType, deviceName);
        
        if (!vendor.empty()) {
          oss << std::format("({}:{}:{})<BR/>", vendor, model, deviceType);
        }
        oss << "<BR/>";

        // Add interface(s)
        bool lPassRespondingState {passRespondingState};
        for ( const auto& ifaceRow
            : rt.exec_prepared("select_device_interfaces"
                              , deviceName, respondingState
                              )
            )
        {
          std::string ipAddr        {ifaceRow.at("ip_addr").c_str()};
          std::string macAddr       {ifaceRow.at("mac_addr").c_str()};
          std::string interfaceName {ifaceRow.at("interface_name").c_str()};
          // NOTE: false if NULL
          bool ipResponding   {ifaceRow.at("ip_responding").as<bool>(false)};
          bool macResponding  {ifaceRow.at("mac_responding").as<bool>(false)};

          // IP
          if (ipResponding && !ipAddr.empty()) {
            oss << std::format(R"(<FONT color="{}">{}</FONT>)", green, ipAddr);
          } else {
            oss << ipAddr;
          }
          oss << ' ';

          // MAC
          oss << '[';
          if (macResponding && !macAddr.empty()) {
            oss << std::format(R"(<FONT color="{}">{}</FONT>)", green, macAddr);
          } else {
            oss << macAddr;
          }
          oss << "] ";

          // Interface name
          oss << std::format(R"(({})<BR align="left"/>)", interfaceName);

          // Hostname(s)
          oss << getHostnames(rt, ipAddr);
          lPassRespondingState = true;
        }

        // Close out table syntax
        oss << closeVertexLabel();

        if (lPassRespondingState) {
          addNodeVertex(deviceName, oss.str(), deviceColor);
        }
      }
    }

    void
    createVertexForUnassociated(pqxx::read_transaction& rt,
                                const std::string& type)
    {
      if (hideUnknown) { // short-circuit if we do not want unknowns
        return;
      }

      for ( const auto& addrRow
          : rt.exec_prepared("select_" + type + "s_without_devices"
                            , respondingState
                            )
          )
      {
        std::string ipAddr  {addrRow.at("ip_addr").c_str()};
        std::string macAddr {addrRow.at("mac_addr").c_str()};
        std::string vendor  {addrRow.at("vendor_name").c_str()};
        std::string addr    {("ip_addr" == type ? ipAddr : macAddr)};

        std::ostringstream oss;

        oss << initVertexLabel("unknown", "Unknown Device") << "<BR/>";

        // Add unknown interface(s)
        oss << std::format(R"({} [{}] <BR align="left"/>)", ipAddr, macAddr);
        if (!vendor.empty()) {
          oss << R"(    {)" << vendor << R"(}<BR align="left"/>)";
        }

        // Add hostname(s)
        oss << getHostnames(rt, ipAddr);

        // Close out table syntax
        oss << closeVertexLabel();

        addNodeVertex(addr, oss.str());
      }
    }

    std::string
    initVertexLabel(const std::string& typeName, const std::string& name)
    {
      return std::format(R"(" + <<TABLE border="0" cellborder="0"><TR>{})"
                         R"(<TD><FONT point-size="10">{}<BR/>)"
                        , getUseIconString(typeName)
                        , name
                        )
        ;
    }

    std::string
    closeVertexLabel()
    {
      return R"(</FONT></TD></TR></TABLE>> + ")";
    }

    void
    addNodeVertex(const std::string& name, const std::string& label,
                  const std::string& fillcolor="")
    {
      Vertex v {boost::add_vertex(graph)};
      vertexLookup[name] = v;

      graph[v].name  = name;
      graph[v].label = label;

      graph[v].shape = nodeShape;
      graph[v].style = "filled";
      graph[v].fillcolor = fillcolor;
      if (!fillcolor.empty()) { // once set, don't unset
        graph[v].fillcolor = fillcolor;
      }
    }

    void
    addNetVertex(std::string name, const std::string& label,
                 double extraWeight=0.0)
    {
      Vertex v {boost::add_vertex(graph)};
      vertexLookup[name] = v;

      graph[v].name  = name;
      graph[v].label = label;

      graph[v].shape = netShape;
      graph[v].extraWeight = extraWeight;
    }

    std::string
    getHostnames(pqxx::read_transaction& rt, const std::string& ipAddr)
    {
      if (ipAddr.empty()) { // short-circuit
        return "";
      }

      std::ostringstream oss;
      for ( const auto& row
          : rt.exec_prepared("select_hostnames_by_ip_addr", ipAddr)
          )
      {
        oss << std::format(R"({:>9}<BR align="left"/>)", row[0].c_str());
      }

      return oss.str();
    }

    std::string
    getUseIconString(const std::string& deviceType)
    {
      if (!useIcons) { // short-circuit
        return "";
      }

      sfs::path imagePath  {opts.getValue("icons-folder")};
      std::string iconPath {imagePath.string() + "/unknown.svg"};

      if (!deviceType.empty()) {
        for (auto& pathIter : sfs::recursive_directory_iterator(imagePath)) {
          std::string fileName {pathIter.path().filename()};

          if (std::equal(deviceType.begin(), deviceType.end(),
                         fileName.begin(), fileName.end(),
                         [](auto a, auto b) {
                            return std::tolower(a) == std::tolower(b);
                         }))
          {
            iconPath = fileName;
            break;
          }
        }
      }

      return std::format(R"(<TD width="60" height="50" fixedsize="true">)"
                         R"(<IMG src="{}" scale="true"/></TD>)"
                        , iconPath
                        )
        ;
    }

    void
    addBidirectionalEdge(const std::string& orig, const std::string& dest)
    {
      if (!verticesExist(orig, dest)) {
        LOG_DEBUG << "(__FUNCTION__) Missing a vertex for edge: "
                  << orig << " -- " << dest
                  << std::endl;
        return;
      }

      const Vertex source {vertexLookup.at(orig)};
      const Vertex target {vertexLookup.at(dest)};

      // Ensure edges in both "directions", "wrong direction" corrected later
      bool stEdgeExists {std::get<1>(boost::edge(source, target, graph))};
      bool tsEdgeExists {std::get<1>(boost::edge(target, source, graph))};
      if (!(stEdgeExists && tsEdgeExists)) {
        Edge edge;
        bool inserted;

        std::tie(edge, inserted) = boost::add_edge(source, target, graph);
        graph[edge].weight += graph[source].extraWeight;

        std::tie(edge, inserted) = boost::add_edge(target, source, graph);
        graph[edge].weight += graph[source].extraWeight;
      }
    }

    template <class... Vs>
    bool
    verticesExist(const std::string& vert, const Vs&... rest)
    {
      bool exist {true};
      if (vertexLookup.end() == vertexLookup.find(vert)) {
        LOG_DEBUG << "Missing vertex: " << vert << '\n';
        exist = false;
      }

      if constexpr (sizeof...(rest) > 0) {
        exist = (exist && verticesExist(rest...));
      }

      return exist;
    }
};

int
main(int argc, char** argv)
{
  Tool tool;
  return tool.start(argc, argv);
}
