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
    +( (windowsTrace | linuxTrace)
        [qi::_val = pnx::bind(&Parser::getData, this)]
    | (garbageLine)
    )
    ;

  windowsTrace =
    windowsHeader > +qi::eol
    > +(windowsHop > qi::eol) > *qi::eol
    > "Trace complete." > *qi::eol
    ;

  windowsHeader =
    "Tracing route to"
    > (ipAddr | (qi::omit[fqdn] > "[" > ipAddr > "]"))
        [pnx::bind(&Parser::dstIpAddr, this) = qi::_1]
    > -qi::eol
    > "over a maximum of" > qi::uint_ > "hops" > -qi::lit(':')
    ;

  windowsHop =
    qi::uint_ [(pnx::bind(&Parser::hopCount, this) = qi::_1)]
    > +(qi::lit('*') | ((qi::lit("<1") | qi::uint_) >> "ms"))
    > ("Request timed out." | windowsDomainIp)
    ;

  windowsDomainIp =
    ( ipAddr
      [(pnx::bind(&Parser::addHop, this, qi::_1))]
    | (fqdn > "[" > ipAddr > "]")
        [(pnx::bind(&nmdo::IpAddress::addAlias, qi::_2, qi::_1, TRACE_REASON),
          pnx::bind(&Parser::addHop, this, qi::_2))]
    )
    ;

  linuxTrace =
    linuxHeader > +qi::eol
    > +(linuxHop > qi::eol)
    > *qi::eol
    ;

  linuxHeader =
    "traceroute to" >> (fqdn | ipAddr)
    > "(" > ipAddr [(pnx::bind(&Parser::dstIpAddr, this) = qi::_1)] > "),"
    > qi::uint_ > "hops max, " > qi::uint_ > "byte packets"
    ;

  linuxHop =
    qi::uint_ [(pnx::bind(&Parser::hopCount, this) = qi::_1)]
    > +( qi::lit('*')
       | (qi::double_ >> "ms")
         // NOTE: below is overly greedy, but we don't leverage the values yet
       | (qi::lexeme[qi::lit('!') > +qi::alnum])
       | linuxDomainIp
      )
    ;

  linuxDomainIp =
    ( (linuxIpAddr > -("(" > linuxIpAddr > ")"))
        [pnx::bind(&Parser::addHop, this, qi::_1)]
    | (fqdn > "(" > linuxIpAddr > ")")
        [(pnx::bind(&nmdo::IpAddress::addAlias, qi::_2, qi::_1, TRACE_REASON),
          pnx::bind(&Parser::addHop, this, qi::_2))]
    )
    ;

  linuxIpAddr =
    ipAddr > -('%' > +(qi::ascii::alnum | qi::ascii::char_("-_.@")))
    ;


  garbageLine =
    (+qi::eol | (+(qi::char_ - qi::eol) > -qi::eol))
    ;

  BOOST_SPIRIT_DEBUG_NODES(
      (start)

      (windowsTrace)
      (windowsHeader)
      (windowsHop)
      (windowsDomainIp)

      (linuxTrace)
      (linuxHeader)
      (linuxHop)
      (linuxDomainIp)

      (garbageLine)
    );
}


// =============================================================================
// Parser helper methods
// =============================================================================
void
Parser::addHop(nmdo::IpAddress& _hop)
{
  _hop.setResponding(true);

  Data trHop;
  trHop.setHopCount(hopCount);
  trHop.setHopIp(_hop);
  trHop.setDstIp(dstIpAddr);

  curHops.emplace_back(trHop);

  // reset variable data
  hopCount = 0;
}

Result
Parser::getData()
{
  return curHops;
}
