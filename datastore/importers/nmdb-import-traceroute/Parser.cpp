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
#include <algorithm>
#include <iostream>

// =============================================================================
// Parser logic
// =============================================================================
Parser::Parser() : Parser::base_type(start)
{
  start =
      *qi::eol > (windowsTrace | linuxTrace)[qi::_val = pnx::bind(&Parser::getData, this)]
    ;

  windowsTrace = 
    windowsHeader >> +qi::eol >> +(windowsHop[pnx::bind(&Parser::flushHops, this)] > qi::eol) >> *qi::eol >> "Trace complete." >> *qi::eol
    ;

  windowsHeader =
    "Tracing route to" >> (ipAddr | (fqdn > "[" > ipAddr > "]")) >> -qi::eol >
      "over a maximum of" > qi::uint_ > "hops:"
    ;

  windowsHop = 
    qi::int_[pnx::bind(&Parser::recordHopNumber, this, qi::_1)] >>
        +(
        qi::string("*") | 
        qi::string("<1 ms") |
        (qi::int_ >> "ms")
        ) 
      >> windowsDomainIP
    ;

  windowsDomainIP =
    ipAddr[pnx::bind(&Parser::recordHopDestination, this, qi::_1)] | 
        (fqdn > "[" >
        ipAddr > "]")[pnx::bind(&Parser::recordHopDestinationWithAlias, this, qi::_2, qi::_1)]
    ;

  linuxTrace = 
    linuxHeader >> +qi::eol >> +(linuxHop[pnx::bind(&Parser::flushHops, this)] >> qi::eol) >> *qi::eol
    ;

  linuxHeader = 
    "traceroute to" >> (fqdn | ipAddr) > "(" > ipAddr > ")," >
      qi::uint_ > "hops max, " > qi::uint_ > "byte packets" 
    ;

  linuxHop = 
    qi::int_[pnx::bind(&Parser::recordHopNumber, this, qi::_1)] >> 
      *qi::string("*") >>
      -(linuxDomainIP >> qi::double_ > "ms" >>
        *((-linuxDomainIP >> qi::double_ > "ms") | "*")
      )
    ;

  linuxDomainIP =
    (ipAddr[pnx::bind(&Parser::recordHopDestination, this, qi::_1)] >> -("(" >> ipAddr >> ")")) |
    (fqdn >> "(" >
    ipAddr > ")")[pnx::bind(&Parser::recordHopDestinationWithAlias, this, qi::_2, qi::_1)]
    ;

  BOOST_SPIRIT_DEBUG_NODES(
      (start)
      
      (windowsTrace)
      (windowsHeader)
      (windowsHop)
      (windowsDomainIP)

      (linuxTrace)
      (linuxHeader)
      (linuxHop)
      (linuxDomainIP)

      );
}


// =============================================================================
// Parser helper methods
// =============================================================================
  void
  Parser::recordHopNumber(int hopNumber) {
    currentHopNumber = hopNumber;
  }

  void
  Parser::recordHopDestination(const nmdo::IpAddress& destination) {
    std::string alias;
    recordHopDestinationWithAlias(destination, alias);
    LOG_DEBUG << "RECORDING HOP DEST" << std::endl;
  }

  void
  Parser::recordHopDestinationWithAlias(const nmdo::IpAddress& destination, const std::string& alias) {
    nmdo::IpAddress dest{destination};
    std::string reason;
    if (!alias.empty()) {
      dest.addAlias(alias, reason);
    }
    if (!prevDests.empty()) {
      for (auto& prevDest : prevDests) {
        curHops.emplace_back();
        curHops.back().rtrIpAddr = prevDest;
        curHops.back().dstIpAddr = dest;
        curHops.back().hopCount = currentHopNumber;
      }
    } else {
      curHops.emplace_back();
      curHops.back().dstIpAddr = dest;
      curHops.back().hopCount = currentHopNumber;
    }
  }

  void
  Parser::flushHops() {
    if (!curHops.empty()) {
      if (curHops.back().rtrIpAddr.isValid()){
        std::copy(curHops.begin(), curHops.end(), std::back_inserter(r));
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
    return r;
  }