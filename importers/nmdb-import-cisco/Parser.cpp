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
      [pnx::bind(&Parser::setVendor, this, "cisco"),
       qi::_val = pnx::bind(&Parser::getData, this)]
    ;


  config =
    *(
        (qi::lit("hostname") >> domainName >> qi::eol)
           [pnx::bind(&Parser::setDevId, this, qi::_1)]

      | (qi::lit("no cdp run") >> qi::eol)
           [pnx::bind(&Parser::setGlobalCdp, this, false)]
      | (qi::lit("no cdp enable") >> qi::eol)
           [pnx::bind(&Parser::setGlobalCdp, this, false)]

      | (qi::lit("PIX Version") >> *token >> qi::eol)
           [pnx::bind(&Parser::unsup, this, "(global) PIX Version"),
            pnx::bind(&Parser::setGlobalCdp, this, false)]
      | (qi::lit("ASA Version") >> *token >> qi::eol)
           [pnx::bind(&Parser::unsup, this, "(global) ASA Version"),
            pnx::bind(&Parser::setGlobalCdp, this, false)]

      | (qi::lit("spanning-tree portfast bpduguard") >> *token >> qi::eol)
           [pnx::bind(&Parser::setGlobalBpduGuard, this, true)]
      | (qi::lit("spanning-tree portfast bpdufilter") >> *token >> qi::eol)
           [pnx::bind(&Parser::setGlobalBpduFilter, this, true)]

      | (interface)
          [pnx::bind(&Parser::addIface, this, qi::_1)]

      | ((qi::lit("ipv6") | qi::lit("ip")) >> qi::lit("route") >>
         (   (ipAddr >> ipAddr >> ipAddr)
               [pnx::bind(&nmco::IpAddress::setNetmask, &qi::_1, qi::_2),
                pnx::bind(&Parser::addRouteIp, this, qi::_1, qi::_3)]
           | (ipAddr >> ipAddr >> token)
               [pnx::bind(&nmco::IpAddress::setNetmask, &qi::_1, qi::_2),
                pnx::bind(&Parser::addRouteIface, this, qi::_1, qi::_3)]
           | (ipAddr >> ipAddr)
               [pnx::bind(&Parser::addRouteIp, this, qi::_1, qi::_2)]
           | (ipAddr >> token)
               [pnx::bind(&Parser::addRouteIface, this, qi::_1, qi::_2)]
         ) >> -token >> qi::eol)

      | (qi::lit("aaa ") >> tokens >> qi::eol)
            [pnx::bind(&Parser::addAaa, this, qi::_1)]

      | (qi::lit("access-list") >> -tokens >> qi::eol)
           [pnx::bind(&Parser::unsup, this, "access-list")]

      | (qi::lit("ntp server") >> ipAddr >> qi::eol)
           [pnx::bind(&Parser::addNtpService, this, qi::_1)]

      | (qi::lit("snmp-server host") >> ipAddr >> -tokens >> qi::eol)
           [pnx::bind(&Parser::addSnmpService, this, qi::_1)]

      | (vlan)
          [pnx::bind(&Parser::addVlan, this, qi::_1)]

      // ignore the rest
      | (qi::omit[+token >> -qi::eol])
      | (qi::omit[+qi::eol])
    )
    ;

  vlan =
    ((qi::lit("vlan") >> qi::ushort_ >> qi::eol)
        [pnx::bind(&nmco::Vlan::setId, &qi::_val, qi::_1)] >>
     *(qi::no_skip[+qi::char_(' ')] >>
       (
        (qi::lit("name") >> token)
           [pnx::bind(&nmco::Vlan::setDescription, &qi::_val, qi::_1)]

        // Ignore all other settings
        | (qi::omit[+token])
       ) >> qi::eol
      )
    )
    ;


  interface =
    ((
        (qi::lit("interface") >> token >> token >> qi::eol)
           [pnx::bind(&Parser::buildIfaceName, this, qi::_1, qi::_2),
            pnx::bind(&nmco::InterfaceNetwork::setName, &qi::_val, qi::_1)]
      | (qi::lit("interface") >> token >> qi::eol)
           [pnx::bind(&nmco::InterfaceNetwork::setName, &qi::_val, qi::_1)]
     ) >>
    *(qi::no_skip[+qi::char_(' ')] >>
      (
         (qi::lit("inherit port-profile"))
            [pnx::bind(&Parser::unsup, this, "port-profile")]

       | (qi::lit("description") >> tokens)
            [pnx::bind(&nmco::InterfaceNetwork::setDescription, &qi::_val, qi::_1)]

       | (qi::lit("no shutdown"))
            [pnx::bind(&nmco::InterfaceNetwork::setState, &qi::_val, true)]
       | (qi::lit("shutdown"))
            [pnx::bind(&nmco::InterfaceNetwork::setState, &qi::_val, false)]

       | (qi::lit("no cdp enable"))
            [pnx::bind(&nmco::InterfaceNetwork::setDiscoveryProtocol, &qi::_val, false),
             pnx::bind(&Parser::addManuallySetCdpIface, this, qi::_val)]
       | (qi::lit("cdp enable"))
            [pnx::bind(&nmco::InterfaceNetwork::setDiscoveryProtocol, &qi::_val, true),
             pnx::bind(&Parser::addManuallySetCdpIface, this, qi::_val)]

       | (qi::lit("switchport mode") >> token)
            [pnx::bind(&nmco::InterfaceNetwork::setSwitchportMode,
              &qi::_val, "L2 " + qi::_1)]
       | (qi::lit("switchport nonegotiate"))
            [pnx::bind(&nmco::InterfaceNetwork::setSwitchportMode,
              &qi::_val, "L2 nonegotiate")]

       | (qi::lit("switchport port-security mac-address") >> -qi::lit("sticky") >> macAddr)
            [pnx::bind(&nmco::InterfaceNetwork::addPortSecurityStickyMac, &qi::_val, qi::_1)]
       | (qi::lit("no switchport port-security mac-address sticky"))
            [pnx::bind(&nmco::InterfaceNetwork::setPortSecurityStickyMac, &qi::_val, false)]
       | (qi::lit("switchport port-security mac-address sticky"))
            [pnx::bind(&nmco::InterfaceNetwork::setPortSecurityStickyMac, &qi::_val, true)]

       | (qi::lit("switchport port-security maximum") >> qi::ushort_)
            [pnx::bind(&nmco::InterfaceNetwork::setPortSecurityMaxMacAddrs, &qi::_val, qi::_1)]
       | (qi::lit("switchport port-security violation") >> token)
            [pnx::bind(&nmco::InterfaceNetwork::setPortSecurityViolationAction, &qi::_val, qi::_1)]

       | (qi::lit("no switchport port-security"))
            [pnx::bind(&nmco::InterfaceNetwork::setPortSecurity, &qi::_val, false)]
       | (qi::lit("switchport port-security"))
            [pnx::bind(&nmco::InterfaceNetwork::setPortSecurity, &qi::_val, true)]

       | (qi::lit("switchport access vlan") /* default, vlan 1 */ >> qi::ushort_)
            [pnx::bind(&nmco::InterfaceNetwork::addVlan, &qi::_val, qi::_1)]

       | (qi::lit("switchport trunk native vlan") /* default, vlan 1 */ >> qi::ushort_)
            [pnx::bind(&nmco::InterfaceNetwork::addVlan, &qi::_val, qi::_1)]
       // { VLAN-LIST | all | none | [add|except|remove] { VLAN-LIST } }
       | (qi::lit("switchport trunk allowed vlan") /* default is all */ >>
          (  (-qi::lit("add") >>
              ((  (qi::ushort_ >> qi::lit('-') >> qi::ushort_)
                     [pnx::bind(&nmco::InterfaceNetwork::addVlanRange,
                       &qi::_val, qi::_1, qi::_2)]
                | (qi::ushort_)
                     [pnx::bind(&nmco::InterfaceNetwork::addVlan,
                       &qi::_val, qi::_1)]
              ) % qi::lit(',')))
           | (token
                [pnx::bind(&Parser::addObservation, this,
                  "VLAN trunk " + qi::_1)] >> token)
          )
         )
       // { VLAN-ID | [dot1p|none|untagged] }
       | (qi::lit("switchport voice vlan") >> 
          (  qi::ushort_
               [pnx::bind(&nmco::InterfaceNetwork::addVlan, &qi::_val, qi::_1)]
           | token
               [pnx::bind(&Parser::addObservation, this,
                 "voice VLAN " + qi::_1)]
          )
         )
       

       | (qi::lit("no spanning-tree bpduguard"))
            [pnx::bind(&nmco::InterfaceNetwork::setBpduGuard, &qi::_val, false),
             pnx::bind(&Parser::addManuallySetBpduGuardIface, this, qi::_val)]
       | (qi::lit("spanning-tree bpduguard"))
            [pnx::bind(&nmco::InterfaceNetwork::setBpduGuard, &qi::_val, true),
             pnx::bind(&Parser::addManuallySetBpduGuardIface, this, qi::_val)]

       | (qi::lit("no spanning-tree bpdufilter"))
            [pnx::bind(&nmco::InterfaceNetwork::setBpduFilter, &qi::_val, false),
             pnx::bind(&Parser::addManuallySetBpduFilterIface, this, qi::_val)]
       | (qi::lit("spanning-tree bpdufilter"))
            [pnx::bind(&nmco::InterfaceNetwork::setBpduFilter, &qi::_val, true),
             pnx::bind(&Parser::addManuallySetBpduFilterIface, this, qi::_val)]

       | (qi::lit("no spanning-tree portfast"))
            [pnx::bind(&nmco::InterfaceNetwork::setPortfast, &qi::_val, false)]
       | (qi::lit("spanning-tree portfast"))
            [pnx::bind(&nmco::InterfaceNetwork::setPortfast, &qi::_val, true)]

       | ((qi::lit("ipv6") | qi::lit("ip")) >> qi::lit("address") >> ipAddr >> ipAddr)
            [pnx::bind(&nmco::IpAddress::setNetmask, &qi::_1, qi::_2),
             pnx::bind(&nmco::InterfaceNetwork::addIpAddress, &qi::_val, qi::_1)]
       | ((qi::lit("ipv6") | qi::lit("ip")) >> qi::lit("address") >> ipAddr)
            [pnx::bind(&nmco::InterfaceNetwork::addIpAddress, &qi::_val, qi::_1)]

       // No examples of this, cannot verify
       | (qi::lit("standby")>> qi::int_ >> qi::lit("ip") >> ipAddr)
            [pnx::bind(&nmco::InterfaceNetwork::addIpAddress, &qi::_val, qi::_2)]

       | (qi::lit("ip helper-address") >> ipAddr)
            [pnx::bind(&Parser::addDhcpService, this, qi::_1, qi::_val)]

       // Capture Cisco N1000V virtual switches
       // No examples of this, cannot verify
       | (qi::lit("vmware vm mac") >> macAddr)
            [pnx::bind(&nmco::InterfaceNetwork::addReachableMac, &qi::_val, qi::_1)]

       // Capture ADX configs
       // No examples of this, cannot verify
       | (qi::lit("ip-address") >> ipAddr)
            [pnx::bind(&nmco::InterfaceNetwork::addIpAddress, &qi::_val, qi::_1)]

       // Ignore all other settings
       | (qi::omit[+token])
      ) >> qi::eol
     )
    )
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
      (interface)
      //(tokens) (token)
      );
}

// =============================================================================
// Parser helper methods
// =============================================================================

// Global Cdp/Bpdu related
void
Parser::setGlobalCdp(bool isEnabled)
{
  globalCdpEnabled = isEnabled;
}

void
Parser::setGlobalBpduGuard(bool isEnabled)
{
  globalBpduGuardEnabled = isEnabled;
}

void
Parser::setGlobalBpduFilter(bool isEnabled)
{
  globalBpduFilterEnabled = isEnabled;
}

void
Parser::addManuallySetCdpIface(const nmco::InterfaceNetwork& iface)
{
  ifacesCdpManuallySet.insert(iface.getName());
}

void
Parser::addManuallySetBpduGuardIface(const nmco::InterfaceNetwork& iface)
{
  ifacesBpduGuardManuallySet.insert(iface.getName());
}

void
Parser::addManuallySetBpduFilterIface(const nmco::InterfaceNetwork& iface)
{
  ifacesBpduFilterManuallySet.insert(iface.getName());
}

// Device related
void
Parser::setVendor(const std::string& vendor)
{
  d.dhi.setVendor(nmcu::trim(vendor));
}

void
Parser::setDevId(const std::string& id)
{
  d.dhi.setDeviceId(nmcu::trim(id));
}

void
Parser::addAaa(const std::string& aaa)
{
  d.aaas.push_back(nmcu::trim("aaa " + aaa));
}

void
Parser::addObservation(const std::string& obs)
{
  d.observations.addNotable(nmcu::trim(obs));
}

// Service related
void
Parser::addDhcpService(const nmco::IpAddress& ip, const nmco::InterfaceNetwork& iface)
{
  nmco::Service service {"dhcps", ip}; // match nmap output
  service.setProtocol("udp");
  service.addDstPort("67"); // port server uses
  service.addSrcPort("68"); // port client uses
  service.setServiceReason(d.dhi.getDeviceId() + "'s config");
  service.setInterfaceName(iface.getName());
  d.services.push_back(service);
}

void
Parser::addNtpService(const nmco::IpAddress& ip)
{
  nmco::Service service {"ntp", ip};
  service.setProtocol("udp");
  service.addDstPort("123"); // same port used by client and server
  service.addSrcPort("123");
  service.setServiceReason(d.dhi.getDeviceId() + "'s config");
  d.services.push_back(service);
}

void
Parser::addSnmpService(const nmco::IpAddress& ip)
{
  nmco::Service service {"snmp", ip};
  service.setProtocol("udp");
  service.addDstPort("162"); // port manager receives on
  service.setServiceReason(d.dhi.getDeviceId() + "'s config");
  d.services.push_back(service);
}

// Route related
void
Parser::addRouteIp(const nmco::IpAddress& dstNet, const nmco::IpAddress& rtrIp)
{
  nmco::Route route;
  route.setDstNet(dstNet);
  route.setRtrIp(rtrIp);

  d.routes.push_back(route);
}

void
Parser::addRouteIface(const nmco::IpAddress& dstNet, const std::string& rtrIface)
{
  nmco::Route route;
  route.setDstNet(dstNet);
  route.setIfaceName(rtrIface);

  d.routes.push_back(route);
}

// Interface related
void
Parser::buildIfaceName(std::string& name, const std::string& id)
{
  name += " " + id;
}

void
Parser::addIface(nmco::InterfaceNetwork& iface)
{
  d.ifaces.push_back(iface);

  std::string vlanPrefix {"vlan"};
  std::string ifaceName {iface.getName()};
  std::string ifacePrefix {ifaceName.substr(0, vlanPrefix.size())};
  if (ifacePrefix == vlanPrefix) {
    std::istringstream iss {ifaceName.erase(0, vlanPrefix.size())};
    unsigned short id;
    iss >> id;
    for (auto ipAddr : iface.getIpAddresses()) {
      nmco::Vlan vlan {id};
      vlan.setIpNet(ipAddr);
      d.vlans.push_back(vlan);
    }
  }
}

// Vlan related
void
Parser::addVlan(nmco::Vlan& vlan)
{
  d.vlans.push_back(vlan);
}

// Unsupported
void
Parser::unsup(const std::string& val)
{
  d.observations.addUnsupportedFeature(nmcu::trim(val));
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
  for (auto& iface : d.ifaces) {
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
