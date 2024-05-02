// =============================================================================
// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
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
    *(defaultRoute | route | nullRoute)
    ;

  defaultRoute =
    dstIpNet [(pnx::bind(&nmdo::Route::setDstIpNet, &qi::_val, qi::_1))]
    >> qi::lit("via")
    >> nextHopIp [(pnx::bind(&nmdo::Route::setNextHopIpAddr, &qi::_val, qi::_1))]
    >> ifaceName [(pnx::bind(&nmdo::Route::setOutIfaceName, &qi::_val, qi::_1))]
    >> qi::omit[*token]
    >> qi::eol [pnx::bind(&Parser::ensureSameFamily, this, qi::_val)]
    ;

  route =
    dstIpNet [(pnx::bind(&nmdo::Route::setDstIpNet, &qi::_val, qi::_1))]
    >> ifaceName
        [(pnx::bind(&nmdo::Route::setOutIfaceName, &qi::_val, qi::_1)
        , pnx::bind(&Parser::curNextHop, this) = pnx::bind([&]()
                        {
                          if (curDestNet.isV4()) {
                            return nmdo::IpAddress::getIpv4Default();
                          } else {
                            return nmdo::IpAddress::getIpv6Default();
                          }
                        }
                   )
        , pnx::bind(&nmdo::Route::setNextHopIpAddr, &qi::_val
                   , pnx::bind(&Parser::curNextHop, this)
                   )
        )]
    // IPv6 doesn't seem to do this, so needs to be optional
    >> -(qi::lit("proto kernel scope link src") >> nextHopIp)
            [(pnx::bind(&nmdo::Route::setNextHopIpAddr, &qi::_val, qi::_1))]
    >> qi::omit[*token]
    >> qi::eol
    ;

  nullRoute =
    ( qi::lit("unreachable") | "blackhole" | "prohibit" )
    > dstIpNet [(pnx::bind(&nmdo::Route::setDstIpNet, &qi::_val, qi::_1))]
    > qi::omit[*token]
    > qi::eol [(pnx::bind(&nmdo::Route::setNullRoute, &qi::_val, true))]
    ;

  dstIpNet =
    ( qi::lit("default")
      [(pnx::bind(&nmdo::IpAddress::setPrefix, &qi::_val, 0))]
    | ipAddr [(qi::_val = qi::_1)]
    ) [( pnx::bind(&nmdo::IpAddress::setReason, &qi::_val, IP_REASON)
       , pnx::bind(&Parser::curDestNet, this) = qi::_val
      )]
    ;

  nextHopIp =
    ipAddr
        [( qi::_val = qi::_1
         , pnx::bind(&nmdo::IpAddress::setReason, &qi::_val, IP_REASON)
         , pnx::bind(&Parser::curNextHop, this) = qi::_val
        )]
    ;

  ifaceName =
    qi::lit("dev ") > token
    ;

  token =
    +qi::ascii::graph
    ;

  BOOST_SPIRIT_DEBUG_NODES(
      (start)
      (defaultRoute) (route)
      (dstIpNet) (nextHopIp)
      (ifaceName)
      //(token)
    );
}

void
Parser::ensureSameFamily(nmdo::Route& _route)
{
  if (curNextHop.isV6() && curDestNet.isV4()) {
    LOG_DEBUG << "Fixing route destination and next-hop family\n";
    curDestNet.setAddress("::");
    curDestNet.setPrefix(0);
    _route.setDstIpNet(curDestNet);
  }
}
