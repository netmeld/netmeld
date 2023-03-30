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
    *( vrf
     | qi::omit[ignoredLine]
     | qi::omit[qi::eol]
     )
    ;

  vrf =
    (vrfHeader >>
     *(ipv4Route | ipv6Route)
    )[(qi::_val = pnx::construct<Vrf>(qi::_1, qi::_2))]
    ;

  vrfHeader =
    -(qi::lit("VRF") >> -qi::lit("name") >> qi::lit(":") >
      (qi::lit("default") | token) >>
      qi::eol
     ) >>
    -(qi::lit("Displaying") >> ignoredLine) >>
    -(qi::lit("IPv6 Routing Table") >> ignoredLine) >>
    codesLegend >>
    *qi::eol >>
    -(qi::lit("Gateway of last resort") >> ignoredLine) >>
    *qi::eol
    ;

  codesLegend =
    (qi::lit("Codes:") >> ignoredLine) >
    *(+qi::ascii::blank >> ignoredLine)
    ;

  ipv4Route =
    ( (qi::omit[dstIpv4Net] >>
       qi::lit("is") >> -qi::lit("variably") >> qi::lit("subnetted") >>
       qi::omit[+(qi::char_ - qi::eol)]
      )
    | ( ( (typeCodeBlank
             [(pnx::bind(&nmdo::Route::setProtocol, &qi::_val, qi::_1),
               pnx::bind(&Parser::setCurrProtocol, this, qi::_1))] >>
           dstIpv4NetBlank
             [(pnx::bind(&nmdo::Route::setDstIpNet, &qi::_val, qi::_1),
               pnx::bind(&Parser::setCurrDstIpNet, this, qi::_1))]
          )
        | (typeCode
             [(pnx::bind(&nmdo::Route::setProtocol, &qi::_val, qi::_1),
               pnx::bind(&Parser::setCurrProtocol, this, qi::_1))] >>
           dstIpv4Net
             [(pnx::bind(&nmdo::Route::setDstIpNet, &qi::_val, qi::_1),
               pnx::bind(&Parser::setCurrDstIpNet, this, qi::_1))]
          )
        ) >>
        -qi::eol >>
        ( (qi::lit("is directly connected")
             [(pnx::bind(&nmdo::Route::setAdminDistance, &qi::_val, 0)
             , pnx::bind(&nmdo::Route::setMetric, &qi::_val, 0)
             , pnx::bind(&nmdo::Route::setNextHopIpAddr, &qi::_val, nmdo::IpAddress::getIpv4Default())
             )]
          )
        | ((qi::lit('[') >>
            distance
              [(pnx::bind(&nmdo::Route::setAdminDistance, &qi::_val, qi::_1))] >>
            qi::lit('/') >>
            metric
              [(pnx::bind(&nmdo::Route::setMetric, &qi::_val, qi::_1))] >>
            qi::lit(']')
           ) >>
           -qi::eol >>
           qi::lit("via") >>
           rtrIpv4Addr
             [(pnx::bind(&nmdo::Route::setNextHopIpAddr, &qi::_val, qi::_1))]
          )
        ) >>
        -(uptime) >>
        -(ifaceName
            [(pnx::bind(&nmdo::Route::setIfaceName, &qi::_val, qi::_1))]
         )
      )
    ) >>
    +qi::eol
    ;

  ipv6Route =
    typeCode
      [(pnx::bind(&nmdo::Route::setProtocol, &qi::_val, qi::_1))] >>
    (dstIpv6Net
    )[(pnx::bind(&nmdo::Route::setDstIpNet, &qi::_val, qi::_1))] >>
    -qi::eol >>
    (qi::lit('[') >>
     distance
       [(pnx::bind(&nmdo::Route::setAdminDistance, &qi::_val, qi::_1))] >>
     qi::lit('/') >>
     metric
       [(pnx::bind(&nmdo::Route::setMetric, &qi::_val, qi::_1))] >>
     qi::lit(']')
    ) >>
    -qi::eol >>
    ( qi::lit("via") >>
      ( (rtrIpv6Addr
        )[(pnx::bind(&nmdo::Route::setNextHopIpAddr, &qi::_val, qi::_1))]
      | (qi::string("Null0") > -qi::lit(", receive")
        )[(pnx::bind(&nmdo::Route::setIfaceName, &qi::_val, qi::_1))]
      | (csvToken > -qi::lit(", directly connected")
        )[(pnx::bind(&nmdo::Route::setIfaceName, &qi::_val, qi::_1)
         , pnx::bind(&nmdo::Route::setNextHopIpAddr, &qi::_val, nmdo::IpAddress::getIpv6Default())
         )]
      )
    | (qi::string("Null0")
      )[(pnx::bind(&nmdo::Route::setIfaceName, &qi::_val, qi::_1))]
    ) >>
    -(uptime) >>
    -(ifaceName
     )[(pnx::bind(&nmdo::Route::setIfaceName, &qi::_val, qi::_1))] >>
    *qi::eol
    ;

  typeCode =
    -qi::lit(' ') >>
    ( (qi::lit("B"))
        [(qi::_val = "bgp")]
    | (qi::lit("C"))  // connected
        [(qi::_val = "direct")]
    | (qi::lit("DH"))
        [(qi::_val = "dhcp")]
    | (qi::lit("D"))
        [(qi::_val = "eigrp")]
    | (qi::lit("E"))
        [(qi::_val = "egp")]
    | (qi::lit("H"))
        [(qi::_val = "nhrp")]
    | (qi::lit("i"))
        [(qi::_val = "is-is")]
    | (qi::lit("I") >> &(qi::lit(" L1") | qi::lit(" L2")))
        [(qi::_val = "is-is")]
    | (qi::lit("I"))
        [(qi::_val = "igrp")]
    | (qi::lit("K"))  // kernel
        [(qi::_val = "direct")]
    | (qi::lit("L"))
        [(qi::_val = "local")]
    | (qi::lit("O3"))
        [(qi::_val = "ospf3")]
    | (qi::lit("O"))
        [(qi::_val = "ospf")]
    | (qi::lit("P"))  // periodic download static route
        [(qi::_val = "static")]
    | (qi::lit("R"))
        [(qi::_val = "rip")]
    | (qi::lit("S"))
        [(qi::_val = "static")]
    | (qi::lit("U"))
        [(qi::_val = "per-user-static")]
    | (qi::char_)
        [(qi::_val = "")]
    ) >>
    qi::repeat(4)[qi::char_]
    ;

  typeCodeBlank =
    &qi::lit('[')
      [(qi::_val = pnx::bind(&Parser::getCurrProtocol, this))]
    ;

  dstIpv4Net =
    ( (ipv4Addr >> ipv4Addr
      )[(qi::_val = qi::_1,
         pnx::bind(&nmdo::IpAddress::setNetmask, qi::_val, qi::_2))]
    | (ipv4Addr
      )[(qi::_val = qi::_1)]
    )
    ;

  dstIpv4NetBlank =
    &qi::lit('[')
      [(qi::_val = pnx::bind(&Parser::getCurrDstIpNet, this))]
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

  distance =
    qi::uint_;

  metric =
    qi::uint_;

  uptime =
    qi::lit(',') >>
    (  (qi::repeat(2)[qi::ascii::digit] >>
        qi::lit(':') >>
        qi::repeat(2)[qi::ascii::digit] >>
        qi::lit(':') >>
        qi::repeat(2)[qi::ascii::digit]
       )
     | (qi::uint_ >> qi::lit('d') >>
        qi::uint_ >> qi::lit('h')
       )
     | (qi::uint_ >> qi::lit('w') >>
        qi::uint_ >> qi::lit('d')
       )
    )
    ;

  ifaceName =
    qi::lit(',') >> csvToken
    ;

  ignoredLine =
    +(qi::char_ - qi::eol) >> -qi::eol
    ;

  csvToken =
    +(qi::ascii::graph - qi::char_(","))
    ;

  token =
    +(qi::ascii::graph)
    ;

  BOOST_SPIRIT_DEBUG_NODES(
      //(start)
      (vrfHeader)(codesLegend)
      (typeCode)
      (ipv4Route)(ipv6Route)
      (dstIpv4Net)(dstIpv4NetBlank)(dstIpv6Net)
      (distance)
      (metric)
      (rtrIpv4Addr)(rtrIpv6Addr)
      (uptime)
      (ifaceName)
      //(csvToken)
      (ignoredLine)
      );
}

void
Parser::setCurrProtocol(const std::string& _currProtocol)
{
  currProtocol = _currProtocol;
}

std::string
Parser::getCurrProtocol()
{
  return currProtocol;
}

void
Parser::setCurrDstIpNet(const nmdo::IpAddress& _currDstIpNet)
{
  currDstIpNet = _currDstIpNet;
}

nmdo::IpAddress
Parser::getCurrDstIpNet()
{
  return currDstIpNet;
}
