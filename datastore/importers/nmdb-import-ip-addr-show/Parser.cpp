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
    config [(qi::_val = pnx::bind(&Parser::getData, this))]
    ;

  config =
    *(  iface [(pnx::bind(&Parser::addIface, this, qi::_1))]
      | garbage
      | qi::eol
     )
    ;

  iface =
    // interface def line
    qi::omit[qi::ushort_] >> qi::lit(':')
    >> ifaceName [(qi::_val = pnx::construct<nmdo::Interface>(qi::_1))]
    > qi::lit(':')
    > token [(pnx::bind(&nmdo::Interface::setFlags, &qi::_val, qi::_1))]
    > qi::lit("mtu")
    > qi::uint_ [(pnx::bind(&nmdo::Interface::setMtu, &qi::_val, qi::_1))]
    > qi::omit[*token]
    > qi::eol

    // link line
    > -(qi::lit("link/")
      > token [(pnx::bind(&nmdo::Interface::setMediaType, &qi::_val, qi::_1))]
      > -macAddr [(pnx::bind(&nmdo::Interface::setMacAddress, &qi::_val, qi::_1))]
      > -(qi::lit("brd") >> qi::omit[macAddr]
          > -((+token) [(pnx::bind(&Parser::addObservation, this,
                                   qi::_1, qi::_val))] ) )
      > qi::eol
    )

    // altname lines
    > *(qi::lit("altname") > token > qi::eol)

    // ip lines
    > *(inetLine [(pnx::bind(&nmdo::Interface::addIpAddress, &qi::_val, qi::_1))])
    ;

  ifaceName =
    +(qi::ascii::alnum | qi::ascii::char_("-_.@"))
    ;

  inetLine =
    // NOTE: keep verbatim, we don't want "inet 61.2.3.4" as "inet6 1.2.3.4"
    (qi::lit("inet6") | qi::lit("inet"))
    > ipAddr
    > -(qi::lit("brd") >> qi::omit[ipAddr])
    > qi::lit("scope") >> qi::omit[+token]
    > -qi::eol
    > -(qi::lit("valid_lft") > qi::omit[+token] > -qi::eol)
    ;

  garbage =
    +(qi::char_ - qi::eol) > -qi::eol
    ;

  token =
    +(qi::ascii::graph)
    ;

  BOOST_SPIRIT_DEBUG_NODES(
      //(start)
      (iface) (inetLine) (ifaceName)
      //(token)
      //(garbage)
      );
}


// =============================================================================
// Parser helper methods
// =============================================================================
void
Parser::addObservation(const std::vector<std::string>& observations,
                       const nmdo::Interface& iface)
{
  std::ostringstream oss;
  oss << "Extra link data for " << iface.getName() << ":";
  for (const auto& observation : observations) {
    oss << " " << observation;
  }
  d.observations.addNotable(oss.str());
}

void
Parser::addIface(const nmdo::Interface& iface)
{
  d.ifaces.push_back(iface);
}

Result
Parser::getData()
{
  Result r;

  if (d != Data()) {
    r.push_back(d);
  }

  return r;
}
