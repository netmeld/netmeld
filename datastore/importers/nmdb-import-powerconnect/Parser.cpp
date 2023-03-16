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

// =============================================================================
// Parser logic
// =============================================================================
Parser::Parser() : Parser::base_type(start)
{
  start =
    config
      [(pnx::bind(&Parser::setVendor, this, "Dell"),
        qi::_val = pnx::bind(&Parser::getData, this))]
    ;

  config =
    *( hostname
     | interface
     | ignoredLine
    )
    ;

  hostname =
    ("hostname" > token > qi::eol)
      [(pnx::bind(&Parser::setDevId, this, qi::_1))]
    ;

  interface =
    "interface" >> typeSlot >> qi::eol >>
    *( (("ip address" >> ipAddr >> ipAddr)
         [(pnx::bind(&Parser::addIfaceIp, this, qi::_1, qi::_2))] >>
        -ipAddr
         [(pnx::bind(&Parser::setIfaceGateway, this, qi::_1))] >>
        qi::eol)
     | ("switchport mode" >> token)
         [(pnx::bind(&Parser::updateIfaceSwitchportMode, this, qi::_1))]
     | ("shutdown" >> qi::eol)
         [(pnx::bind(&Parser::disableIface, this))]
     | (ignoredLine - ("exit" > qi::eol))
    ) >> "exit" > qi::eol
    ;

  typeSlot =
      ("vlan" >> token >> -token)
        [(pnx::bind(&Parser::updateIfaceTypeSlot, this, "vlan", qi::_1))]
    | (qi::as_string[+qi::ascii::alpha] >> qi::as_string[+qi::ascii::graph])
        [(pnx::bind(&Parser::updateIfaceTypeSlot, this, qi::_1, qi::_2))]
    ;

  token =
    +(qi::ascii::graph)
    ;

  ignoredLine =
    (  (+token > -qi::eol)
     | (+qi::eol)
    )
    ;

  BOOST_SPIRIT_DEBUG_NODES(
      //(start)
      (config)
      (hostname)(interface)(typeSlot)
      //(token)
      //(ignoredLine)
    );
}


// =============================================================================
// Parser helper methods
// =============================================================================
void
Parser::setVendor(const std::string& _vendor)
{
  d.devInfo.setVendor(_vendor);
}

void
Parser::setDevId(std::string& _id)
{
  _id.erase(std::remove(_id.begin(), _id.end(), '"'), _id.end());
  d.devInfo.setDeviceId(_id);
}

void
Parser::updateIfaceTypeSlot(const std::string& _type, const std::string& _slot)
{
  tgtIfaceName = _type + _slot;

  auto& iface {d.ifaces[tgtIfaceName]};
  iface.setName(tgtIfaceName);
  iface.setMediaType(_type);
  iface.setState(true);
  updateIfaceSwitchportMode("access");
}

void
Parser::addIfaceIp(nmdo::IpAddress& _ip, const nmdo::IpAddress& _mask)
{
  _ip.setNetmask(_mask);
  auto& iface {d.ifaces[tgtIfaceName]};
  iface.addIpAddress(_ip);
}

void
Parser::setIfaceGateway(const nmdo::IpAddress& _ip)
{
  auto& iface {d.ifaces[tgtIfaceName]};
  for (const auto& ip : iface.getIpAddresses()) { // should only be one ip/route
    d.routes[tgtIfaceName].setDstIpNet(ip);
    d.routes[tgtIfaceName].setNextHopIpAddr(_ip);
  }
}

void
Parser::updateIfaceSwitchportMode(const std::string& _mode)
{
  auto& iface {d.ifaces[tgtIfaceName]};
  iface.setSwitchportMode("L2 " + _mode);
}

void
Parser::disableIface()
{
  auto& iface {d.ifaces[tgtIfaceName]};
  iface.setState(false);
}

Result
Parser::getData()
{
  Result r;
  r.push_back(d);
  return r;
}
