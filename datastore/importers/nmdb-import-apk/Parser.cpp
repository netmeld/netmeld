// =============================================================================
// Copyright 2022 National Technology & Engineering Solutions of Sandia, LLC
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
#include "Parser.hpp"

// =============================================================================
// Parser logic
// =============================================================================
Parser::Parser() : Parser::base_type(prestart)
{
  prestart = start [(qi::_val = pnx::bind(&Parser::getData, this))];

  start =
    *(packageLine [(pnx::bind(&Parser::addPackage, this, qi::_1))]
      | ignoredLine
      | qi::eol
    )
  ;


  // We need to parse this output
//  busybox-openrc-1.35.0-r29 x86_64 {busybox} (GPL-2.0-only) [installed]
//    Size optimized toolbox of many common UNIX utilities
  // name-version arch {skip} {skip} {skip} [installed]
  // description
  // going to have to make architecture optional
  packageLine =
    packageName [(pnx::bind(&nmdo::Package::setName, &qi::_val, qi::_1))]
    >> qi::char_("-")
    > version [(pnx::bind(&nmdo::Package::setVersion, &qi::_val, qi::_1))]
    > architecture [(pnx::bind(&nmdo::Package::setArchitecture, &qi::_val, qi::_1))]
    > (+token > -qi::eol)
    > description [(pnx::bind(&nmdo::Package::setDescription, &qi::_val, qi::_1))]
    > qi::eol
  ;

  packageName =
    // we need to try and parse up until we hit a 0-9
    // this will be the difference between the name and version
    *(qi::char_ - qi::omit[qi::char_("-") >> qi::digit])
  ;
  version =
    +token
  ;
  architecture =
    +token
  ;

  description =
   +qi::ascii::print
  ;

  skipper =
    qi::blank | '-'
  ;

  token =
    +qi::ascii::graph
  ;

  ignoredLine =
    (+token > -qi::eol) | +qi::eol
  ;

  // Allows for error handling and debugging of qi.
  BOOST_SPIRIT_DEBUG_NODES(
      (start)
      (headers)
      (packageName)
      (packageLine)
      (version)
      (skipper)
      (description)
      (token)
      );
}

// =============================================================================
// Parser helper methods
// =============================================================================
void
Parser::addPackage(const nmdo::Package& packg)
{
  data.packages.push_back(packg);
}

Result
Parser::getData()
{
  Result r;
  r.push_back(data);
  return r;
}
