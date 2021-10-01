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
#include <iostream>

// =============================================================================
// Parser logic
// =============================================================================
Parser::Parser() : Parser::base_type(start)
{
  start =
      *qi::eol >> (windowsTrace | linuxTrace)[qi::_val = pnx::bind(&Parser::getData, this)]
    ;

  windowsTrace = 
    windowsHeader >> +qi::eol >> *windowsHop >> "Trace complete." >> *qi::eol
    ;

  windowsHeader =
    "Tracing route to" >> (ipAddr | (fqdn >> "[" > ipAddr > "]")) >> -qi::eol >>
      "over a maximum of" >> qi::uint_ >> "hops:"
    ;

  windowsHop = 
      qi::uint_[pnx::bind(&Parser::addHop, this, qi::_1)] >> 
      qi::repeat(3)[
        (
        qi::string("*") | 
        qi::string("<1 ms") |
        (qi::int_ >> "ms")
        ) 
      ] >> (ipAddr[pnx::bind(&Parser::getDestinationIP, this, qi::_1)] | 
              (fqdn >> "[" > 
              ipAddr[pnx::bind(&Parser::getDestinationIP, this, qi::_1)] >
              "]")) >> +qi::eol
    ;

  linuxTrace = 
    linuxHeader >> +qi::eol >> *linuxHop >> *qi::eol
    ;

  linuxHeader = 
    "traceroute to" >> (fqdn | ipAddr) >> "(" > ipAddr > ")," >>
      qi::uint_ >> "hops max, " >> qi::uint_ >> "byte packets" 
    ;

  linuxHop = 
    qi::uint_[pnx::bind(&Parser::addHop, this, qi::_1)] >> 
      qi::skip[qi::repeat(0, 3)["*"]] >>
      -((fqdn | ipAddr[pnx::bind(&Parser::getDestinationIP, this, qi::_1)] ) >> "(" >
        ipAddr[pnx::bind(&Parser::getDestinationIP, this, qi::_1)] > ")" >>
        (qi::double_ >> "ms") >>
        qi::repeat(0, 2)[-((fqdn | ipAddr[pnx::bind(&Parser::getDestinationIP, this, qi::_1)] ) >> 
          "(" > ipAddr[pnx::bind(&Parser::getDestinationIP, this, qi::_1)] >
          ")") >> qi::double_ >> "ms"]
      ) >> qi::eol
    ;

  BOOST_SPIRIT_DEBUG_NODES(
      (start)
      
      (windowsTrace)
      (windowsHeader)
      (windowsHop)

      (linuxTrace)
      (linuxHeader)
      (linuxHop)

      );
}


// =============================================================================
// Parser helper methods
// =============================================================================
  void
  Parser::addHop(int hopNumber) {
    d = Data();
    d.hopCount = hopNumber;
    if (hopNumber > 1) {
      r.pop_back();
      LOG_DEBUG << "PREVIOUS " << r.back().toString() << std::endl;
      Data previousHop = r.back();
      auto lastHopNumber = previousHop.hopCount;
      auto previousHopIt = r.rbegin();
      for (auto it2 = previousHopIt++; it2 != r.rend(); ++it2) {
        if (it2->hopCount != lastHopNumber) {
          break;
        }
        previousHopIt = it2;
      } 
      previousHop = *previousHopIt;
      d.rtrIpAddr = previousHop.dstIpAddr;
    }
    r.push_back(d);
  }

  void
  Parser::getDestinationIP(const nmdo::IpAddress& destination) {
    const nmdo::IpAddress origin = r.back().rtrIpAddr;
    int prevHopCount = r.back().hopCount;
    r.back().dstIpAddr = destination;
    r.emplace_back();
    r.back().hopCount = prevHopCount;
    r.back().rtrIpAddr = origin;
  }

  Result
  Parser::getData()
  {
    if (r.empty()) {
      return Result();
    }
    Result res;
    for (auto it = r.begin() + 1; it != r.end(); ++it) {
      res.push_back(*it);
    }
    res.pop_back();
    return res;
  }