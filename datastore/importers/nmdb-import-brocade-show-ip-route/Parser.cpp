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
    *(ipv4Route[(pnx::push_back(qi::_val, qi::_1))]
    | ipv6Route[(pnx::push_back(qi::_val, qi::_1))]
    | ignoredLine[(qi::_pass = true)]
    )
    ;

  ipv4Route =
    -rowNumber
    >> dstIpv4Net(qi::_val)
    > ( qi::lit("DIRECT")
      | rtrIpv4Addr(qi::_val)
      )
    > ifaceName(qi::_val)
    > distanceMetric(qi::_val)
    > typeCode(qi::_val)
    > uptime
    > -srcVrf
    > -qi::eol
    ;

  ipv6Route =
    typeCode(qi::_val)
    >> dstIpv6Net(qi::_val)
    > -qi::eol
    > ( qi::lit("::")
      | rtrIpv6Addr(qi::_val)
      )
    > -qi::eol
    > ifaceName(qi::_val)
    > distanceMetric(qi::_val)
    > uptime
    > -qi::eol
    ;

  dstIpv4Net =
    ipv4Addr [(pnx::bind(&nmdo::Route::setDstIpNet, &qi::_r1, qi::_1))]
    ;

  dstIpv6Net =
    ipv6Addr [(pnx::bind(&nmdo::Route::setDstIpNet, &qi::_r1, qi::_1))]
    ;

  rtrIpv4Addr =
    ipv4Addr [(pnx::bind(&nmdo::Route::setNextHopIpAddr, &qi::_r1, qi::_1))]
    ;

  rtrIpv6Addr =
    ipv6Addr [(pnx::bind(&nmdo::Route::setNextHopIpAddr, &qi::_r1, qi::_1))]
    ;

  ifaceName =
    ( qi::hold[qi::as_string[token >> qi::ascii::blank >> token]]
    | qi::hold[token]
    ) [(pnx::bind(&nmdo::Route::setOutIfaceName, &qi::_r1, qi::_1))]
    ;

  distanceMetric =
    (  qi::uint_
    >> qi::lit('/')
    >> qi::uint_
    ) [(pnx::bind(&nmdo::Route::setAdminDistance, &qi::_r1, qi::_1)
      , pnx::bind(&nmdo::Route::setMetric, &qi::_r1, qi::_2)
      )]
    ;

  tcSym.add
    ("B", "bgp")
    ("C", "direct")
    ("D", "direct")
    ("I", "is-is")
    ("L", "local")
    ("O", "ospf")
    ("R", "rip")
    ("S", "static")
    ;
  typeCode =
    ( qi::as_string[tcSym >> qi::omit[*qi::ascii::graph]]
    | qi::as_string[+qi::ascii::graph]
    ) [(pnx::bind(&nmdo::Route::setProtocol, &qi::_r1, qi::_1))]
    ;

  uptime =
    token
    ;

  srcVrf =
    token
    ;

  ignoredLine =
    ( (+(qi::char_ - qi::eol) >> -qi::eol)
    | (+qi::eol)
    )
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
      (typeCode)
      (uptime)
      (srcVrf)
      //(token)
      //(ignoredLine)
      );
}
