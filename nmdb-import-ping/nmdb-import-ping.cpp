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
#include <map>
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

using boost::numeric_cast;
using boost::uuids::uuid;

namespace filesystem = boost::filesystem;
namespace posix_time = boost::posix_time;
namespace program_options = boost::program_options;


boost::uuids::random_generator uuid_generator;


template<typename InputIterator_T>
struct Parser_ping :
  boost::spirit::qi::grammar<InputIterator_T,
                             std::vector<IP_Addr>(),
                             boost::spirit::qi::ascii::blank_type>
{
  ~Parser_ping() = default;

  Parser_ping();

  boost::spirit::qi::rule<InputIterator_T,
                          std::vector<IP_Addr>(),
                          boost::spirit::qi::ascii::blank_type>
  start;

  boost::spirit::qi::rule<InputIterator_T,
                          boost::spirit::qi::ascii::blank_type>
  ping_responses_header;

  boost::spirit::qi::rule<InputIterator_T,
                          std::vector<IP_Addr>(),
                          boost::spirit::qi::ascii::blank_type>
  ping_responses;

  boost::spirit::qi::rule<InputIterator_T,
                          IP_Addr(),
                          boost::spirit::qi::ascii::blank_type>
  ping_response;

  boost::spirit::qi::rule<InputIterator_T,
                          boost::spirit::qi::ascii::blank_type>
  ping_statistics_header;

  boost::spirit::qi::rule<InputIterator_T,
                          boost::spirit::qi::ascii::blank_type>
  ping_statistics;

  Parser_IP_Addr<InputIterator_T>
  ip_addr;

  boost::spirit::qi::rule<InputIterator_T, std::string()>
  iface_name;
};


template<typename InputIterator_T>
Parser_ping<InputIterator_T>::
Parser_ping() :
  Parser_ping::base_type(start),
  start(),
  ping_responses_header(),
  ping_responses(),
  ping_response(),
  ping_statistics_header(),
  ping_statistics(),
  ip_addr(),
  iface_name()
{
  namespace qi = boost::spirit::qi;

  start
    = (ping_responses_header >>
       ping_responses >>
       *qi::eol >>
       -(ping_statistics_header >>
         ping_statistics >>
         *qi::eol))
    ;

  ping_responses_header
    = (qi::lit("PING") >>
       qi::omit[+(+qi::ascii::graph)] >>
       qi::eol)
    ;

  ping_responses
    = (*ping_response)
    ;

  ping_response
    = qi::hold[qi::omit[qi::uint_ >> qi::lit("bytes")] >>
               qi::lit("from") >> ip_addr >> 
               -(qi::lit("%") >> iface_name) >> qi::lit(':') >>
               (qi::lit("icmp_req=") | qi::lit("icmp_seq=")) >>
               qi::omit[qi::uint_] >>
               qi::lit("ttl=") >> qi::omit[qi::uint_] >>
               qi::lit("time=") >> qi::omit[qi::float_] >>
               (qi::lit("us") | qi::lit("ms") | qi::lit("s")) >>
               -qi::lit("(DUP!)") >>
               qi::eol]
    | qi::hold[qi::lit("^") >> qi::omit[+(+qi::ascii::graph)] >>
               -qi::eol]
    | qi::hold[qi::eol]
    ;

  ping_statistics_header
    = (qi::lit("---") >>
       qi::omit[+(+qi::ascii::graph)] >>
       qi::eol)
    ;

  ping_statistics
    = +(qi::omit[+(+qi::ascii::graph)] >>
        -qi::eol)
    ;

  iface_name
    = +(qi::ascii::alnum | qi::ascii::char_("-_.@"))
    ;
}


int
main(int argc, char** argv)
{
  namespace qi = boost::spirit::qi;

  try {
    program_options::options_description general_opts_desc("Options");
    general_opts_desc.add_options()
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
       "Show version information then exit.")
      ("pipe",
       "Read input from STDIN; Save a copy to {input-file}.")
      ;

    program_options::options_description hidden_opts_desc("Hidden options");
    hidden_opts_desc.add_options()
      ("input-file",
       program_options::value<string>()->required(),
       "Input data file to parse.")
      ;
    program_options::positional_options_description position_opts_desc;
    position_opts_desc.add("input-file", -1);

    // Command-line options:
    program_options::options_description cl_opts_desc;
    cl_opts_desc.add(general_opts_desc).add(hidden_opts_desc);

    // Visible options:
    program_options::options_description visible_opts_desc;
    visible_opts_desc.add(general_opts_desc);


    program_options::variables_map opts;
    program_options::store
      (program_options::command_line_parser(argc, argv)
       .options(cl_opts_desc)
       .positional(position_opts_desc)
       .run(), opts);

    if (opts.count("help")) {
      cerr << "Import output from 'ping' and 'ping6'." << endl
           << endl
           << "Usage: " << PROGRAM_NAME << " [options] {input-file}" << endl
           << visible_opts_desc << endl;
      return 0;
    }

    if (opts.count("version")) {
      cerr << "nmdb-import-ping (Netmeld)" << endl;
      return 0;
    }

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

    string const command_line = "ping [unknown arguments]";

    string const db_name = opts.at("db-name").as<string>();
    pqxx::connection db{string("dbname=") + db_name};
    db_prepare_common(db);


    ifstream input_file(input_file_path.string());
    input_file.unsetf(std::ios::skipws);  // disable skipping whitespace

    Parser_ping<boost::spirit::istream_iterator> const parser_ping;
    vector<IP_Addr> results;

    if (true) {
      posix_time::ptime const execute_time_lower =
        posix_time::microsec_clock::universal_time();

      boost::spirit::istream_iterator i(input_file), e;

      bool const parse_success =
        qi::phrase_parse(i, e, parser_ping, qi::ascii::blank, results);

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

        for (auto ip_addr : results) {
          if (!ip_addr.is_unspecified()) {
            t.prepared("insert_raw_ip_addr")
              (tool_run_id)
              (ip_addr)
              (true)  // is_responding
              .exec();
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
