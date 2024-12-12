// =============================================================================
// Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC
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
    +(vrfTable1
     | vrfTable2
     | ignoredLine
    )
    ;

  // Format 1
  vrfTable1 =
    vrfHeader1
    > *(vrfLine1)
    ;

  vrfHeader1 =
    qi::lit("Name")
    > qi::lit("Default RD")
    > qi::lit("Protocols")
    > qi::lit("Interfaces")
    > qi::eol
    ;

  vrfLine1 =
    token [(pnx::bind(&nmdo::Vrf::setId, &qi::_val, qi::_1))]
    >> (qi::lit("<not set>") | routeDistinguisher)
    > csValues
    > +( interfaces
      >> qi::eol) [(
        pnx::bind(
          &nmdo::Vrf::addIface,
          &qi::_val,
          pnx::bind(
            &nmcu::expandCiscoIfaceName,
            qi::_1
          )
        ))]
    ;

  // Format 2
  vrfTable2 =
    vrfHeader2
    > *(vrfLine2)
    ;

  vrfHeader2 =
    qi::lit("VRF-Name")
    > qi::lit("VRF-ID")
    > qi::lit("State")
    > qi::lit("Reason")
    > qi::eol
    ;

  vrfLine2 =
    (token
    >> token
    >> token
    >> token
    >> qi::eol
    )[(pnx::bind(&nmdo::Vrf::setId, &qi::_val, qi::_1))]
    ;

  routeDistinguisher =
    qi::int_
    > ':'
    > qi::int_
    ;
  
  interfaces =
    +((qi::ascii::alnum - qi::char_('/'))
    > -qi::char_('/'))
    ;

  // ----- common usage -----
  csValues =
    qi::lexeme[+(qi::ascii::alnum) % qi::char_(',')]
    ;

  token =
    +qi::ascii::graph
    ;

  ignoredLine =
      (+token > qi::eol)
    | (qi::eol)
    ;

  BOOST_SPIRIT_DEBUG_NODES(
      (start)
        (vrfTable1)
          (vrfHeader1)
            (vrfLine1)
              (routeDistinguisher)
              (interfaces)
        (vrfTable2)
          (vrfHeader2)
            (vrfLine2)
      //(csValues)
      //(ignoredLine)
      //(token)
      );
}


// =============================================================================
// Parser helper methods
// =============================================================================
