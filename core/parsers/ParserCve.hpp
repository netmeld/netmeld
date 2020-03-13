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

#ifndef PARSER_CVE_HPP
#define PARSER_CVE_HPP

#include <netmeld/core/objects/Cve.hpp>
#include <netmeld/core/parsers/ParserHelper.hpp>

namespace nmco = netmeld::core::objects;


namespace netmeld::core::parsers {

  class ParserCve :
    public qi::grammar<IstreamIter, nmco::Cve()>
  {
    public:
      ParserCve() : ParserCve::base_type(start)
      {
        // cppcheck-suppress useInitializationList
        start =
          (qi::no_case[qi::lit("CVE")] >> qi::lit('-')
           >> qi::short_ >> qi::lit('-') >> qi::int_)
          [qi::_val = pnx::construct<nmco::Cve>(qi::_1, qi::_2)]
          ;

        BOOST_SPIRIT_DEBUG_NODES((start))
      }

      qi::rule<IstreamIter, nmco::Cve()>   start;
  };
}
#endif // PARSER_CVE_HPP
