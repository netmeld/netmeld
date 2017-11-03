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

#include <bitset>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>


using std::cin;
using std::cout;
using std::cerr;
using std::endl;

using std::bitset;
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


struct Interface_Info_Linux
{
  std::string name;
  MAC_Addr mac_addr;
  std::vector<IP_Addr_with_Prefix> ip_addrs;
  std::string flags;
  uint32_t mtu;
};


BOOST_FUSION_ADAPT_STRUCT(
  Interface_Info_Linux,
  (std::string, name)
  (MAC_Addr, mac_addr)
  (std::vector<IP_Addr_with_Prefix>, ip_addrs)
  (std::string, flags)
  (uint32_t, mtu)
  )


struct Interface_Info_BSD
{
  std::string name;
  std::string flags;
  uint32_t mtu;
  MAC_Addr mac_addr;
  std::vector<IP_Addr_with_Prefix> ip_addrs;
};


BOOST_FUSION_ADAPT_STRUCT(
  Interface_Info_BSD,
  (std::string, name)
  (std::string, flags)
  (uint32_t, mtu)
  (MAC_Addr, mac_addr)
  (std::vector<IP_Addr_with_Prefix>, ip_addrs)
  )


struct Interface_Info
{
  std::string name;
  MAC_Addr mac_addr;
  std::vector<IP_Addr_with_Prefix> ip_addrs;
  std::string flags;
  uint32_t mtu;

  Interface_Info() = default;
  Interface_Info(Interface_Info_Linux const& other);
  Interface_Info(Interface_Info_BSD const& other);
};


BOOST_FUSION_ADAPT_STRUCT(
  Interface_Info,
  (std::string, name)
  (MAC_Addr, mac_addr)
  (std::vector<IP_Addr_with_Prefix>, ip_addrs)
  (std::string, flags)
  (uint32_t, mtu)
  )


Interface_Info::
Interface_Info(Interface_Info_Linux const& other) :
  name(other.name),
  mac_addr(other.mac_addr),
  ip_addrs(other.ip_addrs),
  flags(other.flags),
  mtu(other.mtu)
{

}


Interface_Info::
Interface_Info(Interface_Info_BSD const& other) :
  name(other.name),
  mac_addr(other.mac_addr),
  ip_addrs(other.ip_addrs),
  flags(other.flags),
  mtu(other.mtu)
{

}


uint16_t
count_bits(uint32_t value);

uint16_t
count_bits(uint32_t value)
{
  return static_cast<uint16_t>(bitset<32>(value).count());
}

BOOST_PHOENIX_ADAPT_FUNCTION(
  uint16_t,
  count_bits_,
  count_bits,
  1)


template<typename InputIterator_T>
struct Parser_ifconfig :
  boost::spirit::qi::grammar<InputIterator_T,
                             std::vector<Interface_Info>(),
                             boost::spirit::qi::ascii::blank_type>
{
  ~Parser_ifconfig() = default;

  Parser_ifconfig();

  boost::spirit::qi::rule<InputIterator_T,
                          std::vector<Interface_Info>(),
                          boost::spirit::qi::ascii::blank_type>
  start;


  boost::spirit::qi::rule<InputIterator_T,
                          Interface_Info_BSD(),
                          boost::spirit::qi::ascii::blank_type>
  bsd_iface;

  boost::spirit::qi::rule<InputIterator_T,
                          IP_Addr_with_Prefix(),
                          boost::spirit::qi::ascii::blank_type>
  bsd_inet_addr;

  boost::spirit::qi::rule<InputIterator_T,
                          IPv4_Addr_with_Prefix(),
                          boost::spirit::qi::ascii::blank_type>
  bsd_inet4_addr;

  boost::spirit::qi::rule<InputIterator_T,
                          IPv6_Addr_with_Prefix(),
                          boost::spirit::qi::ascii::blank_type>
  bsd_inet6_addr;


  boost::spirit::qi::rule<InputIterator_T,
                          Interface_Info_Linux(),
                          boost::spirit::qi::ascii::blank_type>
  linux_iface;

  boost::spirit::qi::rule<InputIterator_T,
                          IP_Addr_with_Prefix(),
                          boost::spirit::qi::ascii::blank_type>
  linux_inet_addr;

  boost::spirit::qi::rule<InputIterator_T,
                          IPv4_Addr_with_Prefix(),
                          boost::spirit::qi::ascii::blank_type>
  linux_inet4_addr;

  boost::spirit::qi::rule<InputIterator_T,
                          IPv6_Addr_with_Prefix(),
                          boost::spirit::qi::ascii::blank_type>
  linux_inet6_addr;


  boost::spirit::qi::rule<InputIterator_T,
                          string(),
                          boost::spirit::qi::ascii::blank_type>
  linux_flags;

  Parser_MAC_Addr<InputIterator_T>
  mac_addr;

  Parser_IPv4_Addr<InputIterator_T>
  ipv4_addr;

  boost::spirit::qi::rule<InputIterator_T, uint16_t()>
  ipv4_mask;

  boost::spirit::qi::rule<InputIterator_T, uint16_t()>
  ipv4_mask_hex;

  Parser_IPv6_Addr<InputIterator_T>
  ipv6_addr;

  boost::spirit::qi::rule<InputIterator_T, std::string()>
  iface_name;

  boost::spirit::qi::rule<InputIterator_T, std::string()>
  token;
};


template<typename InputIterator_T>
Parser_ifconfig<InputIterator_T>::
Parser_ifconfig() :
  Parser_ifconfig::base_type(start),
  start(),
  bsd_iface(),
  bsd_inet_addr(),
  bsd_inet4_addr(),
  bsd_inet6_addr(),
  linux_iface(),
  linux_inet_addr(),
  linux_inet4_addr(),
  linux_inet6_addr(),
  linux_flags(),
  mac_addr(),
  ipv4_addr(),
  ipv4_mask(),
  ipv4_mask_hex(),
  ipv6_addr(),
  iface_name(),
  token()
{
  namespace qi = boost::spirit::qi;

  start
    = (+linux_iface >>
       *qi::eol)
    | (+bsd_iface >>
       *qi::eol)
    ;

  bsd_iface
    = ((iface_name >> qi::lit(':') >>
        qi::lexeme["flags="] >> token >>
        qi::lexeme["mtu"] >> qi::uint_ >>
        qi::eol) >>

       -(qi::lexeme["options="] >> qi::omit[+token] >>
         qi::eol) >>

       -(qi::lexeme["ether"] >> mac_addr >>
         qi::eol) >>

       *(bsd_inet_addr) >>

       -(qi::lexeme["nd6 options="] >> qi::omit[+token] >>
         qi::eol) >>

       -(qi::lexeme["syncpeer:"] >> qi::omit[+token] >>
         qi::eol) >>

       -(qi::lexeme["media:"] >> qi::omit[+token] >>
         qi::eol) >>

       -(qi::lexeme["status:"] >> qi::omit[+token] >>
         qi::eol)
      )
    ;

  linux_iface
    = ((token >>
        qi::lexeme["Link encap:"] >>
        qi::omit[(qi::lexeme[qi::string("Local Loopback")] | token)] >>
        -(qi::lexeme["HWaddr"] >> mac_addr) >>
        qi::eol) >>

       *(linux_inet_addr) >>

       linux_flags >>
       qi::lexeme["MTU:"] >> qi::uint_ >>

       *(qi::omit[+token] >>
         qi::eol) >>

       qi::eol
      )
    ;

  bsd_inet_addr
    = qi::hold[bsd_inet6_addr]
    | qi::hold[bsd_inet4_addr]
    ;

  bsd_inet4_addr
    = (qi::lexeme["inet"] >> ipv4_addr >>
       qi::lexeme["netmask"] >> ipv4_mask_hex >>
       -(qi::lexeme["broadcast"] >> qi::omit[ipv4_addr]) >>
       qi::eol)[qi::_val = ipv4_addr_with_prefix_ctor_(qi::_1, qi::_2)]
    ;

  bsd_inet6_addr
    = (qi::lexeme["inet6"] >>
       ipv6_addr >> -(qi::lit('%') >> qi::omit[iface_name]) >>
       qi::lexeme["prefixlen"] >> qi::uint_ >>
       -(qi::lexeme["scopeid"] >> qi::omit[token]) >>
       qi::eol)[qi::_val = ipv6_addr_with_prefix_ctor_(qi::_1, qi::_2)]
    ;

  linux_inet_addr
    = qi::hold[linux_inet6_addr]
    | qi::hold[linux_inet4_addr]
    ;

  linux_inet4_addr
    = (qi::lexeme["inet addr:"] >> ipv4_addr >>
       -(qi::lexeme["Bcast:"] >> qi::omit[ipv4_addr]) >>
       qi::lexeme["Mask:"] >> ipv4_mask >>
       qi::eol)[qi::_val = ipv4_addr_with_prefix_ctor_(qi::_1, qi::_2)]
    ;

  linux_inet6_addr
    = (qi::lexeme["inet6 addr:"] >>
       ipv6_addr >> qi::lit('/') >> qi::uint_ >>
       qi::lexeme["Scope:"] >> qi::omit[token] >>
       qi::eol)[qi::_val = ipv6_addr_with_prefix_ctor_(qi::_1, qi::_2)]
    ;

  linux_flags
    = +(qi::string("BROADCAST") |
        qi::string("LOOPBACK")  |
        qi::string("MULTICAST") |
        qi::string("RUNNING")   |
        qi::string("UP"))
    ;

  ipv4_mask
    = (ipv4_addr)[qi::_val = ipv4_cidr_from_mask_(qi::_1)]
    ;

  ipv4_mask_hex
    = (qi::lexeme["0x"] >> qi::hex)[qi::_val = count_bits_(qi::_1)]
    ;

  iface_name
    = +(qi::ascii::alnum | qi::ascii::char_("-_."))
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
      cerr << "Import output from 'ifconfig'." << endl
           << endl
           << "Usage: " << PROGRAM_NAME << " [options] {input-file}" << endl
           << visible_opts_desc << endl;
      return 0;
    }

    if (opts.count("version")) {
      cerr << "nmdb-import-ifconfig (Netmeld)" << endl;
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

    string const command_line;  // TODO

    string const db_name = opts.at("db-name").as<string>();
    pqxx::connection db{string("dbname=") + db_name};
    db_prepare_common(db);


    ifstream input_file(input_file_path.string());
    input_file.unsetf(std::ios::skipws);  // disable skipping whitespace

    Parser_ifconfig<boost::spirit::istream_iterator> const parser_ifconfig;
    vector<Interface_Info> results;

    if (true) {
      posix_time::ptime const execute_time_lower =
        posix_time::microsec_clock::universal_time();

      boost::spirit::istream_iterator i(input_file), e;

      bool const parse_success =
        qi::phrase_parse(i, e, parser_ifconfig, qi::ascii::blank, results);

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

        for (auto const& iface : results) {
          bool const is_up =
            (iface.flags.find("UP") != string::npos);

          if (6 <= iface.mac_addr.size()) {
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
              ("Ethernet")  // TODO: actually parse out of data
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
