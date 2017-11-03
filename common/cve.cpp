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

#include <netmeld/common/cve.hpp>

#include <boost/format.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/adapted.hpp>

#include <tuple>


namespace qi = boost::spirit::qi;

using boost::format;

using std::get;
using std::string;
using std::tuple;


template<typename InputIterator_T>
struct Parser_CVE :
  boost::spirit::qi::grammar<InputIterator_T, tuple<uint16_t, uint32_t>()>
{
  ~Parser_CVE() = default;

  Parser_CVE();

  boost::spirit::qi::rule<InputIterator_T, tuple<uint16_t, uint32_t>()>
  start;

  boost::spirit::qi::uint_parser<uint16_t, 10, 4, 4>
  cve_year;

  boost::spirit::qi::uint_parser<uint32_t, 10, 4, -1>
  cve_number;
};


template<typename InputIterator_T>
Parser_CVE<InputIterator_T>::
Parser_CVE() :
  Parser_CVE::base_type(start),
  start(),
  cve_year(),
  cve_number()
{
  namespace qi = boost::spirit::qi;

  start
    = (qi::no_case[qi::lit("CVE")] >> qi::lit('-') >>
       cve_year >> qi::lit('-') >> cve_number)
    ;
}


CVE::
CVE(uint16_t const year, uint32_t const number) :
  year_(year),
  number_(number)
{

}


CVE::
CVE(string const& cve_id)
{
  Parser_CVE<string::const_iterator> const parser_cve;

  string::const_iterator
    i = cve_id.cbegin(),
    e = cve_id.cend();

  tuple<uint16_t, uint32_t> result;

  bool const parse_success =
    qi::parse(i, e, parser_cve, result);

  if ((!parse_success) || (i != e)) {
    throw std::runtime_error("CVE parse error: " + cve_id);
  }

  year_   = get<0>(result);
  number_ = get<1>(result);
}


std::string
CVE::
str() const
{
  return (format("CVE-%|04u|-%|04u|") % year_ % number_).str();
}


std::ostream&
operator<<(std::ostream& os, CVE const& cve)
{
  os << cve.str();
  return os;
}

