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

#include <boost/graph/dijkstra_shortest_paths.hpp>

#include <netmeld/datastore/tools/AbstractGraphTool.hpp>

namespace nmdt = netmeld::datastore::tools;
namespace nmcu = netmeld::core::utils;

// ----------------------------------------------------------------------------
// Graphing Helpers
// ----------------------------------------------------------------------------
struct VertexProperties
{
  std::string name;
  std::string label;
  std::string shape;
  std::string style;
  std::string fillcolor;

  double distance     {std::numeric_limits<double>::infinity()};
  double extraWeight  {0.0};
};


struct EdgeProperties
{
  std::string label;
  std::string style;

  std::string direction;
  std::string arrowhead;
  std::string arrowtail;

  double weight {1.0};
};


using NetworkGraph =
boost::adjacency_list<
  boost::listS,         // OutEdgeList
  boost::vecS,          // VertexList
  boost::undirectedS,   // Directed/Undirected
  VertexProperties,     // VertexProperties
  EdgeProperties,       // EdgeProperties
  boost::no_property    // GraphProperties
>;

using Vertex = NetworkGraph::vertex_descriptor;
using Edge   = NetworkGraph::edge_descriptor;

const std::string nodeShape {"box"};
const std::string netShape  {"oval"};

class LabelWriter
{
  private:
    const NetworkGraph& g_;

  public:
    explicit LabelWriter(const NetworkGraph& g) : g_(g) { }

    void operator()(std::ostream& os, const Vertex& v) const
    {
      os << "[shape=\"" << g_[v].shape << "\", ";

      if (!g_[v].style.empty()) {
        os << "style=\"" << g_[v].style << "\", ";
      }

      if (!g_[v].fillcolor.empty()) {
        os << "fillcolor=\"" << g_[v].fillcolor << "\", ";
      }

      os << "label=\"" << g_[v].label << "\"]";
    }

    void operator()(std::ostream& os, const Edge& e) const
    {
      const Vertex s {boost::source(e, g_)};
      const Vertex t {boost::target(e, g_)};

      const double distanceRatio {(g_[s].distance / g_[t].distance)};

      os << "[";

      if ((0.999 < distanceRatio) && (distanceRatio < 1.001)) {
        os << "constraint=\"false\", ";
      }

      if (!g_[e].style.empty()) {
        os << "style=\"" << g_[e].style << "\", ";
      }

      if (!g_[e].direction.empty()) {
        os << "dir=\"" << g_[e].direction << "\", ";
      }

      if (!g_[e].arrowhead.empty()) {
        os << "arrowhead=\"" << g_[e].arrowhead << "\", ";
      }

      if (!g_[e].arrowtail.empty()) {
        os << "arrowtail=\"" << g_[e].arrowtail << "\", ";
      }

      os << "label=\"" << g_[e].label << "\"]";
    }
};

class GraphWriter
{
  public:
    void operator()(std::ostream& os) const
    {
      os << "splines=true;\n"
         << "nodesep=1.00;\n"
         << "ranksep=2.50;\n"
         << "overlap=false;\n"
         ;
    }
};

class IsRedundantEdge
{
  private:
    const NetworkGraph& g_;
  public:
    explicit IsRedundantEdge(const NetworkGraph& g) : g_(g) { }

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


// ============================================================================
// Main code
// ============================================================================
class Tool : public nmdt::AbstractGraphTool
{
  private:
    std::map<std::string, Vertex> vertexLookup;

    NetworkGraph graph;

    const nmcu::FileManager& nmfm {nmcu::FileManager::getInstance()};

    bool useIcons           {false};
    bool hideUnknown        {false};
    bool removeEmptySubnets {false};
    bool showTracerouteHops {false};

  public:
    Tool() : nmdt::AbstractGraphTool
      ("--layer information",
       PROGRAM_NAME, PROGRAM_VERSION)
    {}

    void addToolOptions() override
    {
      opts.addRequiredOption("layer", std::make_tuple(
          "layer,L",
          po::value<std::string>()->required(),
          "Layer of the network stack to graph")
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
    }

    int
    runTool() override
    {
      pqxx::connection db {getDbConnectString()};

      db.prepare
        ("select_ip_nets_extra_weights",
         "SELECT DISTINCT"
         "   n.ip_net AS ip_net,"
         "   COALESCE(w.extra_weight, 0.0) AS extra_weight"
         " FROM ip_nets AS n"
         " LEFT OUTER JOIN ip_nets_extra_weights AS w"
         "  ON (n.ip_net = w.ip_net)"
         " WHERE NOT (n.ip_net <<= '169.254.0.0/16')"
         "   AND NOT (n.ip_net <<= 'fe80::/10')"
         " ORDER BY ip_net");

      db.prepare
        ("select_network_descriptions",
         "SELECT DISTINCT (LOWER(TRIM(description))) AS description"
         " FROM ("
         "  SELECT DISTINCT ip_net, description FROM ip_nets"
         "  UNION"
         "  SELECT DISTINCT ip_net, description FROM vlans_summaries"
         "  UNION"
         "  SELECT DISTINCT ip_net, description FROM device_vlans_summaries"
         " ) AS foo"
         " WHERE ($1 = ip_net) AND (description IS NOT NULL)"
         );

      db.prepare
        ("select_vlan_by_ip_net",
         "SELECT DISTINCT vlan AS vlan"
         " FROM ("
         "  SELECT vlan, ip_net FROM vlans_ip_nets"
         "  UNION"
         "  SELECT vlan, ip_net FROM device_vlans_ip_nets"
         " ) AS foo"
         " WHERE ($1 = ip_net)");

      db.prepare
        ("select_devices",
         "SELECT DISTINCT"
         "    d.device_id AS device_id,"
         "    dc.color AS device_color,"
         "    dhi.device_type AS device_type,"
         "    dhi.vendor AS vendor,"
         "    COALESCE(dhi.model, '?') AS model"
         " FROM devices AS d"
         " LEFT OUTER JOIN device_colors AS dc"
         "  ON (d.device_id = dc.device_id)"
         " LEFT OUTER JOIN device_hardware_information AS dhi"
         "  ON (d.device_id = dhi.device_id)"
         " ORDER BY d.device_id");

      db.prepare
        ("select_device_ifaces",
         "SELECT DISTINCT"
         "   dia.interface_name AS interface_name,"
         "   dia.ip_addr        AS ip_addr,"
         "   dmaip.mac_addr     AS mac_addr"
         " FROM device_ip_addrs AS dia"
         " LEFT OUTER JOIN device_mac_addrs_ip_addrs AS dmaip"
         "  ON (dia.device_id = dmaip.device_id)"
         "  AND (dia.interface_name = dmaip.interface_name)"
         " WHERE ($1 = dia.device_id)"
         " ORDER BY dia.ip_addr");

      db.prepare
        ("select_devices_in_network",
         "SELECT DISTINCT dia.device_id AS device_id"
         " FROM device_ip_addrs AS dia"
         " JOIN ip_nets AS n"
         "  ON (dia.ip_addr <<= n.ip_net)"
         " WHERE ($1 = n.ip_net)"
         " ORDER BY dia.device_id");

      db.prepare
        ("select_mac_addrs_without_devices",
         "SELECT DISTINCT "
         "  maia.ip_addr AS ip_addr,"
         "  mawd.mac_addr AS mac_addr,"
         "  mav.vendor_name as vendor_name"
         " FROM mac_addrs_without_devices AS mawd"
         " LEFT OUTER JOIN mac_addrs_ip_addrs AS maia"
         "  ON (mawd.mac_addr = maia.mac_addr)"
         " LEFT OUTER JOIN mac_addrs_vendors AS mav"
         "  ON (mawd.mac_addr = mav.mac_addr)"
         " ORDER BY mac_addr");

      db.prepare
        ("select_ip_addrs_without_devices",
         "SELECT DISTINCT "
         "  iawd.ip_addr AS ip_addr,"
         "  maia.mac_addr AS mac_addr,"
         "  mav.vendor_name as vendor_name"
         " FROM ip_addrs_without_devices AS iawd"
         " LEFT OUTER JOIN mac_addrs_ip_addrs AS maia"
         "  ON (iawd.ip_addr = maia.ip_addr)"
         " LEFT OUTER JOIN mac_addrs_vendors AS mav"
         "  ON (maia.mac_addr = mav.mac_addr)"
         " ORDER BY ip_addr");

      db.prepare
        ("select_hostnames_by_ip_addr",
         "SELECT DISTINCT hostname"
         " FROM hostnames"
         " WHERE (ip_addr = host(($1)::INET)::INET)"
         " ORDER BY hostname");

      db.prepare
        ("select_ip_addrs_and_devices_in_network",
         "SELECT DISTINCT"
         "   ia.ip_addr    AS ip_addr,"
         "   dia.device_id AS device_id"
         " FROM ip_addrs AS ia"
         " LEFT OUTER JOIN device_ip_addrs AS dia"
         "  ON (ia.ip_addr = dia.ip_addr)"
         " JOIN ip_nets AS n"
         "  ON (ia.ip_addr <<= n.ip_net)"
         " WHERE ($1 = n.ip_net) AND (ia.is_responding)"
         " ORDER BY ia.ip_addr");

      db.prepare
        ("select_device_connections",
         "SELECT DISTINCT"
         "   self_device_id, self_interface_name,"
         "   peer_device_id, peer_interface_name"
         " FROM device_connections");

      db.prepare
        ("select_device_virtualizations",
         "SELECT DISTINCT"
         "   host_device_id, guest_device_id"
         " FROM device_virtualizations");

      db.prepare
        ("select_traceroutes",
         "SELECT DISTINCT"
         "   origin, last_hop, hop_count, next_hop_ip_addr"
         " FROM ip_traceroutes");

      useIcons = opts.exists("icons");
      hideUnknown = opts.exists("no-unknown");
      removeEmptySubnets = opts.exists("no-empty-subnets");
      showTracerouteHops = opts.exists("show-traceroute-hops");

      int layer {std::stoi(opts.getValue("layer"))};
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

      std::string const deviceId {nmcu::toLower(opts.getValue("device-id"))};
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
      pqxx::work t {db};

      // Create a graph vertex (and label)
      // for each device.
      createVertexForAssociated(t);
      // for each MAC address that is not yet associated with a device.
      createVertexForUnassociated(t, "mac_addr");

      // Create graph edges that connect two physical ports.
      pqxx::result physConnRows {t.exec_prepared("select_device_connections")};
      for (const auto& physConnRow : physConnRows) {
        std::string selfDeviceName;
        physConnRow.at("self_device_id").to(selfDeviceName);
        std::string peerDeviceName;
        physConnRow.at("peer_device_id").to(peerDeviceName);

        addBidirectionalEdge(selfDeviceName, peerDeviceName);
      }

      t.commit();
    }

    void
    buildLayer3Graph(pqxx::connection& db)
    {
      pqxx::work t {db};

      // Create a graph vertex (and label)
      // for each device.
      createVertexForAssociated(t);
      // for each MAC address that is not yet associated with a device.
      createVertexForUnassociated(t, "ip_addr");

      // Create a graph vertex for each IP network.
      pqxx::result ipNetRows {t.exec_prepared("select_ip_nets_extra_weights")};
      for (const auto& ipNetRow : ipNetRows) {
        std::string ipNet;
        ipNetRow.at("ip_net").to(ipNet);
        double extraWeight;
        ipNetRow.at("extra_weight").to(extraWeight);

        // Skip empty subnets if requested
        pqxx::result netConnection {
            t.exec_prepared("select_devices_in_network", ipNet)
          };
        if (removeEmptySubnets && netConnection.size() <= 1) {
          continue;
        }

        std::string label {(ipNet + "\\n")};

        // Add any VLAN tag information
        pqxx::result vlanRows {
            t.exec_prepared("select_vlan_by_ip_net", ipNet)
          };
        if (vlanRows.size()) {
          label += "VLAN:";
          for (const auto& vlanRow : vlanRows) {
            uint16_t vlan;
            vlanRow.at("vlan").to(vlan);
            label += (" " + std::to_string(static_cast<uint32_t>(vlan)));
          }
          label += "\\n";
        }

        // Add any IP net description
        pqxx::result descRows {
            t.exec_prepared("select_network_descriptions", ipNet)
          };
        for (const auto& descRow : descRows) {
          std::string description;
          descRow.at("description").to(description);
          label += description + "\\n";
        }

        addNetVertex(ipNet, label, extraWeight);

        // Create graph edges that connect the IP network to the devices and IP
        // addresses in that network.
        pqxx::result ipAddrRows {
            t.exec_prepared("select_ip_addrs_and_devices_in_network", ipNet)
          };
        for (const auto& ipAddrRow : ipAddrRows) {
          std::string ipAddr;
          ipAddrRow.at("ip_addr").to(ipAddr);
          std::string deviceName;
          ipAddrRow.at("device_id").to(deviceName);

          std::string const vertexName {
              deviceName.size() ? (deviceName) : (ipAddr)
            };

          addBidirectionalEdge(ipNet, vertexName);
        }
      }

      t.commit();
    }

    // Create graph edges that connect VM host and guest devices.
    void
    buildVirtualizationGraph(pqxx::connection& db)
    {
      pqxx::work t {db};

      pqxx::result vmRows {t.exec_prepared("select_device_virtualizations")};
      for (const auto& vmRow : vmRows) {
        std::string hostDeviceName;
        vmRow.at("host_device_id").to(hostDeviceName);
        std::string guestDeviceName;
        vmRow.at("guest_device_id").to(guestDeviceName);

        if (!verticesExist(guestDeviceName, hostDeviceName)) {
          LOG_DEBUG << "buildVirtualizationGraph: Adding edge without vertex: "
                    << guestDeviceName << " -- " << hostDeviceName
                    << std::endl;
          continue;
        }

        const Vertex u {vertexLookup.at(guestDeviceName)};
        const Vertex v {vertexLookup.at(hostDeviceName)};

        Edge e;
        bool inserted;

        tie(e, inserted) = boost::add_edge(u, v, graph);
        if (inserted) {
          graph[e].style = "dashed";
          graph[e].direction = "forward";
        }
      }

      t.commit();
    }

    // Create graph edges that represent hops found in traceroutes
    void
    buildTracerouteGraph(pqxx::connection& db)
    {
      if (!showTracerouteHops) {
        return;
      }

      pqxx::work t {db};

      pqxx::result tracerouteRows {t.exec_prepared("select_traceroutes")};
      for (const auto& tracerouteRow : tracerouteRows) {
        std::string origin;
        tracerouteRow.at("origin").to(origin);
        std::string destination;
        tracerouteRow.at("last_hop").to(destination);
        std::string hopNumber;
        tracerouteRow.at("hop_count").to(hopNumber);
        std::string nextHop;
        tracerouteRow.at("next_hop_ip_addr").to(nextHop);

        if (!verticesExist(origin, nextHop)) {
          LOG_DEBUG << "buildTracerouteGraph: Adding edge without vertex: "
                    << origin << " -- " << nextHop
                    << std::endl;
          continue;
        }

        const Vertex u {vertexLookup.at(origin)};
        const Vertex v {vertexLookup.at(nextHop)};

        Edge e;
        bool inserted;

        tie(e, inserted) = boost::add_edge(u, v, graph);
        if (inserted) {
          graph[e].style = "dashed";
          graph[e].direction = "forward";

          const std::string hopLabel
              {"hop " + hopNumber + " to " + destination};
          if (graph[e].label.empty()) {
            graph[e].label = hopLabel;
          } else {
            graph[e].label += '\n' + hopLabel;
          }
          graph[e].weight = 2.0;
        }
      }

      t.commit();
    }

    void
    createVertexForAssociated(pqxx::transaction_base& t)
    {
      pqxx::result deviceRows {t.exec_prepared("select_devices")};
      for (const auto& deviceRow : deviceRows) {
        std::string deviceName;
        deviceRow.at("device_id").to(deviceName);
        std::string deviceColor;
        deviceRow.at("device_color").to(deviceColor);
        std::string deviceType;
        deviceRow.at("device_type").to(deviceType);
        std::string vendor;
        deviceRow.at("vendor").to(vendor);
        std::string model;
        deviceRow.at("model").to(model);

        std::string label {initVertexLabel(deviceType, deviceName)};

        label += ((!vendor.empty())
                   ? "(" + vendor + ":" + model + ":" + deviceType + ")<br/>"
                   : "") +
                 "<br/>";

        // Add interface(s)
        pqxx::result ifaceRows {
            t.exec_prepared("select_device_ifaces", deviceName)
          };
        for (const auto& ifaceRow : ifaceRows) {
          std::string ipAddr;
          ifaceRow.at("ip_addr").to(ipAddr);
          std::string macAddr;
          ifaceRow.at("mac_addr").to(macAddr);
          std::string interfaceName;
          ifaceRow.at("interface_name").to(interfaceName);

          label += ipAddr + " "
                 + "[" + macAddr + "] "
                 + "(" + interfaceName + ")"
                 + "<br align=\"left\"/>";

          // Add hostname(s)
          label += getHostnames(t, ipAddr);
        }
        label += closeVertexLabel();

        addNodeVertex(deviceName, label, deviceColor);
      }
    }

    void
    createVertexForUnassociated(pqxx::transaction_base& t,
                                const std::string& type)
    {
      if (hideUnknown) { // short-circuit if we do not want unknowns
        return;
      }

      pqxx::result addrRows {
          t.exec_prepared("select_" + type + "s_without_devices")
        };
      for (const auto& addrRow : addrRows) {
        std::string ipAddr;
        addrRow.at("ip_addr").to(ipAddr);
        std::string macAddr;
        addrRow.at("mac_addr").to(macAddr);
        std::string vendor;
        addrRow.at("vendor_name").to(vendor);
        std::string addr {("ip_addr" == type ? ipAddr : macAddr)};

        std::string label {initVertexLabel("unknown", "Unknown Device")};

        label += "<br/>";

        // Add unknown interface(s)
        label += ipAddr + " "
               + "[" + macAddr + "] "
               + ((!vendor.empty())
                   ? "<br align=\"left\"/>    {" + vendor + "}"
                   : "")
               + "<br align=\"left\"/>";

        // Add hostname(s)
        label += getHostnames(t, ipAddr);

        label += closeVertexLabel();

        addNodeVertex(addr, label);
      }
    }

    std::string
    initVertexLabel(const std::string& typeName, const std::string& name)
    {
      std::string label {"\" + <<TABLE border=\"0\" cellborder=\"0\"><TR>"};

      label += getUseIconString(typeName);
      label += "<td><font point-size=\"10\">" + name + "<br/>";

      return label;
    }

    std::string
    closeVertexLabel()
    {
      return "</font></td></TR></TABLE>> + \"";
    }

    void
    addNodeVertex(std::string name, const std::string& label,
                  std::string fillcolor="")
    {
      Vertex v {boost::add_vertex(graph)};
      vertexLookup[name] = v;

      graph[v].name  = name;
      graph[v].label = label;

      graph[v].shape = nodeShape;
      graph[v].style = "filled";
      if (!fillcolor.empty()) {
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
    getHostnames(pqxx::transaction_base& t, std::string ipAddr)
    {
      std::string label {""};

      if (!ipAddr.empty()) {
        pqxx::result hostnameRows {
            t.exec_prepared("select_hostnames_by_ip_addr", ipAddr)
          };
        for (const auto& hostnameRow : hostnameRows) {
          std::string hostname;
          hostnameRow.at("hostname").to(hostname);
          label += ("        " + hostname + "<br align=\"left\"/>");
        }
      }

      return label;
    }

    std::string
    getUseIconString(std::string deviceType)
    {
      if (!useIcons) {
        return "";
      }

      sfs::path imagePath  {opts.getValue("icons-folder")};
      std::string iconPath {imagePath.string() + "/unknown.svg"};
      std::string label {
          "<TD width=\"60\" height=\"50\" fixedsize=\"true\"><IMG SRC=\""
        };

      if (!deviceType.empty()) {
        for (auto& pathIter : sfs::recursive_directory_iterator(imagePath)) {
          std::string fileName {pathIter.path().filename()};

          if (std::equal(deviceType.begin(), deviceType.end(),
                         fileName.begin(), fileName.end(),
                         [](auto a, auto b) {
                            return std::tolower(a) == std::tolower(b);
                         })) {
            iconPath = fileName;
            break;
          }
        }
      }

      label += iconPath + "\" scale=\"true\"/></TD>";

      return label;
    }
    
    void
    addBidirectionalEdge(std::string orig, std::string dest)
    {

      if (!verticesExist(orig, dest)) {
        LOG_DEBUG << "addBidirectionalEdge: Adding edge without vertex: "
                  << orig << " -- " << dest
                  << std::endl;
        return;
      }

      const Vertex u {vertexLookup.at(orig)};
      const Vertex v {vertexLookup.at(dest)};

      // Add the edge in both "directions", "wrong direction" corrected later
      if (!(get<1>(boost::edge(u, v, graph)) &&
            get<1>(boost::edge(v, u, graph)))) {
        Edge e;
        bool inserted;

        tie(e, inserted) = boost::add_edge(u, v, graph);
        graph[e].weight += graph[u].extraWeight;

        tie(e, inserted) = boost::add_edge(v, u, graph);
        graph[e].weight += graph[u].extraWeight;
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
        exist = exist && verticesExist(rest...);
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
