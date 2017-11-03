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
#include <boost/spirit/repository/include/qi_confix.hpp>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
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
namespace program_options = boost::program_options;


boost::uuids::random_generator uuid_generator;


struct Juniper_Config
{
  Juniper_Config();
  Juniper_Config(std::map<std::string, Juniper_Config> node);

  std::map<std::string, Juniper_Config> children;
};

Juniper_Config::
Juniper_Config()
{

}

Juniper_Config::
Juniper_Config(std::map<std::string, Juniper_Config> node) :
  children(node)
{

}


using Juniper_Config_Nodes = std::map<std::string, Juniper_Config>;


BOOST_FUSION_ADAPT_STRUCT(
  Juniper_Config,
  (Juniper_Config_Nodes, children)
  )


template<typename InputIterator_T>
struct Parser_Juniper_Config :
  boost::spirit::qi::grammar<InputIterator_T,
                             Juniper_Config(),
                             boost::spirit::qi::ascii::blank_type>
{
  ~Parser_Juniper_Config() = default;

  Parser_Juniper_Config();

  boost::spirit::qi::rule<InputIterator_T,
                          Juniper_Config(),
                          boost::spirit::qi::ascii::blank_type>
  start;

  boost::spirit::qi::rule<InputIterator_T,
                          std::map<std::string, Juniper_Config>(),
                          boost::spirit::qi::ascii::blank_type>
  config_nodes;

  boost::spirit::qi::rule<InputIterator_T,
                          std::pair<std::string, Juniper_Config>(),
                          boost::spirit::qi::ascii::blank_type>
  config_node;

  boost::spirit::qi::rule<InputIterator_T,
                          std::string(),
                          boost::spirit::qi::ascii::blank_type>
  config_line;


  boost::spirit::qi::rule<InputIterator_T,
                          std::pair<std::string, Juniper_Config>(),
                          boost::spirit::qi::ascii::blank_type>
  config_block;


  boost::spirit::qi::rule<InputIterator_T,
                          std::string()>
  config_string;

  boost::spirit::qi::rule<InputIterator_T,
                          std::string()>
  config_token;

  boost::spirit::qi::rule<InputIterator_T,
                          std::string()>
  config_quote;

  boost::spirit::qi::rule<InputIterator_T,
                          std::string()>
  config_quote_content_fragment;

  boost::spirit::qi::rule<InputIterator_T,
                          boost::spirit::qi::ascii::blank_type>
  comment;
};


template<typename InputIterator_T>
Parser_Juniper_Config<InputIterator_T>::
Parser_Juniper_Config() :
  Parser_Juniper_Config::base_type(start),
  start(),
  config_nodes(),
  config_node(),
  config_line(),
  config_block(),
  config_string(),
  config_token(),
  config_quote(),
  config_quote_content_fragment(),
  comment()
{
  namespace qi = boost::spirit::qi;

  start
    = (config_nodes)
    ;

  config_nodes
    = *(config_node)
    ;

  config_node
    = (config_line
       [qi::_val =
        boost::phoenix::construct< std::pair<std::string, Juniper_Config> >
        (qi::_1, boost::phoenix::construct<Juniper_Config>())])
    | (config_block[qi::_val = qi::_1])
    ;

  config_line
    = (config_string >> qi::lit(';') >> -(comment) >> qi::eol)
    | (comment >> qi::eol)
    | (qi::eol)
    | (qi::lit('{') >> "master" >> ":" >> qi::lexeme[+qi::ascii::digit] >> qi::lit('}') >> qi::eol)
    ;

  config_block
    = (config_string >> qi::lit('{') >> -(comment) >> qi::eol >>
       config_nodes >>
       qi::lit('}') >> -(comment) >> -(qi::eol))
    ;

  config_string
    = (qi::lexeme
       [(config_token | config_quote) >>
        *(qi::hold[+qi::ascii::blank >> (config_token | config_quote)])])
    ;

  config_token
    = (qi::lexeme[+(qi::ascii::graph - qi::ascii::char_("\"\\;{}#"))])
    ;

  config_quote
    = (qi::lexeme
       [qi::ascii::char_('"') >>
        *(config_quote_content_fragment) >>
        qi::ascii::char_('"')])
    ;

  config_quote_content_fragment
    = (qi::lexeme[+(qi::ascii::print - qi::ascii::char_("\"\\"))])
    | (qi::lexeme[qi::ascii::char_("\\") >> qi::ascii::graph])
    ;

  comment
    = (+qi::lit('#') >> qi::lexeme[*(qi::ascii::print)])
    | (confix("/*", "*/")[*(qi::char_ - "*/")])
    ;
}


void
process_interfaces(pqxx::transaction<>& t,
                   uuid const& tool_run_id, string const& device_id,
                   Juniper_Config const& interfaces);

void
process_address_book(pqxx::transaction<>& t,
                     uuid const& tool_run_id, string const& device_id,
                     Juniper_Config const& address_book);

void
process_applications(pqxx::transaction<>& t,
                     uuid const& tool_run_id, string const& device_id,
                     Juniper_Config const& applications);

void
process_acls(pqxx::transaction<>& t,
             uuid const& tool_run_id, string const& device_id,
             Juniper_Config const& juniper_config);

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
      cerr << "Import Juniper's Junos configuration file." << endl
           << endl
           << "Usage: " << PROGRAM_NAME << " [options] {input-file}" << endl
           << visible_opts_desc << endl;
      return 0;
    }

    if (opts.count("version")) {
      cerr << "nmdb-import-juniper-junos (Netmeld)" << endl;
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

    string const command_line = "show configuration";

    string const db_name = opts.at("db-name").as<string>();
    pqxx::connection db{string("dbname=") + db_name};
    db_prepare_common(db);


    ifstream input_file(input_file_path.string());
    input_file.unsetf(std::ios::skipws);  // disable skipping whitespace

    Parser_Juniper_Config<boost::spirit::istream_iterator> const
      parser_juniper_config;

    Juniper_Config juniper_config;

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

        // groups { node X { interfaces { ... } } }.
        if (juniper_config.children.count("groups")) {
          for (auto const& node :
                 juniper_config.children.at("groups").children) {
            string const node_name = get<0>(node);
            if (get<1>(node).children.count("interfaces")) {
              process_interfaces(t, tool_run_id, device_id,
                                 get<1>(node).children.at("interfaces"));
            }
          }
        }

        // interfaces { ... }.
        if (juniper_config.children.count("interfaces")) {
          process_interfaces(t, tool_run_id, device_id,
                             juniper_config.children.at("interfaces"));
        }

        // ACLs
        //process_acls(t, tool_run_id, device_id, juniper_config);

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


void
process_interfaces(pqxx::transaction<>& t,
                   uuid const& tool_run_id, string const& device_id,
                   Juniper_Config const& interfaces)
{
  for (auto const& interface : interfaces.children) {
    string const interface_name = get<0>(interface);
    //cout << interface_name << endl;

    t.prepared("insert_raw_device_interface")
      (tool_run_id)
      (device_id)
      (interface_name)
      ("Ethernet") // Assume ethernet
      (true)       // Assume is up
      .exec();

    for (auto const& unit : get<1>(interface).children) {
      if (get<1>(unit).children.count("family inet")) {
        for (auto const& addr :
               get<1>(unit).children.at("family inet").children) {
          string ip_addr_with_cidr = get<0>(addr);
          if (0 == ip_addr_with_cidr.find("address ")) {
            ip_addr_with_cidr.erase(0, string("address ").size());
            //cout << ip_addr_with_cidr << endl;

            t.prepared("insert_raw_ip_addr")
              (tool_run_id)
              (ip_addr_with_cidr)
              (true)
              .exec();

            t.prepared("insert_raw_ip_net")
              (tool_run_id)
              (ip_addr_with_cidr)
              ()  // NULL: Don't have a description.
              .exec();

            t.prepared("insert_raw_device_ip_addr")
              (tool_run_id)
              (device_id)
              (interface_name)
              (ip_addr_with_cidr)
              .exec();
          }
        }
      }

      if (get<1>(unit).children.count("family ethernet-switching")) {
        /*
        for (auto const& addr :
               get<1>(unit).children.at("family ethernet-switching").children) {
        }
        */
      }
    }

    //cout << endl;
  }
}


#if false  // disable this testing/prototype code

void
process_address_book
(pqxx::transaction<>& t,
 uuid const& tool_run_id, string const& device_id,
 Juniper_Config const& address_book)
{
  // Lambda to parse address line into a vector of name and optionally ip/cidr
  //   address NAME
  //   address NAME IP/CIDR
  auto insert_address = [](pqxx::transaction<>& t,
                           uuid const& tool_run_id,
                           string const& device_id, 
                           string const& address_line)
  {
    // Only process address lines
    if (0 == address_line.find("address ") ||
        0 == address_line.find("address-set ")) {
      vector<string> address_parsed;
    
      boost::algorithm::split(address_parsed, address_line,
                              boost::is_any_of(" "), 
                              boost::token_compress_on);

      string const& ip_net_set = address_parsed.at(1);

      if (true) {
        t.prepared("insert_raw_device_acl_ip_net_set")
          (tool_run_id)
          (device_id)
          (ip_net_set)
          .exec();
      }

      if (true && address_parsed.size() > 2) {
        string const& ip_net = address_parsed.at(2);
        t.prepared("insert_raw_device_acl_ip_net")
          (tool_run_id)
          (device_id)
          (ip_net_set)
          (ip_net)
          .exec();
      }

      return true; // parsed something
    }

    return false; // did not parse anything
  };

  // address-book NAME { ... }
  for (auto const& property : address_book.children) {
    auto address_type = get<0>(property);

    // address-set NAME { ... }
    if (0 == address_type.find("address-set ")) {
      insert_address(t, tool_run_id, device_id, address_type);

      for (auto const& address : get<1>(property).children) {
        insert_address(t, tool_run_id, device_id, get<0>(address));
      }
    }

    // address NAME IP/CIDR
    if (0 == address_type.find("address ")) {
      insert_address(t, tool_run_id, device_id, address_type);
    }
  }
}

void
process_applications
(pqxx::transaction<>& t,
 uuid const& tool_run_id, string const& device_id,
 Juniper_Config const& applications)
{
  // application NAME { ... }
  for (auto const& application : applications.children) {
    // Get application name
    string port_range_set = get<0>(application);
    port_range_set.erase(0, string("application ").size());

    // Get applicaiton protocol and destination port
    string protocol, port_range;
    for (auto const& property_x : get<1>(application).children) {
      string property = get<0>(property_x);
      if (0 == property.find("protocol ")) {
        protocol = property.erase(0, string("protocol ").size());
      }
      else if (0 == property.find("destination-port ")) {
        port_range = property.erase(0, string("destination-port ").size());

        // convert port value to a range for DB insert
        vector<string> ports;
        boost::algorithm::split(ports, port_range,
                                boost::is_any_of("-"),
                                boost::token_compress_on);

        port_range = "[" + ports[0] + ",";
        if (ports.size() == 2) {
          port_range += ports[1] + "]";
        }
        else {
          port_range += ports[0] + "]";
        }
      }
    }

    if (true) {
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
        (port_range)
        .exec();
    }
  }
}

void
process_acls
(pqxx::transaction<>& t,
 uuid const& tool_run_id, string const& device_id,
 Juniper_Config const& juniper_config)
{
  if (true) {
    t.prepared("insert_raw_device_acl_ip_net_set")
      (tool_run_id)
      (device_id)
      ("any")
      .exec();

    t.prepared("insert_raw_device_acl_ip_net")
      (tool_run_id)
      (device_id)
      ("any")
      ("0.0.0.0/0")  // IPv4 "any" network.
      .exec();

    t.prepared("insert_raw_device_acl_ip_net")
      (tool_run_id)
      (device_id)
      ("any")
      ("::/0")  // IPv6 "any" network.
      .exec();
  }

  if (true) {
    t.prepared("insert_raw_device_acl_action_set")
      (tool_run_id)
      (device_id)
      ("permit")
      .exec();

    t.prepared("insert_raw_device_acl_action")
      (tool_run_id)
      (device_id)
      ("permit")
      ("permit")
      .exec();

    t.prepared("insert_raw_device_acl_action_set")
      (tool_run_id)
      (device_id)
      ("deny")
      .exec();

    t.prepared("insert_raw_device_acl_action")
      (tool_run_id)
      (device_id)
      ("deny")
      ("deny")
      .exec();

    t.prepared("insert_raw_device_acl_action_set")
      (tool_run_id)
      (device_id)
      ("log")
      .exec();

    t.prepared("insert_raw_device_acl_action")
      (tool_run_id)
      (device_id)
      ("log")
      ("log")
      .exec();

    t.prepared("insert_raw_device_acl_action_set")
      (tool_run_id)
      (device_id)
      ("count")
      .exec();

    t.prepared("insert_raw_device_acl_action")
      (tool_run_id)
      (device_id)
      ("count")
      ("count")
      .exec();
  }

  if (true) {
    t.prepared("insert_raw_device_acl_port_range_set")
      (tool_run_id)
      (device_id)
      ("any") // port_range_set
      .exec();
    
    t.prepared("insert_raw_device_acl_port_range")
      (tool_run_id)
      (device_id)
      ("any") // port_range_set
      ("any") // protocol
      ("[0,65535]") // port_range
      .exec();
  }

  if (true) {
    // Mapping of "junos-*" default applications for insert
    std::map<string, std::map<string, string>> junos_default_apps;
    junos_default_apps["junos-ssh"]["tcp"] = "[22,22]";
    // TODO 20JUN17 Insert more "junos-*" default applications
    
    for (auto const& junos_default_app : junos_default_apps) {
      string const port_range_set = get<0>(junos_default_app);

      t.prepared("insert_raw_device_acl_port_range_set")
        (tool_run_id)
        (device_id)
        (port_range_set)
        .exec();

      for (auto const& protocol_info : get<1>(junos_default_app)) {
        string const protocol = get<0>(protocol_info);
        string const port_range = get<1>(protocol_info);

        t.prepared("insert_raw_device_acl_port_range")
          (tool_run_id)
          (device_id)
          (port_range_set)
          (protocol)
          (port_range)
          .exec();
      }
    }
  }

  // applications { ... }
  if (juniper_config.children.count("applications")) {
    process_applications(t, tool_run_id, device_id,
                         juniper_config.children.at("applications"));
  }

  // security { ... }
  if (juniper_config.children.count("security")) {
    Juniper_Config const& security = juniper_config.children.at("security");

    // zones { security-zone NAME { ... } } 
    if (security.children.count("zones")) {
      for (auto const& zone : security.children.at("zones").children) {
        Juniper_Config const& zone_contents = get<1>(zone);

        // interfaces { ... }
        if (zone_contents.children.count("interfaces")) {
          // TODO: 20JUN16 - Process interfaces
        }

        // address-book { ... }
        if (zone_contents.children.count("address-book")) {
          process_address_book
            (t, tool_run_id, device_id,
             zone_contents.children.at("address-book"));
        }
      }
    }

    // address-book { ... }
    if (security.children.count("address-book")) {
      process_address_book
        (t, tool_run_id, device_id,
         security.children.at("address-book"));
    }

    // policies { from-zone NAME to-zone NAME { policy NAME { ... } } }
    if (security.children.count("policies")) {
      // from-zone NAME to-zone NAME { ... }
      for (auto const& from_zone_to_zone :
             security.children.at("policies").children) {
        string acl_set = get<0>(from_zone_to_zone);
        size_t acl_number = 0;

        // policy NAME { ... }
        for (auto const& policy : get<1>(from_zone_to_zone).children) {
          ++acl_number;

          string policy_name = get<0>(policy);

          vector<string> src_addrs;
          vector<string> dst_addrs;
          vector<string> applications;
          vector<string> actions;

          if (get<1>(policy).children.count("match")) {
            for (auto const& property_x :
                   get<1>(policy).children.at("match").children) {
              string property = get<0>(property_x);

              if (0 == property.find("application ")) {
                property.erase(0, string("application ").size());
                applications.push_back(property);

                // TODO 20JUN16 how to handle multiple applications
              }
              else if (0 == property.find("source-address ")) {
                property.erase(0, string("source-address ").size());
                vector<string> ips;
                boost::algorithm::split(ips, property, 
                                        boost::is_any_of(" ]["), 
                                        boost::token_compress_on);
                for (auto const& ip : ips) {
                  if (ip.size() > 0) {
                    src_addrs.push_back(ip);
                  }
                }
              }
              else if (0 == property.find("destination-address ")) {
                property.erase(0, string("destination-address ").size());
                vector<string> ips;
                boost::algorithm::split(ips, property, 
                                        boost::is_any_of(" ]["), 
                                        boost::token_compress_on);
                for (auto const& ip : ips) {
                  if (ip.size() > 0) {
                    dst_addrs.push_back(ip);
                  }
                }
              }
            }
          } // end match

          if (get<1>(policy).children.count("then")) {
            for (auto const& action :
                   get<1>(policy).children.at("then").children) {
              string action_name = get<0>(action);
              actions.push_back(action_name);
            }
          }

          for (auto const& src_addr : src_addrs) {
            for (auto const& dst_addr : dst_addrs) {
              for (auto const& application : applications) {
                for (auto const& action : actions) {
                  t.prepared("insert_raw_device_acl")
                    (tool_run_id)
                    (device_id)
                    (acl_set)
                    (acl_number)
                    (action)
                    (src_addr)
                    (dst_addr)
                    ("any")
                    (application)
                    .exec();
                }
              }
            }
          }


          cout << "applications:" << endl;
          for (auto const& x : applications) {
            cout << "\t" << x << endl;
          }

          cout << "actions:" << endl;
          for (auto const& x : actions) {
            cout << "\t" << x << endl;
          }

          cout << "src_addrs:" << endl;
          for (auto const& x : src_addrs) {
            cout << "\t" << x << endl;
          }

          cout << "dst_addrs:" << endl;
          for (auto const& x : dst_addrs) {
            cout << "\t" << x << endl;
          }
        }
      }
    }
  }
}

#endif  // end of testing/prototype code
