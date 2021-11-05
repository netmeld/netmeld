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

#ifndef PARSER_HELPER_HPP
#define PARSER_HELPER_HPP

#include <iostream>
#include <fstream>

#include <netmeld/core/utils/LoggerSingleton.hpp>

namespace nmcu = netmeld::core::utils;

/* Boost Spirit includes should come after Netmeld as we want to ensure our
   definitions are established before Boost sets them.  We intentionally don't
   wrap our initial defines which alter Boost behavior in #ifndef to flag an
   issue.
 */
#define BOOST_SPIRIT_DEBUG
#ifndef BOOST_SPIRIT_USE_PHOENIX_V3
  #define BOOST_SPIRIT_USE_PHOENIX_V3
#endif

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/iostreams/device/mapped_file.hpp>


namespace qi  = boost::spirit::qi;
namespace pnx = boost::phoenix;


namespace netmeld::datastore::parsers {

  typedef boost::spirit::istream_iterator  IstreamIter;
  typedef const char*                      ConstIter;

  template<class P, class R>
  R fromStdIn()
  {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    R result;
    IstreamIter i(std::cin >> std::noskipws), e;

    bool const success = qi::phrase_parse(i, e, P(), qi::ascii::blank, result);
    std::ios::sync_with_stdio(true);

    if ((!success) || (i != e)) {
      LOG_ERROR << "Parser failed around:\n";
      std::ostringstream oss;
      for (size_t count {0}; (count < 20) && (i != e); ++count, ++i) {
        oss << *i;
      }
      LOG_ERROR << oss.str() << std::endl;
      std::exit(nmcu::Exit::FAILURE);
    }

    return result;
  }

  template<class P, class R>
  bool matchString(const std::string& data)
  {
    R temp;
    std::istringstream dataStream(data);
    dataStream.unsetf(std::ios::skipws); // disable skipping whitespace
    IstreamIter i(dataStream), e;

    bool const success = qi::parse(i, e, P(), temp);

    return ((success) && (i == e));
  }

  template<class P, class R>
  R fromString(const std::string& data)
  {
    R result;
    std::istringstream dataStream(data);
    dataStream.unsetf(std::ios::skipws); // disable skipping whitespace
    IstreamIter i(dataStream), e;

    bool const success = qi::parse(i, e, P(), result);

    if ((!success) || (i != e)) {
      LOG_ERROR << "Parser failed around:\n";
      std::ostringstream oss;
      for (size_t count {0}; (count < 20) && (i != e); ++count, ++i) {
        oss << *i;
      }
      LOG_ERROR << oss.str() << std::endl;
      std::exit(nmcu::Exit::FAILURE);
    }

    return result;
  }

  template<class P, class R>
  R fromFilePath(const std::string& data)
  {
    R result;
    std::ifstream dataStream(data);
    dataStream.unsetf(std::ios::skipws); // disable skipping whitespace
    IstreamIter i(dataStream), e;

    bool const success = qi::phrase_parse(i, e, P(), qi::ascii::blank, result);

    if ((!success) || (i != e)) {
      LOG_ERROR << "Parser failed around:\n";
      std::ostringstream oss;
      for (size_t count {0}; (count < 20) && (i != e); ++count, ++i) {
        oss << *i;
      }
      LOG_ERROR << oss.str() << std::endl;
      std::exit(nmcu::Exit::FAILURE);
    }

    return result;
  }

  template<class P, class R>
  R fromFilePathMM(const std::string& data)
  {
    R result;
    boost::iostreams::mapped_file_source mmap(data);

    auto i = mmap.begin();
    auto e = mmap.end();

    bool const success = qi::phrase_parse(i, e, P(), qi::ascii::blank, result);

    if ((!success) || (i != e)) {
      LOG_ERROR << "Parser failed around:\n";
      std::ostringstream oss;
      for (size_t count {0}; (count < 20) && (i != e); ++count, ++i) {
        oss << *i;
      }
      LOG_ERROR << oss.str() << std::endl;
      std::exit(nmcu::Exit::FAILURE);
    }

    return result;
  }

  class DummyParser :
    public qi::grammar<IstreamIter>
  {
    public:
      DummyParser() : DummyParser::base_type(start)
      {
        // cppcheck-suppress useInitializationList
        start =
          *(*(qi::omit[qi::char_] - qi::eol) % qi::eol)
          ;

        BOOST_SPIRIT_DEBUG_NODES((start));
      }

      qi::rule<IstreamIter>
        start;
  };
}
#endif // PARSER_HELPER_HPP
