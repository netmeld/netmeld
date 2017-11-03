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

#include <netmeld/common/networking.hpp>
#include <netmeld/common/queries_common.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

#include <boost/numeric/conversion/cast.hpp>
#include <boost/program_options.hpp>

#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <stdio.h>

#include <algorithm>
#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

using std::cin;
using std::cout;
using std::cerr;
using std::endl;

using std::get;
using std::map;
using std::string;
using std::to_string;
using std::vector;

using boost::numeric_cast;

namespace program_options = boost::program_options;


// Bundled properties:

struct Vertex_Properties
{
  string name;
  string label;
  string shape;
  string style;
  string fillcolor;

  double distance = std::numeric_limits<double>::infinity();
  double extra_weight = 0.0;
};


struct Edge_Properties
{
  string label;
  string style;

  string direction;
  string arrowhead;
  string arrowtail;

  double weight = 1.0;
};


using Network_Graph =
boost::adjacency_list<
  boost::listS,         // OutEdgeList
  boost::vecS,          // VertexList
  boost::undirectedS,   // Directed/Undirected
  Vertex_Properties,    // VertexProperties
  Edge_Properties,      // EdgeProperties
  boost::no_property    // GraphProperties
  >;

using Vertex = Network_Graph::vertex_descriptor;
using Edge   = Network_Graph::edge_descriptor;

class Label_Writer
{
public:
  Label_Writer(Network_Graph& g) : g_(g) { }

  void operator()(std::ostream& os, Vertex const& v) const {
    os << "["
       << "shape=\"" << g_[v].shape << "\""
       << ", ";

    if (!g_[v].style.empty()) {
      os << "style=\"" << g_[v].style << "\""
         << ", ";
    }

    if (!g_[v].fillcolor.empty()) {
      os << "fillcolor=\"" << g_[v].fillcolor << "\""
         << ", ";
    }

    os << "label=\"" << g_[v].label << "\""
       << "]";
  }

  void operator()(std::ostream& os, Edge const& e) const {
    Vertex const s = boost::source(e, g_);
    Vertex const t = boost::target(e, g_);

    double const distance_ratio = (g_[s].distance / g_[t].distance);

    os << "[";

    if ((0.999 < distance_ratio) && (distance_ratio < 1.001)) {
      os << "constraint=\"false\""
         << ", ";
    }

    if (!g_[e].style.empty()) {
      os << "style=\"" << g_[e].style << "\""
         << ", ";
    }

    if (!g_[e].direction.empty()) {
      os << "dir=\"" << g_[e].direction << "\""
         << ", ";
    }

    if (!g_[e].arrowhead.empty()) {
      os << "arrowhead=\"" << g_[e].arrowhead << "\""
         << ", ";
    }

    if (!g_[e].arrowtail.empty()) {
      os << "arrowtail=\"" << g_[e].arrowtail << "\""
         << ", ";
    }

    os << "label=\"" << g_[e].label << "\""
       << "]";
  }

private:
  Network_Graph const& g_;
};

class Graph_Writer
{
public:
  void operator()(std::ostream& os) const {
    os << "splines=true;" << endl;
    os << "nodesep=1.00;" << endl;
    os << "ranksep=2.50;" << endl;
    os << "overlap=false;" << endl;
  }
};

class Is_Redundant_Edge
{
public:
  Is_Redundant_Edge(Network_Graph& g) : g_(g) { }

  bool operator()(Edge const& e) const {
    Vertex const s = boost::source(e, g_);
    Vertex const t = boost::target(e, g_);

    return (get<1>(boost::edge(t, s, g_)) &&
            ((g_[s].distance > g_[t].distance) ||
             ((g_[s].distance >= g_[t].distance) && (s > t))));
  }

private:
  Network_Graph const& g_;
};


string
get_graphic_string(string device_type);

void
createVertexForAssociated(pqxx::transaction<>& t, Network_Graph& graph,
                           map<string, Vertex>& vertex_lookup, bool graphic);
void
createVertexForUnassociated(pqxx::transaction<>& t, Network_Graph& graph,
                           map<string, Vertex>& vertex_lookup, bool graphic,
                           string type);

void
build_layer2_graph(pqxx::connection& db, Network_Graph& graph,
                   map<string, Vertex>& vertex_lookup, bool graphic);

void
build_layer3_graph(pqxx::connection& db, Network_Graph& graph,
                   map<string, Vertex>& vertex_lookup, bool graphic);

void
build_virtualization_graph(pqxx::connection& db, Network_Graph& graph,
                           map<string, Vertex>& vertex_lookup);


string
get_graphic_string(string device_type)
{
  boost::filesystem::path               image_dir(NETMELD_IMAGE_DIR);
  boost::filesystem::directory_iterator image_dir_iter(image_dir);

  string icon_path = string(NETMELD_IMAGE_DIR) + "/unknown.svg";

  if (!device_type.empty()) {
    for (auto& icon : boost::make_iterator_range(image_dir_iter, {}))
    {
      string temp_icon_path = icon.path().string();

      size_t firstidx = temp_icon_path.find_last_of("\\/") + 1;
      size_t lastidx = temp_icon_path.find_last_of(".");
      string base_icon = temp_icon_path.substr(firstidx, lastidx - firstidx);

      if (std::equal(device_type.begin(), device_type.end(), 
                     base_icon.begin(),
                     [](auto a, auto b) { 
                        return std::tolower(a) == std::tolower(b); 
                     })) {
        icon_path = temp_icon_path;
        break;
      }
    }
  }

  return string("<TD width=\"60\" height=\"50\" fixedsize=\"true\">") +
         "<IMG SRC=\"" + icon_path + "\" scale=\"true\"/></TD>";
}

void
createVertexForAssociated(pqxx::transaction<>& t, Network_Graph& graph,
                           map<string, Vertex>& vertex_lookup, bool graphic)
{
  if (true) {
    // Create a graph vertex (and label) for each device.

    pqxx::result device_rows =
      t.prepared("select_devices").exec();
    for (auto const& device_row : device_rows) {
      string device_name;
      device_row.at("device_id").to(device_name);
      string device_color;
      device_row.at("device_color").to(device_color);
      string device_type;
      device_row.at("device_type").to(device_type);
      string vendor;
      device_row.at("vendor").to(vendor);
      string model;
      device_row.at("model").to(model);

      string label = "\" + <<TABLE border=\"0\" cellborder=\"0\"><TR>";
      if (graphic) {
        label += get_graphic_string(device_type);
      }
      label += "<td><font point-size=\"10\">" + device_name + 
               ((!vendor.empty()) 
                  ? "<br/>(" + vendor + ":" + model + ")" 
                  : "") +
               "<br/><br/>";

      if (true) {
        pqxx::result iface_rows =
          t.prepared("select_device_ifaces")(device_name).exec();
        for (auto const& iface_row : iface_rows) {
          IP_Addr ip_addr;
          iface_row.at("ip_addr").to(ip_addr);
          string mac_addr;
          iface_row.at("mac_addr").to(mac_addr);
          string interface_name;
          iface_row.at("interface_name").to(interface_name);

          label += (ip_addr.to_string() + " " +
                    "[" + mac_addr + "]" + " "
                    "(" + interface_name + ")" + "<br align=\"left\"/>");

          pqxx::result hostname_rows =
            t.prepared("select_hostnames_by_ip_addr")(ip_addr).exec();
          for (auto const& hostname_row : hostname_rows) {
            string hostname;
            hostname_row.at("hostname").to(hostname);
            label += ("        " + hostname + "<br align=\"left\"/>");
          }
        }
      }
      label += "</font></td></TR></TABLE>> + \"";

      Vertex v = boost::add_vertex(graph);
      vertex_lookup[device_name] = v;

      graph[v].name  = device_name;
      graph[v].label = label;

      graph[v].shape = "box";
      graph[v].style = "filled";
      graph[v].fillcolor = device_color;
    }
  }
}

void
createVertexForUnassociated(pqxx::transaction<>& t, Network_Graph& graph,
                           map<string, Vertex>& vertex_lookup, bool graphic,
                           string type)
{
  if (true) {

    pqxx::result addr_rows =
      t.prepared("select_" + type + "s_without_devices").exec();
    for (auto const& addr_row : addr_rows) {
      IP_Addr ip_addr;
      addr_row.at("ip_addr").to(ip_addr);
      string mac_addr;
      addr_row.at("mac_addr").to(mac_addr);
      string vendor;
      addr_row.at("vendor_name").to(vendor);
      string addr = ("ip_addr" == type ? ip_addr.to_string() : mac_addr);

      string label = "\" + <<TABLE border=\"0\" cellborder=\"0\"><TR>";
      if (graphic) {
        label += get_graphic_string("unknown");
      }
      label += "<td><font point-size=\"10\">Unknown Device<br/><br/>" +
               ip_addr.to_string() + " " +
               ((!mac_addr.empty())
                ? "[" + mac_addr + "]<br align=\"left\"/>" +
                  "    {" + vendor + "}"
                : "") +
               "<br align=\"left\"/>";

      pqxx::result hostname_rows =
        t.prepared("select_hostnames_by_ip_addr")(ip_addr).exec();
      for (auto const& hostname_row : hostname_rows) {
        string hostname;
       hostname_row.at("hostname").to(hostname);
        label += ("        " + hostname + "<br align=\"left\"/>");
      }

      label += "</font></td></TR></TABLE>> + \"";

      Vertex v = boost::add_vertex(graph);
      vertex_lookup[addr] = v;

      graph[v].name  = addr;
      graph[v].label = label;

      graph[v].shape = "box";
      graph[v].style = "filled";
    }
  }
}

void
build_layer2_graph(pqxx::connection& db, Network_Graph& graph,
                   map<string, Vertex>& vertex_lookup, bool graphic)
{
  pqxx::transaction<> t{db};

  // Create a graph vertex (and label) for each device.
  createVertexForAssociated(t, graph, vertex_lookup, graphic);
  // Create a graph vertex (and label) for each MAC address that is not yet
  // associated with a device.
  createVertexForUnassociated(t, graph, vertex_lookup, graphic, "mac_addr");
  
  if (true) {
    // Create graph edges that connect two physical ports.

    pqxx::result phys_conn_rows =
      t.prepared("select_device_connections").exec();
    for (auto const& phys_conn_row : phys_conn_rows) {
      string self_device_name;
      phys_conn_row.at("self_device_id").to(self_device_name);
      string peer_device_name;
      phys_conn_row.at("peer_device_id").to(peer_device_name);

      Vertex const u = vertex_lookup.at(self_device_name);
      Vertex const v = vertex_lookup.at(peer_device_name);

      Edge e;
      bool inserted;

      // Add the edge in both "directions".
      // A later call to boost::remove_edge_if(...)
      // will remove the "wrong direction" edges.

      if (!(get<1>(boost::edge(u, v, graph)) &&
            get<1>(boost::edge(v, u, graph)))) {
        tie(e, inserted) = boost::add_edge(u, v, graph);
        graph[e].weight += graph[u].extra_weight;

        tie(e, inserted) = boost::add_edge(v, u, graph);
        graph[e].weight += graph[u].extra_weight;
      }
    }
  }

  t.commit();
}


void
build_layer3_graph(pqxx::connection& db, Network_Graph& graph,
                   map<string, Vertex>& vertex_lookup, bool graphic)
{
  pqxx::transaction<> t{db};

  // Create a graph vertex (and label) for each device.
  createVertexForAssociated(t, graph, vertex_lookup, graphic);
  // Create a graph vertex (and label) for each IP address that is not yet 
  // associated with a device.
  createVertexForUnassociated(t, graph, vertex_lookup, graphic, "ip_addr");

  if (true) {
    // Create a graph vertex for each IP network.

    pqxx::result ip_net_rows =
      t.prepared("select_ip_nets_extra_weights").exec();
    for (auto const& ip_net_row : ip_net_rows) {
      IP_Addr_with_Prefix ip_net;
      ip_net_row.at("ip_net").to(ip_net);
      double extra_weight;
      ip_net_row.at("extra_weight").to(extra_weight);

      string label = (ip_net.to_string() + "\\n");
      if (true) {
        pqxx::result rows =
          t.prepared("select_vlan_by_ip_net")(ip_net).exec();
        if (rows.size()) {
          label += "VLAN:";
          for (auto const& row : rows) {
            uint16_t vlan;
            row.at("vlan").to(vlan);
            label += (" " + to_string(static_cast<uint32_t>(vlan)));
          }
          label += "\\n";
        }
      }
      if (true) {
        pqxx::result rows =
          t.prepared("select_network_descriptions")(ip_net).exec();
        for (auto const& row : rows) {
          string description;
          row.at("description").to(description);
          label += (description + "\\n");
        }
      }

      Vertex u = boost::add_vertex(graph);
      vertex_lookup[ip_net.to_string()] = u;

      graph[u].name  = ip_net.to_string();
      graph[u].label = label;

      graph[u].shape = "oval";
      graph[u].extra_weight = extra_weight;

      if (true) {
        // Create graph edges that connect the IP network
        // to the devices and IP addresses in that network.

        pqxx::result ip_addr_rows =
          t.prepared("select_ip_addrs_and_devices_in_network")(ip_net).exec();
        for (auto const& ip_addr_row : ip_addr_rows) {
          IP_Addr ip_addr;
          ip_addr_row.at("ip_addr").to(ip_addr);
          string device_name;
          ip_addr_row.at("device_id").to(device_name);

          string const vertex_name = device_name.size()
                                   ? (device_name)
                                   : (ip_addr.to_string());

          Vertex const v = vertex_lookup.at(vertex_name);

          Edge e;
          bool inserted;

          // Add the edge in both "directions".
          // A later call to boost::remove_edge_if(...)
          // will remove the "wrong direction" edges.

          if (!(get<1>(boost::edge(u, v, graph)) &&
                get<1>(boost::edge(v, u, graph)))) {
            tie(e, inserted) = boost::add_edge(u, v, graph);
            graph[e].weight += graph[u].extra_weight;

            tie(e, inserted) = boost::add_edge(v, u, graph);
            graph[e].weight += graph[u].extra_weight;
          }
        }
      }

    }
  }

  t.commit();
}


void
build_virtualization_graph(pqxx::connection& db, Network_Graph& graph,
                           map<string, Vertex>& vertex_lookup)
{
  pqxx::transaction<> t{db};

  if (true) {
    // Create graph edges that connect VM host and guest devices.

    pqxx::result vm_rows =
      t.prepared("select_device_virtualizations").exec();
    for (auto const& vm_row : vm_rows) {
      string host_device_name;
      vm_row.at("host_device_id").to(host_device_name);
      string guest_device_name;
      vm_row.at("guest_device_id").to(guest_device_name);

      Vertex const u = vertex_lookup.at(guest_device_name);
      Vertex const v = vertex_lookup.at(host_device_name);

      Edge e;
      bool inserted;

      tie(e, inserted) = boost::add_edge(u, v, graph);
      graph[e].style = "dashed";
      graph[e].direction = "forward";
    }
  }

  t.commit();
}


int
main(int argc, char** argv)
{
  try {
    program_options::options_description general_opts_desc("Options");
    general_opts_desc.add_options()
      ("layer,L",
       program_options::value<int>()->required(),
       "Layer of the network stack to graph.")
      ("start-device-id",
       program_options::value<string>()->required(),
       "Device ID or subnet CIDR address to use as graph's root node.")
      ("db-name",
       program_options::value<string>()->required()->
       default_value(DEFAULT_DB_NAME),
       "Database to connect to.")
      ("no-icons,n",
        "Disable device icons in graph.")
      ("help,h",
       "Show this help message, then exit.")
      ("version,v",
       "Show version information, then exit.")
      ;

    // Command-line options (accepted on the command-line)
    program_options::options_description cl_opts_desc;
    cl_opts_desc.add(general_opts_desc);

    // Visible options (shown in help message)
    program_options::options_description visible_opts_desc;
    visible_opts_desc.add(general_opts_desc);

    // Parse command-line options
    program_options::variables_map opts;
    program_options::store
      (program_options::command_line_parser(argc, argv)
       .options(cl_opts_desc)
       .run(), opts);

    if (opts.count("help")) {
      cerr << "Output dot formatted graph of --layer information." << endl
           << endl
           << "Usage: " << PROGRAM_NAME << " [options]" << endl
           << visible_opts_desc << endl;
      return 0;
    }

    if (opts.count("version")) {
      cerr << "nmdb-graph-network (Netmeld)" << endl;
      return 0;
    }

    // Enforce required options and other sanity checks.
    program_options::notify(opts);


    string const db_name = opts.at("db-name").as<string>();

    pqxx::connection db{string("dbname=") + db_name};
    db_prepare_common(db);

    db.prepare
      ("select_ip_nets_extra_weights",
       "SELECT DISTINCT"
       "   n.ip_net AS ip_net,"
       "   COALESCE(w.extra_weight, 0.0) AS extra_weight"
       " FROM ip_nets AS n"
       " LEFT OUTER JOIN ip_nets_extra_weights AS w"
       "  ON (n.ip_net = w.ip_net)"
       " WHERE NOT (n.ip_net <<= '169.254.0.0/16') AND"
       "       NOT (n.ip_net <<= 'fe80::/10')"
       " ORDER BY ip_net");

    db.prepare
      ("select_network_descriptions",
       "SELECT DISTINCT description AS description"
       " FROM ip_nets"
       " WHERE ($1 = ip_net) AND (description IS NOT NULL)");

    db.prepare
      ("select_vlan_by_ip_net",
       "SELECT DISTINCT vlan AS vlan"
       " FROM vlans_ip_nets"
       " WHERE ($1 = ip_net)");

    db.prepare
      ("select_devices",
       "SELECT DISTINCT"
       "    d.device_id AS device_id," 
       "    dc.color AS device_color,"
       "    dt.type AS device_type,"
       "    dt.vendor AS vendor,"
       "    coalesce(dt.model, '?') AS model"
       " FROM devices AS d"
       " LEFT OUTER JOIN device_colors AS dc"
       "  ON (d.device_id = dc.device_id)"
       " LEFT OUTER JOIN device_types AS dt"
       "  ON (d.device_id = dt.device_id)"
       " ORDER BY d.device_id");

    db.prepare
      ("select_device_ifaces",
       "SELECT DISTINCT"
       "   dia.interface_name AS interface_name,"
       "   dia.ip_addr        AS ip_addr,"
       "   dmaip.mac_addr     AS mac_addr"
       " FROM device_ip_addrs AS dia"
       " LEFT OUTER JOIN device_mac_addrs_ip_addrs AS dmaip"
       "  ON (dia.device_id = dmaip.device_id) AND"
       "     (dia.interface_name = dmaip.interface_name)"
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

    Network_Graph graph;
    map<string, Vertex> vertex_lookup;

    switch (opts.at("layer").as<int>()) {
    case 2:
      build_layer2_graph(db, graph, vertex_lookup, !opts.count("no-icons"));
      break;
    case 3:
      build_layer3_graph(db, graph, vertex_lookup, !opts.count("no-icons"));
      break;
    default:
      break;
    }

    string const start_device_id = opts.at("start-device-id").as<string>();
    if (!vertex_lookup.count(start_device_id)) {
      throw std::runtime_error("Specified start-device-id does not exist.");
    }

    boost::dijkstra_shortest_paths
      (graph, vertex_lookup.at(start_device_id),
       weight_map(boost::get(&Edge_Properties::weight, graph)).
       distance_map(boost::get(&Vertex_Properties::distance, graph)));

    // Remove all the "wrong direction" and redundant edges.
    boost::remove_edge_if(Is_Redundant_Edge(graph), graph);

    build_virtualization_graph(db, graph, vertex_lookup);

    boost::write_graphviz
      (cout, graph,
       Label_Writer(graph),   // VertexPropertyWriter
       Label_Writer(graph),   // EdgePropertyWriter
       Graph_Writer(),        // GraphPropertyWriter
       boost::get(&Vertex_Properties::name, graph));  // VertexID
  }
  catch (std::exception& e) {
    cerr << "Error: " << e.what() << endl;
    return -1;
  }

  return 0;
}
