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
#include <boost/lexical_cast.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/program_options.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <cstdint>
#include <fstream>
#include <iostream>
#include <map>
#include <string>


using std::cin;
using std::cout;
using std::cerr;
using std::endl;

using std::string;
using std::vector;

using boost::lexical_cast;
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


string const
extract_host_mac_addr(xml_node const& node_host);

IP_Addr const
extract_host_ip_addr(xml_node const& node_host);

bool
extract_host_is_responding(xml_node const& node_host);


string const
extract_host_mac_addr(xml_node const& node_host)
{
  xml_node const node_mac_addr =
    node_host
    .select_single_node("address[@addrtype='mac']")
    .node();

  string const mac_addr =
    node_mac_addr.attribute("addr").as_string();

  return mac_addr;
}


IP_Addr const
extract_host_ip_addr(xml_node const& node_host)
{
  xml_node const node_ip_addr =
    node_host
    .select_single_node("address[@addrtype='ipv4' or @addrtype='ipv6']")
    .node();

  return IP_Addr::from_string(node_ip_addr.attribute("addr").as_string());
}


bool
extract_host_is_responding(xml_node const& node_host)
{
  bool is_responding = false;

  xml_node const node_status = node_host.child("status");

  if (string("up") == node_status.attribute("state").as_string()) {
    if (string("user-set") !=
        node_status.attribute("reason").as_string()) {
      // Any "up" reason other than "user-set" indicates
      // that the host (or something else) responded.
      is_responding = true;
    }
    else {  // "user-set" hosts:
      if (0 < (node_host
               .select_nodes("ports/port/state"
                             "[@state='open' or @state='closed']")
               .size())) {
        // The presence of "open" or "closed" ports indicates
        // that the host (or something else) responded.
        is_responding = true;
      }
    }
  }

  return is_responding;
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
      cerr << "Import Nmap's XML output (.xml files)." << endl
           << endl
           << "Usage: " << PROGRAM_NAME << " [options] {input-file}" << endl
           << visible_opts_desc << endl;
      return 0;
    }

    if (opts.count("version")) {
      cerr << "nmdb-import-nmap (Netmeld)" << endl;
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
      throw std::runtime_error("Could not open nmap XML file: " +
                               input_file_path.string());
    }

    xml_node node_nmaprun = doc.select_single_node("/nmaprun").node();
    if (!node_nmaprun) {
      throw std::runtime_error("Could not find XML element: /nmaprun");
    }

    pqxx::transaction<> t{db};


    // ----------------------------------------------------------------------
    // TABLE: tool_runs
    // ----------------------------------------------------------------------

    if (!opts.count("tool-run-id")) {
      string const command_line =
        node_nmaprun.attribute("args").as_string();

      posix_time::ptime const execute_time_lower =
        posix_time::from_time_t
        (lexical_cast<time_t>
         (node_nmaprun.attribute("start").as_string("0")));

      posix_time::ptime const execute_time_upper =
        posix_time::from_time_t
        (lexical_cast<time_t>
         (node_nmaprun.select_single_node("runstats/finished")
          .node().attribute("time").as_string("0")));

      t.prepared("insert_tool_run")
        (tool_run_id)
        ("nmap")
        (command_line)
        (input_file_path.string())
        (execute_time_lower)
        (execute_time_upper)
        .exec();
    }


    // ----------------------------------------------------------------------
    // TABLE: mac_addrs, ip_addrs, and mac_addrs_ip_addrs
    // ----------------------------------------------------------------------

    // This code block ensures that all of the ip_addrs and mac_addrs
    // from the scan are present for the other table's foreign keys.

    if (true) {
      for (auto const& x_host : node_nmaprun.select_nodes("host")) {
        xml_node node_host = x_host.node();

        bool const is_responding = extract_host_is_responding(node_host);

        string const mac_addr = extract_host_mac_addr(node_host);
        if (!mac_addr.empty()) {
          t.prepared("insert_raw_mac_addr")
            (tool_run_id)
            (mac_addr)
            (is_responding)
            .exec();
        }

        IP_Addr const ip_addr = extract_host_ip_addr(node_host);
        if (!ip_addr.is_unspecified()) {
          t.prepared("insert_raw_ip_addr")
            (tool_run_id)
            (ip_addr)
            (is_responding)
            .exec();
        }

        if (!mac_addr.empty() && !ip_addr.is_unspecified()) {
          t.prepared("insert_raw_mac_addr_ip_addr")
            (tool_run_id)
            (mac_addr)
            (ip_addr)
            .exec();
        }
      }
    }


    // ----------------------------------------------------------------------
    // TABLE: ip_addrs, ip_traceroutes
    // ----------------------------------------------------------------------

    // This code block identifies ip_addrs of routers along a route.
    // The routers may or may not be in the target address space,
    // so might need to be inserted into the ip_addrs table.

    if (true) {
      for (auto const& x_hop :
             node_nmaprun.select_nodes("host/trace/hop")) {
        xml_node node_hop = x_hop.node();

        bool const is_rtr_responding = true;
        size_t const hop_count = node_hop.attribute("ttl").as_uint();
        IP_Addr const rtr_ip_addr =
          IP_Addr::from_string(node_hop.attribute("ipaddr").as_string());
        IP_Addr const dst_ip_addr =
          extract_host_ip_addr(node_hop.parent().parent());

        if (!rtr_ip_addr.is_unspecified() && !dst_ip_addr.is_unspecified()) {
          t.prepared("insert_raw_ip_addr")
            (tool_run_id)
            (rtr_ip_addr)
            (is_rtr_responding)
            .exec();

          // The dst_ip_addr was already inserted by the previous block
          // that is handling "host" nodes.

          t.prepared("insert_raw_ip_traceroute")
            (tool_run_id)
            (hop_count)
            (rtr_ip_addr)
            (dst_ip_addr)
            .exec();
        }
      }
    }


    // ----------------------------------------------------------------------
    // TABLE: hostnames
    // ----------------------------------------------------------------------

    if (true) {
      for (auto const& x_hostname :
             node_nmaprun.select_nodes("host/hostnames/hostname")) {
        xml_node node_hostname = x_hostname.node();
        xml_node node_host = node_hostname.parent().parent();

        IP_Addr const ip_addr = extract_host_ip_addr(node_host);

        string const hostname =
          node_hostname.attribute("name").as_string();

        string const reason =
          node_hostname.attribute("type").as_string();

        t.prepared("insert_raw_hostname")
          (tool_run_id)
          (ip_addr)
          (hostname)
          (reason)
          .exec();
      }
    }


    // ----------------------------------------------------------------------
    // TABLE: operating_systems
    // ----------------------------------------------------------------------

    if (true) {
      for (auto const& x_osclass :
             node_nmaprun.select_nodes("host/os/osmatch/osclass")) {
        xml_node node_osclass = x_osclass.node();
        xml_node node_host = node_osclass.parent().parent().parent();

        IP_Addr const ip_addr = extract_host_ip_addr(node_host);

        string const vendor_name =
          node_osclass.attribute("vendor").as_string();

        string const product_name =
          node_osclass.attribute("osfamily").as_string();

        string const product_version =
          node_osclass.attribute("osgen").as_string();

        double const accuracy =
          node_osclass.attribute("accuracy").as_double() / 100.0;

        string const cpe =
          node_osclass.select_single_node("cpe").node().text().as_string();

        t.prepared("insert_raw_operating_system")
          (tool_run_id)
          (ip_addr)
          (vendor_name)
          (product_name)
          (product_version)
          (cpe)
          (accuracy)
          .exec();
      }
    }


    // ----------------------------------------------------------------------
    // TABLE: ports and services
    // ----------------------------------------------------------------------

    if (true) {
      for (auto const& x_extrareasons :
             node_nmaprun.select_nodes("host/ports/extraports/extrareasons")) {
        xml_node node_extrareasons = x_extrareasons.node();
        xml_node node_extraports = node_extrareasons.parent();
        xml_node node_host = node_extraports.parent().parent();

        IP_Addr const ip_addr = extract_host_ip_addr(node_host);

        int const port = -1;

        string const port_state =
          node_extraports.attribute("state").as_string();

        string const port_reason =
          node_extrareasons.attribute("reason").as_string();

        string protocol;
        if (1 == node_nmaprun.select_nodes("scaninfo").size()) {
          // If there is only a single scaninfo element,
          // all extraports protocols must be the scaninfo's protocol.
          protocol =
            node_nmaprun.select_single_node("scaninfo")
            .node().attribute("protocol").as_string();
        }
        else if ((port_reason == "tcp-response") ||
                 (port_reason == "tcp-responses") ||
                 (port_reason == "syn-ack") ||
                 (port_reason == "syn-acks") ||
                 (port_reason == "reset") ||
                 (port_reason == "resets")) {
          // If there are multiple scaninfo elements
          // (meaning the scan was a multi-protocol scan),
          // certain port_reason values indicate or imply TCP.
          protocol = "tcp";
        }
        else if ((port_reason == "udp-response") ||
                 (port_reason == "udp-responses") ||
                 (port_reason == "port-unreach") ||
                 (port_reason == "port-unreaches")) {
          // If there are multiple scaninfo elements
          // (meaning the scan was a multi-protocol scan),
          // certain port_reason values indicate or imply UDP.
          protocol = "udp";
        }

        t.prepared("insert_raw_port")
          (tool_run_id)
          (ip_addr)
          (protocol)
          (port)
          (port_state)
          (port_reason)
          .exec();
      }


      for (auto const& x_port :
             node_nmaprun.select_nodes("host/ports/port")) {
        xml_node node_port = x_port.node();
        xml_node node_port_state = node_port.child("state");
        xml_node node_host = node_port.parent().parent();

        IP_Addr const ip_addr = extract_host_ip_addr(node_host);

        string const protocol =
          node_port.attribute("protocol").as_string();

        int const port =
          node_port.attribute("portid").as_int();

        string const port_state =
          node_port_state.attribute("state").as_string();

        string const port_reason =
          node_port_state.attribute("reason").as_string();

        t.prepared("insert_raw_port")
          (tool_run_id)
          (ip_addr)
          (protocol)
          (port)
          (port_state)
          (port_reason)
          .exec();

        xml_node node_service = node_port.child("service");
        if (node_service) {
          string const service_name =
            node_service.attribute("name").as_string();

          string const service_description =
            node_service.attribute("product").as_string();

          string const service_reason =
            node_service.attribute("method").as_string();

          t.prepared("insert_raw_service")
            (tool_run_id)
            (ip_addr)
            (protocol)
            (port)
            (service_name)
            (service_description)
            (service_reason)
            .exec();
        }
      }
    }

    // ----------------------------------------------------------------------
    // TABLE: nse_results and other more specific tables
    // ----------------------------------------------------------------------

    if (true) {
      for (auto const& x_script :
             node_nmaprun.select_nodes("host/ports/port/script")) {
        xml_node node_script = x_script.node();
        xml_node node_port = node_script.parent();
        xml_node node_host = node_port.parent().parent();

        IP_Addr const ip_addr = extract_host_ip_addr(node_host);

        string const protocol =
          node_port.attribute("protocol").as_string();

        int const port =
          node_port.attribute("portid").as_int();

        string const script_id =
          node_script.attribute("id").as_string();

        string const script_output =
          node_script.attribute("output").as_string();

        t.prepared("insert_raw_nse_result")
          (tool_run_id)
          (ip_addr)
          (protocol)
          (port)
          (script_id)
          (script_output)
          .exec();

        if (false) {

        }
        else if (script_id == "ssh-hostkey") {
          for (auto const& x_table :
                 node_script.select_nodes("table")) {
            xml_node node_table = x_table.node();

            string const ssh_key_type =
              node_table.select_single_node("elem[@key='type']")
              .node().text().as_string();

            int const ssh_key_bits =
              node_table.select_single_node("elem[@key='bits']")
              .node().text().as_int();

            string const ssh_key_fingerprint =
              node_table.select_single_node("elem[@key='fingerprint']")
              .node().text().as_string();

            string const ssh_key_public =
              node_table.select_single_node("elem[@key='key']")
              .node().text().as_string();

            t.prepared("insert_raw_ssh_host_public_key")
              (tool_run_id)
              (ip_addr)
              (protocol)
              (port)
              (ssh_key_type)
              (ssh_key_bits)
              (ssh_key_fingerprint)
              (ssh_key_public)
              .exec();
          }
        }
        else if (script_id == "ssh2-enum-algos") {
          for (auto const& x_table :
                 node_script.select_nodes("table")) {
            xml_node node_table = x_table.node();

            string const ssh_algo_type =
              node_table.attribute("key").as_string();

            for (auto const& x_elem :
                   node_table.select_nodes("elem")) {
              xml_node node_elem = x_elem.node();

              string const ssh_algo_name =
                node_elem.text().as_string();

              t.prepared("insert_raw_ssh_host_algorithm")
                (tool_run_id)
                (ip_addr)
                (protocol)
                (port)
                (ssh_algo_type)
                (ssh_algo_name)
                .exec();
            }
          }
        }

      }
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
