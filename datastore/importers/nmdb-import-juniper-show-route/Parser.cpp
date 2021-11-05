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
    *( logicalSystem
     | garbageLine
     )
    ;

  logicalSystem =
    (logicalSystemHeader >>
     *routingInstance >>
     logicalSystemFooter
    )[(qi::_val = pnx::construct<LogicalSystem>(qi::_1, qi::_2))]
    ;

  logicalSystemHeader =
    (qi::lit("logical-system:") > (qi::lit("default") | token) > +qi::eol)
    ;

  logicalSystemFooter =
    ( (qi::lit("-----"))
    | (qi::lit('{') >>
       *(qi::ascii::graph - qi::char_("}")) >>
       qi::lit('}')
      )
    | (+(qi::ascii::graph - qi::char_('@')) >>
       qi::lit('@') >>
       +(qi::ascii::graph - qi::char_('>')) >>
       qi::lit('>') >>
       +(qi::char_ - qi::eol)
      )
    ) >>
    +qi::eol
    ;

  routingInstance =
    (routingInstanceHeader >
     *route
    )[(qi::_val = pnx::construct<RoutingInstance>(qi::_1, qi::_2))]
    ;

  routingInstanceHeader =
    routingInstanceId >>
    qi::lit(':') >>
    qi::omit[+(qi::char_ - qi::eol)] >> +qi::eol >>
    -(qi::lit("Restart Complete") > +qi::eol) >>
    (qi::lit("+ = Active Route, - = Last Active, * = Both") > +qi::eol)
    ;

  routingInstanceId =
    +(qi::ascii::graph - qi::char_(":"))
    ;

  route =
    ( ((dstIpNet | dstIpNetBlank)
         [(pnx::bind(&nmdo::Route::setDstIpNet, &qi::_val, qi::_1),
           pnx::bind(&Parser::setCurrDstIpNet, this, qi::_1))] >>
       -qi::eol >>
       ( -qi::lit("*") >>
         qi::lit("[") >>
         (qi::as_string[+qi::ascii::alnum])
           [(pnx::bind(&nmdo::Route::setProtocol, &qi::_val, qi::_1))] >>
         qi::lit("/") >>
         (qi::ulong_)
           [(pnx::bind(&nmdo::Route::setAdminDistance, &qi::_val, qi::_1))] >>
         qi::lit("]")
       ) >>
       qi::omit[+token] >>
       qi::eol >
       ( (qi::lit("Local"))
       | (qi::lit("Multicast") >> qi::omit[*token])
       | (qi::lit("MultiRecv"))
       | (qi::lit("MultiResolve"))
       | ((qi::lit("to table") >> token)
            [(pnx::bind(&nmdo::Route::setNextVrfId, &qi::_val, qi::_1))]
         )
       | ( qi::string("Discard")
         | qi::string("MultiDiscard")
         | qi::string("Reject")
         ) [(pnx::bind(&nmdo::Route::setIfaceName, &qi::_val, qi::_1))]
       | (-qi::lit(">") >>
          -(qi::lit("to") >> rtrIpAddr)
             [(pnx::bind(&nmdo::Route::setNextHopIpAddr, &qi::_val, qi::_1))]
         )
       ) >>
       -(qi::lit("via") >> ifaceName)
          [(pnx::bind(&nmdo::Route::setIfaceName, &qi::_val, qi::_1))]
      )
    | ((-qi::lit(">") >>
        (qi::lit("to") >> rtrIpAddr)
          [(pnx::bind(&nmdo::Route::setNextHopIpAddr, &qi::_val, qi::_1))]
       ) >>
       -(qi::lit("via") >> ifaceName)
          [(pnx::bind(&nmdo::Route::setIfaceName, &qi::_val, qi::_1))]
      )
    ) >>
    +qi::eol
    ;

  dstIpNet =
    ipAddr
    ;

  dstIpNetBlank =
    &(-(qi::lit('+') | qi::lit('-') | qi::lit('*')) >> qi::lit('['))
       [(qi::_val = pnx::bind(&Parser::getCurrDstIpNet, this))]
    ;

  rtrIpAddr =
    ipAddr
    ;

  ifaceName =
    token
    ;

  garbageLine =
    (qi::eol | (+(qi::char_ - qi::eol) >> -qi::eol))
    ;

  token =
    +qi::ascii::graph
    ;

  BOOST_SPIRIT_DEBUG_NODES(
      //(start)
      (logicalSystemHeader)
      (logicalSystemFooter)
      (routingInstanceHeader)
      (route)
      (dstIpNet)(dstIpNetBlank)
      (rtrIpAddr)
      (ifaceName)
      //(token)
      //(garbageLine)
      );
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
