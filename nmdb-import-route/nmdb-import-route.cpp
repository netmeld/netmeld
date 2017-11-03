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
  IPv4_Addr dst_ip_net;
  IPv4_Addr rtr_ip_addr;
  IPv4_Addr dst_ip_mask;
  std::string interface_name;
};


BOOST_FUSION_ADAPT_STRUCT(
  Route4_Info,
  (IPv4_Addr, dst_ip_net)
  (IPv4_Addr, rtr_ip_addr)
  (IPv4_Addr, dst_ip_mask)
  (std::string, interface_name)
  )


struct Route6_Info
{
  IPv6_Addr dst_ip_net;
  uint16_t dst_ip_cidr;
  IPv6_Addr rtr_ip_addr;
  std::string interface_name;
};


BOOST_FUSION_ADAPT_STRUCT(
  Route6_Info,
  (IPv6_Addr, dst_ip_net)
  (uint16_t, dst_ip_cidr)
  (IPv6_Addr, rtr_ip_addr)
  (std::string, interface_name)
  )


struct Route_Info
{
  Route_Info();
  Route_Info(Route4_Info const& route4_info);
  Route_Info(Route6_Info const& route6_info);

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
Route_Info(Route4_Info const& route4_info) :
  dst_ip_net(IPv4_Addr_with_Prefix(route4_info.dst_ip_net,
                                   route4_info.dst_ip_mask)),
  rtr_ip_addr(route4_info.rtr_ip_addr),
  interface_name(route4_info.interface_name)
{

}


Route_Info::
Route_Info(Route6_Info const& route6_info) :
  dst_ip_net(IPv6_Addr_with_Prefix(route6_info.dst_ip_net,
                                   route6_info.dst_ip_cidr)),
  rtr_ip_addr(route6_info.rtr_ip_addr),
  interface_name(route6_info.interface_name)
{

}


template<typename InputIterator_T>
struct Parser_route :
  boost::spirit::qi::grammar<InputIterator_T,
                             std::vector<Route_Info>(),
                             boost::spirit::qi::ascii::blank_type>
{
  ~Parser_route() = default;

  Parser_route();

  boost::spirit::qi::rule<InputIterator_T,
                          std::vector<Route_Info>(),
                          boost::spirit::qi::ascii::blank_type>
  start;

  boost::spirit::qi::rule<InputIterator_T,
                          std::vector<Route_Info>(),
                          boost::spirit::qi::ascii::blank_type>
  start_linux_ipv4;

  boost::spirit::qi::rule<InputIterator_T,
                          std::vector<Route_Info>(),
                          boost::spirit::qi::ascii::blank_type>
  start_linux_ipv6;

  boost::spirit::qi::rule<InputIterator_T,
                          Route4_Info(),
                          boost::spirit::qi::ascii::blank_type>
  linux_ipv4_route;

  boost::spirit::qi::rule<InputIterator_T,
                          Route6_Info(),
                          boost::spirit::qi::ascii::blank_type>
  linux_ipv6_route;

  boost::spirit::qi::rule<InputIterator_T,
                          IPv4_Addr(),
                          boost::spirit::qi::ascii::blank_type>
  linux_special_ipv4_addr;

  Parser_IPv4_Addr<InputIterator_T>
  ipv4_addr;

  Parser_IPv6_Addr<InputIterator_T>
  ipv6_addr;

  boost::spirit::qi::rule<InputIterator_T, std::string()>
  token;
};


template<typename InputIterator_T>
Parser_route<InputIterator_T>::
Parser_route() :
  Parser_route::base_type(start),
  start(),
  start_linux_ipv4(),
  start_linux_ipv6(),
  linux_ipv4_route(),
  linux_ipv6_route(),
  linux_special_ipv4_addr(),
  ipv4_addr(),
  ipv6_addr(),
  token()
{
  namespace qi = boost::spirit::qi;

  start
    = qi::hold[start_linux_ipv4]
    | qi::hold[start_linux_ipv6]
    ;

  start_linux_ipv4
    = (qi::lexeme["Kernel IP routing table"] >> qi::eol >>
       qi::lexeme["Destination"] >> qi::lexeme["Gateway"] >>
       qi::lexeme["Genmask"] >> qi::lexeme["Flags"] >>
       qi::omit[token] >> qi::omit[token] >> qi::omit[token] >>
       qi::lexeme["Iface"] >>
       qi::eol >>
       *(linux_ipv4_route))
    ;

  start_linux_ipv6
    = (qi::lexeme["Kernel IPv6 routing table"] >> qi::eol >>
       qi::lexeme["Destination"] >> qi::lexeme["Next Hop"] >>
       qi::lexeme["Flag"] >> qi::lexeme["Met"] >> qi::lexeme["Ref"] >>
       qi::lexeme["Use"] >> qi::lexeme["If"] >>
       qi::eol >>
       *(linux_ipv6_route))
    ;

  linux_ipv4_route
    = (linux_special_ipv4_addr >> linux_special_ipv4_addr >> ipv4_addr >>
       qi::omit[token] >> qi::omit[qi::uint_] >>
       qi::omit[+qi::ulong_long] >>
       token >>
       qi::eol)
    ;

  linux_ipv6_route
    = (ipv6_addr >> qi::lit('/') >> qi::ushort_ >> ipv6_addr >>
       qi::omit[token] >> qi::omit[qi::int_] >>
       qi::omit[+qi::ulong_long] >>
       token >>
       qi::eol)
    ;

  linux_special_ipv4_addr
    %= ((qi::lit("default") | qi::lit("*"))
        [qi::_val =
         boost::phoenix::construct<IPv4_Addr>
         (IPv4_Addr::from_string("0.0.0.0"))])
    |  (ipv4_addr)
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
      cerr << "Import output from 'route -n' or 'netstat -nr'." << endl 
		   << endl 
           << "Usage: " << PROGRAM_NAME << " [options] {input-file}" << endl
           << visible_opts_desc << endl;
      return 0;
    }

    if (opts.count("version")) {
      cerr << "nmdb-import-route (Netmeld)" << endl;
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

    string const command_line;  // Empty, a couple of different ways

    string const db_name = opts.at("db-name").as<string>();
    pqxx::connection db{string("dbname=") + db_name};
    db_prepare_common(db);


    ifstream input_file(input_file_path.string());
    input_file.unsetf(std::ios::skipws);  // disable skipping whitespace

    Parser_route<boost::spirit::istream_iterator> const parser_route;
    vector<Route_Info> results;

    if (true) {
      posix_time::ptime const execute_time_lower =
        posix_time::microsec_clock::universal_time();

      boost::spirit::istream_iterator i(input_file), e;

      bool const parse_success =
        qi::phrase_parse(i, e, parser_route, qi::ascii::blank, results);

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
            (device_type).exec();
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
