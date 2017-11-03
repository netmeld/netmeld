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

#include <netmeld/common/queries_common.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

#include <boost/format.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/program_options.hpp>

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
using std::make_pair;
using std::map;
using std::pair;
using std::string;
using std::to_string;
using std::vector;

using boost::format;
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

  double weight = 1.0;
};


using ACL_Graph =
boost::adjacency_list<
  boost::listS,         // OutEdgeList
  boost::vecS,          // VertexList
  boost::directedS,     // Directed/Undirected
  Vertex_Properties,    // VertexProperties
  Edge_Properties,      // EdgeProperties
  boost::no_property    // GraphProperties
  >;

using Vertex = ACL_Graph::vertex_descriptor;
using Edge   = ACL_Graph::edge_descriptor;


class Label_Writer
{
public:
  Label_Writer(ACL_Graph& g) : g_(g) { }

  void operator()(std::ostream& os, Vertex const& v) const {
    os << "["
       << "shape=\"" << g_[v].shape << "\""
       << ", "
       << "style=\"" << g_[v].style << "\""
       << ", "
       << "fillcolor=\"" << g_[v].fillcolor << "\""
       << ", "
       << "label=\"" << g_[v].label << "\""
       << "]";
  }

  void operator()(std::ostream& os, Edge const& e) const {
    Vertex const s = boost::source(e, g_);
    Vertex const t = boost::target(e, g_);

    double const distance_ratio = (g_[s].distance / g_[t].distance);
    string constraint;
    if ((0.999 < distance_ratio) && (distance_ratio < 1.001)) {
      constraint = "constraint=false, ";
    }

    os << "["
       << constraint
       << "label=\"" << g_[e].label << "\""
       << "]";
  }

private:
  ACL_Graph const& g_;
};


class Graph_Writer
{
public:
  void operator()(std::ostream& os) const {
    os << "rankdir=LR" << endl;
    os << "splines=true;" << endl;
    os << "nodesep=1.00;" << endl;
    os << "ranksep=2.50;" << endl;
    os << "overlap=false;" << endl;
  }
};


class Is_Redundant_Edge
{
public:
  Is_Redundant_Edge(ACL_Graph& g) : g_(g) { }

  bool operator()(Edge const& e) const {
    Vertex const s = boost::source(e, g_);
    Vertex const t = boost::target(e, g_);

    return (get<1>(boost::edge(t, s, g_)) &&
            ((g_[s].distance > g_[t].distance) ||
             ((g_[s].distance >= g_[t].distance) && (s > t))));
  }

private:
  ACL_Graph const& g_;
};


string
format_acl_actions
(pqxx::transaction<>& t,
 string const& device_id,
 string const& action_set);


string
format_acl_ports
(pqxx::transaction<>& t,
 string const& device_id,
 string const& port_range_set);


void
build_acl_graph(pqxx::connection& db, ACL_Graph& graph,
                map<string, Vertex>& vertex_lookup,
                string const& device_id);

void
build_acl_graph(pqxx::connection& db, ACL_Graph& graph,
                map<string, Vertex>& vertex_lookup,
                string const& device_id)
{
  pqxx::transaction<> t{db};

  if (true) {
    // Create a graph vertex (and label) for each set of IP addrs/nets.

    pqxx::result ip_net_set_rows =
      t.prepared("select_device_acl_ip_net_sets")
      (device_id)
      .exec();
    for (auto const& ip_net_set_row : ip_net_set_rows) {
      string ip_net_set;
      ip_net_set_row.at("ip_net_set").to(ip_net_set);

      string label = (ip_net_set + "\\n");
      if (true) {
        pqxx::result ip_net_rows =
          t.prepared("select_device_acl_ip_nets")
          (device_id)
          (ip_net_set)
          .exec();
        for (auto const& ip_net_row : ip_net_rows) {
          string ip_net;
          ip_net_row.at("ip_net").to(ip_net);

          label += (ip_net + "\\l");
        }
      }

      Vertex v = boost::add_vertex(graph);
      vertex_lookup[ip_net_set] = v;

      graph[v].name  = ip_net_set;
      graph[v].label = label;

      graph[v].shape = "box";
    }
  }

  if (true) {
    // Create graph edges for ACLs between sets of IP addrs/nets.

    pqxx::result acl_edge_rows =
      t.prepared("select_device_acl_edges")
      (device_id)
      .exec();
    for (auto const& acl_edge_row : acl_edge_rows) {
      string src_ip_net_set;
      acl_edge_row.at("src_ip_net_set").to(src_ip_net_set);
      string dst_ip_net_set;
      acl_edge_row.at("dst_ip_net_set").to(dst_ip_net_set);

      Vertex const u = vertex_lookup.at(src_ip_net_set);
      Vertex const v = vertex_lookup.at(dst_ip_net_set);

      Edge e;
      bool inserted;
      tie(e, inserted) = boost::add_edge(u, v, graph);

      pqxx::result acl_rows =
        t.prepared("select_device_acls")
        (device_id)
        (src_ip_net_set)
        (dst_ip_net_set)
        .exec();
      for (auto const& acl_row : acl_rows) {
        string acl_set;
        acl_row.at("acl_set").to(acl_set);
        string action_set;
        acl_row.at("action_set").to(action_set);
        string src_port_range_set;
        acl_row.at("src_port_range_set").to(src_port_range_set);
        string dst_port_range_set;
        acl_row.at("dst_port_range_set").to(dst_port_range_set);

        graph[e].label +=
          ((format("%1%: %2% -> %3%\\l")
            % (format_acl_actions(t, device_id, action_set))
            % (format_acl_ports(t, device_id, src_port_range_set))
            % (format_acl_ports(t, device_id, dst_port_range_set))
            ).str());
      }
    }
  }

  t.commit();
}


string
format_acl_actions
(pqxx::transaction<>& t,
 string const& device_id,
 string const& action_set)
{
  string result;

  pqxx::result action_rows =
    t.prepared("select_device_acl_actions")
    (device_id)
    (action_set)
    .exec();
  for (auto const& action_row : action_rows) {
    string action;
    action_row.at("action").to(action);

    result += (action + ",");
  }

  if (1 < result.size()) {
    result.pop_back();  // remove trailing ","
  }

  return result;
}


string
format_acl_ports
(pqxx::transaction<>& t,
 string const& device_id,
 string const& port_range_set)
{
  string result;

  map<string, vector<pair<uint16_t, uint16_t> > > ports;

  pqxx::result port_range_rows =
    t.prepared("select_device_acl_port_ranges")
    (device_id)
    (port_range_set)
    .exec();
  for (auto const& port_range_row : port_range_rows) {
    string protocol;
    port_range_row.at("protocol").to(protocol);
    uint16_t port_range_min;
    port_range_row.at("port_range_min").to(port_range_min);
    uint16_t port_range_max;
    port_range_row.at("port_range_max").to(port_range_max);

    ports[protocol].push_back(make_pair(port_range_min, port_range_max));
  }

  for (auto const& protocol : ports) {
    result += get<0>(protocol);
    result += ":";
    for (auto const& port_range : get<1>(protocol)) {
      uint16_t const port_range_min = get<0>(port_range);
      uint16_t const port_range_max = get<1>(port_range);

      if (port_range_min == port_range_max) {
        result += to_string(static_cast<uint32_t>(port_range_min));
      }
      else if ((port_range_min <= 1) && (port_range_max == 65535)) {
        result += "any";
      }
      else {
        result += to_string(static_cast<uint32_t>(port_range_min));
        result += "-";
        result += to_string(static_cast<uint32_t>(port_range_max));
      }
      result += ",";
    }
  }

  if (1 < result.size()) {
    result.pop_back();  // remove trailing ","
  }

  return result;
}


int
main(int argc, char** argv)
{
  try {
    program_options::options_description general_opts_desc("Options");
    general_opts_desc.add_options()
      ("device-id",
       program_options::value<string>()->required(),
       "Name of device.")
      ("db-name",
       program_options::value<string>()->required()->
       default_value(DEFAULT_DB_NAME),
       "Database to connect to.")
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
      cerr << "Create dot formatted graph of access control lists "
           << "between ips and networks." << endl
           << endl
           << "Usage: " << PROGRAM_NAME << " [options]" << endl
           << visible_opts_desc << endl;
      return 0;
    }

    if (opts.count("version")) {
      cerr << "nmdb-graph-acls (Netmeld)" << endl;
      return 0;
    }

    // Enforce required options and other sanity checks.
    program_options::notify(opts);


    string const db_name = opts.at("db-name").as<string>();

    pqxx::connection db{string("dbname=") + db_name};
    db_prepare_common(db);

    db.prepare
      ("select_device_acl_ip_net_sets",
       "SELECT *"
       " FROM device_acl_ip_net_sets"
       " WHERE ($1 = device_id)");

    db.prepare
      ("select_device_acl_ip_nets",
       "SELECT *"
       " FROM device_acl_ip_nets"
       " WHERE ($1 = device_id) AND"
       "       ($2 = ip_net_set)"
       " ORDER BY ip_net");

    db.prepare
      ("select_device_acl_edges",
       "SELECT DISTINCT"
       "   src_ip_net_set, dst_ip_net_set"
       " FROM device_acls"
       " WHERE ($1 = device_id)");

    db.prepare
      ("select_device_acls",
       "SELECT *"
       " FROM device_acls"
       " WHERE ($1 = device_id) AND"
       "       ($2 = src_ip_net_set) AND"
       "       ($3 = dst_ip_net_set)"
       " ORDER BY acl_set, acl_number");

    db.prepare
      ("select_device_acl_actions",
       "SELECT *"
       " FROM device_acl_actions"
       " WHERE ($1 = device_id) AND"
       "       ($2 = action_set)"
       " ORDER BY action");

    db.prepare
      ("select_device_acl_port_ranges",
       "SELECT DISTINCT"
       "   protocol,"
       "   lower(port_range)    AS port_range_min,"
       "   upper(port_range)-1  AS port_range_max"
       " FROM device_acl_port_ranges"
       " WHERE ($1 = device_id) AND"
       "       ($2 = port_range_set)"
       " ORDER BY protocol, port_range_min, port_range_max");


    ACL_Graph graph;
    map<string, Vertex> vertex_lookup;

    string const device_id = opts.at("device-id").as<string>();

    build_acl_graph(db, graph, vertex_lookup, device_id);

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
