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

#include "Parser.hpp"


// =============================================================================
// Parser logic
// =============================================================================

Parser::Parser() : Parser::base_type(start)
{
  start =
    *((!link) > ignoredLine)//-qi::omit[+token] > qi::eol)
    > *link
    > *ignoredLine
    ;

  link =
    -qi::ascii::char_("GRO*+")
    >> vlanId [pnx::bind(&Data::addVlan, &qi::_val, qi::_1)]
    >> macAddr [pnx::bind(&Data::addReachableMac, &qi::_val, qi::_1)]
    >> portName [pnx::bind(&Data::setName, &qi::_val, qi::_1)]
    >> qi::omit[*token]
    >  -qi::eol [pnx::bind(&Data::setPartial, &qi::_val, true),
                 pnx::bind(&Data::setState, &qi::_val, true)]
    ;

  vlanId =
    (qi::ushort_ > !qi::digit)
    ;

  portName =
    -qi::hold[typeValue > -(+qi::blank)]
    > token [qi::_val = qi::_1]
    ;

  typeValue =
    +qi::ascii::char_("a-zA-Z") >> +qi::blank
    > -( // age >> secure >> ntfy
         (+qi::ascii::char_("-0-9") > +qi::blank
          > qi::ascii::char_("TFC~") > +qi::blank
          > qi::ascii::char_("TFC~")
         )
         // learn >> age
       | ((qi::lit("Yes") | qi::lit("No")) > +qi::blank
          > +qi::ascii::char_("-0-9")
         )
         // protocol | pv
       | (+qi::ascii::char_("a-z,") >> !qi::digit)
      )
    ;

  token =
    +qi::ascii::graph
    ;

  ignoredLine =
    (  (+(qi::char_ - qi::eol) > -qi::eol)
     | (qi::eol)
    )
    ;

  BOOST_SPIRIT_DEBUG_NODES(
      (start)
      (link)
      (portName)(vlanId)(typeValue)
      //(token) (ignoredLine)
      );
}

// =============================================================================
// Parser helper methods
// =============================================================================
