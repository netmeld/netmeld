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

/* Notes:
   - This unit is part of the complilation process to help ensure consistency
     between templates and the actual data
   - Various data is included and most is commented solely for educational
     purposes
     - In non-template, remove data as makes sense

   Guidelines:
   - If using a custom Parser
     - Data ordering is different as the focus is the parsing logic, not rule
       instantiation
     - It occasionally is more reasonable to interact and place data with an
       intermediary object
       - The code can be collocated or a separate file, depending on complexity
*/

#include "Parser.hpp"

// =============================================================================
// Parser logic
// =============================================================================
Parser::Parser() : Parser::base_type(start)
{
  start =
      *qi::eol >> (windowsTrace | linuxTrace)
    ;

  // cppcheck-suppress useInitializationList`
  ignore =
    *(qi::omit[qi::char_] - qi::eol) % qi::eol
    ;

  windowsTrace = 
    windowsHeader >> +qi::eol >> *windowsHop >> "Trace complete." >> *qi::eol
    ;

  windowsHeader =
    "Tracing route to" >> (ipAddr | (fqdn >> "[" > ipAddr > "]")) >> qi::eol >>
      "over a maximum of" >> +qi::digit >> "hops:"
    ;

  windowsHop = 
      qi::lexeme[+qi::digit] >> 
      qi::repeat(3)[
        (
        qi::string("*") | 
        qi::string("<1 ms") |
        ms
        ) 
      ] >> (ipAddr | (fqdn >> "[" > ipAddr > "]")) >> +qi::eol
    ;

  linuxTrace = 
    linuxHeader >> +qi::eol >> *linuxHop >> *qi::eol
    ;

  linuxHeader = 
    "traceroute to" >> (fqdn | ipAddr) >> "(" > ipAddr > ")," >>
      +qi::digit >> "hops max, " >> +qi::digit >> "byte packets" 
    ;

  linuxHop = 
    +qi::digit >> 
      qi::skip[qi::repeat(0, 3)["*"]] >>
      -((fqdn | ipAddr ) >> "(" > ipAddr > ")" >>
        ms >>
        qi::repeat(0, 2)[-((fqdn | ipAddr ) >> "(" > ipAddr > ")") >> ms]
      ) >> *(qi::omit[qi::char_] - qi::eol) >> qi::eol
    ;

  /*>> +qi::space >> qi::repeat(0, 3)[qi::string("*") >> +qi::space] >>
      >> +qi::digit >> +qi::space >> qi::repeat(0, 3)[qi::string("*")] >>
      +qi::space >> (fqdn | ipAddr) >> " (" > ipAddr > ")" >>
      -(
        +qi::space >> +qi::digit >> " ms" >>
        qi::repeat(0, 2)[+qi::space >> -((fqdn | ipAddr) >> " (" > ipAddr > ")") >> +qi::digit >> " ms"]
      )*/
    ms =
      +qi::digit >> -("." >> +qi::digit) >> "ms"
      ;

  BOOST_SPIRIT_DEBUG_NODES(
      (start)
      
      (windowsTrace)
      (windowsHeader)
      (windowsHop)

      //(linuxTrace)
      //(linuxHeader)
      //(linuxHop)

      (ms)

      (ignore)
      );
}


// =============================================================================
// Parser helper methods
// =============================================================================
