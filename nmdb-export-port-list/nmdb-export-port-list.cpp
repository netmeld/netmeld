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

#include <netmeld/common/queries_common.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/adapted.hpp>

#include <boost/filesystem.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/program_options.hpp>

#include <bitset>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

using std::cin;
using std::cout;
using std::cerr;
using std::endl;

using std::bitset;
using std::get;
using std::ifstream;
using std::string;
using std::to_string;
using std::tuple;
using std::vector;

using boost::numeric_cast;

namespace filesystem = boost::filesystem;
namespace program_options = boost::program_options;


using Port_Range = tuple<string, size_t, size_t>;
using Port_Range_List = vector<Port_Range>;


template<typename InputIterator_T>
struct Parser_Port_List_File :
  boost::spirit::qi::grammar<InputIterator_T, Port_Range_List(),
			     boost::spirit::qi::ascii::blank_type>
{
  ~Parser_Port_List_File() = default;
  Parser_Port_List_File();

  boost::spirit::qi::rule<InputIterator_T, Port_Range_List(),
			  boost::spirit::qi::ascii::blank_type>
  start;

  boost::spirit::qi::rule<InputIterator_T, Port_Range(),
			  boost::spirit::qi::ascii::blank_type>
  line;

  boost::spirit::qi::rule<InputIterator_T, Port_Range(),
			  boost::spirit::qi::ascii::blank_type>
  port_range;

  boost::spirit::qi::rule<InputIterator_T, string()>
  protocol;

  boost::spirit::qi::rule<InputIterator_T,
			  boost::spirit::qi::ascii::blank_type>
  comment;
};


template<typename InputIterator_T>
Parser_Port_List_File<InputIterator_T>::
Parser_Port_List_File() :
  Parser_Port_List_File::base_type(start),
  start(),
  line(),
  port_range(),
  protocol(),
  comment()
{
  namespace qi = boost::spirit::qi;

  start
    = *(line)
    ;

  line
    = (port_range >> -(comment) >> qi::eol)
    | (comment >> qi::eol)
    | (qi::eol)
    ;

  port_range
    = (protocol >> qi::lit(':') >>
       qi::uint_ >> -(qi::lit('-') >> qi::uint_))
    ;

  protocol
    = +(qi::char_("TUY"))
    ;

  comment
    = (qi::lit('#') >> *(qi::ascii::graph | qi::ascii::blank))
    ;
}


string
ports_string_from_bitset(bitset<65536> const& ports);

string
ports_string_from_bitset(bitset<65536> const& ports)
{
  string ports_string;

  uint16_t port_range_first = 0;
  uint16_t port_range_last  = 0;
  bool in_port_range = false;

  for (size_t i = 0; i <= ports.size(); ++i) {
    if ((i < ports.size()) && ports.test(i)) {
      // Port's bit is set: start a port range if not already in a range.
      if (!in_port_range) {
	port_range_first = numeric_cast<uint16_t>(i);
	in_port_range = true;
      }
    }
    else {
      // Port's bit is not set or max port: end port range if in a range.
      if (in_port_range) {
	port_range_last = numeric_cast<uint16_t>(i-1);
	in_port_range = false;

	// Append the completed range onto the ports string.
	ports_string += to_string(static_cast<uint32_t>(port_range_first));
	if (port_range_first < port_range_last) {
	  ports_string += "-";
	  ports_string += to_string(static_cast<uint32_t>(port_range_last));
	}
	ports_string += ",";
      }
    }
  }

  return ports_string;
}


int
main(int argc, char** argv)
{
  namespace qi = boost::spirit::qi;

  try {
    // Parse command-line options.
    program_options::options_description opts_desc("Options");
    opts_desc.add_options()
      ("tcp,t",      "Enable output of TCP ports from resource file or DB.")
      ("tcp-all,T",  "Enable output of TCP ports 0-65535.")
      ("udp,u",      "Enable output of UDP ports from resource file or DB.")
      ("udp-all,U",  "Enable output of UDP ports 0-65535.")
      ("sctp,y",     "Enable output of SCTP ports from resource file or DB.")
      ("sctp-all,Y", "Enable output of SCTP ports 0-65535.")
      ("from-db,D",  "Use ports from database instead of from resource file.")
      ("db-name",
       program_options::value<string>()->required()->
       default_value(DEFAULT_DB_NAME),
       "Database to connect to.")
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
      cerr << "Generate list of ports suitable for use with Nmap." << endl
           << endl
           << "Usage: " << PROGRAM_NAME << " [options]" << endl
           << endl
           << opts_desc << endl;
      return 0;
    }

    if (opts.count("version")) {
      cerr << "nmdb-export-port-list (Netmeld)" << endl;
      return 0;
    }

    // Enforce required options and other sanity checks.
    program_options::notify(opts);


    // Open and parse the port list file.

    filesystem::path const netmeld_conf_dir{NETMELD_CONF_DIR};
    filesystem::path const port_list_path{netmeld_conf_dir/"port-list.conf"};
    ifstream port_list(port_list_path.string());
    port_list.unsetf(std::ios::skipws);  // disable skipping whitespace

    Parser_Port_List_File<boost::spirit::istream_iterator> const parser_config;

    boost::spirit::istream_iterator i(port_list), e;

    Port_Range_List port_ranges;

    bool const parse_success =
      qi::phrase_parse(i, e, parser_config, qi::ascii::blank, port_ranges);


    // Convert the parsed port list into bitsets and then
    // output Nmap-compatible port list strings.

    if ((!parse_success) || (i != e)) {
      cerr << "parse error" << endl;
    }
    else {
      bitset<65536> tcp_ports;
      bitset<65536> udp_ports;
      bitset<65536> sctp_ports;

      // If this program is being used, ports are being specified to nmap.
      // Nmap is unhappy if there are no ports, so ensure at least one.
      // Focus on services that are very common inside enterprise networks.
      if (true) {
        tcp_ports.set(80);    // HTTP
        tcp_ports.set(443);   // HTTPS
        tcp_ports.set(22);    // SSH
        tcp_ports.set(25);    // SMTP
        tcp_ports.set(53);    // DNS
        tcp_ports.set(445);   // Microsoft CIFS
        tcp_ports.set(88);    // Kerberos v5
        tcp_ports.set(389);   // LDAP
        tcp_ports.set(636);   // LDAPS

        udp_ports.set(53);    // DNS
        udp_ports.set(123);   // NTP
        udp_ports.set(161);   // SNMP
        udp_ports.set(88);    // Kerberos v5
        udp_ports.set(389);   // LDAP

        sctp_ports.set(80);   // HTTP
        sctp_ports.set(443);  // HTTPS
        sctp_ports.set(2905); // M3UA
        sctp_ports.set(3868); // Diameter
      }

      if (opts.count("from-db")) {
        string const db_name = opts.at("db-name").as<string>();
        pqxx::connection db{string("dbname=") + db_name};

        pqxx::transaction<> t{db};

        pqxx::result port_rows =
          t.exec("SELECT DISTINCT protocol, port"
                 " FROM ports"
                 " WHERE ((port_state = 'open') OR"
                 "        (port_state = 'closed')) AND"
                 "       (0 <= port)");
        for (auto const& port_row : port_rows) {
          string protocol;
          port_row.at("protocol").to(protocol);

          uint16_t port;
          port_row.at("port").to(port);

          if ("tcp" == protocol) {
            tcp_ports.set(port);
          }
          else if ("udp" == protocol) {
            udp_ports.set(port);
          }
          else if ("sctp" == protocol) {
            sctp_ports.set(port);
          }
        }

        t.commit();
      }
      else {
        for (auto const& x : port_ranges) {
          string const& protocol  = get<0>(x);
          size_t const port_first = get<1>(x);
          size_t const port_last  = get<2>(x) ? get<2>(x) : port_first;

          if (opts.count("tcp") && (string::npos != protocol.find('T'))) {
            for (size_t p = port_first; p <= port_last; ++p) {
              tcp_ports.set(p);
            }
          }

          if (opts.count("udp") && (string::npos != protocol.find('U'))) {
            for (size_t p = port_first; p <= port_last; ++p) {
              udp_ports.set(p);
            }
          }

          if (opts.count("sctp") && (string::npos != protocol.find('Y'))) {
            for (size_t p = port_first; p <= port_last; ++p) {
              sctp_ports.set(p);
            }
          }
        }
      }

      if (opts.count("tcp-all")) {
        tcp_ports.set();
      }

      if (opts.count("udp-all")) {
        udp_ports.set();
      }

      if (opts.count("sctp-all")) {
        sctp_ports.set();
      }

      string nmap_ports;
      if (opts.count("tcp") || opts.count("tcp-all")) {
        nmap_ports += "T:";
        nmap_ports += ports_string_from_bitset(tcp_ports);
      }
      if (opts.count("udp") || opts.count("udp-all")) {
        nmap_ports += "U:";
        nmap_ports += ports_string_from_bitset(udp_ports);
      }
      if (opts.count("sctp") || opts.count("sctp-all")) {
        nmap_ports += "Y:";
        nmap_ports += ports_string_from_bitset(sctp_ports);
      }
      if (nmap_ports.size()) {
        nmap_ports.pop_back();  // Remove trailing ","
      }
      cout << nmap_ports << endl;
    }
  }
  catch (std::exception& e) {
    cerr << "Error: " << e.what() << endl;
    return -1;
  }

  return 0;
}
