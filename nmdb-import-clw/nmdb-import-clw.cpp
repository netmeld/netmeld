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

#include <netmeld/common/fork_exec.hpp>
#include <netmeld/common/parser_networking.hpp>
#include <netmeld/common/queries_common.hpp>

#include <pqxx/pqxx>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/program_options.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <exception>
#include <iostream>
#include <fstream>
#include <functional>
#include <map>
#include <sstream>
#include <string>
#include <vector>


using std::cin;
using std::cout;
using std::cerr;
using std::endl;

using std::exception;
using std::ifstream;
using std::function;
using std::map;
using std::ostringstream;
using std::stringstream;
using std::string;
using std::vector;

using boost::numeric_cast;
using boost::uuids::uuid;

namespace filesystem = boost::filesystem;
namespace posix_time = boost::posix_time;
namespace program_options = boost::program_options;


uuid
read_uuid(filesystem::path const& p);

posix_time::ptime
read_timestamp(filesystem::path const& p);

string
read_command_line(filesystem::path const& p);

string
db_import_clw(pqxx::connection& db,
              string const& db_name, uuid const& tool_run_id,
              filesystem::path const& tool_run_results);


uuid
read_uuid(filesystem::path const& p)
{
  ifstream f{p.string()};
  uuid u;
  f >> u;
  f.close();

  return u;
}


posix_time::ptime
read_timestamp(filesystem::path const& p)
{
  ifstream f{p.string()};
  string s;
  getline(f, s);
  f.close();

  return boost::date_time::parse_delimited_time<posix_time::ptime>(s, 'T');
}


string
read_command_line(filesystem::path const& p)
{
  ifstream f{p.string()};
  string s;
  getline(f, s);
  f.close();

  return s;
}


string
db_import_clw(pqxx::connection& db,
              string const& db_name, uuid const& tool_run_id,
              filesystem::path const& tool_run_results)
{
  posix_time::ptime const execute_time_lower =
    read_timestamp(tool_run_results/"timestamp_start.txt");

  posix_time::ptime const execute_time_upper =
    read_timestamp(tool_run_results/"timestamp_end.txt");

  string const command_line =
    read_command_line(tool_run_results/"command_line_modified.txt");

  string const tool_name =
    command_line.substr(0, command_line.find(' '));


  // ----------------------------------------------------------------------
  // TABLE: tool_runs
  // ----------------------------------------------------------------------

  if (true) {
    pqxx::transaction<> t{db};

    t.prepared("insert_tool_run")
      (tool_run_id)
      (tool_name)
      (command_line)
      (tool_run_results.string().c_str())
      (execute_time_lower)
      (execute_time_upper)
      .exec();

    t.commit();
  }


  // ----------------------------------------------------------------------
  // TABLE: tool_run_interfaces, tool_run_mac_addrs, tool_run_ip_addrs
  // ----------------------------------------------------------------------

  if (true) {
    vector<string> args = {
      "nmdb-import-ip-addr-show",
      "--db-name", db_name,
      "--tool-run-id", to_string(tool_run_id),
      "--tool-run-metadata",
      "--device-id", "__tool-run-metadata__",
      (tool_run_results/"ip_addr_show.txt").string()
    };

    fork_exec_wait(args);
  }


  // ----------------------------------------------------------------------
  // TABLE: tool_run_ip_routes
  // ----------------------------------------------------------------------

  if (true) {
    vector<string> args = {
      "nmdb-import-ip-route-show",
      "--db-name", db_name,
      "--tool-run-id", to_string(tool_run_id),
      "--tool-run-metadata",
      "--device-id", "__tool-run-metadata__"
    };

    // IPv4 routes
    args.push_back((tool_run_results/"ip4_route_show.txt").string());
    fork_exec_wait(args);
    args.pop_back();

    // IPv6 routes
    args.push_back((tool_run_results/"ip6_route_show.txt").string());
    fork_exec_wait(args);
    args.pop_back();
  }

  // ----------------------------------------------------------------------

  return tool_name;
}


int
main(int argc, char** argv)
{
  try {
    program_options::options_description general_opts_desc("Options");
    general_opts_desc.add_options()
      ("db-name",
       program_options::value<string>()->required()->
       default_value(DEFAULT_DB_NAME),
       "Database to connect to.")
      ("help,h",
       "Show this help message, then exit.")
      ("version,v",
       "Show version information, then exit.")
      ;

    program_options::options_description hidden_opts_desc("Hidden options");
    hidden_opts_desc.add_options()
      ("input-directory",
       program_options::value<string>()->required(),
       "Input data file to parse.")
      ;
    program_options::positional_options_description position_opts_desc;
    position_opts_desc.add("input-directory", -1);

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
      cerr << "Import information captured by clw command." << endl
           << endl
           << "Usage: " << PROGRAM_NAME << " [options] {input-directory}" << endl
           << visible_opts_desc << endl;
      return 0;
    }

    if (opts.count("version")) {
      cerr << "nmdb-import-clw (Netmeld)" << endl;
      return 0;
    }

    program_options::notify(opts);


    filesystem::path const idp = opts.at("input-directory").as<string>();
    if (!filesystem::exists(idp)) {
      throw std::runtime_error("Specified input-directory does not exist: " +
                               idp.string());
    }
    filesystem::path const input_dir_path = filesystem::canonical(idp);

    filesystem::path const tool_run_results{input_dir_path};

    string const db_name = opts.at("db-name").as<string>();
    pqxx::connection db{string("dbname=") + db_name};
    db_prepare_common(db);

    uuid const tool_run_id = read_uuid(tool_run_results/"tool_run_id.txt");

    string const tool_name =
      db_import_clw(db, db_name, tool_run_id, tool_run_results);

    if (tool_name == "nmap") {
      vector<string> args = {
        "nmdb-import-nmap",
        "--db-name", db_name,
        "--tool-run-id", to_string(tool_run_id),
        (tool_run_results/"results.xml").string()
      };

      fork_exec_wait(args);
    }
    else if ((tool_name == "ping") || (tool_name == "ping6")) {
      vector<string> args = {
        "nmdb-import-ping",
        "--db-name", db_name,
        "--tool-run-id", to_string(tool_run_id),
        (tool_run_results/"stdout.txt").string()
      };

      fork_exec_wait(args);
    }
  }
  catch (std::exception& e) {
    cerr << "Error: " << e.what() << endl;
    return -1;
  }

  return 0;
}
