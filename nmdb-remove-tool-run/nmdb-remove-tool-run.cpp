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
      ("tool-run-id",
       program_options::value<string>()->required(),
       "Tool run UUID to remove from the database.")
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
      cerr << "Remove tool run and all accosiated data." << endl
           << endl
           << "Usage: " << PROGRAM_NAME << " [options]" << endl
           << visible_opts_desc << endl;
      return 0;
    }

    if (opts.count("version")) {
      cerr << "nmdb-remove-tool-run (Netmeld)" << endl;
      return 0;
    }

    // Enforce required options and other sanity checks.
    program_options::notify(opts);


    uuid const tool_run_id =
      boost::uuids::string_generator()(opts.at("tool-run-id").as<string>());

    string const db_name = opts.at("db-name").as<string>();
    pqxx::connection db{string("dbname=") + db_name};

    db.prepare
      ("delete_tool_run",
       "DELETE FROM tool_runs"
       " WHERE (id = $1)");

    if (true) {
      pqxx::transaction<> t{db};

      if (true) {
        t.prepared("delete_tool_run")
          (tool_run_id)
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
