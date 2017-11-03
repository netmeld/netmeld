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
      ("mac-addr",
       program_options::value<string>(),
       "MAC address.")
      ("ip-addr",
       program_options::value<string>(),
       "IP address.")
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
      cerr << "Add manually specified ip and/or mac address." << endl
           << endl
           << "Usage: " << PROGRAM_NAME << " [options]" << endl
           << visible_opts_desc << endl;
      return 0;
    }

    if (opts.count("version")) {
      cerr << "nmdb-insert-address (Netmeld)" << endl;
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

    string const db_name = opts.at("db-name").as<string>();
    pqxx::connection db{string("dbname=") + db_name};
    db_prepare_common(db);


    if (true) {
      pqxx::transaction<> t{db};

      bool const is_responding = true;

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

      if (opts.count("mac-addr")) {
        string const mac_addr = opts.at("mac-addr").as<string>();

        t.prepared("insert_raw_mac_addr")
          (tool_run_id)
          (mac_addr)
          (is_responding)
          .exec();
      }

      if (opts.count("ip-addr")) {
        string const ip_addr = opts.at("ip-addr").as<string>();

        t.prepared("insert_raw_ip_addr")
          (tool_run_id)
          (ip_addr)
          (is_responding)
          .exec();
      }

      if (opts.count("mac-addr") && opts.count("ip-addr")) {
        string const mac_addr = opts.at("mac-addr").as<string>();
        string const ip_addr  = opts.at("ip-addr").as<string>();

        t.prepared("insert_raw_mac_addr_ip_addr")
          (tool_run_id)
          (mac_addr)
          (ip_addr)
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
