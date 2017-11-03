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
#include <boost/numeric/conversion/cast.hpp>
#include <boost/program_options.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <bitset>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>

#include <iostream>

using std::cin;
using std::cout;
using std::cerr;
using std::endl;

using std::bitset;
using std::get;
using std::ifstream;
using std::string;
using std::vector;

using boost::numeric_cast;
using boost::uuids::uuid;

namespace filesystem = boost::filesystem;
namespace posix_time = boost::posix_time;
namespace program_options = boost::program_options;


boost::uuids::random_generator uuid_generator;


struct Route4_Info
{
  IPv4_Addr_with_Prefix dst_ip_net;
  IPv4_Addr rtr_ip_addr;
  std::string interface_name;
};


BOOST_FUSION_ADAPT_STRUCT(
  Route4_Info,
  (IPv4_Addr_with_Prefix, dst_ip_net)
  (IPv4_Addr, rtr_ip_addr)
  (std::string, interface_name)
  )


struct Route6_Info
{
  IPv6_Addr_with_Prefix dst_ip_net;
  IPv6_Addr rtr_ip_addr;
  std::string interface_name;
};


BOOST_FUSION_ADAPT_STRUCT(
  Route6_Info,
  (IPv6_Addr_with_Prefix, dst_ip_net)
  (IPv6_Addr, rtr_ip_addr)
  (std::string, interface_name)
  )


struct Route_Info
{
  Route_Info();
  Route_Info(Route4_Info const& other);
  Route_Info(Route6_Info const& other);

  IP_Addr_with_Prefix dst_ip_net;
  IP_Addr rtr_ip_addr;
  std::string interface_name;
};


Route_Info::
Route_Info() :
  dst_ip_net(),
  rtr_ip_addr(),
  interface_name()
{

}


Route_Info::
Route_Info(Route4_Info const& other) :
  dst_ip_net(other.dst_ip_net),
  rtr_ip_addr(other.rtr_ip_addr),
  interface_name(other.interface_name)
{

}


Route_Info::
Route_Info(Route6_Info const& other) :
  dst_ip_net(other.dst_ip_net),
  rtr_ip_addr(other.rtr_ip_addr),
  interface_name(other.interface_name)
{

}


BOOST_FUSION_ADAPT_STRUCT(
  Route_Info,
  (IP_Addr_with_Prefix, dst_ip_net)
  (IP_Addr, rtr_ip_addr)
  (std::string, interface_name)
  )


template<typename InputIterator_T>
struct Parser_ip_route_show :
  boost::spirit::qi::grammar<InputIterator_T,
                             std::vector<Route_Info>(),
                             boost::spirit::qi::ascii::blank_type>
{
  ~Parser_ip_route_show() = default;

  Parser_ip_route_show();

  boost::spirit::qi::rule<InputIterator_T,
                          std::vector<Route_Info>(),
                          boost::spirit::qi::ascii::blank_type>
  start;

  boost::spirit::qi::rule<InputIterator_T,
                          Route_Info(),
                          boost::spirit::qi::ascii::blank_type>
  ip_route;

  boost::spirit::qi::rule<InputIterator_T,
                          Route4_Info(),
                          boost::spirit::qi::ascii::blank_type>
  ipv4_route;

  boost::spirit::qi::rule<InputIterator_T,
                          Route6_Info(),
                          boost::spirit::qi::ascii::blank_type>
  ipv6_route;

  boost::spirit::qi::rule<InputIterator_T,
                          IPv4_Addr_with_Prefix(),
                          boost::spirit::qi::ascii::blank_type>
  dst_ipv4_net;

  boost::spirit::qi::rule<InputIterator_T,
                          IPv6_Addr_with_Prefix(),
                          boost::spirit::qi::ascii::blank_type>
  dst_ipv6_net;

  boost::spirit::qi::rule<InputIterator_T,
                          IPv4_Addr(),
                          boost::spirit::qi::ascii::blank_type>
  rtr_ipv4_addr;

  boost::spirit::qi::rule<InputIterator_T,
                          IPv6_Addr(),
                          boost::spirit::qi::ascii::blank_type>
  rtr_ipv6_addr;

  boost::spirit::qi::rule<InputIterator_T,
                          std::string(),
                          boost::spirit::qi::ascii::blank_type>
  interface_name;

  Parser_IPv4_Addr_with_CIDR<InputIterator_T>
  ipv4_addr_with_cidr;

  Parser_IPv6_Addr_with_CIDR<InputIterator_T>
  ipv6_addr_with_cidr;

  Parser_IPv4_Addr<InputIterator_T>
  ipv4_addr;

  Parser_IPv6_Addr<InputIterator_T>
  ipv6_addr;

  boost::spirit::qi::rule<InputIterator_T, std::string()>
  token;
};


template<typename InputIterator_T>
Parser_ip_route_show<InputIterator_T>::
Parser_ip_route_show() :
  Parser_ip_route_show::base_type(start),
  start(),
  ip_route(),
  ipv4_route(),
  ipv6_route(),
  dst_ipv4_net(),
  dst_ipv6_net(),
  rtr_ipv4_addr(),
  rtr_ipv6_addr(),
  interface_name(),
  ipv4_addr_with_cidr(),
  ipv6_addr_with_cidr(),
  ipv4_addr(),
  ipv6_addr(),
  token()
{
  namespace qi = boost::spirit::qi;

  start
    = (*ip_route >> *qi::eol)
    ;

  ip_route
    = qi::hold[ipv4_route]
    | qi::hold[ipv6_route]
    ;

  ipv4_route
    = qi::hold[dst_ipv4_net >>
               rtr_ipv4_addr >>
               interface_name >>
               qi::omit[*token] >>
               qi::eol]
    ;

  ipv6_route
    = qi::hold[dst_ipv6_net >>
               rtr_ipv6_addr >>
               interface_name >>
               qi::omit[*token] >>
               qi::eol]
    ;

  dst_ipv4_net
    %= (qi::lit("default")
       [qi::_val =
        boost::phoenix::construct<IPv4_Addr_with_Prefix>
        (IPv4_Addr::from_string("0.0.0.0"), 0)])
    |  (ipv4_addr_with_cidr)
    |  (ipv4_addr[qi::_val = ipv4_addr_with_prefix_ctor_(qi::_1)])
    ;

  dst_ipv6_net
    %= (qi::lit("default")
       [qi::_val =
        boost::phoenix::construct<IPv6_Addr_with_Prefix>
        (IPv6_Addr::from_string("::"), 0)])
    |  (ipv6_addr_with_cidr)
    |  (ipv6_addr[qi::_val = ipv6_addr_with_prefix_ctor_(qi::_1)])
    ;

  rtr_ipv4_addr
    %= qi::hold[qi::lit("via") >> ipv4_addr]
    |  (qi::eps
        [qi::_val =
         boost::phoenix::construct<IPv4_Addr>
         (IPv4_Addr::from_string("0.0.0.0"))])
    ;

  rtr_ipv6_addr
    %= qi::hold[qi::lit("via") >> ipv6_addr]
    |  (qi::eps
        [qi::_val =
         boost::phoenix::construct<IPv6_Addr>
         (IPv6_Addr::from_string("::"))])
    ;

  interface_name
    = (qi::lit("dev") >> token)
    ;

  token
    = +(qi::ascii::graph)
    ;
}


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
      ("tool-run-metadata",
        "Insert data into tool_run tables instead of device tables.")
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
      cerr << "Import output from 'ip route show'." << endl
           << endl
           << "Usage: " << PROGRAM_NAME << " [options] {input-file}" << endl
           << visible_opts_desc << endl;
      return 0;
    }

    if (opts.count("version")) {
      cerr << "nmdb-import-ip-route-show (Netmeld)" << endl;
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

    string const command_line = "ip route show";

    string const db_name = opts.at("db-name").as<string>();
    pqxx::connection db{string("dbname=") + db_name};
    db_prepare_common(db);


    ifstream input_file(input_file_path.string());
    input_file.unsetf(std::ios::skipws);  // disable skipping whitespace

    Parser_ip_route_show<boost::spirit::istream_iterator> const
      parser_ip_route_show;

    vector<Route_Info> results;

    if (true) {
      posix_time::ptime const execute_time_lower =
        posix_time::microsec_clock::universal_time();

      boost::spirit::istream_iterator i(input_file), e;

      bool const parse_success =
        qi::phrase_parse(i, e, parser_ip_route_show, qi::ascii::blank, results);

      posix_time::ptime const execute_time_upper =
        posix_time::microsec_clock::universal_time();

      if ((!parse_success) || (i != e)) {
        cerr << "parse error" << endl;
        for (size_t count = 0; (count < 20) && (i != e); ++count, ++i) {
          cerr << *i;
        }
        cerr << endl;
      }
      else if (opts.count("tool-run-metadata")) {
        pqxx::transaction<> t{db};

        for (auto route : results) {
          t.prepared("insert_tool_run_ip_route")
            (tool_run_id)
            (route.interface_name)
            (route.dst_ip_net)
            (route.rtr_ip_addr)
            .exec();
        }

        t.commit();
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

        for (auto route : results) {
          t.prepared("insert_raw_ip_addr")
            (tool_run_id)
            (route.rtr_ip_addr)
            ()  // NULL: Don't know if router is_responding.
            .exec();

          t.prepared("insert_raw_device_ip_route")
            (tool_run_id)
            (device_id)
            (route.interface_name)
            (route.dst_ip_net)
            (route.rtr_ip_addr)
            .exec();
        }

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
