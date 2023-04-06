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
Parser::Parser() : Parser::base_type(start)
{
  start =
    (*(packageLine [(pnx::bind(&Parser::addPackage, this, qi::_1))]
      | ignoredLine
      | qi::eol
    )) [(qi::_val = pnx::bind(&Parser::getData, this))]
  ;

  token =
    +qi::ascii::graph
  ;

  packageLine =
    packageName [(pnx::bind(&nmdo::Package::setName, &qi::_val, qi::_1))]
    > version [(pnx::bind(&nmdo::Package::setVersion, &qi::_val, qi::_1))]
    > architecture [(pnx::bind(&nmdo::Package::setArchitecture, &qi::_val, qi::_1))]
    > description [(pnx::bind(&nmdo::Package::setDescription, &qi::_val, qi::_1))]
    > qi::eol
  ;

  packageName =
    token
  ;
  version =
    token
  ;

  architecture =
    token
  ;

  description =
    +qi::ascii::print
  ;

  ignoredLine =
    (token > -qi::eol) | +qi::eol
  ;

  // Allows for error handling and debugging of qi.
  BOOST_SPIRIT_DEBUG_NODES(
      (start)
      (headers)
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
