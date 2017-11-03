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
bool skip_print(false);

#include <netmeld/common/acls.hpp>
#include <netmeld/common/parser_networking.hpp>
#include <netmeld/common/piped_input.hpp>
#include <netmeld/common/queries_common.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/adapted.hpp>
#include <boost/spirit/repository/include/qi_confix.hpp>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
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
#include <map>
#include <string>
#include <tuple>


using std::cin;
using std::cout;
using std::cerr;
using std::endl;

using std::get;
using std::ifstream;
using std::make_tuple;
using std::pair;
using std::tuple;
using std::string;
using std::vector;

using boost::numeric_cast;
using boost::uuids::uuid;
using boost::spirit::repository::confix;

namespace filesystem = boost::filesystem;
namespace posix_time = boost::posix_time;
namespace qi = boost::spirit::qi;
namespace program_options = boost::program_options;


boost::uuids::random_generator uuid_generator;


template <typename InputIterator_T>
struct Parser_Juniper_Config : 
  qi::grammar<InputIterator_T,
              std::multimap<string, string>(),
              qi::ascii::blank_type>
{

  Parser_Juniper_Config() :
    Parser_Juniper_Config::base_type(start)
  {
    start
      = *(config_nodes);

    config_nodes
      = ((&(qi::lit("set policy id ") >> +qi::ascii::digit >> qi::eol) >> 
           (config_token >> config_token >> multiline_config)
            [qi::_val =
             boost::phoenix::construct< std::pair<string, string> >
             (qi::_1 + " " + qi::_2, qi::_3)])
        | (config_token >> config_token >> config_line)
           [qi::_val =
            boost::phoenix::construct< std::pair<string, string> >
            (qi::_1 + " " + qi::_2, qi::_3)]
        | (config_token >> config_token)
           [qi::_val =
            boost::phoenix::construct< std::pair<string, string> >
            (qi::_1 + " " + qi::_2, "")]
        | (config_token | qi::string("exit"))
           [qi::_val =
            boost::phoenix::construct< std::pair<string, string> >
            (qi::_1, "")]
        ) >> *qi::eol
    ;

  config_line
    = (qi::lexeme
       [(config_token | config_quote) >>
        *(qi::hold[+qi::ascii::blank >> (config_token | config_quote)])])
    ;

  multiline_config
    = (qi::lexeme
       [+(config_line | (qi::eol >> -qi::attr(" "))) >> qi::lit("exit")])
    ;

  config_token
    = (qi::lexeme[+(qi::ascii::graph - qi::ascii::char_("\"")) - qi::lit("exit")])
    ;

  config_quote
    = (qi::lexeme
       [qi::ascii::char_('"') >>
        *(*qi::ascii::blank >> config_token >> *qi::ascii::blank) >>
        qi::ascii::char_('"')])
    ;

  BOOST_SPIRIT_DEBUG_NODES(
      (start)
      (config_nodes)
      (config_line)
      (multiline_config)
      (config_token)
      (config_quote)
      );
  }

  qi::rule<InputIterator_T, 
           std::multimap<string, string>(),
           qi::ascii::blank_type> 
    start;

  qi::rule<InputIterator_T, 
           std::pair<string, string>(),
           qi::ascii::blank_type> 
    config_nodes;

  qi::rule<InputIterator_T, string()> 
    config_line,
    multiline_config,
    config_token,
    config_quote;
};

template<typename InputIterator_T>
string
to_string(InputIterator_T iter1, InputIterator_T iter2);

template<typename InputIterator_T>
void
process_interfaces(pqxx::transaction<>& t, 
                   uuid const& tool_run_id, string const& device_id,
                   InputIterator_T const& range,
                   std::map<string, std::tuple<IP_Addr_with_Prefix, 
                                               IP_Addr_with_Prefix>>&
                     translation_map);

void
process_interface(pqxx::transaction<>& t, 
                  uuid const& tool_run_id, string const& device_id,
                  string config_line,
                  std::map<string, string>& unnumbered_map,
                  std::multimap<string, IP_Addr_with_Prefix>& numbered_map,
                  std::map<string, std::set<string>>& zone_interface_map);

void
process_route(pqxx::transaction<>& t,
              uuid const& tool_run_id, string const& device_id,
              string const config_line,
              std::map<string, std::tuple<IP_Addr_with_Prefix, 
                                          IP_Addr_with_Prefix>>&
                translation_map);

void
process_acls(pqxx::transaction<>& t,
             uuid const& tool_run_id, string const& device_id,
             std::multimap<string, string> const& config);

void
process_service(pqxx::transaction<>& t,
                uuid const& tool_run_id, string const& device_id,
                string config_line);

void
process_zone(pqxx::transaction<>& t,
             uuid const& tool_run_id, string const& device_id,
             string config_line);

void
process_address(pqxx::transaction<>& t,
                uuid const& tool_run_id, string const& device_id,
                string config_line);

void
process_group(pqxx::transaction<>& t,
              uuid const& tool_run_id, string const& device_id,
              string config_line);

void
process_policy(//pqxx::transaction<>& t,
               //uuid const& tool_run_id, string const& device_id,
               string const config_line,
               std::map<int, ACL_Policy>& id_policies);

// Global constant 
const string SRC_KEY=" (src)";


int
main(int argc, char** argv)
{
  namespace qi = boost::spirit::qi;

  try {
    program_options::options_description general_opts_desc("Options");
    general_opts_desc.add_options()
      ("device-id",
       program_options::value<string>()->required(),
       "Name of device.")
      ("device-color",
       program_options::value<string>(),
       "Graph color of device.")
      ("device-type",
       program_options::value<string>()->required()->
       default_value("Juniper"),
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
      cerr << "Import Juniper's Junos configuration file." << endl
           << endl
           << "Usage: " << PROGRAM_NAME << " [options] {input-file}" << endl
           << visible_opts_desc << endl;
      return 0;
    }

    if (opts.count("version")) {
      cerr << "nmdb-import-juniper-screenos (Netmeld)" << endl;
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

    string const command_line = "get config";

    string const db_name = opts.at("db-name").as<string>();
    pqxx::connection db{string("dbname=") + db_name};
    db_prepare_common(db);

    db.prepare
      ("select_raw_device_ip_addrs",
       "SELECT *"
       " FROM raw_device_ip_addrs"
       " WHERE ($1 = tool_run_id) AND"
       "       ($2 = device_id) AND"
       "       ($3 = interface_name)" 
       " ORDER BY ip_addr");

    db.prepare
      ("select_raw_device_acl_port_ranges",
       "SELECT *"
       " FROM raw_device_acl_port_ranges"
       " WHERE ($1 = tool_run_id) AND"
       "       ($2 = device_id) AND"
       "       (port_range_set like $3)"
       " ORDER BY port_range_set");

    db.prepare
      ("select_raw_device_acl_ip_nets",
       "SELECT *"
       " FROM raw_device_acl_ip_nets"
       " WHERE ($1 = tool_run_id) AND"
       "       ($2 = device_id) AND"
       "       ($3 = ip_net_set)"
       " ORDER BY ip_net");


    ifstream input_file(input_file_path.string());
    input_file.unsetf(std::ios::skipws);  // disable skipping whitespace

    Parser_Juniper_Config<boost::spirit::istream_iterator> const
      parser_juniper_config;

    std::multimap<string, string> juniper_config;

    if (true) {
      posix_time::ptime const execute_time_lower =
        posix_time::microsec_clock::universal_time();

      boost::spirit::istream_iterator i(input_file), e;

      bool const parse_success =
        qi::phrase_parse
        (i, e, parser_juniper_config, qi::ascii::blank, juniper_config);

      posix_time::ptime const execute_time_upper =
        posix_time::microsec_clock::universal_time();

      if ((!parse_success) || (i != e)) {
        cerr << "parse error" << endl;
        for (size_t count = 0; (count < 10) && (i != e); ++count, ++i) {
          cerr << *i;
        }
        cerr << endl;
      }
      else {
        pqxx::transaction<> t{db};

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

        t.prepared("insert_raw_device")
          (tool_run_id)
          (device_id)
          .exec();

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

        t.prepared("insert_raw_device_hardware")
          (tool_run_id)
          (device_id)
          ("Juniper")
          () // model
          () // hardware revision
          () // serial_number
          () // description
          .exec();
                
        // Mapping to handle interface<->route translations
        std::map<string, std::tuple<IP_Addr_with_Prefix, IP_Addr_with_Prefix>>
          translation_map;

        // Process interfaces
        auto range = juniper_config.equal_range("set interface");
        process_interfaces(t, tool_run_id, device_id, range,
                           translation_map);

        // Process routes
        range = juniper_config.equal_range("set route");
        for (auto iter = get<0>(range); iter != get<1>(range); iter++) {
          process_route(t, tool_run_id, device_id, get<1>(*iter),
                        translation_map);
        }
        
        // Process acls
        process_acls(t, tool_run_id, device_id, juniper_config);

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


template<typename InputIterator_T>
string
to_string(InputIterator_T iter1, InputIterator_T iter2)
{
  // create string
  string line = std::accumulate(iter1, iter2, string(""),
      [](auto op1, auto op2) { return op1 + " " + op2; });

  // trim whitespace
  auto front_ws = std::find_if_not(line.begin(), line.end(),
      [](int c){return std::isspace(c);});
  auto back_ws = std::find_if_not(line.rbegin(), line.rend(),
      [](int c){return std::isspace(c);}).base();

  return ((back_ws <= front_ws) 
      ? string() 
      : string(front_ws, back_ws));
}

template<typename InputIterator_T>
void
process_interfaces(pqxx::transaction<>& t, 
                   uuid const& tool_run_id, string const& device_id,
                   InputIterator_T const& range,
                   std::map<string, std::tuple<IP_Addr_with_Prefix, 
                                               IP_Addr_with_Prefix>>&
                     translation_map)
{
  // Various mappings to help with data tracking and updating
  std::map<string, string> unnumbered_map;
  std::multimap<string, IP_Addr_with_Prefix> numbered_map;
  std::map<string, std::set<string>> zone_interface_map;

  // Process interface lines
  for (auto iter = get<0>(range); iter != get<1>(range); iter++) {
    process_interface(t, tool_run_id, device_id, get<1>(*iter), 
                      unnumbered_map, numbered_map,
                      zone_interface_map);
  }

  // Map zone<->interface IP nets for ACL processing
  for (auto const& zone_interface_map_iter : zone_interface_map) {
    string const zone = zone_interface_map_iter.first;
    
    for (auto const& interface : zone_interface_map_iter.second) {
      pqxx::result ip_addrs_rows =
        t.prepared("select_raw_device_ip_addrs")
          (tool_run_id)
          (device_id)
          (interface)
          .exec();

      for (auto const& ip_addrs_row : ip_addrs_rows) {
        string ip_addr;
        ip_addrs_row.at("ip_addr").to(ip_addr);
        
        t.prepared("insert_raw_device_acl_ip_net")
          (tool_run_id)
          (device_id)
          (zone) // ip_net_set
          (ip_addr) // ip_net
          .exec();
      }
    }
  }

  // Create translation map for route processing
  for (auto const& um_iter : unnumbered_map) {
    string const um_name = um_iter.first;
    string const um_target = um_iter.second;

    // Initialize map with empty data
    translation_map.emplace(um_name, std::make_tuple(IPv4_Addr_with_Prefix(), 
                                                     IPv6_Addr_with_Prefix()));

    for (auto const& nm_iter : numbered_map) {
      string const nm_name = nm_iter.first;
      IP_Addr_with_Prefix ip_addr = nm_iter.second;

      // Skip non-matching
      if (um_target != nm_name) {
        continue;
      }

      // Update entry
      auto um_pos = translation_map.find(um_name);
      auto& ips = um_pos->second;
      if (ip_addr.addr().is_v4()) {
        get<0>(ips) = ip_addr;
      } else {
        get<1>(ips) = ip_addr;
      }
    }
  }
}

void
process_interface(pqxx::transaction<>& t, 
                  uuid const& tool_run_id, string const& device_id,
                  string config_line,
                  std::map<string, string>& unnumbered_map,
                  std::multimap<string, IP_Addr_with_Prefix>& numbered_map,
                  std::map<string, std::set<string>>& zone_interface_map)
{
  boost::tokenizer< boost::escaped_list_separator<char> >
    tokenizer(config_line, 
              boost::escaped_list_separator<char>('\\', ' ', '\"'));
  auto iter = tokenizer.begin();

  /* String of the form (not exhaustive):
       interface { ip|ipv6|mip|tag|zone } 
     See further breakdown in individual section of if/else statement below
  */
  string const interface_name_str = *(iter); // interface name

  /* The following pragmas are to silence a warning which should not occur. 
     While it is possible for std::distance to return a negative value, we
     should not be using a random-access iterator. The iterators it is working 
     on are for a string and starting from the front. It therefore should never
     occur that the distance is negative.
  */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
  auto not_alpha_dist = 
    std::find_if_not(interface_name_str.begin(), 
                     interface_name_str.end(),
                     [](char c) { return std::isalpha(c); });
  auto not_alpha_pos = 
    std::distance(interface_name_str.begin(), not_alpha_dist);
  string const interface_type_str =
    interface_name_str.substr(0, not_alpha_pos); // interface type
#pragma GCC diagnostic pop

  if (true) {
    t.prepared("insert_raw_device_interface")
      (tool_run_id)
      (device_id)
      (interface_name_str)
      (interface_type_str)
      (true)
      .exec();
  }

  string const sub_cmd_str = *(std::next(iter, 1)); // name
  if ("ip" == sub_cmd_str || "ipv6" == sub_cmd_str) {
    // ip {ip_addr/cidr|unnumbered interface interface} |
    // ipv6 ip ip_addr/cidr

    int const offset = ("ip" == *(std::next(iter, 2))) ? 1 : 0;
    try {
      int ip_addr_loc = 2 + offset;
      string const ip_addr_str = *(std::next(iter, ip_addr_loc));

      if ("unnumbered" != ip_addr_str) {
        IP_Addr_with_Prefix ip_addr =
          IP_Addr_with_Prefix::from_string(ip_addr_str);

        // Add to multimap for later translations
        numbered_map.emplace(interface_name_str, ip_addr);

        if (true) { // DB inserts
          t.prepared("insert_raw_ip_addr")
            (tool_run_id)
            (ip_addr)
            (true) // Assume reachable and up 
            .exec();
          
          t.prepared("insert_raw_ip_net")
            (tool_run_id)
            (ip_addr)
            () // NULL
            .exec();
          
          t.prepared("insert_raw_device_ip_addr")
            (tool_run_id)
            (device_id)
            (interface_name_str)
            (ip_addr)
            .exec();

        }
      }
      else {
        string const alt_intf = *(std::next(iter, ip_addr_loc + 2));
        // Add to map for later translations
        unnumbered_map.emplace(interface_name_str, alt_intf);
      }
    }
    catch (std::exception& e) {
      string const line = to_string(std::next(iter,1), tokenizer.end()); 
      if (skip_print) {cout << "SKIPPING: " << line << " -- " << e.what() << endl;}
    }
  }
  else if ("mip" == sub_cmd_str) {
    // mip ip_addr host ip_addr [netmask mask] [vrouter vrouter]

    string const ip_addr_str  = *(std::next(iter, 2));
    string const host_ip_addr_str = *(std::next(iter, 4));
    auto netmask_cmd = std::find(tokenizer.begin(), tokenizer.end(), 
                                 "netmask");

    IP_Addr_with_Prefix ip_addr, host_ip_addr;
    if (netmask_cmd != tokenizer.end()) {
      string const netmask_str = *(std::next(netmask_cmd, 1)); // mask
      ip_addr =
        IP_Addr_with_Prefix::from_string(ip_addr_str + "/" + netmask_str);
      host_ip_addr =
        IP_Addr_with_Prefix::from_string(host_ip_addr_str + "/" + netmask_str);
    }
    else {
      ip_addr =
        IP_Addr_with_Prefix::from_string(ip_addr_str + "/255.255.255.255");
      host_ip_addr =
        IP_Addr_with_Prefix::from_string(host_ip_addr_str + "/255.255.255.255");
    }

    if (true) { // DB inserts
      t.prepared("insert_raw_ip_addr")
        (tool_run_id)
        (ip_addr)
        () // Don't assume reachable and up 
        .exec();
      t.prepared("insert_raw_ip_addr")
        (tool_run_id)
        (host_ip_addr)
        () // Don't assume reachable and up 
        .exec();

      string const description = "mip " + ip_addr.to_string() 
                               + " host " + host_ip_addr.to_string();

      t.prepared("insert_raw_ip_net")
        (tool_run_id)
        (ip_addr)
        (description) // NULL
        .exec();
      t.prepared("insert_raw_ip_net")
        (tool_run_id)
        (host_ip_addr)
        (description) // NULL
        .exec();

      string const ip_net_set = "mip(" + ip_addr.to_string() + ")";
      t.prepared("insert_raw_device_acl_ip_net_set")
        (tool_run_id)
        (device_id)
        (ip_net_set)
        .exec();
      t.prepared("insert_raw_device_acl_ip_net")
        (tool_run_id)
        (device_id)
        (ip_net_set)
        (ip_addr)
        .exec();
      t.prepared("insert_raw_device_acl_ip_net")
        (tool_run_id)
        (device_id)
        (ip_net_set)
        (host_ip_addr)
        .exec();

    }
  }
  else if ("tag" == sub_cmd_str) {
    // tag id_num zone zone
    // TODO Still need to handle vlan tagging


    // Security policy zone binding
    string const ip_net_set = *(std::next(iter, 4));

    // Update zone to interface mapping for later ACL processing
    auto ziter = zone_interface_map.find(ip_net_set);
    if (ziter == zone_interface_map.end()) {
      zone_interface_map.emplace(ip_net_set, std::set<string>());
      ziter = zone_interface_map.find(ip_net_set);
    }
    auto& interface_set = (std::get<1>(*ziter));
    interface_set.emplace(interface_name_str);

    if (true) { // DB inserts
      t.prepared("insert_raw_device_acl_ip_net_set")
        (tool_run_id)
        (device_id)
        (ip_net_set)
        .exec();
    }
  }
  else if ("zone" == sub_cmd_str) {
    // zone zone

    // Security policy zone binding
    string const ip_net_set = *(std::next(iter, 2));

    // Update zone to interface mapping for later ACL processing
    auto ziter = zone_interface_map.find(ip_net_set);
    if (ziter == zone_interface_map.end()) {
      zone_interface_map.emplace(ip_net_set, std::set<string>());
      ziter = zone_interface_map.find(ip_net_set);
    }
    auto& interface_set = (std::get<1>(*ziter));
    interface_set.emplace(interface_name_str);

    if (true) { // DB inserts
      t.prepared("insert_raw_device_acl_ip_net_set")
        (tool_run_id)
        (device_id)
        (ip_net_set)
        .exec();
    }
  }
  else {
    // TODO use set interface interface route to populate raw_device_ip_routes?
    string const line = to_string(std::next(iter,1), tokenizer.end()); 
    if (skip_print){cout << "SKIPPING: " << line << endl;}
  }
}

void
process_route(pqxx::transaction<>& t,
              uuid const& tool_run_id, string const& device_id,
              string const config_line,
              std::map<string, std::tuple<IP_Addr_with_Prefix, 
                                          IP_Addr_with_Prefix>>&
                translation_map)
{
  boost::tokenizer< boost::escaped_list_separator<char> >
    tokenizer(config_line, 
              boost::escaped_list_separator<char>('\\', ' ', '\"'));
  auto iter = tokenizer.begin();

  IP_Addr_with_Prefix dst_ip_net = 
    IP_Addr_with_Prefix::from_string(*iter);
  string const interface_name = *(std::next(iter, 2));
  
  auto rtr_pos = translation_map.find(interface_name);
  if (rtr_pos == translation_map.end()) {
    cout << "WARNING: Route set without known target interface" << endl;
    return;
  }

  IP_Addr_with_Prefix rtr_ip_addr =
    (dst_ip_net.addr().is_v4()) ? get<0>(rtr_pos->second)
                                : get<1>(rtr_pos->second);

  if (true) {
    t.prepared("insert_raw_ip_addr")
      (tool_run_id)
      (dst_ip_net)
      () // Don't assume up
      .exec();

    t.prepared("insert_raw_ip_net")
      (tool_run_id)
      (dst_ip_net)
      () // Empty description
      .exec();

    t.prepared("insert_raw_device_ip_route")
      (tool_run_id)
      (device_id)
      (interface_name)
      (dst_ip_net)
      (rtr_ip_addr)
      .exec();
  }
}

void
process_acls(pqxx::transaction<>& t,
             uuid const& tool_run_id, string const& device_id,
             std::multimap<string, string> const& config)
{
  // Process services
  auto range = config.equal_range("set service");
  for (auto iter = get<0>(range); iter != get<1>(range); iter++) {
    process_service(t, tool_run_id, device_id, get<1>(*iter));
  }

  // Process zones
  range = config.equal_range("set zone");
  for (auto iter = get<0>(range); iter != get<1>(range); iter++) {
    process_zone(t, tool_run_id, device_id, get<1>(*iter));
  }

  // Process address
  range = config.equal_range("set address");
  for (auto iter = get<0>(range); iter != get<1>(range); iter++) {
    process_address(t, tool_run_id, device_id, get<1>(*iter));
  }
  
  // Process groups
  range = config.equal_range("set group");
  for (auto iter = get<0>(range); iter != get<1>(range); iter++) {
    process_group(t, tool_run_id, device_id, get<1>(*iter));
  }
  
  // Process policies
  //   multiline commands, so two phases
  std::map<int, ACL_Policy> id_policies;
  range = config.equal_range("set policy");
  for (auto iter = get<0>(range); iter != get<1>(range); iter++) {
    //process_policy(t, tool_run_id, device_id, get<1>(*iter));
    process_policy(get<1>(*iter), id_policies);
  }

  for (auto const& p_iter : id_policies) {
    auto policy = p_iter.second;

    if (true) { // DB inserts
      // Actions
      t.prepared("insert_raw_device_acl_action_set")
        (tool_run_id)
        (device_id)
        (policy.action_set)
        .exec();

      for (auto action : policy.actions) {
        t.prepared("insert_raw_device_acl_action")
          (tool_run_id)
          (device_id)
          (policy.action_set)
          (action)
          .exec();
      }

      // Whole policy
      if (!policy.is_disabled()) {
        // Start of ridiculous loop nesting
        for (auto src_ip_net : policy.src_ip_nets) {
          // Ensure addr is in net_set, handles special cases of any or mip
          if (true) { // DB inserts
            t.prepared("insert_raw_device_acl_ip_net_set")
              (tool_run_id)
              (device_id)
              (src_ip_net)
              .exec();
          }

          for (auto dst_ip_net : policy.dst_ip_nets) {
            // Ensure addr is in net_set, handles special cases of any or mip
            if (true) { // DB inserts
              t.prepared("insert_raw_device_acl_ip_net_set")
                (tool_run_id)
                (device_id)
                (dst_ip_net)
                .exec();
            }

            for (auto service : policy.services) {
              t.prepared("insert_raw_device_acl")
                (tool_run_id)
                (device_id)
                (policy.acl_set)
                (policy.acl_number)
                (policy.action_set)
                (src_ip_net)
                (dst_ip_net)
                (service + SRC_KEY)
                (service)
                .exec();
            }
          }
        }
        // End of ridiculous loop nesting
      }
    }
  }
}

void
process_service(pqxx::transaction<>& t,
                uuid const& tool_run_id, string const& device_id,
                string const config_line)
{
  boost::tokenizer< boost::escaped_list_separator<char> >
    tokenizer(config_line, 
              boost::escaped_list_separator<char>('\\', ' ', '\"'));
  auto iter = tokenizer.begin();
  
  // name protocol|+ proto src-port num-num dst-port num-num
  string const service_name = *(iter);
  string const protocol = *(std::next(iter, 2));
  string const sub_cmd_str = *(std::next(iter, 1));
  
  if ("protocol" == sub_cmd_str || "+" == sub_cmd_str) {
    auto src_port_iter = std::find(tokenizer.begin(), tokenizer.end(), "src-port");
    auto dst_port_iter = std::find(tokenizer.begin(), tokenizer.end(), "dst-port");

    //acl_policies.add_service(service_name, protocol, 
    //    *(std::next(dst_port_iter, 1)), *(std::next(src_port_iter, 1)));

    if (true) { // DB inserts
      t.prepared("insert_raw_device_acl_port_range_set")
        (tool_run_id)
        (device_id)
        (service_name)
        .exec();

      if (src_port_iter != tokenizer.end()) {
        // protocol may not use ports (e.g. icmp, ipsec, etc)
        string ports = *(std::next(src_port_iter, 1));
        std::replace(ports.begin(), ports.end(), '-', ',');
        string src_ports = "[" + ports + "]";
        string port_range_set = service_name + SRC_KEY;

        t.prepared("insert_raw_device_acl_port_range_set")
          (tool_run_id)
          (device_id)
          (port_range_set)
          .exec();

        t.prepared("insert_raw_device_acl_port_range")
          (tool_run_id)
          (device_id)
          (port_range_set)
          (protocol)
          (src_ports)
          .exec();
      }

      if (dst_port_iter != tokenizer.end()) {
        // protocol may not use ports (e.g. icmp, ipsec, etc)
        string ports = *(std::next(dst_port_iter, 1));
        std::replace(ports.begin(), ports.end(), '-', ',');
        string dst_ports = "[" + ports + "]";
        t.prepared("insert_raw_device_acl_port_range")
          (tool_run_id)
          (device_id)
          (service_name) // port_range_set
          (protocol)
          (dst_ports)
          .exec();
      }
    }
  }
  else {
    // unhandled service
    string const line = to_string(std::next(iter,1), tokenizer.end()); 
    if (skip_print){cout << "SKIPPING: " << line << " -- " << endl;}
  }
}

void
process_zone(pqxx::transaction<>& t,
             uuid const& tool_run_id, string const& device_id,
             string const config_line)
{
  boost::tokenizer< boost::escaped_list_separator<char> >
    tokenizer(config_line, 
              boost::escaped_list_separator<char>('\\', ' ', '\"'));
  auto iter = tokenizer.begin();

  // [id num] name [vrouter vr_name]
  string const id_or_name_str = *(iter); // either id or zone name
  string const zone_name_str = ("id" == id_or_name_str)
                             ? *(std::next(iter, 2))
                             : id_or_name_str;

  //acl_policies.add_zone(zone_name_str);

  if (true) { // DB inserts
    t.prepared("insert_raw_device_acl_ip_net_set")
      (tool_run_id)
      (device_id)
      (zone_name_str) // ip_net_set
      .exec();
  }
}

void
process_address(pqxx::transaction<>& t, 
                uuid const& tool_run_id, string const& device_id,
                string config_line)
{
  boost::tokenizer< boost::escaped_list_separator<char> >
    tokenizer(config_line, 
              boost::escaped_list_separator<char>('\\', ' ', '\"'));
  auto iter = tokenizer.begin();
  auto count = std::distance(tokenizer.begin(), tokenizer.end());

  // String of the form:
  //   zone name {fqdn|ip/cidr|ip mask} [comment]
  string const zone_name_str = *(iter); // zone
  string const ip_name_str = *(std::next(iter, 1)); // name
  string const ip_addr_str = *(std::next(iter, 2)); // fqdn | ip/cidr | ip

  // check if ip_addr_str is FQDN or IP
  IP_Addr_with_Prefix ip_addr;
  auto comment_iter = tokenizer.end(); // default to end
  try { // IP
    if (ip_addr_str.find("/") == string::npos 
       && std::next(iter, 3) != tokenizer.end()) {
      string const netmask_str = *(std::next(iter, 3)); // mask
      ip_addr =
        IP_Addr_with_Prefix::from_string(ip_addr_str + "/" + netmask_str);
      
      if (count >= 5) {
        comment_iter = std::next(iter, 4);
      }
    } else {
      ip_addr =
        IP_Addr_with_Prefix::from_string(ip_addr_str);

      if (count >= 4) {
        comment_iter = std::next(iter, 3);
      }
    }
  }
  catch (std::exception& e) { // FQDN
    string const line = to_string(std::next(iter, 1), tokenizer.end());
    cout << "WARNING: Use of FQDN instead of IPv4/6 -- set address " 
         << line << endl;
    
    if (count >= 4) {
      comment_iter = std::next(iter, 3);
    }

    return;
  }

  string const ip_addr_with_cidr = ip_addr.to_string();

  // description: ip_name_str concated with remainder of tokens, if any
  string const comment_str = to_string(comment_iter, tokenizer.end()); 
  string const description = ("" == comment_str)
                           ? ip_name_str
                           : ip_name_str + " -- " + comment_str;

  //acl_policies.add_address(ip_addr, ip_name_str, zone_name_str, description);

  if (true) {
    t.prepared("insert_raw_ip_addr")
      (tool_run_id)
      (ip_addr_with_cidr)
      () // Don't assume up
      .exec();
    
    t.prepared("insert_raw_ip_net")
      (tool_run_id)
      (ip_addr_with_cidr)
      (description)
      .exec();

    t.prepared("insert_raw_device_acl_ip_net")
      (tool_run_id)
      (device_id)
      (zone_name_str) // ip_net_set
      (ip_addr_with_cidr)
      .exec();
  }
}

void
process_group(pqxx::transaction<>& t,
              uuid const& tool_run_id, string const& device_id,
              string const config_line)
{
  boost::tokenizer< boost::escaped_list_separator<char> >
    tokenizer(config_line, 
              boost::escaped_list_separator<char>('\\', ' ', '\"'));
  auto iter = tokenizer.begin();

  // {address zone}|service grp_name [add name]
  string const group_type = *(iter); // address|service
  auto add_iter = std::find(tokenizer.begin(), tokenizer.end(), "add");

  if (add_iter != tokenizer.end()) {
    if ("address" == group_type) {
      string const zone_name_str = *(std::next(iter, 1));
      string const group_name_str = *(std::next(iter, 2));
      string const addr_name_str = *(std::next(iter, 4));

      //acl_policies.add_address_set(group_name_str, addr_name_str, zone_name_str);

      if (true) { // DB insert
        t.prepared("insert_raw_device_acl_ip_net_set")
          (tool_run_id)
          (device_id)
          (group_name_str) // ip_net_set
          .exec();

        // Map address information to this grouping
        pqxx::result ip_net_rows =
          t.prepared("select_raw_device_acl_ip_nets")
          (tool_run_id)
          (device_id)
          (addr_name_str) // ip_net_set
          .exec();

        for (auto const& ip_net_row : ip_net_rows) {
          string ip_net;
          ip_net_row.at("ip_net").to(ip_net);

          t.prepared("insert_raw_device_acl_ip_net")
            (tool_run_id)
            (device_id)
            (group_name_str) // ip_net_set
            (ip_net)
            .exec();
        }
      }
    } 
    else if ("service" == group_type) {
      string const group_name_str = *(std::next(iter, 1)); // grp_name
      string const srvc_name_str = *(std::next(iter, 3));  // name

      //acl_policies.add_service_set(group_name_str, srvc_name_str);

      if (true) { // DB insert
        t.prepared("insert_raw_device_acl_port_range_set")
          (tool_run_id)
          (device_id)
          (group_name_str + SRC_KEY) // port_range_set
          .exec();
        t.prepared("insert_raw_device_acl_port_range_set")
          (tool_run_id)
          (device_id)
          (group_name_str) // port_range_set
          .exec();
        
        // Map service information to this grouping
        pqxx::result port_range_rows = 
          t.prepared("select_raw_device_acl_port_ranges")
            (tool_run_id)
            (device_id)
            (srvc_name_str + "%")
            .exec();
       
        for (auto const& port_range_row : port_range_rows) {
          string port_range_set;
          port_range_row.at("port_range_set").to(port_range_set);
          string protocol;
          port_range_row.at("protocol").to(protocol);
          string port_range;
          port_range_row.at("port_range").to(port_range);

          if (std::equal(SRC_KEY.rbegin(), SRC_KEY.rend(), 
                         port_range_set.rbegin())) {
            t.prepared("insert_raw_device_acl_port_range")
              (tool_run_id)
              (device_id)
              (group_name_str + SRC_KEY)
              (protocol)
              (port_range)
              .exec();
          }
          else {
            t.prepared("insert_raw_device_acl_port_range")
              (tool_run_id)
              (device_id)
              (group_name_str)
              (protocol)
              (port_range)
              .exec();
          }
        }
      }
    }
  }
}

void
process_policy(//pqxx::transaction<>& t,
               //uuid const& tool_run_id, string const& device_id,
               string const config_line,
               std::map<int, ACL_Policy>& id_policies)
{
  boost::tokenizer< boost::escaped_list_separator<char> >
    tokenizer(config_line, 
              boost::escaped_list_separator<char>('\\', ' ', '\"'));
  auto iter = tokenizer.begin();

  int acl_number = std::stoi(*(std::next(iter, 1)));
  string sub_cmd_str = *(std::next(iter, 2));

  if ("from" == sub_cmd_str) {
    // id number from zone1 to zone2 src dst service action(s)
    ACL_Policy policy = ACL_Policy(acl_number,
                                   sub_cmd_str + " " + 
                                   *(std::next(iter, 3)) + " " +
                                   *(std::next(iter, 4)) + " " +
                                   *(std::next(iter, 5)));
    string const src_ip_addr = *(std::next(iter, 6));
    policy.add_src_net(src_ip_addr);
    string const dst_ip_addr = *(std::next(iter, 7));
    policy.add_dst_net(dst_ip_addr);
    policy.add_service(*(std::next(iter, 8)));
    for (auto action = std::next(iter, 9); 
         action != tokenizer.end(); 
         action++) {
      policy.add_action(*action);
    }
    id_policies[acl_number] = policy;
  }
  else if ("set" == sub_cmd_str) {
    // id number set src-address|dst-address|service|log value
    auto& policy = (id_policies.find(acl_number))->second;
    for (auto set_tok = iter; set_tok != tokenizer.end(); set_tok++) {
      string const cmd = *set_tok;

      if ("src-address" == cmd) {
        string value = *(std::next(set_tok, 1));
        policy.add_src_net(value);
      } else if ("dst-address" == cmd) {
        string value = *(std::next(set_tok, 1));
        policy.add_dst_net(value);
      } else if ("service" == cmd) {
        string value = *(std::next(set_tok, 1));
        policy.add_service(value);
      } else if ("log" == cmd) {
        string value = *(std::next(set_tok, 1));
        policy.add_action(value);
      } else {
        // ignore unknown values
      }
    }
  }
  else if ("disable" == sub_cmd_str) {
    // id number disable
    auto& policy = (id_policies.find(acl_number))->second;
    policy.set_disabled(true);
  }
  else {
    string const line = to_string(std::next(iter,1), tokenizer.end());
    if (skip_print){cout << "SKIPPING: " << line << endl;}
  }
}
