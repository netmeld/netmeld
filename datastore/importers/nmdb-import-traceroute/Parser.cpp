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
    qi::int_[pnx::bind(&Parser::recordHopNumber, this, qi::_1)] >> 
      //qi::uint_[pnx::bind(&Parser::addHop, this, qi::_1)] >> 
      qi::repeat(3)[
        (
        qi::string("*") | 
        qi::string("<1 ms") |
        (qi::int_ >> "ms")
        ) 
      ] >> windowsDomainIP >> +qi::eol
    ;

  windowsDomainIP =
    ipAddr[pnx::bind(&Parser::recordHopDestination, this, qi::_1)] | 
        (fqdn >> "[" > 
        ipAddr[pnx::bind(&Parser::recordHopDestination, this, qi::_1)] >
        "]")
    ;

  linuxTrace = 
    linuxHeader >> +qi::eol >> *linuxHop >> *qi::eol
    ;

  linuxHeader = 
    "traceroute to" >> (fqdn | ipAddr) >> "(" > ipAddr > ")," >>
      qi::uint_ >> "hops max, " >> qi::uint_ >> "byte packets" 
    ;

  linuxHop = 
    qi::int_[pnx::bind(&Parser::recordHopNumber, this, qi::_1)] >> 
      qi::skip[qi::repeat(0, 3)["*"]] >>
      -(linuxDomainIP >> qi::double_ >> "ms" >>
        qi::repeat(0, 2)[(-linuxDomainIP >> qi::double_ >> "ms") | "*"]
      ) >> qi::eol
    ;

  linuxDomainIP =
    (
      (fqdn | ipAddr) >>
        "(" > ipAddr[pnx::bind(&Parser::recordHopDestination, this, qi::_1)] > ")"
    ) |
      ipAddr[pnx::bind(&Parser::recordHopDestination, this, qi::_1)]
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
  Parser::recordHopNumber(int hopNumber) {
    LOG_DEBUG << "RECORDING HOP NUMBER: " << hopNumber << std::endl;
    currentHopNumber = hopNumber;
    flushHops();
  }

  void
  Parser::recordHopDestination(const nmdo::IpAddress& destination) {
    if (!prevDests.empty()) {
      for (auto& prevDest : prevDests) {
        curHops.emplace_back();
        curHops.back().rtrIpAddr = prevDest;
        curHops.back().dstIpAddr = destination;
        curHops.back().hopCount = currentHopNumber;
      }
    } else {
      curHops.emplace_back();
      curHops.back().dstIpAddr = destination;
      curHops.back().hopCount = currentHopNumber;
    }
  }

  void
  Parser::flushHops() {
    if (!curHops.empty()) {
      if (curHops.back().hopCount != 1){
        for (auto& hop: curHops) {
          r.emplace_back(hop);
        }
      }
      prevDests.clear();
      for (auto& hop: curHops) {
        prevDests.emplace(hop.dstIpAddr);
      }
      curHops.clear();
    }
  }

  Result
  Parser::getData()
  {
    flushHops();
    return r;
  }