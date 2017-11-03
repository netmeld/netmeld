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


struct Interface_Info
{
  std::string name;
  std::string flags;
  uint32_t mtu;
  std::string media_type;
  MAC_Addr mac_addr;
  std::vector<IP_Addr_with_Prefix> ip_addrs;
};


BOOST_FUSION_ADAPT_STRUCT(
  Interface_Info,
  (std::string, name)
  (std::string, flags)
  (uint32_t, mtu)
  (std::string, media_type)
  (MAC_Addr, mac_addr)
  (std::vector<IP_Addr_with_Prefix>, ip_addrs)
  )


template<typename InputIterator_T>
struct Parser_ip_addr_show :
  boost::spirit::qi::grammar<InputIterator_T,
                             std::vector<Interface_Info>(),
                             boost::spirit::qi::ascii::blank_type>
{
  ~Parser_ip_addr_show() = default;

  Parser_ip_addr_show();

  boost::spirit::qi::rule<InputIterator_T,
                          std::vector<Interface_Info>(),
                          boost::spirit::qi::ascii::blank_type>
  start;

  boost::spirit::qi::rule<InputIterator_T,
                          Interface_Info(),
                          boost::spirit::qi::ascii::blank_type>
  linux_iface;

  Parser_MAC_Addr<InputIterator_T>
  mac_addr;

  boost::spirit::qi::rule<InputIterator_T,
                          IP_Addr_with_Prefix(),
                          boost::spirit::qi::ascii::blank_type>
  inet_addr;

  boost::spirit::qi::rule<InputIterator_T,
                          IPv4_Addr_with_Prefix(),
                          boost::spirit::qi::ascii::blank_type>
  inet4_addr;

  boost::spirit::qi::rule<InputIterator_T,
                          IPv6_Addr_with_Prefix(),
                          boost::spirit::qi::ascii::blank_type>
  inet6_addr;

  Parser_IPv4_Addr<InputIterator_T>
  ipv4_addr;

  Parser_IPv6_Addr<InputIterator_T>
  ipv6_addr;

  Parser_IPv4_Addr_with_CIDR<InputIterator_T>
  ipv4_addr_with_cidr;

  Parser_IPv6_Addr_with_CIDR<InputIterator_T>
  ipv6_addr_with_cidr;

  boost::spirit::qi::rule<InputIterator_T, std::string()>
  iface_name;

  boost::spirit::qi::rule<InputIterator_T, std::string()>
  token;
};


template<typename InputIterator_T>
Parser_ip_addr_show<InputIterator_T>::
Parser_ip_addr_show() :
  Parser_ip_addr_show::base_type(start),
  start(),
  linux_iface(),
  mac_addr(),
  inet_addr(),
  inet4_addr(),
  inet6_addr(),
  ipv4_addr(),
  ipv6_addr(),
  ipv4_addr_with_cidr(),
  ipv6_addr_with_cidr(),
  iface_name(),
  token()
{
  namespace qi = boost::spirit::qi;

  start
    = (*linux_iface >>
       *qi::eol)
    ;

  linux_iface
    = ((qi::omit[qi::ushort_] >> qi::lit(':') >>
        iface_name >> qi::lit(':') >>
        token >>
        qi::lit("mtu") >> qi::uint_ >>
        qi::omit[*token] >>
        qi::eol) >>

       (qi::lit("link/") >> token >> mac_addr >>
        -(qi::lit("brd") >> qi::omit[mac_addr] >> -(qi::omit[+token])) >> //TODO process ns
        qi::eol) >>

       (*inet_addr)
      )
    ;

  inet_addr
    = qi::hold[inet6_addr]
    | qi::hold[inet4_addr]
    ;

  inet4_addr
    = (qi::lit("inet") >> ipv4_addr_with_cidr >>
       -(qi::lit("brd") >> qi::omit[ipv4_addr]) >>
       qi::lit("scope") >> qi::omit[+token] >>
       qi::eol >>
       -(qi::lit("valid_lft") >> qi::omit[+token] >>
         qi::eol))
    ;

  inet6_addr
    = (qi::lit("inet6") >> ipv6_addr_with_cidr >>
       qi::lit("scope") >> qi::omit[+token] >>
       qi::eol >>
       -(qi::lit("valid_lft") >> qi::omit[+token] >>
         qi::eol))
    ;

  iface_name
    = +(qi::ascii::alnum | qi::ascii::char_("-_.@"))
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
      cerr << "Import output from 'ip addr show'." << endl
           << endl
           << "Usage: " << PROGRAM_NAME << " [options] {input-file}" << endl
           << visible_opts_desc << endl;
      return 0;
    }

    if (opts.count("version")) {
      cerr << "nmdb-import-ip-addr-show (Netmeld)" << endl;
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

    string const command_line = "ip addr show";

    string const db_name = opts.at("db-name").as<string>();
    pqxx::connection db{string("dbname=") + db_name};
    db_prepare_common(db);


    ifstream input_file(input_file_path.string());
    input_file.unsetf(std::ios::skipws);  // disable skipping whitespace

    Parser_ip_addr_show<boost::spirit::istream_iterator> const
      parser_ip_addr_show;

    vector<Interface_Info> results;

    if (true) {
      posix_time::ptime const execute_time_lower =
        posix_time::microsec_clock::universal_time();

      boost::spirit::istream_iterator i(input_file), e;

      bool const parse_success =
        qi::phrase_parse(i, e, parser_ip_addr_show, qi::ascii::blank, results);

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

        for (auto const& iface : results) {
          if ((6 <= iface.mac_addr.size()) &&
              ("loopback" != iface.media_type)) {
            string const mac_addr_str =
              (format("%02x:%02x:%02x:%02x:%02x:%02x")
               % static_cast<uint16_t>(iface.mac_addr[0])
               % static_cast<uint16_t>(iface.mac_addr[1])
               % static_cast<uint16_t>(iface.mac_addr[2])
               % static_cast<uint16_t>(iface.mac_addr[3])
               % static_cast<uint16_t>(iface.mac_addr[4])
               % static_cast<uint16_t>(iface.mac_addr[5])).str();

            t.prepared("insert_tool_run_interface")
              (tool_run_id)
              (iface.name)
              (iface.media_type)
              (iface.flags.find(",UP") != string::npos)
              .exec();

            t.prepared("insert_tool_run_mac_addr")
              (tool_run_id)
              (iface.name)
              (mac_addr_str)
              .exec();

            for (auto const& ip_addr : iface.ip_addrs) {
              t.prepared("insert_tool_run_ip_addr")
                (tool_run_id)
                (iface.name)
                (ip_addr)
                .exec();
            }
          }
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
            (device_type).exec();
        }

        for (auto const& iface : results) {
          bool const is_up =
            (iface.flags.find(",UP") != string::npos);

          if ((6 <= iface.mac_addr.size()) &&
              ("loopback" != iface.media_type)) {
            string const mac_addr_str =
              (format("%02x:%02x:%02x:%02x:%02x:%02x")
               % static_cast<uint16_t>(iface.mac_addr[0])
               % static_cast<uint16_t>(iface.mac_addr[1])
               % static_cast<uint16_t>(iface.mac_addr[2])
               % static_cast<uint16_t>(iface.mac_addr[3])
               % static_cast<uint16_t>(iface.mac_addr[4])
               % static_cast<uint16_t>(iface.mac_addr[5])).str();

            t.prepared("insert_raw_device_interface")
              (tool_run_id)
              (device_id)
              (iface.name)
              (iface.media_type)
              (is_up)
              .exec();

            t.prepared("insert_raw_mac_addr")
              (tool_run_id)
              (mac_addr_str)
              (is_up)  // is_responding
              .exec();

            t.prepared("insert_raw_device_mac_addr")
              (tool_run_id)
              (device_id)
              (iface.name)
              (mac_addr_str)
              .exec();

            for (auto const& ip_addr : iface.ip_addrs) {
              string const ip_addr_str = ip_addr.to_string();

              t.prepared("insert_raw_ip_addr")
                (tool_run_id)
                (ip_addr_str)
                (is_up)  // is_responding
                .exec();

              t.prepared("insert_raw_ip_net")
                (tool_run_id)
                (ip_addr_str)
                ()  // NULL: Don't have a description.
                .exec();

              t.prepared("insert_raw_mac_addr_ip_addr")
                (tool_run_id)
                (mac_addr_str)
                (ip_addr_str)
                .exec();

              t.prepared("insert_raw_device_ip_addr")
                (tool_run_id)
                (device_id)
                (iface.name)
                (ip_addr_str)
                .exec();
            }
          }
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
