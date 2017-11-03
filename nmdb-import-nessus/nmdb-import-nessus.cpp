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

#ifndef BOOST_SPIRIT_USE_PHOENIX_V3
#define BOOST_SPIRIT_USE_PHOENIX_V3
#endif

#include <netmeld/common/cve.hpp>
#include <netmeld/common/parser_networking.hpp>
#include <netmeld/common/piped_input.hpp>
#include <netmeld/common/queries_common.hpp>

#include <pugixml.hpp>
#include <pqxx/pqxx>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/adapted.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/program_options.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <cstdint>
#include <fstream>
#include <iostream>
#include <locale>
#include <map>
#include <sstream>
#include <string>


using std::cin;
using std::cout;
using std::cerr;
using std::endl;

using std::string;
using std::stringstream;
using std::vector;

using boost::numeric_cast;
using boost::uuids::uuid;

namespace filesystem = boost::filesystem;
namespace posix_time = boost::posix_time;
namespace program_options = boost::program_options;

using pugi::xml_document;
using pugi::xml_node;
using pugi::xml_attribute;
using pugi::xpath_node;


boost::uuids::random_generator uuid_generator;


void
parse_ReportHost
(pqxx::transaction<>& t,
 uuid const& tool_run_id,
 xml_node const& node_ReportHost);


void
parse_ReportItem
(pqxx::transaction<>& t,
 uuid const& tool_run_id,
 IP_Addr const& ip_addr,
 xml_node const& node_ReportHost);



void
parse_ReportHost
(pqxx::transaction<>& t,
 uuid const& tool_run_id,
 xml_node const& node_ReportHost)
{
  bool const is_responding = true;

  // Extract host's FQDN (hostname).
  string hostname;
  for (auto const& x_tag :
         node_ReportHost.select_nodes
         ("HostProperties/tag[@name='host-fqdn']")) {
    xml_node node_tag = x_tag.node();

    hostname = node_tag.text().as_string();
  }

  // Extract host's MAC address.
  string mac_addrs;
  for (auto const& x_tag :
         node_ReportHost.select_nodes
         ("HostProperties/tag[@name='mac-address']")) {
    xml_node node_tag = x_tag.node();

    mac_addrs = node_tag.text().as_string();

    if (!mac_addrs.empty()) {
      // Nessus puts multiple MACs under one tag, so split and iterate
      boost::char_separator<char> sep("\n");
      boost::tokenizer<boost::char_separator<char>> tokens
        (mac_addrs, sep);

      for (auto it = tokens.begin(); it != tokens.end(); it++) {
        auto mac_addr = (*it);

        t.prepared("insert_raw_mac_addr")
          (tool_run_id)
          (mac_addr)
          (is_responding)
          .exec();
      }
    }
  }

  // Extract host's IP address.
  IP_Addr ip_addr;
  for (auto const& x_tag :
         node_ReportHost.select_nodes
         ("HostProperties/tag[@name='host-ip' or @name='container-host']")) {
    xml_node node_tag = x_tag.node();

    string ip_addr_string = node_tag.text().as_string();
    if (ip_addr_string.empty()) {
      continue;
    }
    ip_addr = IP_Addr::from_string(ip_addr_string);

    if (!ip_addr.is_unspecified()) {
      t.prepared("insert_raw_ip_addr")
        (tool_run_id)
        (ip_addr)
        (is_responding)
        .exec();

      if (!hostname.empty() && (hostname != ip_addr.to_string())) {
        string const reason = "nessus scan";

        t.prepared("insert_raw_hostname")
          (tool_run_id)
          (ip_addr)
          (hostname)
          (reason)
          .exec();
      }
    }
  }

  // Extract host's operating system.
  for (auto const& x_tag :
         node_ReportHost.select_nodes
         ("HostProperties/tag[starts-with(@name, 'cpe')]")) {
    xml_node node_tag = x_tag.node();

    string const cpe = node_tag.text().as_string();
    double const accuracy = 1.0;

    if (0 == cpe.find("cpe:/o:")) {
      t.prepared("insert_raw_operating_system")
        (tool_run_id)
        (ip_addr)
        ()
        ()
        ()
        (cpe)
        (accuracy)
        .exec();
    }
  }

  // Extract host's ReportItem elements.
  for (auto const& x_ReportItem :
         node_ReportHost.select_nodes("ReportItem")) {
    xml_node node_ReportItem = x_ReportItem.node();
    parse_ReportItem(t, tool_run_id, ip_addr, node_ReportItem);
  }
}


void
parse_ReportItem
(pqxx::transaction<>& t,
 uuid const& tool_run_id,
 IP_Addr const& ip_addr,
 xml_node const& node_ReportItem)
{
  string const protocol =
    node_ReportItem.attribute("protocol").as_string();
  uint16_t const port =
    numeric_cast<uint16_t>
    (node_ReportItem.attribute("port").as_uint());

  // Nessus uses (0 == port) for general info.
  string const port_state = (port ? "open" : "none");
  string const port_reason = "nessus scan";

  string const service_name =
    node_ReportItem.attribute("svc_name").as_string();
  uint16_t const severity =
    numeric_cast<uint16_t>
    (node_ReportItem.attribute("severity").as_uint());
  uint32_t const plugin_id =
    numeric_cast<uint32_t>
    (node_ReportItem.attribute("pluginID").as_uint());
  string const plugin_name =
    node_ReportItem.attribute("pluginName").as_string();
  string const plugin_family =
    node_ReportItem.attribute("pluginFamily").as_string();

  string const plugin_type =
    node_ReportItem.select_single_node("plugin_type")
    .node().text().as_string();
  string const plugin_output =
    node_ReportItem.select_single_node("plugin_output")
    .node().text().as_string();
  string const description =
    node_ReportItem.select_single_node("description")
    .node().text().as_string();
  string const solution =
    node_ReportItem.select_single_node("solution")
    .node().text().as_string();

  t.prepared("insert_raw_port")
    (tool_run_id)
    (ip_addr)
    (protocol)
    (port)
    (port_state)
    (port_reason)
    .exec();

  t.prepared("insert_raw_nessus_result")
    (tool_run_id)
    (ip_addr)
    (protocol)
    (port)
    (plugin_id)
    (plugin_name)
    (plugin_family)
    (plugin_type)
    (plugin_output)
    (severity)
    (description)
    (solution)
    .exec();

  // Extract CVE identifiers for this ReportItem.
  for (auto const& x_cve :
         node_ReportItem.select_nodes("cve")) {
    xml_node node_cve = x_cve.node();

    CVE const cve_id{node_cve.text().as_string()};

    t.prepared("insert_raw_nessus_result_cve")
      (tool_run_id)
      (ip_addr)
      (protocol)
      (port)
      (plugin_id)
      (cve_id)
      .exec();
  }

  // Extract Metasploit modules that apply to this ReportItem.
  for (auto const& x_msf_name :
         node_ReportItem.select_nodes("metasploit_name")) {
    xml_node node_msf_name = x_msf_name.node();

    string const msf_name = node_msf_name.text().as_string();

    t.prepared("insert_raw_nessus_result_metasploit_module")
      (tool_run_id)
      (ip_addr)
      (protocol)
      (port)
      (plugin_id)
      (msf_name)
      .exec();
  }
}



int
main(int argc, char** argv)
{
  namespace qi = boost::spirit::qi;

  try {
    program_options::options_description general_opts_desc("Options");
    general_opts_desc.add_options()
      ("db-name",
       program_options::value<string>()->required()->
       default_value(DEFAULT_DB_NAME),
       "Database to connect to.")
      ("tool-run-id",
       program_options::value<string>(),
       "UUID for this run of the tool.")
      ("help,h",
       "Show this help message, then exit.")
      ("version,v",
       "Show version information, then exit.")
      ("pipe",
       "Read input from STDIN; Save a copy to {input-file}.")
      ;

    program_options::options_description hidden_opts_desc("Hidden options");
    hidden_opts_desc.add_options()
      ("input-file",
       program_options::value<string>()->required(),
       "Input data file to parse.")
      ;
    program_options::positional_options_description position_opts_desc;
    position_opts_desc.add("input-file", -1);

    // Command-line options:
    program_options::options_description cl_opts_desc;
    cl_opts_desc.add(general_opts_desc).add(hidden_opts_desc);

    // Visible options:
    program_options::options_description visible_opts_desc;
    visible_opts_desc.add(general_opts_desc);


    program_options::variables_map opts;
    program_options::store
      (program_options::command_line_parser(argc, argv)
       .options(cl_opts_desc)
       .positional(position_opts_desc)
       .run(), opts);

    if (opts.count("help")) {
      cerr << "Import Nessus' XML output (.nessus file)." << endl
           << endl
           << "Usage: " << PROGRAM_NAME << " [options] {input-file}" << endl
           << visible_opts_desc << endl;
      return 0;
    }

    if (opts.count("version")) {
      cerr << "nmdb-import-nessus (Netmeld)" << endl;
      return 0;
    }

    program_options::notify(opts);


    filesystem::path const ifp = opts.at("input-file").as<string>();
    if (opts.count("pipe")) {
      piped_input_file(ifp);
    }
    if (!filesystem::exists(ifp)) {
      throw std::runtime_error("Specified input-file does not exist: " +
                               ifp.string());
    }
    filesystem::path const input_file_path = filesystem::canonical(ifp);

    uuid const tool_run_id =
      opts.count("tool-run-id")
      ? (boost::uuids::string_generator()(opts.at("tool-run-id").as<string>()))
      : (uuid_generator());

    string const db_name = opts.at("db-name").as<string>();
    pqxx::connection db{string("dbname=") + db_name};
    db_prepare_common(db);

    xml_document doc;
    if (!doc.load_file(input_file_path.string().c_str())) {
      throw std::runtime_error("Could not open nessus XML file: " +
                               input_file_path.string());
    }

    xml_node node_Report =
      doc.select_single_node("/NessusClientData_v2/Report").node();
    if (!node_Report) {
      throw std::runtime_error
        ("Could not find XML element: /NessusClientData_v2/Report");
    }

    pqxx::transaction<> t{db};


    // ----------------------------------------------------------------------
    // TABLE: tool_runs
    // ----------------------------------------------------------------------

    // <tag name="HOST_START"/> and <tag name="HOST_END"/>

    if (!opts.count("tool-run-id")) {
      string const command_line; // Empty, as unknown source 

      posix_time::time_input_facet* facet =
        new posix_time::time_input_facet("%a %b %d %H:%M:%S %Y");

      stringstream ss;
      ss.imbue(std::locale(std::locale(), facet));

      // Use the earliest HOST_START time as the tool-run start time.
      posix_time::ptime execute_time_lower;
      for (auto const& x_host_time_lower :
             node_Report.select_nodes
             ("ReportHost/HostProperties/tag[@name='HOST_START']")) {
        xml_node node_host_time_lower = x_host_time_lower.node();

        ss.str(node_host_time_lower.text().as_string());
        posix_time::ptime host_lower;
        ss >> host_lower;

        if ((execute_time_lower.is_not_a_date_time()) ||
            (host_lower < execute_time_lower)) {
          execute_time_lower = host_lower;
        }
      }

      // Use the latest HOST_END time as the tool-run end time.
      posix_time::ptime execute_time_upper;
      for (auto const& x_host_time_upper :
             node_Report.select_nodes
             ("ReportHost/HostProperties/tag[@name='HOST_END']")) {
        xml_node node_host_time_upper = x_host_time_upper.node();

        ss.str(node_host_time_upper.text().as_string());
        posix_time::ptime host_upper;
        ss >> host_upper;

        if ((execute_time_upper.is_not_a_date_time()) ||
            (execute_time_upper < host_upper)) {
          execute_time_upper = host_upper;
        }
      }

      t.prepared("insert_tool_run")
        (tool_run_id)
        ("nessus")
        (command_line)
        (input_file_path.string())
        (execute_time_lower)
        (execute_time_upper)
        .exec();
    }


    // ----------------------------------------------------------------------
    // Parse the ReportHost element and sub-elements.
    // ----------------------------------------------------------------------

    for (auto const& x_ReportHost :
           node_Report.select_nodes("ReportHost")) {
      xml_node node_ReportHost = x_ReportHost.node();
      parse_ReportHost(t, tool_run_id, node_ReportHost);
    }


    // ----------------------------------------------------------------------

    t.commit();

    if (!opts.count("tool-run-id")) {
      cerr << "tool_run_id: " << tool_run_id << endl;
    }
  }
  catch (std::exception& e) {
    cerr << "Error: " << e.what() << endl;
    return -1;
  }

  return 0;
}
