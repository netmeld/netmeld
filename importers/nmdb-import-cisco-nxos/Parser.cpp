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
      [pnx::bind([&](){d.devInfo.setVendor("cisco");}),
       qi::_val = pnx::bind(&Parser::getData, this)]
    ;

  // Unknown where belongs
  // vmware vm mac
  // ip vrrp-extended (vrrrpv3 for nexus?)
  // standby \d+ ip

  config =
    *(
        ((qi::lit("switchname") | qi::lit("hostname")) >> domainName >> qi::eol)
           [pnx::bind([&](const std::string& val)
                      {d.devInfo.setDeviceId(val);}, qi::_1)]
      | (qi::lit("ip domain-name") >> domainName >> qi::eol)
           [pnx::bind(&Parser::unsup, this, "ip domain-name")]
      | (qi::lit("no cdp") >> (qi::lit("run") | qi::lit("enable")) > qi::eol)
           [pnx::bind(&Parser::globalCdpEnabled, this) = false]
      | (qi::lit("spanning-tree portfast") >>
         (  qi::lit("bpduguard")
              [pnx::bind(&Parser::globalBpduGuardEnabled, this) = true]
          | qi::lit("bpdufilter")
              [pnx::bind(&Parser::globalBpduFilterEnabled, this) = true]
         ) > *token > qi::eol)
      | (interface)
           [pnx::bind(&Parser::vlanAddIfaceData, this)]
      | (qi::lit("ntp server") >> ipAddr >> -qi::omit[+token] >> qi::eol)
           [pnx::bind(&Parser::serviceAddNtp, this, qi::_1)]
      | (qi::lit("snmp-server host") >> ipAddr >> -qi::omit[+token] >> qi::eol)
           [pnx::bind(&Parser::serviceAddSnmp, this, qi::_1)]
      | (qi::lit("radius-server host") >> ipAddr >> -qi::omit[+token] >> qi::eol)
           [pnx::bind(&Parser::serviceAddRadius, this, qi::_1)]
      | (qi::lit("ip name-server") >> +ipAddr >> qi::eol)
           [pnx::bind(&Parser::serviceAddDns, this, qi::_1)]
      | (qi::lit("logging server") >> ipAddr >> -qi::omit[+token] >> qi::eol)
           [pnx::bind(&Parser::serviceAddSyslog, this, qi::_1)]
      | (((qi::lit("ipv6") | qi::lit("ip")) >> qi::lit("route") >>
          ipAddr >> token >> -token) >> qi::eol)
           [pnx::bind(&Parser::routeAdd, this, qi::_1, qi::_2)]
      | (vlanDef)
      | (qi::omit[+token >> qi::eol])
      | (qi::omit[+qi::eol])
    )
    ;
  // aaa
  // vlan
  // ip(v6) dhcp relay
  // aaa group server radius ISE
  // system default switchport
  //  (makes all ifaces layer 2 access port mode instead of layer3 default)


  interface =
    ((qi::lit("interface") >> token >> qi::eol)
       [pnx::bind(&Parser::ifaceInit, this, qi::_1)] >>
     // Explicitly check to ensure still in interface scope
    *(qi::no_skip[+qi::char_(' ')] >>
      qi::matches[qi::lit("no")]
        [pnx::bind(&Parser::isNo, this) = qi::_1] >>
      (
         (qi::lit("description") >> tokens)
            [pnx::bind(&nmco::InterfaceNetwork::setDescription,
                       pnx::bind(&Parser::tgtIface, this), qi::_1)]
       | (qi::lit("shutdown"))
            [pnx::bind(&nmco::InterfaceNetwork::setState,
                       pnx::bind(&Parser::tgtIface, this),
                       pnx::bind(&Parser::isNo, this))]
       | ((qi::lit("ipv6") | qi::lit("ip")) >> qi::lit("address") >>
          (  (ipAddr >> ipAddr)
                [pnx::bind(&nmco::IpAddress::setNetmask, &qi::_1, qi::_2),
                 pnx::bind(&nmco::InterfaceNetwork::addIpAddress,
                           pnx::bind(&Parser::tgtIface, this), qi::_1)]
           | (ipAddr)
                [pnx::bind(&nmco::InterfaceNetwork::addIpAddress,
                           pnx::bind(&Parser::tgtIface, this), qi::_1)]
          )
         )
       | (qi::lit("inherit port-profile"))
            [pnx::bind(&Parser::unsup, this, "port-profile")]
       | (qi::lit("cdp enable"))
            [pnx::bind(&nmco::InterfaceNetwork::setDiscoveryProtocol,
                       pnx::bind(&Parser::tgtIface, this),
                       !pnx::bind(&Parser::isNo, this)),
             pnx::bind(&Parser::ifaceSetUpdate, this, &ifacesCdpManuallySet)]
//      | (qi::lit("ip helper-address") >> ipAddr)
//           [pnx::bind(&nmco::InterfaceNetwork::addAddress,
//                      pnx::bind(&Parser::tgtIface, this), qi::_1)]
       | (qi::lit("ip dhcp relay address") >> ipAddr)
            [pnx::bind(&Parser::serviceAddDhcp, this, qi::_1)]
       | switchport
       | spanningTree
       // Ignore all other settings
       | (qi::omit[+token])
      ) >> qi::eol
     )
    )
    ;

  switchport =
    qi::lit("switchport") >>
    (  (qi::lit("mode") >> token)
          [pnx::bind(&nmco::InterfaceNetwork::setSwitchportMode,
            pnx::bind(&Parser::tgtIface, this), "L2 " + qi::_1)]
     | (qi::lit("nonegotiate"))
          [pnx::bind(&nmco::InterfaceNetwork::setSwitchportMode,
            pnx::bind(&Parser::tgtIface, this), "L2 nonegotiate")]
     | (qi::lit("port-security maximum") >> qi::ushort_ >>
        qi::lit("vlan") > qi::ushort_)
          [pnx::bind(&Parser::unsup, this,
            pnx::bind([&](size_t a, size_t b)
              {return "port-security maximum " + std::to_string(a) +
                      " vlan " + std::to_string(b);}, qi::_1, qi::_2))]
     | (qi::lit("port-security maximum") >> qi::ushort_)
          [pnx::bind(&nmco::InterfaceNetwork::setPortSecurityMaxMacAddrs,
                     pnx::bind(&Parser::tgtIface, this), qi::_1)]
     | (qi::lit("port-security violation") >> token)
          [pnx::bind(&nmco::InterfaceNetwork::setPortSecurityViolationAction,
                     pnx::bind(&Parser::tgtIface, this), qi::_1)]
     | (qi::lit("port-security mac-address") >> -qi::lit("sticky") >>
        macAddr)
          [pnx::bind(&nmco::InterfaceNetwork::addPortSecurityStickyMac,
                     pnx::bind(&Parser::tgtIface, this), qi::_1)]
     | (qi::lit("port-security mac-address sticky"))
          [pnx::bind(&nmco::InterfaceNetwork::setPortSecurityStickyMac,
                     pnx::bind(&Parser::tgtIface, this),
                     !pnx::bind(&Parser::isNo, this))]
     | (qi::lit("port-security"))
          [pnx::bind(&nmco::InterfaceNetwork::setPortSecurity,
                     pnx::bind(&Parser::tgtIface, this),
                     !pnx::bind(&Parser::isNo, this))]
     | (qi::lit("access vlan") /* default, vlan 1 */ >> qi::ushort_)
          [pnx::bind(&nmco::InterfaceNetwork::addVlan,
                     pnx::bind(&Parser::tgtIface, this), qi::_1)]
     | (qi::lit("trunk native vlan") /* default, vlan 1 */ >> qi::ushort_)
          [pnx::bind(&nmco::InterfaceNetwork::addVlan,
                     pnx::bind(&Parser::tgtIface, this), qi::_1)]
     // { VLAN-LIST | all | none | [add|except|remove] { VLAN-LIST } }
     | (qi::lit("trunk allowed vlan") /* default is all */ >>
        (  (-qi::lit("add") >>
            ((  (qi::ushort_ >> qi::lit('-') >> qi::ushort_)
                   [pnx::bind(&nmco::InterfaceNetwork::addVlanRange,
                     pnx::bind(&Parser::tgtIface, this), qi::_1, qi::_2)]
              | (qi::ushort_)
                   [pnx::bind(&nmco::InterfaceNetwork::addVlan,
                     pnx::bind(&Parser::tgtIface, this), qi::_1)]
            ) % qi::lit(',')))
         | (token >> token)
              [pnx::bind(&Parser::addObservation, this,
                "VLAN trunk " + qi::_1)]
        )
       )
     // { VLAN-ID | [dot1p|none|untagged] }
     | (qi::lit("voice vlan") >> 
        (  qi::ushort_
             [pnx::bind(&nmco::InterfaceNetwork::addVlan,
                        pnx::bind(&Parser::tgtIface, this), qi::_1)]
         | token
             [pnx::bind(&Parser::addObservation, this,
                        "voice VLAN " + qi::_1)]
        )
       )
    )
    ;

  spanningTree =
    qi::lit("spanning-tree") >
    (  (qi::lit("bpduguard") >
        (  qi::lit("enable")
             [pnx::bind([&](){tgtIface->setBpduGuard(true);})]
         | qi::lit("disable")
             [pnx::bind([&](){tgtIface->setBpduGuard(false);})]
        )
       ) [pnx::bind(&Parser::ifaceSetUpdate, this, &ifacesBpduGuardManuallySet)]
     | (qi::lit("bpdufilter") >
        (  qi::lit("enable")
             [pnx::bind([&](){tgtIface->setBpduFilter(true);})]
         | qi::lit("disable")
             [pnx::bind([&](){tgtIface->setBpduFilter(false);})]
        )
       ) [pnx::bind(&Parser::ifaceSetUpdate, this, &ifacesBpduFilterManuallySet)]
     | (qi::lit("port type") >
        (  qi::lit("edge")    // == portfast on
             [pnx::bind([&](){tgtIface->setPortfast(true);})]
         | qi::lit("network") // == portfast off
             [pnx::bind([&](){tgtIface->setPortfast(false);})]
        )
       )
     | qi::omit[+token]
    )
    ;

  vlanDef =
    ((qi::lit("vlan") >> qi::ushort_ >> qi::eol) >>
     (qi::lit("name") >> token >> qi::eol)
    )
      [pnx::bind(&Parser::vlanAdd, this, qi::_1, qi::_2)]
    ;

  tokens =
    qi::as_string[+(token >> *qi::blank)]
    ;

  token =
    +(qi::ascii::graph)
    ;

  BOOST_SPIRIT_DEBUG_NODES(
      //(start)
      (config)
      (interface)(switchport)(spanningTree)
      (vlanDef)
      (tokens) (token)
      );
}

// =============================================================================
// Parser helper methods
// =============================================================================

// Interface related
void
Parser::ifaceInit(const std::string& _name)
{
  tgtIface = &d.ifaces[_name];
  tgtIface->setName(_name);
}

void
Parser::ifaceSetUpdate(std::set<std::string>* const set)
{
  set->insert(tgtIface->getName());
}


// Services related
void
Parser::serviceAddNtp(const nmco::IpAddress& ip)
{
  nmco::Service service {"ntp", ip};
  service.setProtocol("udp");
  service.addDstPort("123"); // same port used by client and server
  service.addSrcPort("123");
  service.setServiceReason(d.devInfo.getDeviceId() + "'s config");
  d.services.push_back(service);
}

void
Parser::serviceAddDhcp(const nmco::IpAddress& ip)
{
  nmco::Service service {"dhcps", ip}; // match nmap output
  service.setProtocol("udp");
  service.addDstPort("67"); // port server uses
  service.addSrcPort("68"); // port client uses
  service.setServiceReason(d.devInfo.getDeviceId() + "'s config");
  d.services.push_back(service);
}

void
Parser::serviceAddSnmp(const nmco::IpAddress& ip)
{
  nmco::Service service {"snmp", ip};
  service.setProtocol("udp");
  service.addDstPort("162"); // port manager receives on
  service.setServiceReason(d.devInfo.getDeviceId() + "'s config");
  d.services.push_back(service);
}

void
Parser::serviceAddRadius(const nmco::IpAddress& ip)
{
  nmco::Service service {"radius", ip};
  service.setProtocol("udp");
  service.addDstPort("1812"); // authentication and authorization
  service.addDstPort("1813"); // accounting
  service.setServiceReason(d.devInfo.getDeviceId() + "'s config");
  d.services.push_back(service);
}

void
Parser::serviceAddDns(const std::vector<nmco::IpAddress>& ips)
{
  for (auto& ip : ips) {
    nmco::Service service {"dns", ip};
    service.setProtocol("udp");
    service.addDstPort("53");
    service.setServiceReason(d.devInfo.getDeviceId() + "'s config");
    d.services.push_back(service);
  }
}

void
Parser::serviceAddSyslog(const nmco::IpAddress& ip)
{
  nmco::Service service {"syslog", ip};
  service.setProtocol("udp");
  service.addDstPort("514");
  service.setServiceReason(d.devInfo.getDeviceId() + "'s config");
  d.services.push_back(service);
}


// Route related
void
Parser::routeAdd(const nmco::IpAddress& dstNet, const std::string& rtrIpStr)
{
  nmco::IpAddress rtrIp;
  if ("Null0" != rtrIpStr) {
    rtrIp = nmco::IpAddress {rtrIpStr};
  }

  nmco::Route route;
  route.setDstNet(dstNet);
  route.setRtrIp(rtrIp);

  d.routes.push_back(route);
}


// VLAN related
void
Parser::vlanAdd(unsigned short id, const std::string& description)
{
  nmco::Vlan vlan {id, description};
  d.vlans.push_back(vlan);
}

void
Parser::vlanAddIfaceData()
{
  std::string vlanPrefix {"vlan"};
  std::string ifaceName {tgtIface->getName()};
  std::string ifacePrefix {ifaceName.substr(0, vlanPrefix.size())};
  if (ifacePrefix == vlanPrefix) {
    std::istringstream iss {ifaceName.erase(0, vlanPrefix.size())};
    unsigned short id;
    iss >> id;
    for (auto& ipAddr : tgtIface->getIpAddresses()) {
      nmco::Vlan vlan {id};
      vlan.setIpNet(ipAddr);
      d.vlans.push_back(vlan);
    }
  }
}


// Unsupported
void
Parser::unsup(const std::string& val)
{
  d.observations.addUnsupportedFeature(val);
}

void
Parser::addObservation(const std::string& obs)
{
  d.observations.addNotable(nmcu::trim(obs));
}


// Object return
Result
Parser::getData()
{
  if (globalCdpEnabled) {
    d.observations.addNotable("CDP is enabled at global scope.");
  }

  if (!globalBpduGuardEnabled && !globalBpduFilterEnabled) {
    d.observations.addNotable(
      "BPDU guard/filter is not enabled at global scope.");
  }

  // Apply global settings to those interfaces that did not manually set them
  for (auto& [name, iface] : d.ifaces) {
    if (!ifacesCdpManuallySet.count(iface.getName())){
      iface.setDiscoveryProtocol(globalCdpEnabled);
    }
    if (!ifacesBpduGuardManuallySet.count(iface.getName())){
      iface.setBpduGuard(globalBpduGuardEnabled);
    }
    if (!ifacesBpduFilterManuallySet.count(iface.getName())){
      iface.setBpduFilter(globalBpduFilterEnabled);
    }
  }

  Result r;
  r.push_back(d);
  return r;
}
