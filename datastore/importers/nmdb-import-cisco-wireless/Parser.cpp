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

#include <netmeld/datastore/utils/ServiceFactory.hpp>

namespace nmdu = netmeld::datastore::utils;

// =============================================================================
// Parser logic
// =============================================================================
Parser::Parser() : Parser::base_type(start)
{
  start =
    (config)
      [(pnx::bind(&Parser::setVendor, this, "cisco"),
        qi::_val = pnx::bind(&Parser::getData, this))]
    ;

  config =
    *(
        (qi::lit("sysname") >> domainName >> qi::eol)
           [(pnx::bind(&Parser::setDevId, this, qi::_1))]
      | (phyIface >> qi::eol)
      | (qi::lit("time ntp server") >> qi::omit[qi::uint_] >> ipAddr)
           [(pnx::bind(&Parser::addNtpService, this, qi::_1))]
      | ignoredLine
     )
    ;

  phyIface =
    (  ((qi::lit("interface address") >> -qi::lit("dynamic-interface")) >>
        token >> ipAddr >> ipAddr >> ipAddr)
          [(pnx::bind(&Parser::addIface2, this,
                      qi::_1, qi::_2, qi::_3, qi::_4))]
     | (qi::lit("interface address") >> token >> ipAddr >> ipAddr)
          [(pnx::bind(&Parser::addIface1, this, qi::_1, qi::_2, qi::_3))]
    )
    ;

  ignoredLine =
      (+token >> qi::eol) // ignored config lines
    | (+qi::eol)          // ignored empty lines
    ;

  tokens =
    qi::as_string[+(token >> *qi::ascii::blank)]
    ;

  token =
    +(qi::ascii::graph)
    ;


  BOOST_SPIRIT_DEBUG_NODES(
      //(start)
      (config)
      (phyIface)
      (ignoredLine)
      //(tokens) (token)
      );
}


// =============================================================================
// Parser helper methods
// =============================================================================
// Device related
void
Parser::setVendor(const std::string& vendor)
{
  d.devInfo.setVendor(vendor);
}

void
Parser::setDevId(const std::string& id)
{
  d.devInfo.setDeviceId(id);
}

void
Parser::addNtpService(const nmdo::IpAddress& ip)
{
  auto service {nmdu::ServiceFactory::makeNtp()};
  service.setDstAddress(ip);
  service.setServiceReason(d.devInfo.getDeviceId() + "'s config");
  d.services.push_back(service);
}

// Interface related
void
Parser::addIface1(const std::string& name,
                  nmdo::IpAddress& ip, const nmdo::IpAddress& mask)
{
  nmdo::IpAddress temp;
  addIface2(name, ip, mask, temp);
}

void
Parser::addIface2(const std::string& name,
                  nmdo::IpAddress& ip, const nmdo::IpAddress& mask,
                  const nmdo::IpAddress& rtr)
{
  ip.setNetmask(mask);

  nmdo::InterfaceNetwork iface;
  iface.setName(name);
  iface.addIpAddress(ip);
  iface.setDiscoveryProtocol(isCdpEnabled);
  iface.setState(true); // assume up for now
  d.ifaces.push_back(iface);

  if (rtr.isValid()){
    nmdo::Route route;
    route.setOutIfaceName(name);
    route.setDstIpNet(ip);
    route.setNextHopIpAddr(rtr);
    route.setVrfId(DEFAULT_VRF_ID);
    d.routes.push_back(route);
  }
}

// Object return
Result
Parser::getData()
{
  Result r;

  if (d != Data()) {
    r.push_back(d);
  }

  return r;
}
