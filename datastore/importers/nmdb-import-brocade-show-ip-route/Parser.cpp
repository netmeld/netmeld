// =============================================================================
// Copyright 2020 National Technology & Engineering Solutions of Sandia, LLC
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
    *( ipv4Route
     | ipv6Route
     | garbage
     | qi::eol
     )
    ;

  ipv4Route =
    -rowNumber >>
    dstIpv4Net
      [(pnx::bind(&nmdo::Route::setDstIpNet, &qi::_val, qi::_1))] >>
    ( qi::lit("DIRECT")
    | rtrIpv4Addr
        [(pnx::bind(&nmdo::Route::setNextHopIpAddr, &qi::_val, qi::_1))]
    ) >>
    ifaceName
      [(pnx::bind(&nmdo::Route::setIfaceName, &qi::_val, qi::_1))] >>
    distance
      [(pnx::bind(&nmdo::Route::setAdminDistance, &qi::_val, qi::_1))] >>
    qi::lit('/') >>
    metric
      [(pnx::bind(&nmdo::Route::setMetric, &qi::_val, qi::_1))] >>
    typeCode
      [(pnx::bind(&nmdo::Route::setProtocol, &qi::_val, qi::_1))] >>
    uptime >>
    -srcVrf >>
    +qi::eol
    ;

  ipv6Route =
    typeCode
      [(pnx::bind(&nmdo::Route::setProtocol, &qi::_val, qi::_1))] >>
    dstIpv6Net
      [(pnx::bind(&nmdo::Route::setDstIpNet, &qi::_val, qi::_1))] >>
    -qi::eol >>
    ( qi::lit("::")
    | rtrIpv6Addr
        [(pnx::bind(&nmdo::Route::setNextHopIpAddr, &qi::_val, qi::_1))]
    ) >>
    -qi::eol >>
    ifaceName
      [(pnx::bind(&nmdo::Route::setIfaceName, &qi::_val, qi::_1))] >>
    distance
      [(pnx::bind(&nmdo::Route::setAdminDistance, &qi::_val, qi::_1))] >>
    qi::lit('/') >>
    metric
      [(pnx::bind(&nmdo::Route::setMetric, &qi::_val, qi::_1))] >>
    uptime >>
    +qi::eol
    ;

  dstIpv4Net =
    ipv4Addr
    ;

  dstIpv6Net =
    ipv6Addr
    ;

  rtrIpv4Addr =
    ipv4Addr
    ;

  rtrIpv6Addr =
    ipv6Addr
    ;

  ifaceName =
    ( (token >> qi::ascii::blank >> token)
    | (token)
    )
    ;

  distance =
    qi::uint_;

  metric =
    qi::uint_;

  typeCode =
    ( (qi::lit("B") >> *qi::ascii::graph)
        [(qi::_val = "bgp")]
    | (qi::lit("C") >> *qi::ascii::graph)  // connected (IPv6)
        [(qi::_val = "direct")]
    | (qi::lit("D") >> *qi::ascii::graph)  // connected (IPv4)
        [(qi::_val = "direct")]
    | (qi::lit("I") >> *qi::ascii::graph)
        [(qi::_val = "is-is")]
    | (qi::lit("L") >> *qi::ascii::graph)
        [(qi::_val = "local")]
    | (qi::lit("O") >> *qi::ascii::graph)
        [(qi::_val = "ospf")]
    | (qi::lit("R") >> *qi::ascii::graph)
        [(qi::_val = "rip")]
    | (qi::lit("S") >> *qi::ascii::graph)
        [(qi::_val = "static")]
    | (+qi::ascii::graph)
        [(qi::_val = "")]
    );

  uptime =
    token
    ;

  srcVrf =
    token
    ;

  garbage =
    +(qi::char_ - qi::eol) >> -qi::eol
    ;

  rowNumber =
    qi::uint_ >> qi::ascii::blank;
    ;

  token =
    +qi::ascii::graph
    ;

  BOOST_SPIRIT_DEBUG_NODES(
      //(start)
      (rowNumber)
      (ipv4Route)
      (ipv6Route)
      (dstIpv4Net)
      (dstIpv6Net)
      (rtrIpv4Addr)
      (rtrIpv6Addr)
      (ifaceName)
      (distance)
      (metric)
      (typeCode)
      (uptime)
      (srcVrf)
      //(token)
      //(garbage)
      );
}
