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
#include <boost/lexical_cast.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/program_options.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <algorithm>
#include <bitset>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <map>
#include <experimental/optional>
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
using boost::lexical_cast;
using boost::numeric_cast;
using boost::uuids::uuid;

namespace filesystem = boost::filesystem;
namespace posix_time = boost::posix_time;
namespace program_options = boost::program_options;


boost::uuids::random_generator uuid_generator;

using ipconfig_line = std::map<std::string, std::vector<std::string> >;

struct Adapter_Info
{
  std::string media_type;
  std::string description_name;
  ipconfig_line lines;
};

struct Compartment
{
  std::string hostname;
  std::vector<Adapter_Info> adapters;
};

BOOST_FUSION_ADAPT_STRUCT(
  Compartment,
  (std::string, hostname)
  (std::vector<Adapter_Info>, adapters)
)


BOOST_FUSION_ADAPT_STRUCT(
  Adapter_Info,
  (std::string, media_type)
  (std::string, description_name)
  (ipconfig_line, lines)
)

template<typename InputIterator_T>
struct Parser_ipconfig :
  boost::spirit::qi::grammar<InputIterator_T,
                             std::vector<Compartment>(),
                             boost::spirit::qi::ascii::blank_type>
{
  ~Parser_ipconfig() = default;

  Parser_ipconfig();

  boost::spirit::qi::rule<InputIterator_T,
                          std::vector<Compartment>(),
                          boost::spirit::qi::ascii::blank_type>
  start;

  boost::spirit::qi::rule<InputIterator_T,
                          std::string(),
                          boost::spirit::qi::ascii::blank_type>
  windows_compartment;

  boost::spirit::qi::rule<InputIterator_T,
                          Adapter_Info(),
                          boost::spirit::qi::ascii::blank_type>
  windows_adapter;

  boost::spirit::qi::rule<InputIterator_T,
                          std::pair<std::string, std::vector<std::string> >(),
                          boost::spirit::qi::ascii::blank_type>
  config_line;

  boost::spirit::qi::rule<InputIterator_T, 
                          boost::spirit::qi::ascii::blank_type>
  seperator;

  boost::spirit::qi::rule<InputIterator_T, 
                          std::string(), 
                          boost::spirit::qi::ascii::blank_type>
  key;

  boost::spirit::qi::rule<InputIterator_T,
                          std::string(),
                          boost::spirit::qi::ascii::blank_type>
  value;
};

// We want to keep intermediate values of spaces in the keys,
// this is to trim the trailing whitespace
static string rtrim(vector<char> c) {
  std::string s(c.begin(), c.end());
  s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
      return !std::isspace(ch);
  }).base(), s.end());
  return s;
}

BOOST_PHOENIX_ADAPT_FUNCTION(
  std::string,
  rtrim_,
  rtrim,
  1
)

template<typename InputIterator_T>
Parser_ipconfig<InputIterator_T>::
Parser_ipconfig() :
  Parser_ipconfig::base_type(start),
  start(),
  windows_compartment(),
  windows_adapter(),
  config_line(),
  seperator(),
  key(),
  value()
{
  namespace qi = boost::spirit::qi;

  start
    = *qi::eol >> 
      qi::lexeme["Windows IP Configuration"] >> 
      +qi::eol >>
      +(windows_compartment >> *qi::eol >>
        *(windows_adapter >> *qi::eol));

  windows_compartment
    = (-(+qi::char_('=') >> qi::eol >>
         qi::omit[+qi::graph] >> qi::eol >>
         +qi::char_('=') >> qi::eol) >>
       qi::lexeme["Host Name"] >> seperator >> value >> qi::eol >>
       *(qi::omit[key >> (+(value >> qi::eol) | qi::eol)]));

  windows_adapter
    = (qi::lexeme[+(qi::ascii::graph)] >> qi::lexeme["adapter"] >>
       +(qi::ascii::graph - qi::char_(':')) >> qi::lexeme[":"] >>
       qi::eol >>
       qi::eol >>
       +(config_line)
      )
    ;
  config_line
    = (key >> (+(value >> qi::eol) | qi::eol));

  seperator
    // TODO I'm pretty sure there always has to be one '.', 
    // if there doesn't the value parser will fail on IPv6 addresses
    // since an IPv6 addr 'xxxx:xxxx' would now match the key parser
    // having the key still break on '.' or ':' should create a parse error
    // whenever no '.'s are used
    = qi::omit[+qi::ascii::char_('.')] >> qi::lexeme[":"];

  key
    = (qi::lexeme[+(qi::ascii::print - qi::ascii::char_(".:"))] >> seperator)[qi::_val = rtrim_(qi::_1)];

  value
    = !key >> qi::lexeme[+(qi::ascii::print)];
}

string
clean_ip(string ip_addr_str);

void
process_ip
(pqxx::transaction<>& t,
 uuid const& tool_run_id,
 string ip_addr_str, bool is_up);

void
process_ip_and_mac
(pqxx::transaction<>& t,
 uuid const& tool_run_id, string const& device_id,
 string ip_addr_str, string mac_addr_str, string iface_name, bool is_up);

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
       "Graph color of device or host.")
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
      cerr << "Import output from 'ipconfig', optionally with the /all /allcompartment flags." << endl
           << endl
           << "Usage: " << PROGRAM_NAME << " [options] {input-file}" << endl
           << visible_opts_desc << endl;
      return (0);
    }

    if (opts.count("version")) {
      cerr << "nmdb-import-ipconfig (Netmeld)." << endl;
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
      ? (lexical_cast<uuid>(opts.at("tool-run-id").as<string>()))
      : (uuid_generator());

    string const device_id = opts.at("device-id").as<string>();

    string const command_line;  // TODO

    string const db_name = opts.at("db-name").as<string>();
    pqxx::connection db{string("dbname=") + db_name};
    db_prepare_common(db);


    ifstream input_file(input_file_path.string());
    input_file.unsetf(std::ios::skipws);  // disable skipping whitespace

    Parser_ipconfig<boost::spirit::istream_iterator> const parser_ipconfig;
    std::vector<Compartment> results;

    if (true) {
      posix_time::ptime const execute_time_lower =
        posix_time::microsec_clock::universal_time();

      boost::spirit::istream_iterator i(input_file), e;

      bool const parse_success =
        qi::phrase_parse(i, e, parser_ipconfig, qi::ascii::blank, results);

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

        for(unsigned int k = 0; k < results.size(); k++) {
          for(unsigned int i = 0; i < results[k].adapters.size(); i++) {
            Adapter_Info iface = results[k].adapters[i];
            bool const is_up = true;
            string iface_name = iface.lines["Description"][0];
            /*for(auto const& line : iface.lines) {
              cout << "Key:" << line.first << ":" << endl;
              for(auto const& value : line.second) {
                cout << "\t " << value << endl;
              }
            }*/
            if (iface.lines["Physical Address"].size() > 0) {
              string mac_addr_str = iface.lines["Physical Address"][0];
              if (mac_addr_str.size() > 17) {
                cout << "WARNING: adapter " << iface.description_name << 
                      " with > 6-byte Physical Address skipped." << endl;
              }
              else {
                std::replace(mac_addr_str.begin(), mac_addr_str.end(), '-', ':');

                t.prepared("insert_raw_device_interface")
                  (tool_run_id)
                  (device_id)
                  (iface_name)
                  (iface.media_type)
                  (is_up)       // TODO: actually parse out of data
                  .exec();

                t.prepared("insert_raw_mac_addr")
                  (tool_run_id)
                  (mac_addr_str)
                  (is_up)  // is_responding
                  .exec();

                t.prepared("insert_raw_device_mac_addr")
                  (tool_run_id)
                  (device_id)
                  (iface_name)
                  (mac_addr_str)
                  .exec();

                if(iface.lines.find("IPv4 Address") != iface.lines.end()) {
                  string ip_addr = clean_ip(iface.lines["IPv4 Address"][0]);
                  IPv4_Addr mask = IPv4_Addr::from_string(iface.lines["Subnet Mask"][0]);
                  string cidr = lexical_cast<string>(cidr_from_mask(mask));
                  ip_addr = ip_addr + "/" + cidr;
                  process_ip_and_mac(t, tool_run_id, device_id, 
                                     ip_addr, mac_addr_str, iface_name, is_up);
                }
                if(iface.lines.find("IPv6 Address") != iface.lines.end()) {
                  string ip_addr = clean_ip(iface.lines["IPv6 Address"][0]);
                  process_ip_and_mac(t, tool_run_id, device_id,
                                     ip_addr, mac_addr_str, iface_name, is_up);
                }
              }
            }
            for(auto ip_addr : iface.lines["DNS Servers"]) {
              process_ip(t, tool_run_id, clean_ip(ip_addr), false);
            }
            for(auto ip_addr : iface.lines["DHCP Server"]) {
              process_ip(t, tool_run_id, clean_ip(ip_addr), false);
            }
            for(auto ip_addr : iface.lines["Primary WINS Server"]) {
              process_ip(t, tool_run_id, clean_ip(ip_addr), false);
            }
            for(auto ip_addr : iface.lines["Secondary WINS Server"]) {
              process_ip(t, tool_run_id, clean_ip(ip_addr), false);
            }
            for(auto ip_addr : iface.lines["Default Gateway"]) {
              process_ip(t, tool_run_id, clean_ip(ip_addr), false);
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
    return (-1);
  }

  return (0);
}

string
clean_ip(string ip_addr_str) {
  ip_addr_str = ip_addr_str.substr(0, ip_addr_str.find("(Preferred)"));
  // TODO we might actually want to just remove addrs with the below in them
  ip_addr_str = ip_addr_str.substr(0, ip_addr_str.find("%"));
  return ip_addr_str;
}

void
process_ip
(pqxx::transaction<>& t,
 uuid const& tool_run_id,
 string ip_addr_str, bool is_up)
{
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
}

void
process_ip_and_mac
(pqxx::transaction<>& t,
 uuid const& tool_run_id, string const& device_id,
 string ip_addr_str, string mac_addr_str, string iface_name, bool is_up)
{
  process_ip(t, tool_run_id, ip_addr_str, is_up);
  t.prepared("insert_raw_mac_addr_ip_addr")
    (tool_run_id)
    (mac_addr_str)
    (ip_addr_str)
    .exec();

  t.prepared("insert_raw_device_ip_addr")
    (tool_run_id)
    (device_id)
    (iface_name)
    (ip_addr_str)
    .exec();
}
