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
  start %=
    +(vrf [(pnx::bind(&Parser::finalizeVrfData, this, qi::_1))])
    ;

  vrf =
    // skips until vrfHeader found; to clean out heading junk data
    *((!vrfHeader) >> (ignoredLine | qi::eol))
    // rule needs to be able to backtrack for EOF up to below rule
    >> ( vrfHeader
       > *(( (qi::eps(!pnx::ref(isIpv6)) >> ipv4Route)
           | (qi::eps( pnx::ref(isIpv6)) >> ipv6Route)
           ) > *qi::eol
         ) [(pnx::bind(&Parser::addRouteToData, this, qi::_val, qi::_1))]
       )
    // skips until vrfHeader found; to clean out trailing junk data
    >> *((!vrfHeader) >> (ignoredLine | qi::eol))
    ;

  vrfHeader =
    ( (vrfHeaderIos > qi::eps(pnx::ref(isIos) = true))
    | (vrfHeaderNxos > qi::eps(pnx::ref(isNxos) = true))
    | (vrfHeaderIosOld > qi::eps(pnx::ref(isIosOld) = true))
    )
    ;

  vrfHeaderIos =
    -(qi::lit("VRF") > -qi::lit("name") > qi::lit(":") > vrfName > qi::eol)
    >> -( qi::lit("Displaying") >> qi::uint_ >> qi::lit("of") >> qi::uint_
       >> -ipv6Routing > ignoredLine
       )
    >> -(ipv6Routing >> qi::lit("Table") > ignoredLine)
    >> codesLegend
    >> *qi::eol
    >> -(qi::lit("Gateway of last resort") > ignoredLine)
    >> *qi::eol
    ;

  vrfName %=
    -qi::lit('"')
    >> (qi::as_string[+(qi::ascii::graph - qi::lit('"'))])
          [(pnx::ref(curVrf) = qi::_1)]
    >> -qi::lit('"')
    ;

  codesLegend =
    qi::lit("Codes:") > ignoredLine
    > *(+qi::ascii::blank > ignoredLine)
    ;

  vrfHeaderIosOld =
    qi::lit("Default gateway is ") >> ignoredLine
    >> qi::eol
    >> qi::lit("Host")
    > qi::lit("Gateway")
    > qi::lit("Last Use")
    > qi::lit("Total Uses")
    > qi::lit("Interface")
    > *qi::eol
    ;

  vrfHeaderNxos =
    (qi::lit("IP Route") | ipv6Routing)
    >> qi::lit("Table for VRF") >> vrfName > qi::eol
    > -(qi::lit("'*' denotes best ucast next-hop") > qi::eol)
    > -(qi::lit("'**' denotes best mcast next-hop") > qi::eol)
    > -(qi::lit("'[x/y]' denotes [preference/metric]") > qi::eol)
    > -(qi::lit("'%<string>' in via output denotes VRF <string>") > qi::eol)
    > *qi::eol
    ;

  ipv6Routing =
    qi::lit("IPv6 ")
    >> qi::char_("rR") >> qi::lit("outing ")
    > qi::eps[(pnx::ref(isIpv6) = true)]
    ;

  ipv4Route %=
    qi::eps[(pnx::ref(curIfaceName) = "")]
    >> ( (qi::eps(pnx::ref(isIos)) >> ipv4RouteIos)
       | (qi::eps(pnx::ref(isNxos)) >> ipv4RouteNxos)
       | (qi::eps(pnx::ref(isIosOld)) >> ipv4RouteIosOld)
       )
    ;

  ipv4RouteIos =
    ( (ipv4RouteHeader)
    | ( ipv4TypeCodeDstIpNet(qi::_val) >> -qi::eol
      >> ( ( qi::lit("is directly connected")
           > -(qi::lit("(source VRF ") > csvToken)
           ) [(pnx::bind(&Parser::updateDistanceMetric, this , qi::_val, 0, 0)
             , pnx::bind(&nmdo::Route::setNextHopIpAddr, &qi::_val
                        , nmdo::IpAddress::getIpv4Default()
                        )
             )]
         | (qi::lit("is a summary"))
         | ( distanceMetric(qi::_val) > -(qi::lit("(source VRF ") > csvToken)
           > ( (qi::lit("via") > rtrIpv4Addr(qi::_val))
             | (qi::lit(",") > uptime > qi::lit(",") > ifaceName(qi::_val))
             )
           )
         )
      > -(qi::lit(",") >> uptime)
      > -(qi::lit(",") >> ifaceName(qi::_val))
      > -egressVrf(qi::_val)
      )
    )
    ;

  ipv4RouteHeader =
    ipv4Addr
    >> qi::lit("is") >> -qi::lit("variably") >> qi::lit("subnetted")
    > +token
    ;

  ipv4TypeCodeDstIpNet =
    typeCode(qi::_r1)
    >> ( (dstIpv4Net(qi::_r1))
       | (token >> ipv4Addr) [(pnx::bind( &Parser::addUnsupported, this
                                        , "Subnet alias -- " + qi::_1
                                        )
                             )]
       )
    ;

  ipv4RouteIosOld =
      qi::lit("ICMP redirect cache is empty")
    | ( dstIpv4Net(qi::_val)
      >> rtrIpv4Addr(qi::_val)
      >> qi::omit[qi::uint_ > qi::lit(':') > qi::uint_]
      > qi::omit[qi::uint_]
      > ifaceName(qi::_val)
      )
    ;

  ipv4RouteNxos =
    -(dstIpv4Net(qi::_val) >> qi::lit(", ubest/mbest: ") >> ignoredLine)
    >> qi::lit("*via")
    > rtrIpv4Addr(qi::_val)
    > -(qi::lit(",") >> ifaceName(qi::_val))
    > qi::lit(",") > distanceMetric(qi::_val)
    > qi::lit(",") > uptime
    > qi::lit(",") > type(qi::_val)
    > -qi::omit[+token]
    ;

  dstIpv4Net =
    ( (ipv4Net)
    | (altRoutePath > qi::attr(pnx::ref(curDstIpNet)))
    ) [(pnx::bind(&Parser::updateDstIpNet, this, qi::_r1, qi::_1))]
    ;

  ipv4Net =
    ( (qi::eps(!pnx::ref(isIosOld)) >> ipv4Addr >> ipv4Addr)
        [(qi::_val = qi::_1
        , pnx::bind(&nmdo::IpNetwork::setNetmask, &qi::_val, qi::_2)
        )]
    | (ipv4Addr)
        [(qi::_val = qi::_1)]
    )
    ;

  rtrIpv4Addr =
      ( ipv4Addr [(pnx::bind(&nmdo::Route::setNextHopIpAddr, &qi::_r1, qi::_1))]
      > -egressVrf(qi::_r1)
      )
    | ifaceName(qi::_r1)
    ;

  ipv6Route %=
    qi::eps[(pnx::ref(curIfaceName) = "")]
    >> ( (qi::eps(pnx::ref(isIos)) >> ipv6RouteIos)
       | (qi::eps(pnx::ref(isNxos)) >> ipv6RouteNxos)
       | (qi::eps(pnx::ref(isIosOld)) >> ipv6RouteIosOld)
       )
    ;

  ipv6RouteIos =
    -ipv6DstLineIos(qi::_val)
    >> ( qi::lit("via")
      > ( (rtrIpv6Addr(qi::_val) > -(qi::lit(",") >> ifaceName(qi::_val)))
        | (ifaceName(qi::_val))
        )
      > -egressVrf(qi::_val)
      > -(qi::lit(", receive"))
      > -(qi::lit(", directly connected")
             [(pnx::bind(&Parser::updateDistanceMetric, this , qi::_val, 0, 0)
             , pnx::bind(&nmdo::Route::setNextHopIpAddr, &qi::_val
                        , nmdo::IpAddress::getIpv6Default()
                        )
             )]
        )
      )
    ;

  ipv6RouteIosOld =
    ipv6RouteIos.alias()
    ;

  ipv6RouteNxos =
    dstIpv6Net(qi::_val) >> -(qi::lit(", ubest/mbest: ") > ignoredLine)
    > qi::lit("*via")
    > ( (rtrIpv6Addr(qi::_val) > -(qi::lit(",") >> ifaceName(qi::_val)))
      | (ifaceName(qi::_val))
      )
    > qi::lit(",") > distanceMetric(qi::_val)
    > qi::lit(",") > uptime
    > qi::lit(",") > type(qi::_val)
    > -qi::omit[+token]
    ;

  ipv6DstLineIos =
       typeCode(qi::_r1) >> dstIpv6Net(qi::_r1) >> -qi::eol
    >> distanceMetric(qi::_r1)
    >> -( ( qi::lit("(source VRF ")
          | qi::lit(", tag ")
          )
        >> qi::omit[*token]
        )
    >> -qi::eol
    ;

  dstIpv6Net =
    ( (ipv6Net)
    | (altRoutePath > qi::attr(pnx::ref(curDstIpNet)))
    ) [(pnx::bind(&Parser::updateDstIpNet, this, qi::_r1, qi::_1))]
    ;

  ipv6Net = // NOTE: just for casting IpAddress to IpNetwork
    ipv6Addr
    ;

  rtrIpv6Addr =
    ipv6Addr [(pnx::bind(&nmdo::Route::setNextHopIpAddr, &qi::_r1, qi::_1))]
    > -egressVrf(qi::_r1)
    ;

  distanceMetric = // [distance/metric]
    ( ( qi::lit('[') > qi::uint_ > qi::lit('/') > qi::uint_ > qi::lit(']')
      ) [(pnx::bind( &Parser::updateDistanceMetric, this
                   , qi::_r1, qi::_1, qi::_2
                   )
        )]
    | (altRoutePath) [(pnx::bind( &Parser::updateDistanceMetric, this
                                , qi::_r1
                                , pnx::ref(curAdminDistance)
                                , pnx::ref(curMetric)
                                )
                     )]
    )
    ;

  typeCode =
    ( (altRoutePath > qi::attr(pnx::ref(curProtocol)))
    | (qi::as_string[qi::repeat(4)[qi::char_]])
    ) [(pnx::bind(&Parser::updateProtocol, this, qi::_r1, qi::_1))]
    ;

  type =
    csvToken [(pnx::bind(&nmdo::Route::setProtocol, &qi::_r1, qi::_1))]
    ;

  altRoutePath =
    ( &qi::lit('[')
    | &qi::lit("via")
    | &qi::lit("*via")
    )
    ;

  uptime = // hh:mm:ss | 0d0h | 0w0d | 0.000000
    ( (qi::repeat(2)[qi::uint_ >> qi::lit(':')] > qi::uint_)
    | (qi::uint_ >> qi::char_("dw") > qi::uint_ > qi::char_("hd"))
    | (qi::lit("0.000000"))
    )
    ;

  ifaceName =
    (!&qi::lit('['))
    > csvToken [(pnx::bind(&nmdo::Route::setOutIfaceName, &qi::_r1, qi::_1)
               , pnx::bind(&Parser::determineNullRoute, this, qi::_r1, qi::_1)
               )]
    ;

  egressVrf =
      ( qi::lit("(egress VRF ")
      > (qi::as_string[+(qi::ascii::graph - qi::char_(")"))]
        ) [(pnx::bind(&nmdo::Route::setNextVrfId, &qi::_r1, qi::_1))]
      > qi::lit(')')
      )
    | ( qi::lit('%')
      > (qi::as_string[+(qi::ascii::graph - qi::char_(":,"))]
        ) [(pnx::bind(&nmdo::Route::setNextVrfId, &qi::_r1, qi::_1))]
      > -(qi::lit(':') > csvToken
        ) [(pnx::bind(&nmdo::Route::setNextTableId, &qi::_r1, qi::_1))]
      )
    ;

  ignoredLine =
    +(qi::char_ - qi::eol) > -qi::eol
    ;

  csvToken =
    +(qi::ascii::graph - qi::char_(","))
    ;

  token =
    +qi::ascii::graph
    ;

  BOOST_SPIRIT_DEBUG_NODES(
      //(start)
      (altRoutePath)
      (codesLegend)
      (distanceMetric)
      (dstIpv4Net)
      (dstIpv6Net)
      (egressVrf)
      (ifaceName)
      (ipv4Net)
      (ipv4Route)
      (ipv4RouteHeader)
      (ipv4RouteIos)
      (ipv4RouteIosOld)
      (ipv4RouteNxos)
      (ipv4TypeCodeDstIpNet)
      (ipv6DstLineIos)
      (ipv6Net)
      (ipv6Route)
      (ipv6RouteIos)
      (ipv6RouteIosOld)
      (ipv6RouteNxos)
      (ipv6Routing)
      (rtrIpv4Addr)
      (rtrIpv6Addr)
      (type)
      (typeCode)
      (uptime)
      (vrf)
      (vrfHeader)
      (vrfHeaderIos)
      (vrfHeaderIosOld)
      (vrfHeaderNxos)
      (vrfName)
      //(csvToken)
      //(token)
      //(ignoredLine)
      //(codesLegend)
      );
}


void
Parser::determineNullRoute(nmdo::Route& _route, const std::string& _ifaceName)
{
  if ("Null0" == _ifaceName) {
    _route.setNullRoute(true);
  }
  if (!curIfaceName.empty()) {
    const auto msg {
      std::format( "Router alias '{}' on interface '{}'"
                 , curIfaceName, _ifaceName
                 )
      };
    addUnsupported(msg);
    curIfaceName = "";
  } else {
    curIfaceName = _ifaceName;
  }
}

void
Parser::addRouteToData(Data& _d, nmdo::Route& _route)
{
  if (nmdo::Route() == _route) { // don't add defaults
    return;
  }
  // NOTE: configs vary between default vrf name, so remove for consistency
  if ("default" == curVrf) {
    curVrf = DEFAULT_VRF_ID;
  }
  _route.setVrfId(curVrf);
  _d.routes.push_back(_route);
}

void
Parser::finalizeVrfData(Data& _d)
{
  _d.observations = curObservations;

  // clear cur's
  curVrf = DEFAULT_VRF_ID;
  curIfaceName = "";
  curProtocol = "";
  curAdminDistance = 0;
  curMetric = 0;
  curDstIpNet = nmdo::IpNetwork();
  curObservations = nmdo::ToolObservations();

  isIpv6    = false;
  isIos     = false;
  isIosOld  = false;
  isNxos    = false;
}

void
Parser::addUnsupported(const std::string& _obs)
{
  std::ostringstream oss;
  oss << "VRF '" << curVrf << "' -- " << _obs;
  curObservations.addUnsupportedFeature(oss.str());
}

void
Parser::updateProtocol(nmdo::Route& _route, const std::string& _proto)
{
  std::string proto {nmcu::trim(_proto)};

  if (typeCodeLookups.contains(proto)) {
    proto = typeCodeLookups.at(proto);
  }

  _route.setProtocol(proto);
  curProtocol = proto;
}

void
Parser::updateDstIpNet(nmdo::Route& _route, nmdo::IpNetwork& _dstIpNet)
{
  _route.setDstIpNet(_dstIpNet);
  curDstIpNet = _dstIpNet;
}

void
Parser::updateDistanceMetric( nmdo::Route& _route
                            , unsigned int adminDistance
                            , unsigned int metric
                            )
{
  _route.setAdminDistance(adminDistance);
  curAdminDistance = adminDistance;
  _route.setMetric(metric);
  curMetric = metric;
}
