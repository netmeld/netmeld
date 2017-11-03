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

namespace filesystem = boost::filesystem;
namespace posix_time = boost::posix_time;
namespace program_options = boost::program_options;

boost::uuids::random_generator uuid_generator;

struct Port_Info
{
  unsigned short vlan_id;
  MAC_Addr mac_addr;
  std::string type; // TODO: Where to put this? (17JUN16)
  std::string name;
};

BOOST_FUSION_ADAPT_STRUCT(
    Port_Info,
    (unsigned short, vlan_id)
    (MAC_Addr, mac_addr)
    (std::string, type)
    (std::string, name)
    )

template<typename InputIterator_T>
struct Parser_show_mac_address_table :
  boost::spirit::qi::grammar<InputIterator_T,
                             std::vector<Port_Info>(),
                             boost::spirit::qi::ascii::blank_type>
{
  ~Parser_show_mac_address_table() = default;

  Parser_show_mac_address_table();

  boost::spirit::qi::rule<InputIterator_T,
                          std::vector<Port_Info>(),
                          boost::spirit::qi::ascii::blank_type>
    start;

  boost::spirit::qi::rule<InputIterator_T,
                          Port_Info(),
                          boost::spirit::qi::ascii::blank_type>
    switch_iface;

  Parser_MAC_Addr<InputIterator_T>
    mac_addr;

  boost::spirit::qi::rule<InputIterator_T>
    garbage;


};

template<typename InputIterator_T>
Parser_show_mac_address_table<InputIterator_T>::
Parser_show_mac_address_table() :
  Parser_show_mac_address_table::base_type(start),
  start(),
  switch_iface(),
  mac_addr(),
  garbage()
{
  namespace qi = boost::spirit::qi;

  // Get vector of matches, one per line (CAM or garbage)
  start
    = (*switch_iface)
    ;

  // Get individual CAM matches
  switch_iface
    = (// Valid input match
       (// vlan id
        (qi::ushort_) >>
        // MAC addr
        (mac_addr) >>
        // Assignment type
        (qi::lexeme[+qi::ascii::char_("A-Z") >> -qi::string(" pv")]) >>
        // Interface name
        (qi::lexeme[+(qi::ascii::alnum | qi::ascii::char_("/"))]) >>
        // Rest is garbage
        (garbage))
      |// Invalid grabage input match
        (garbage)
      )
    ;

  // Parse everything until eol
  garbage
    = (qi::omit[*(qi::ascii::char_ - qi::eol) >> qi::eol])
    ;
}

int 
main(int argc, char** argv)
{
  namespace qi = boost::spirit::qi;

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
      cerr << "Import output from 'show mac address-table' on Cisco devices."
           << endl << endl
           << "Usage: " << PROGRAM_NAME << " [options] {input-file}" << endl
           << visible_opts_desc << endl;
      return 0;
    }

    if (opts.count("version")) {
      cerr << "nmdb-import-show-mac-address-table (Netmeld)" << endl;
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

    string const command_line = "show mac address-table";


    // Prep DB connection
    string const db_name = opts.at("db-name").as<string>();
    pqxx::connection db{string("dbname=") + db_name};
    db_prepare_common(db);


    // Prep for parsing input
    ifstream input_file(input_file_path.string());
    input_file.unsetf(std::ios::skipws); // disable skipping whitespace

    Parser_show_mac_address_table<boost::spirit::istream_iterator> const
      parser_show_mac_address_table;

    vector<Port_Info> results;

    if (true) {
      posix_time::ptime const execute_time_lower =
        posix_time::microsec_clock::universal_time();

      boost::spirit::istream_iterator i(input_file), e;

      // Parse input
      bool const parse_success =
        qi::phrase_parse(i, e, parser_show_mac_address_table, 
                         qi::ascii::blank, results);

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
            (device_type).exec();
        }

        // Run through results
        string const interface_media_type = "Ethernet";
        for (auto const& iface : results) {
          // Skip non-applicable parsings 
          if ((6 <= iface.mac_addr.size())) {
            string const mac_addr_str =
              (format("%02x:%02x:%02x:%02x:%02x:%02x")
               % static_cast<uint16_t>(iface.mac_addr[0])
               % static_cast<uint16_t>(iface.mac_addr[1])
               % static_cast<uint16_t>(iface.mac_addr[2])
               % static_cast<uint16_t>(iface.mac_addr[3])
               % static_cast<uint16_t>(iface.mac_addr[4])
               % static_cast<uint16_t>(iface.mac_addr[5])).str();

          // Insert into raw_mac_addrs
            t.prepared("insert_raw_mac_addr")
              (tool_run_id)
              (mac_addr_str)
              () // NULL: Unknown if still responding
              .exec();

          // Insert into raw_device_interfaces 
          //   required for raw_device_link_connections
            t.prepared("insert_raw_device_interface")
              (tool_run_id)
              (device_id)
              (iface.name)
              (interface_media_type)
              (true) // is_up
              .exec();

          // insert into raw_device_link_connections
            t.prepared("insert_raw_device_link_connections")
              (tool_run_id)
              (device_id)
              (iface.name)
              (mac_addr_str)
              .exec();

          // Insert into raw_vlans
            t.prepared("insert_raw_vlan")
              (tool_run_id)
              (iface.vlan_id)
              () // NULL: Description field
              .exec();
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
