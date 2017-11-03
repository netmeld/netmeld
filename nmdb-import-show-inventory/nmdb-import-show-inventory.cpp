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

//#define BOOST_SPIRIT_DEBUG

#include <netmeld/common/parser_networking.hpp>
#include <netmeld/common/piped_input.hpp>
#include <netmeld/common/queries_common.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/adapted.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/program_options.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <cstdint>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <tuple>

using std::cin;
using std::cout;
using std::cerr;
using std::endl;

using std::get;
using std::ifstream;
using std::string;
using std::vector;

using boost::format;
using boost::numeric_cast;
using boost::uuids::uuid;

namespace filesystem      = boost::filesystem;
namespace posix_time      = boost::posix_time;
namespace program_options = boost::program_options;
namespace qi              = boost::spirit::qi;

boost::uuids::random_generator uuid_generator;

struct InventoryInfo
{
  std::string name;
  std::string description;
  std::string pid; // product identifier
  std::string vid; // version identifier
  std::string sn;  // serial number
};

BOOST_FUSION_ADAPT_STRUCT(
    InventoryInfo,
    (std::string, name)
    (std::string, description)
    (std::string, pid)
    (std::string, vid)
    (std::string, sn)
    )

template<typename InputIterator_T>
struct ParserShowInventory :
  qi::grammar<InputIterator_T,
              std::vector<InventoryInfo>(),
              qi::blank_type>
{
  ~ParserShowInventory() = default;

  ParserShowInventory():
    ParserShowInventory::base_type(start)
  {
    // Get vector of matches
    start
      = *qi::eol >> -(inventory_entry % +qi::eol) >> *qi::eol
      ;

    inventory_entry
      = // first line
        qi::lit("NAME:") >> quoted_text >> -qi::lit(',') >>
        qi::lit("DESCR:") >> quoted_text >> 
        qi::eol >>
        // second line
        qi::lit("PID:") >> unquoted_text >> -qi::lit(',') >>
        qi::lit("VID:") >> unquoted_text >> -qi::lit(',') >>
        qi::lit("SN:") >> unquoted_text
      ;
    quoted_text
      = // Everything between quotes is fair game, unknown if quote can be too
        qi::lit('"') >> 
          qi::lexeme[+(~qi::char_('"'))] >> 
        qi::lit('"')
      ;
    unquoted_text
      = // If unquoted is empty the parse fails, so return empty string
        qi::lexeme[+(~qi::char_(" ,") - qi::eol)] | qi::attr("")
      ;

    BOOST_SPIRIT_DEBUG_NODES(
        (start)
        (inventory_entry)
        (quoted_text)
        (unquoted_text)
        );
  }

  qi::rule<InputIterator_T,
           std::vector<InventoryInfo>(),
           qi::blank_type>
    start;

  qi::rule<InputIterator_T,
           InventoryInfo(),
           qi::blank_type>
    inventory_entry;

  qi::rule<InputIterator_T,
           string(),
           qi::blank_type>
    unquoted_text;

  qi::rule<InputIterator_T, string()>
    quoted_text;
};


int 
main(int argc, char** argv)
{
  string vendor = "Cisco";

  try {
    // Define program options
    program_options::options_description general_opts_desc("Options");
    general_opts_desc.add_options()
      ("device-id",
       program_options::value<string>()->required(),
       "Name of device.")
      ("device-color",
       program_options::value<string>(),
       "Graph color of device.")
      ("device-type",
       program_options::value<string>(),
       "Type of device, determines graph icon.")
      ("pipe",
       "Read input from STDIN; Save a copy to {input-file}.")
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
      ;

    program_options::options_description hidden_opts_desc("Hidden options");
    hidden_opts_desc.add_options()
      ("input-file",
       program_options::value<string>()->required(),
       "Input data file to parse.")
      ;

    program_options::positional_options_description position_opts_desc;
    position_opts_desc.add("input-file", -1);

    // Command-line options (accepted on the command-line)
    program_options::options_description cl_opts_desc;
    cl_opts_desc.add(general_opts_desc).add(hidden_opts_desc);

    // Visible options (shown in help message)
    program_options::options_description visible_opts_desc;
    visible_opts_desc.add(general_opts_desc);


    // Parse command-line options
    program_options::variables_map opts;
    program_options::store
      (program_options::command_line_parser(argc, argv)
       .options(cl_opts_desc)
       .positional(position_opts_desc)
       .run(), opts);

    if (opts.count("help")) {
      cerr << "Import output from 'show inventory' on " << vendor << " devices."
           << endl << endl
           << "Usage: " << PROGRAM_NAME << " [options] {input-file}" << endl
           << visible_opts_desc << endl;
      return 0;
    }

    if (opts.count("version")) {
      cerr << "nmdb-import-show-inventory (Netmeld)" << endl;
      return 0;
    }

    // Enforce required options and other sanity checks.
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

    string const device_id = opts.at("device-id").as<string>();

    string const command_line = "show inventory";


    // Prep DB connection
    string const db_name = opts.at("db-name").as<string>();
    pqxx::connection db{string("dbname=") + db_name};
    db_prepare_common(db);


    // Prep for parsing input
    ifstream input_file(input_file_path.string());
    input_file.unsetf(std::ios::skipws); // disable skipping whitespace

    ParserShowInventory<boost::spirit::istream_iterator> const
      parser_show_inventory;

    vector<InventoryInfo> results;

    if (true) {
      posix_time::ptime const execute_time_lower =
        posix_time::microsec_clock::universal_time();

      boost::spirit::istream_iterator i(input_file), e;

      // Parse input
      bool const parse_success =
        qi::phrase_parse(i, e, parser_show_inventory, 
                         qi::blank, results);

      posix_time::ptime const execute_time_upper =
        posix_time::microsec_clock::universal_time();

      if ((!parse_success) || (i != e)) {
        cerr << "parse error" << endl;
        for (size_t count = 0; (count < 20) && (i != e); ++count, ++i) {
          cerr << *i;
        }
        cerr << endl;
      }
      else {
        pqxx::transaction<> t{db};

        // Add tool run entry
        if (!opts.count("tool-run-id")) {
          t.prepared("insert_tool_run")
            (tool_run_id)
            (PROGRAM_NAME)
            (command_line)
            (input_file_path.string())
            (execute_time_lower)
            (execute_time_upper)
            .exec();
        }

        // Add known device entry
        t.prepared("insert_raw_device")
          (tool_run_id)
          (device_id)
          .exec();

        // Add device coloring
        if (opts.count("device-color")) {
          string const device_color = opts.at("device-color").as<string>();
          
          t.prepared("insert_device_color")
            (device_id)
            (device_color)
            .exec();
        }

        if (opts.count("device-type")) {
          string const device_type = opts.at("device-type").as<string>();

          t.prepared("insert_raw_device_type")
            (tool_run_id)
            (device_id)
            (device_type)
            .exec();
        }

        // Run through results
        for (auto const& inventory_entry : results) {
          std::regex chassis_test("chassis|1", std::regex::icase);

          if (std::regex_match(inventory_entry.name, chassis_test)) {
            string model = inventory_entry.pid;
            string hardware_rev = inventory_entry.vid;
            string serial_number = inventory_entry.sn;
            string description = inventory_entry.description;

            if (true) {
              t.prepared("insert_raw_device_hardware")
                (tool_run_id)
                (device_id)
                (vendor)
                (model)
                (hardware_rev)
                (serial_number)
                (description)
                .exec();
            }
          }
        }

        // Commit all changes
        t.commit();

        if (!opts.count("tool-run-id")) {
          cerr << "tool_run_id: " << tool_run_id << endl;
        }
      }
    }
  }
  catch (std::exception& e) {
    cerr << "Error: " << e.what() << endl;
    return -1;
  }

  return 0;
}
