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
    // Skip garbage before
    *(!qi::lit("Windows IP Configuration") >> -qi::omit[+token] >> qi::eol) >>
    (qi::lit("Windows IP Configuration") > +qi::eol >>
     (
      +adapter |
      +(
        -compartmentHeader >>
        hostData >>
        *(adapter)
       )
     )
    ) [qi::_val = pnx::bind(&Parser::getData, this)] >>
    *(ignoredLine) // Skip garbage after
    ;

  compartmentHeader =
    +qi::lit('=') > qi::eol > ignoredLine > +qi::lit('=') > qi::eol
    ;

  hostData =
    +( ("Host Name" >> dots > fqdn > qi::eol)
        [pnx::bind(&Parser::addDevInfo, this, qi::_1)]
        | ("Primary Dns Suffix" >> dots > -fqdn > qi::eol)
        | ignoredLine
       ) >> qi::eol
    ;

  adapter =
    ifaceTypeName > +qi::eol >
    +( ("Physical Address" >> dots > macAddr > qi::eol)
          [(pnx::bind(&Parser::addIfaceMac, this, qi::_1))]
     | (ipLine > qi::eol)
         [(pnx::bind(&Parser::addIfaceIp, this, qi::_1))]
     | ("Media State" >> dots > "Media disconnected" > qi::eol)
          [(pnx::bind(&Parser::setIfaceDown, this))]
     | ("Connection-specific DNS Suffix" >> dots > -token
          [(pnx::bind(&Parser::setIfaceDnsSuffix, this, qi::_1))] > qi::eol)
     | ("Default Gateway" >> dots > (+(getIp
          [(pnx::bind(&Parser::addRoute, this, qi::_1))] > qi::eol) | qi::eol ))
     | servers
     | ignoredLine
    ) >> *qi::eol
    ;

  ifaceTypeName =
    (ifaceType >> "adapter" >>
     qi::as_string[qi::lexeme[+(qi::char_ - ':')]] > ':')
       [(pnx::bind(&Parser::addIface, this, qi::_2, qi::_1))]
    ;

  dots =
    *qi::char_(". ") > ':'
    ;

  servers =
    ( ("DHCP Server" >> dots > getIp > qi::eol)
          [(pnx::bind(&Parser::addService, this, "DHCP", qi::_1))]
     | ("DNS Servers" >> dots > +(getIp > qi::eol)
          [(pnx::bind(&Parser::addService, this, "DNS", qi::_1))])
     | (qi::hold[token >> qi::lit("WINS Server")] >> dots > getIp > qi::eol)
          [(pnx::bind(&Parser::addService, this, "WINS", qi::_2))]
    )
    ;

  ipLine =
    ("IP" >> -("v" > qi::char_("46"))) >> "Address" >> dots > getIp
      [(qi::_val = qi::_1)] >>
    -(qi::eol >> "Subnet Mask" >> dots > getIp)
      [(pnx::bind(&nmdo::IpAddress::setNetmask, &qi::_val, qi::_1))]
    ;

  getIp =
    ipAddr >> -(qi::omit[token])
    ;

  token =
    +qi::ascii::graph
    ;

  ifaceType =
    +(qi::ascii::print - "adapter")
    ;

  ignoredLine =
    +(qi::char_ - qi::eol) > qi::eol
    ;

  BOOST_SPIRIT_DEBUG_NODES(
      //(start)
      (compartmentHeader)
      (hostData)
      (adapter)(ifaceTypeName)
      (servers)
      (ipLine)(getIp)
      //(ignoredLine)(dots)
      //(token)
      );
}

// =============================================================================
// Parser helper methods
// =============================================================================
void
Parser::addDevInfo(const std::string& _hostname)
{
  nmdo::DeviceInformation devInfo;
  devInfo.setDeviceId(_hostname);

  curHostname = devInfo.getDeviceId();
  d.devInfos[curHostname] = devInfo;
}

void
Parser::addIface(const std::string& _name, const std::string& _type)
{
  nmdo::Interface iface;
  std::string whitespace = "\t\n\v\f\r ";
  std::string type = _type;
  type = type.erase(type.find_last_not_of(whitespace) + 1);
  type = type.erase(0, type.find_first_not_of(whitespace));

  iface.setName(_name);
  iface.setMediaType(type);
  iface.setUp();

  curIfaceName = iface.getName();
  d.ifaces[curIfaceName] = iface;
}

void
Parser::addIfaceMac(nmdo::MacAddress& _macAddr)
{
  auto& iface {d.ifaces[curIfaceName]};
  iface.setMacAddress(_macAddr);
}

void
Parser::addIfaceIp(nmdo::IpAddress& _ipAddr)
{
  auto& iface {d.ifaces[curIfaceName]};
  if (dnsSuffix.count(curIfaceName)) {
    auto alias {curHostname};

    if (!curHostname.empty()) {
      alias += '.';
    }
    alias += dnsSuffix[curIfaceName];

    _ipAddr.addAlias(alias, "ipconfig");
  }
  iface.addIpAddress(_ipAddr);
}

void
Parser::setIfaceDown()
{
  auto& iface {d.ifaces[curIfaceName]};
  iface.setDown();
}

void
Parser::setIfaceDnsSuffix(const std::string& _suffix)
{
  dnsSuffix[curIfaceName] = _suffix;
}

void
Parser::addRoute(const nmdo::IpAddress& _ipAddr)
{
  for (auto& ipAddr : d.ifaces[curIfaceName].getIpAddresses()) {
    if (ipAddr.isV4() && _ipAddr.isV4()) {
      nmdo::Route route;
      route.setIfaceName(curIfaceName);
      route.setNextHopIpAddr(_ipAddr);
      route.setDstIpNet(ipAddr);
      d.routes.push_back(route);
    }
    if (ipAddr.isV6() && _ipAddr.isV6()) {
      nmdo::Route route;
      route.setIfaceName(curIfaceName);
      route.setNextHopIpAddr(_ipAddr);
      route.setDstIpNet(ipAddr);
      d.routes.push_back(route);
    }
  }
}

void
Parser::addService(const std::string& _name, const nmdo::IpAddress& _ipAddr)
{
  nmdo::Service service;
  service.setServiceName(_name);
  service.setDstAddress(_ipAddr);
  service.setInterfaceName(curIfaceName);
  service.setServiceDescription("ipconfig");
  if ("DNS" == _name) {
    service.addDstPort("53");
  } else if ("DHCP" == _name) {
    service.addDstPort("67");
  } else if ("WINS" == _name) {
    service.addDstPort("42");
  }

  d.services.push_back(service);
}

// Object return
Result
Parser::getData()
{
  for (auto& [name, iface] : d.ifaces) {
    if (!dnsSuffix.count(name)) { continue; }
    for (auto& ipAddr : iface.getIpAddresses()) {
      ipAddr.addAlias(dnsSuffix[name], "ipconfig");
    }
  }

  Result r;
  r.push_back(d);
  return r;
}
