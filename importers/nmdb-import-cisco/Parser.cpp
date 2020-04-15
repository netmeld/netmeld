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
        (qi::lit("hostname") > domainName > qi::eol)
           [pnx::bind(&Parser::setDevId, this, qi::_1)]

      | (qi::lit("no cdp") >> (qi::lit("run") | qi::lit("enable")) > qi::eol)
           [pnx::bind(&Parser::globalCdpEnabled, this) = false]

      | ((qi::string("PIX") | qi::string("ASA")) >>
         qi::lit("Version") > *token > qi::eol)
           [pnx::bind(&Parser::unsup, this, "(global) " + qi::_1 + " Version"),
            pnx::bind(&Parser::globalCdpEnabled, this) = false]

      | (qi::lit("spanning-tree portfast") >>
          (  qi::lit("bpduguard")
               [pnx::bind(&Parser::globalBpduGuardEnabled, this) = true]
           | qi::lit("bpdufilter")
               [pnx::bind(&Parser::globalBpduFilterEnabled, this) = true]
          ) > *token > qi::eol)

      | (interface)
          [pnx::bind(&Parser::ifaceFinalize, this)]

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

      | (qi::lit("ip access-list") >> policy)

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
     *(indent >>
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
    qi::lit("interface") >>
    (  (token >> token > qi::eol)
          [pnx::bind(&Parser::ifaceInit, this, qi::_1 + " " + qi::_2)]
     | (token > qi::eol)
          [pnx::bind(&Parser::ifaceInit, this, qi::_1)]
    ) >>
    *(indent >>
      qi::matches[qi::lit("no")]
        [pnx::bind(&Parser::isNo, this) = qi::_1] >>
      (  (qi::lit("inherit port-profile"))
            [pnx::bind(&Parser::unsup, this, "port-profile")]
       | (qi::lit("description") >> tokens)
            [pnx::bind(&nmco::InterfaceNetwork::setDescription,
                       pnx::bind(&Parser::tgtIface, this), qi::_1)]
       | (qi::lit("shutdown"))
            [pnx::bind(&nmco::InterfaceNetwork::setState,
                       pnx::bind(&Parser::tgtIface, this),
                       pnx::bind(&Parser::isNo, this))]
       | (qi::lit("cdp enable"))
            [pnx::bind(&nmco::InterfaceNetwork::setDiscoveryProtocol,
                       pnx::bind(&Parser::tgtIface, this),
                       !pnx::bind(&Parser::isNo, this)),
             pnx::bind(&Parser::addManuallySetCdpIface, this)]

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

       | (qi::lit("ip helper-address") >> ipAddr)
            [pnx::bind(&Parser::addDhcpService, this, qi::_1)]

       /* START: No examples of these, cannot verify */
       // HSRP, virtual IP target for redundant network setup
       | (qi::lit("standby")>> qi::int_ >> qi::lit("ip") >> ipAddr)
            [pnx::bind(&nmco::InterfaceNetwork::addIpAddress,
                       pnx::bind(&Parser::tgtIface, this), qi::_2)]
       // Capture Cisco N1000V virtual switches
       | (qi::lit("vmware vm mac") >> macAddr)
            [pnx::bind(&nmco::InterfaceNetwork::addReachableMac,
                       pnx::bind(&Parser::tgtIface, this), qi::_1)]
       // Capture ADX configs
       | (qi::lit("ip-address") >> ipAddr)
            [pnx::bind(&nmco::InterfaceNetwork::addIpAddress,
                       pnx::bind(&Parser::tgtIface, this), qi::_1)]
       /* END: No examples of these, cannot verify */

       | switchport
       | spanningTree

       | (qi::lit("ip access-group") >> token >> token)
            [pnx::bind(&Parser::createAccessGroup, this,
                pnx::bind(&Parser::tgtIface, this), qi::_1, qi::_2)]

       // Ignore all other settings
       | (qi::omit[+token])
      ) >> qi::eol
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

     | (qi::lit("port-security mac-address") >> -qi::lit("sticky") >> macAddr)
          [pnx::bind(&nmco::InterfaceNetwork::addPortSecurityStickyMac,
                     pnx::bind(&Parser::tgtIface, this), qi::_1)]
     | (qi::lit("port-security mac-address sticky"))
          [pnx::bind(&nmco::InterfaceNetwork::setPortSecurityStickyMac,
                     pnx::bind(&Parser::tgtIface, this),
                     !pnx::bind(&Parser::isNo, this))]
     | (qi::lit("port-security maximum") >> qi::ushort_)
          [pnx::bind(&nmco::InterfaceNetwork::setPortSecurityMaxMacAddrs,
                     pnx::bind(&Parser::tgtIface, this), qi::_1)]
     | (qi::lit("port-security violation") >> token)
          [pnx::bind(&nmco::InterfaceNetwork::setPortSecurityViolationAction,
                     pnx::bind(&Parser::tgtIface, this), qi::_1)]
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
                              pnx::bind(&Parser::tgtIface, this),
                              qi::_1, qi::_2)]
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
    (  qi::lit("bpduguard")
         [pnx::bind(&nmco::InterfaceNetwork::setBpduGuard,
                    pnx::bind(&Parser::tgtIface, this),
                    !pnx::bind(&Parser::isNo, this)),
          pnx::bind(&Parser::addManuallySetBpduGuardIface, this)]
     | qi::lit("bpdufilter")
         [pnx::bind(&nmco::InterfaceNetwork::setBpduFilter,
                    pnx::bind(&Parser::tgtIface, this),
                    !pnx::bind(&Parser::isNo, this)),
          pnx::bind(&Parser::addManuallySetBpduFilterIface, this)]
     | (qi::lit("portfast"))
          [pnx::bind(&nmco::InterfaceNetwork::setPortfast,
                     pnx::bind(&Parser::tgtIface, this),
                     !pnx::bind(&Parser::isNo, this))]
     | qi::omit[+token]
    )
    ;

  policy =
    ( // "ip access-list standard" NAME
      (qi::lit("standard") >> token >> qi::eol)
        [pnx::bind(&Parser::unsup, this, "ip access-list standard " + qi::_1)]
     |
       // "ip access-list extended" NAME
       //    ACTION PROTOCOL SOURCE [PORTS] [ DEST [PORTS] ] ["log"]
       //   ---
       //    ACTION ( "permit" | "deny" )
       //    PROTOCOL ( name )
       //    SOURCE ( IpAddr *mask | "host" IpAddr | "any" )
       //      - Note: *mask (wildcard mask) takes bit representation of mask and
       //              compares with bits of IpAddr. (1=wild, 0=must match IpAddr)
       //    DEST ( IpAddr *mask | "host" IpAddr | "any" | "object-group" name )
       //    PORTS ( "eq" port | "range" startPort endPort )
       //    "log" (Couldn't find anything about options to this in this context)
      (qi::lit("extended") >> token >> qi::eol) // NAME
        [pnx::bind(&Parser::updateCurRuleBook, this, qi::_1)] >>
      *(indent
         [pnx::bind(&Parser::updateCurRule, this)] >>
        token // ACTION
          [pnx::bind(&Parser::setCurRuleAction, this, qi::_1)] >>
        token // PROTOCOL
          [pnx::bind(&Parser::setCurRuleProtocol, this, qi::_1)] >>
        source >>
        -(ports
           [pnx::bind(&Parser::setCurRuleSrcPorts, this, qi::_1)]
         ) >>
        -(destination >>
          -(ports
             [pnx::bind(&Parser::setCurRuleDstPorts, this, qi::_1)]
           )
         ) >>
        -qi::lit("log") >>
        qi::eol
       ) [pnx::bind(&Parser::curRuleFinalize, this)]
     )
    ;

  source =
    ( (ipAddr >> ipAddr)
        [pnx::bind(&Parser::setCurRuleSrcIpMask, this, qi::_1, qi::_2)]
     |
      (qi::lit("host") >> ipAddr)
        [pnx::bind(&Parser::setCurRuleSrcHostIp, this, qi::_1)]
     |
      (qi::lit("any"))
        [pnx::bind(&Parser::setCurRuleSrcAny, this)]
    )
    ;

  destination =
    ( (ipAddr >> ipAddr)
        [pnx::bind(&Parser::setCurRuleDstIpMask, this, qi::_1, qi::_2)]
     |
      (qi::lit("host") >> ipAddr)
        [pnx::bind(&Parser::setCurRuleDstHostIp, this, qi::_1)]
     |
      (qi::lit("any"))
        [pnx::bind(&Parser::setCurRuleDstAny, this)]
     |
      (qi::lit("object-group") >> token)
        [pnx::bind(&Parser::setCurRuleDstObjectGroup, this, qi::_1)]
    )
    ;

  ports =
    ( (qi::lit("eq") >> token)
        [qi::_val = qi::_1]
     |
      (qi::lit("range") >> token >> token)
        [qi::_val = (qi::_1 + "-" + qi::_2)]
    )
    ;

  indent =
    qi::no_skip[+qi::char_(' ')]
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
      (policy)(source)(destination)(ports)
      (vlan)
      (tokens)(token)
      );
}

// =============================================================================
// Parser helper methods
// =============================================================================

void
Parser::addManuallySetCdpIface()
{
  ifacesCdpManuallySet.insert(tgtIface->getName());
}

void
Parser::addManuallySetBpduGuardIface()
{
  ifacesBpduGuardManuallySet.insert(tgtIface->getName());
}

void
Parser::addManuallySetBpduFilterIface()
{
  ifacesBpduFilterManuallySet.insert(tgtIface->getName());
}

// Device related
void
Parser::setVendor(const std::string& vendor)
{
  d.devInfo.setVendor(nmcu::trim(vendor));
}

void
Parser::setDevId(const std::string& id)
{
  d.devInfo.setDeviceId(nmcu::trim(id));
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
Parser::addDhcpService(const nmco::IpAddress& ip)
{
  nmco::Service service {"dhcps", ip}; // match nmap output
  service.setProtocol("udp");
  service.addDstPort("67"); // port server uses
  service.addSrcPort("68"); // port client uses
  service.setServiceReason(d.devInfo.getDeviceId() + "'s config");
  service.setInterfaceName(tgtIface->getName());
  d.services.push_back(service);
}

void
Parser::addNtpService(const nmco::IpAddress& ip)
{
  nmco::Service service {"ntp", ip};
  service.setProtocol("udp");
  service.addDstPort("123"); // same port used by client and server
  service.addSrcPort("123");
  service.setServiceReason(d.devInfo.getDeviceId() + "'s config");
  d.services.push_back(service);
}

void
Parser::addSnmpService(const nmco::IpAddress& ip)
{
  nmco::Service service {"snmp", ip};
  service.setProtocol("udp");
  service.addDstPort("162"); // port manager receives on
  service.setServiceReason(d.devInfo.getDeviceId() + "'s config");
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
Parser::ifaceInit(const std::string& _name)
{
  tgtIface = &d.ifaces[_name];
  tgtIface->setName(_name);
}

void
Parser::ifaceFinalize()
{
  std::string vlanPrefix {"vlan"};
  std::string ifaceName {tgtIface->getName()};
  std::string ifacePrefix {ifaceName.substr(0, vlanPrefix.size())};
  if (ifacePrefix == vlanPrefix) {
    std::istringstream iss {ifaceName.erase(0, vlanPrefix.size())};
    unsigned short id;
    iss >> id;
    for (auto ipAddr : tgtIface->getIpAddresses()) {
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


// Policy Related
void
Parser::createAccessGroup(nmco::InterfaceNetwork* iface,
                          const std::string& bookName,
                          const std::string& direction)
{
  appliedRuleSets[bookName] = {iface->getName(), direction};
}

void
Parser::updateCurRuleBook(const std::string& name)
{
  curRuleBook = name;
  curRuleId = 0;
}

void
Parser::updateCurRule()
{
  ++curRuleId;
  curRuleProtocol = "";
  curRuleSrcPort = "";
  curRuleDstPort = "";

  d.ruleBooks[curRuleBook][curRuleId].setRuleId(curRuleId);
  d.ruleBooks[curRuleBook][curRuleId].setRuleDescription(curRuleBook);
}

void
Parser::setCurRuleAction(const std::string& action)
{
  d.ruleBooks[curRuleBook][curRuleId].addAction(action);
}

void
Parser::setCurRuleProtocol(const std::string& protocol)
{
  curRuleProtocol = protocol;
}

void
Parser::setCurRuleSrcPorts(const std::string& ports)
{
  curRuleSrcPort = ports;
}

void
Parser::setCurRuleDstPorts(const std::string& ports)
{
  curRuleDstPort = ports;
}

void
Parser::setCurRuleSrcIpMask(nmco::IpAddress& ipAddr, const nmco::IpAddress& mask)
{
  const std::string ipAddrString {setWildcardNetmask(ipAddr, mask)};

  d.ruleBooks[curRuleBook][curRuleId].setSrcId(ZONE);
  d.ruleBooks[curRuleBook][curRuleId].addSrc(ipAddrString);
  d.networkBooks[ZONE][ipAddrString].addData(ipAddrString);
}

void
Parser::setCurRuleSrcHostIp(const nmco::IpAddress& ipAddr)
{
  const std::string ipAddrString {ipAddr.toString()};

  d.ruleBooks[curRuleBook][curRuleId].setSrcId(ZONE);
  d.ruleBooks[curRuleBook][curRuleId].addSrc(ipAddrString);
  d.networkBooks[ZONE][ipAddrString].addData(ipAddrString);
}

void
Parser::setCurRuleSrcAny()
{
  d.ruleBooks[curRuleBook][curRuleId].setSrcId(ZONE);
  d.ruleBooks[curRuleBook][curRuleId].addSrc("any");
  d.networkBooks[ZONE]["any"].addData("any");
}

void
Parser::setCurRuleDstIpMask(nmco::IpAddress& ipAddr, const nmco::IpAddress& mask)
{
  const std::string ipAddrString {setWildcardNetmask(ipAddr, mask)};

  d.ruleBooks[curRuleBook][curRuleId].setDstId(ZONE);
  d.ruleBooks[curRuleBook][curRuleId].addDst(ipAddrString);
  d.networkBooks[ZONE][ipAddrString].addData(ipAddrString);
}

void
Parser::setCurRuleDstHostIp(const nmco::IpAddress& ipAddr)
{
  const std::string ipAddrString {ipAddr.toString()};

  d.ruleBooks[curRuleBook][curRuleId].setDstId(ZONE);
  d.ruleBooks[curRuleBook][curRuleId].addDst(ipAddrString);
  d.networkBooks[ZONE][ipAddrString].addData(ipAddrString);
}

void
Parser::setCurRuleDstAny()
{
  d.ruleBooks[curRuleBook][curRuleId].setDstId(ZONE);
  d.ruleBooks[curRuleBook][curRuleId].addDst("any");
  d.networkBooks[ZONE]["any"].addData("any");
}

void
Parser::setCurRuleDstObjectGroup(const std::string& objectGroup)
{
  d.ruleBooks[curRuleBook][curRuleId].setDstId(ZONE);
  d.ruleBooks[curRuleBook][curRuleId].addDst(objectGroup);
  d.networkBooks[ZONE][objectGroup].addData(objectGroup);

  d.observations.addNotable("access-list rule destination object-group " + objectGroup);
}

std::string
Parser::setWildcardNetmask(nmco::IpAddress& ipAddr, const nmco::IpAddress& mask)
{
  bool is_contiguous = ipAddr.setWildcardNetmask(mask);
  if (!is_contiguous) {
    std::ostringstream msg;
    msg << "IpAddress (" << ipAddr
      << ") set with non-contiguous wildcard netmask (" << mask << ")";

    LOG_WARN << msg.str() << '\n';
    d.observations.addUnsupportedFeature(msg.str());
  }

  return ipAddr.toString();
}

void
Parser::curRuleFinalize()
{
  const std::string serviceString = nmcu::getSrvcString(curRuleProtocol,
                                                        curRuleSrcPort,
                                                        curRuleDstPort);
  d.ruleBooks[curRuleBook][curRuleId].addService(serviceString);
  d.serviceBooks[ZONE][serviceString].addData(serviceString);
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

  // Apply interface apply-groups to affected rules
  for (auto& [bookName, dataPair] : appliedRuleSets) {
    auto& [ifaceName, direction] = dataPair;

    for (auto& [id, rule] : d.ruleBooks[bookName]) {
      if ("in" == direction) {
        // in: filter traffic entering iface, regardless destination
        rule.addSrcIface(ifaceName);
        rule.addDstIface("any");
      } else if ("out" == direction) {
        // out: filter traffic leaving iface, regardless origin
        rule.addSrcIface("any");
        rule.addDstIface(ifaceName);
      } else {
        LOG_ERROR << "Parser::assignRules: Unknown rule direction parsed: "
                  << direction << std::endl;
      }
    }
  }

  Result r;
  r.push_back(d);
  return r;
}
