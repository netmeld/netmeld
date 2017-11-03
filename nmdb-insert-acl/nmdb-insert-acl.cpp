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

#include <boost/algorithm/string/join.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/program_options.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <cstdint>
#include <iostream>
#include <string>


using std::cin;
using std::cout;
using std::cerr;
using std::endl;

using std::string;

using boost::numeric_cast;
using boost::uuids::uuid;

namespace program_options = boost::program_options;

boost::uuids::random_generator uuid_generator;


int
main(int argc, char** argv)
{
  try {
    program_options::options_description general_opts_desc("Options");
    general_opts_desc.add_options()
      ("device-id",
       program_options::value<string>()->required(),
       "Name of device.")

      ("acl-set",
       program_options::value<string>(),
       "Access control list set to insert into (e.g. from-zone any to-zone any).")
      ("acl-number",
       program_options::value<string>(),
       "Access control list in --acl-set.")

      ("action-set",
       program_options::value<string>(),
       "Action set to insert into (ACCEPT, DROP, REJECT, LOG, etc).")
      ("action",
       program_options::value<string>(),
       "Action to add to --action-set.")

      ("ip-net-set",
       program_options::value<string>(),
       "IP or network set to insert into (ADMIN, MAINT, NETS, etc).")
      ("ip-net",
       program_options::value<string>(),
       "IP or network to add into --ip-net-set.")

      ("port-range-set",
       program_options::value<string>(),
       "Port range set to insert into (any, standard_ssh, stadard_web, etc.)")
      ("protocol",
       program_options::value<string>(),
       "Transport layer protocol for --port-range (tcp, udp, etc.)")
      ("port-range",
       program_options::value<string>(),
       "Port(s) to add to --port-range-set and --protocol (e.g. [22,22])")

      ("src-ip-net-set",
       program_options::value<string>(),
       "Source IP or network.")
      ("dst-ip-net-set",
       program_options::value<string>(),
       "Destination IP or network")
      ("src-port-range-set",
       program_options::value<string>(),
       "Source port range.")
      ("dst-port-range-set",
       program_options::value<string>(),
       "Destination port range.")

      ("db-name",
       program_options::value<string>()->required()->
       default_value(DEFAULT_DB_NAME),
       "Database to connect to.")
      ("tool-run-id",
       program_options::value<string>(),
//       ->required()->default_value("32b2fd62-08ff-4d44-8da7-6fbd581a90c6"),
       "UUID for this run of the tool.")
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
      cerr << "Add manually specified ACL." << endl
           << endl
           << "Usage: " << PROGRAM_NAME << " [options]" << endl
           << visible_opts_desc << endl;
      return 0;
    }

    if (opts.count("version")) {
      cerr << "nmdb-insert-acl (Netmeld)" << endl;
      return 0;
    }

    // Enforce required options and other sanity checks.
    program_options::notify(opts);


    uuid const tool_run_id =
      opts.count("tool-run-id")
      ? (boost::uuids::string_generator()(opts.at("tool-run-id").as<string>()))
      : (uuid_generator());

    std::vector<std::string> args(argv, argv+argc);
    string const command_line = boost::algorithm::join(args, " ");

    string const device_id = opts.at("device-id").as<string>();

    string const db_name = opts.at("db-name").as<string>();
    pqxx::connection db{string("dbname=") + db_name};
    db_prepare_common(db);


    if (true) {
      pqxx::transaction<> t{db};

      if (true) {
        t.prepared("insert_tool_run")
          (tool_run_id)
          ("human")
          (command_line)
          ("human")
          ()
          ()
          .exec();
      }

      if (true) {
        t.prepared("insert_raw_device")
          (tool_run_id)
          (device_id)
          .exec();
      }

      // Sets of actions (ACCEPT, DROP, REJECT, LOG, etc).
      if (opts.count("action-set")) {
        string const action_set =
          opts.at("action-set").as<string>();

        t.prepared("insert_raw_device_acl_action_set")
          (tool_run_id)
          (device_id)
          (action_set)
          .exec();

        if (opts.count("action")) {
          string const action =
            opts.at("action").as<string>();

          t.prepared("insert_raw_device_acl_action")
            (tool_run_id)
            (device_id)
            (action_set)
            (action)
            .exec();
        }
      }

      // Sets of IP addresses and networks.
      if (opts.count("ip-net-set")) {
        string const ip_net_set =
          opts.at("ip-net-set").as<string>();

        t.prepared("insert_raw_device_acl_ip_net_set")
          (tool_run_id)
          (device_id)
          (ip_net_set)
          .exec();

        if (opts.count("ip-net")) {
          string const ip_net =
            opts.at("ip-net").as<string>();

          t.prepared("insert_raw_device_acl_ip_net")
            (tool_run_id)
            (device_id)
            (ip_net_set)
            (ip_net)
            .exec();
        }
      }

      // Sets of protocols and port ranges.
      if (opts.count("port-range-set")) {
        string const port_range_set =
          opts.at("port-range-set").as<string>();

        t.prepared("insert_raw_device_acl_port_range_set")
          (tool_run_id)
          (device_id)
          (port_range_set)
          .exec();

        if (opts.count("protocol") && opts.count("port-range")) {
          string const protocol =
            opts.at("protocol").as<string>();
          string const port_range =
            opts.at("port-range").as<string>();

          t.prepared("insert_raw_device_acl_port_range")
            (tool_run_id)
            (device_id)
            (port_range_set)
            (protocol)
            (port_range)
            .exec();
        }
      }

      // ACLs using the previously defined sets.
      if (opts.count("acl-set") &&
          opts.count("acl-number") &&
          opts.count("action-set") &&
          opts.count("src-ip-net-set") &&
          opts.count("dst-ip-net-set") &&
          opts.count("src-port-range-set") &&
          opts.count("dst-port-range-set")) {

        string const acl_set =
          opts.at("acl-set").as<string>();
        string const acl_number =
          opts.at("acl-number").as<string>();

        string const action_set =
          opts.at("action-set").as<string>();

        string const src_ip_net_set =
          opts.at("src-ip-net-set").as<string>();
        string const dst_ip_net_set =
          opts.at("dst-ip-net-set").as<string>();

        string const src_port_range_set =
          opts.at("src-port-range-set").as<string>();
        string const dst_port_range_set =
          opts.at("dst-port-range-set").as<string>();

        t.prepared("insert_raw_device_acl")
          (tool_run_id)
          (device_id)
          (acl_set)
          (acl_number)
          (action_set)
          (src_ip_net_set)
          (dst_ip_net_set)
          (src_port_range_set)
          (dst_port_range_set)
          .exec();
      }

      t.commit();
    }
  }
  catch (std::exception& e) {
    cerr << "Error: " << e.what() << endl;
    return -1;
  }

  return 0;
}
