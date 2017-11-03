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

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/program_options.hpp>

#include <regex>

using std::cout;
using std::cerr;
using std::endl;

using std::string;

namespace filesystem = boost::filesystem;
namespace io = boost::iostreams;
namespace posix_time = boost::posix_time;
namespace program_options = boost::program_options;


void
all_tool_run_script(pqxx::connection& db, std::ostream& os,
                    bool igrnore_tool_run_id);


void
all_tool_run_script(pqxx::connection& db, std::ostream& os,
                    bool ignore_tool_run_id)
{
  pqxx::transaction<> t{db};

  if (true) {
    // Recreate tool runs with specific run ID

    pqxx::result tool_rows =
      t.prepared("select_all_tool_runs").exec();

    os << "# Data from: tool_runs" << endl;
    // TODO Maybe should echo command running before hand and/or silence stdout
    for (auto const& tool_row : tool_rows) {
      string tool_run_id;
      tool_row.at("id").to(tool_run_id);
      string db_name;
      string tool_name;
      tool_row.at("tool_name").to(tool_name);
      string command_line;
      tool_row.at("command_line").to(command_line);
      string device_id;
      tool_row.at("device_id").to(device_id);
      string device_color;
      tool_row.at("device_color").to(device_color);
      string device_type;
      tool_row.at("device_type").to(device_type);
      string data_path;
      tool_row.at("data_path").to(data_path);

      // Format values as needed
      tool_run_id = " --tool-run-id \"" + tool_run_id + "\"";
      db_name = " --db-name \"${DB_NAME}\"";
      if (!device_id.empty()) {
        device_id = " --device-id \"" + device_id + "\"";
      }
      if (!device_type.empty()) {
        device_type = " --device-color \"" + device_type + "\"";
      }
      if (!device_color.empty()) {
        device_color = " --device-type \"" + device_color + "\"";
      }
      if (!data_path.empty()) {
        data_path = " \"" + data_path + "\"";
      }

      if (tool_name.compare(0, 12, "nmdb-import-") == 0) {
        // Handle nmdb-import-* tools generally similarly

        // Standard for all tools
        os << tool_name 
           << db_name
           << ((ignore_tool_run_id) ? "" : tool_run_id)
           << device_id
           << device_type
           << device_color
           << data_path
           ;

      } else if (command_line.compare(0, 12, "nmdb-insert-") == 0) {
        // Handle nmdb-insert-* tools generally similarly

        string mod_cl = command_line;

        // Add quotes around parameters, might not be perfect...but good enough
        std::regex option_regex("(--[^ ]+) ((?=[^(--)])(.+?)(?= --|$))?");
        mod_cl = std::regex_replace(mod_cl,
                                    option_regex,
                                    "$1 \"$2\"");

        // Replace any given DB name with variable one and add tool_run_id
        std::regex db_regex(" --db-name [^ ]+");
        mod_cl = std::regex_replace(mod_cl,
                                    db_regex,
                                    db_name + ((ignore_tool_run_id) 
                                               ? "" 
                                               : tool_run_id));

        // Remove any accidental empty double quotes (easier this way)
        std::regex double_quote_regex("\"\"");
        mod_cl = std::regex_replace(mod_cl,
                                    double_quote_regex,
                                    "");

        os << mod_cl;
      } else if (std::regex_match(tool_name, std::regex("(nessus|nmap)"))) {
        // Handle nessus,nmap special as they don't prefix nmdb-import

        os << "nmdb-import-" << tool_name 
           << db_name
           << ((ignore_tool_run_id) ? "" : tool_run_id)
           << data_path
           ;
      } else if (data_path.find(".clw") != string::npos) {
        // Handle clw imports special, clw only appears in data_path and
        //   tool-run-id is encoded into data_path

        os << "nmdb-import-clw"
           << db_name
           << data_path
           ;
      } else {
        // Dump unknowns as comments

        os << "# " 
           << tool_name
           << db_name
           << ((ignore_tool_run_id) ? "" : tool_run_id)
           << device_id
           << device_type
           << device_color
           << data_path
           ;
      }
      // End of command actions
      os << " > /dev/null" << endl;
    }

    // End of file actions
    os << endl;
  }
  t.commit();
}


int
main(int argc, char** argv)
{
  try {
    // Parse command-line options.
    program_options::options_description opts_desc("Options");
    opts_desc.add_options()
      ("db-name",
       program_options::value<string>()->required()->
       default_value(DEFAULT_DB_NAME),
       "Database to connect to.")
      ("outfile",
       program_options::value<string>()->required()->
       default_value("tool-runs-export"),
       "Dump output to this file prefix, with suffix of timestamp and .sh")
      ("ignore-tool-run-id",
       "Do not save existing tool-run-id values for future import usage")
      ("help,h",
       "Show this help message, then exit.")
      ("version,v",
       "Show version information, then exit.")
      ;

    // Parse command-line options
    program_options::variables_map opts;
    program_options::store
      (program_options::parse_command_line(argc, argv, opts_desc), opts);

    if (opts.count("help")) {
      cerr << "Generate script to rerun all tool_run commands in the database"
           << endl << endl
           << "Usage: " << PROGRAM_NAME << " [options]" << endl
           << endl
           << opts_desc << endl;
      return 0;
    }

    if (opts.count("version")) {
      cerr << "nmdb-export-tool-runs (Netmeld)" << endl;
      return 0;
    }

    // Enforce required options and other sanity checks.
    program_options::notify(opts);


    // Prep DB calls
    string const db_name = opts.at("db-name").as<string>();

    pqxx::connection db{string("dbname=") + db_name};
    db_prepare_common(db);

    db.prepare
      ("select_all_tool_runs",
       "SELECT"
       "  tr.id AS id,"
       "  tr.tool_name AS tool_name,"
       "  rd.device_id AS device_id,"
       "  dt.device_type AS device_type,"
       "  dc.color AS device_color,"
       "  tr.command_line AS command_line,"
       "  tr.data_path AS data_path,"
       "  tr.execute_time AS execute_time"
       " FROM tool_runs AS tr"
       " LEFT OUTER JOIN raw_devices AS rd"
       "  ON (tr.id = rd.tool_run_id)"
       " LEFT OUTER JOIN device_colors AS dc"
       "  ON (rd.device_id = dc.device_id)"
       " LEFT OUTER JOIN device_types AS dt"
       "  ON (rd.device_id = dt.device_id)"
       // Consistency sort, as the JOIN randomizes it
       " ORDER BY tool_name, device_id, data_path");

    bool const ignore_tool_run_id = opts.count("ignore-tool-run-id");

    // Prep file for command output
    posix_time::ptime const timestamp_start =
      posix_time::microsec_clock::universal_time();

    string const os_file = 
      opts.at("outfile").as<string>() + "_" + 
      to_iso_string(timestamp_start) + ".sh";
    io::stream_buffer<io::file_sink> buffer(os_file);
    std::ostream os(&buffer);

    os << "#!/bin/bash --" << endl
       << "# This script was auto-generated by: " << endl
       << "#  ";
    for (int i=0; i < argc; i++) {
      os << " " << argv[i];
    }
    os << endl
       << "# NOTE: This tool currently does not dump manually inserted" << endl
       << "#       data (i.e. using psql or nmdb-insert-*). It mainly" << endl
       << "#       processes data from the DB table tool_runs with" << endl
       << "#       joins from raw_devices, device_colors, and " << endl
       << "#       device_types." << endl
       << endl;
    os << "DB_NAME=\"" << db_name << "\"" << endl
       << endl;


    // Populate file with content
    all_tool_run_script(db, os, ignore_tool_run_id);
  }
  catch (std::exception& e) {
    cerr << "Error: " << e.what() << endl;
    return -1;
  }

  return 0;
}
