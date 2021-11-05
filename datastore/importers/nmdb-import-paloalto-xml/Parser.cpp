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

#include <netmeld/core/utils/StringUtilities.hpp>
#include <netmeld/datastore/parsers/ParserHelper.hpp>
#include <netmeld/datastore/parsers/ParserIpAddress.hpp>
#include <netmeld/datastore/utils/ServiceFactory.hpp>

#include <regex>


namespace nmcu = netmeld::core::utils;
namespace nmdo = netmeld::datastore::objects;
namespace nmdp = netmeld::datastore::parsers;
namespace nmdu = netmeld::datastore::utils;


Data
Parser::getData()
{
  return data;
}


void
Parser::parseConfig(const pugi::xml_node& configNode)
{
  const std::string vsysName;  // Empty virtual-system name for base hardware
  auto& logicalSystem{data.logicalSystems[vsysName]};
  logicalSystem.name = vsysName;

  for (const auto& devicesEntryMatch :
       configNode.select_nodes("devices/entry")) {
    const pugi::xml_node devicesEntryNode{devicesEntryMatch.node()};
    const std::string deviceName{
      devicesEntryNode.attribute("name").value()
    };

    for (const auto& deviceconfigMatch :
         devicesEntryNode.select_nodes("deviceconfig")) {
      const pugi::xml_node deviceconfigNode{deviceconfigMatch.node()};
      parseConfigDeviceconfig(deviceconfigNode, logicalSystem);
    }

    for (const auto& interfaceMatch :
         devicesEntryNode.select_nodes("network/interface")) {
      const pugi::xml_node interfaceNode{interfaceMatch.node()};
      auto parsedIfaces{parseConfigInterface(interfaceNode)};
      logicalSystem.ifaces.merge(parsedIfaces);
      for (const auto& [conflictName, conflict] : parsedIfaces) {
        std::cerr << "ifaces merge conflict: " << conflictName << std::endl;
      }
    }

    for (const auto& virtualRouterMatch :
         devicesEntryNode.select_nodes("network/virtual-router")) {
      const pugi::xml_node virtualRouterNode{virtualRouterMatch.node()};
      auto parsedVrfs{parseConfigVirtualRouter(virtualRouterNode)};
      logicalSystem.vrfs.merge(parsedVrfs);
      for (const auto& [conflictName, conflict] : parsedVrfs) {
        std::cerr << "ifaces merge conflict: " << conflictName << std::endl;
      }
    }

    for (const auto& vsysMatch :
         devicesEntryNode.select_nodes("vsys")) {
      const pugi::xml_node vsysNode{vsysMatch.node()};
      parseConfigVsys(vsysNode);
    }
  }
}


void
Parser::parseConfigDeviceconfig(const pugi::xml_node& deviceconfigNode,
                                LogicalSystem& logicalSystem)
{
  for (const auto& dnsServerMatch :
       deviceconfigNode.select_nodes("system/dns-setting/servers/*")) {
    const pugi::xml_node dnsServerNode{dnsServerMatch.node()};
    nmdo::IpAddress dnsServerIpAddr{dnsServerNode.text().as_string()};
    // Service version
    nmdo::Service dnsService{nmdu::ServiceFactory::makeDns()};
    dnsService.setDstAddress(dnsServerIpAddr);
    logicalSystem.services.emplace_back(dnsService);
    // DnsResolver version
    nmdo::DnsResolver dnsResolver;
    dnsResolver.setDstAddress(dnsServerIpAddr);
    logicalSystem.dnsResolvers.emplace_back(dnsResolver);
  }
  for (const auto& dnsDomainMatch :
       deviceconfigNode.select_nodes("system/domain")) {
    const pugi::xml_node dnsDomainNode{dnsDomainMatch.node()};
    const std::string dnsSearchDomain{dnsDomainNode.text().as_string()};
    logicalSystem.dnsSearchDomains.emplace_back(dnsSearchDomain);
  }

  for (const auto& ntpServerMatch :
       deviceconfigNode.select_nodes("system/ntp-servers/*")) {
    const pugi::xml_node ntpServerNode{ntpServerMatch.node()};
    for (const auto& ntpServerAddrMatch :
         ntpServerNode.select_nodes("ntp-server-address")) {
      const pugi::xml_node ntpServerAddrNode{ntpServerAddrMatch.node()};
      nmdo::IpAddress ntpServerIpAddr{ntpServerAddrNode.text().as_string()};
      nmdo::Service ntpService{nmdu::ServiceFactory::makeNtp()};
      ntpService.setDstAddress(ntpServerIpAddr);
      logicalSystem.services.emplace_back(ntpService);
    }
  }
}


void
Parser::parseConfigVsys(const pugi::xml_node& vsysNode)
{
  for (const auto& entryMatch : vsysNode.select_nodes("entry")) {
    const pugi::xml_node entryNode{entryMatch.node()};
    const std::string vsysName{
      entryNode.attribute("name").value()
    };
    auto& baseSystem{data.logicalSystems[""]};
    auto& logicalSystem{data.logicalSystems[vsysName]};
    logicalSystem.name = vsysName;

    // Pull in the parent device config.
    for (const auto& deviceconfigMatch :
         entryNode.select_nodes("../../deviceconfig")) {
      const pugi::xml_node deviceconfigNode{deviceconfigMatch.node()};
      parseConfigDeviceconfig(deviceconfigNode, logicalSystem);
    }

    // For each virtual system, parse the child element objects
    // and also parse any "shared" objects of the same type.

    for (const auto& importInterfaceMatch :
         entryNode.select_nodes(
           "(import/network/interface/member)|(/config/shared/import/network/interface/member)")) {
      const std::string ifaceName{
        importInterfaceMatch.node().text().as_string()
      };
      // Import copy of iface from base system.
      logicalSystem.ifaces[ifaceName] = baseSystem.ifaces.at(ifaceName);
    }

    for (const auto& zoneMatch :
         entryNode.select_nodes("(zone)|(/config/shared/zone)")) {
      const pugi::xml_node zoneNode{zoneMatch.node()};
      auto parsedZones{parseConfigZone(zoneNode)};
      logicalSystem.aclZones.merge(parsedZones);
      for (const auto& [conflictName, conflict] : parsedZones) {
        std::cerr << "aclZones merge conflict: " << conflictName << std::endl;
      }
    }
    // Due to how intrazone and interzone rules work in Palo Alto,
    // "any" zones need to be expanded to a list where used.
    // So don't create an "any" zone in aclZones.

    for (const auto& addressMatch :
         entryNode.select_nodes("(address)|(/config/shared/address)")) {
      const pugi::xml_node addressNode{addressMatch.node()};
      auto parsedAclIpNetSets{parseConfigAddress(addressNode)};
      logicalSystem.aclIpNetSets.merge(parsedAclIpNetSets);
      for (const auto& [conflictName, conflict] : parsedAclIpNetSets) {
        std::cerr << "aclIpNetSets merge conflict: " << conflictName << std::endl;
      }
    }
    for (const auto& addressGroupMatch :
         entryNode.select_nodes("(address-group)|(/config/shared/address-group)")) {
      const pugi::xml_node addressGroupNode{addressGroupMatch.node()};
      auto parsedAclIpNetSets{parseConfigAddressGroup(addressGroupNode)};
      logicalSystem.aclIpNetSets.merge(parsedAclIpNetSets);
      for (const auto& [conflictName, conflict] : parsedAclIpNetSets) {
        std::cerr << "aclIpNetSets merge conflict: " << conflictName << std::endl;
      }
    }
    logicalSystem.aclIpNetSets["any"].setId("any");
    logicalSystem.aclIpNetSets["any"].addIpNet(nmdo::IpNetwork("0.0.0.0/0"));
    logicalSystem.aclIpNetSets["any"].addIpNet(nmdo::IpNetwork("::/0"));

    for (const auto& serviceMatch :
         entryNode.select_nodes("(service)|(/config/shared/service)")) {
      const pugi::xml_node serviceNode{serviceMatch.node()};
      for (const auto& aclService : parseConfigService(serviceNode)) {
        logicalSystem.aclServices.emplace_back(aclService);
      }
    }
    for (const auto& serviceGroupMatch :
         entryNode.select_nodes("(service-group)|(/config/shared/service-group)")) {
      const pugi::xml_node serviceGroupNode{serviceGroupMatch.node()};
      for (const auto& aclService : parseConfigServiceGroup(serviceGroupNode)) {
        logicalSystem.aclServices.emplace_back(aclService);
      }
    }

    for (const auto& rulebaseMatch :
         entryNode.select_nodes("(rulebase)|(/config/shared/rulebase)")) {
      const pugi::xml_node rulebaseNode{rulebaseMatch.node()};
      for (const auto& aclRuleService : parseConfigRulebase(rulebaseNode, logicalSystem)) {
        logicalSystem.aclRules.emplace_back(aclRuleService);
      }
    }
  }
}


std::map<std::string, nmdo::InterfaceNetwork>
Parser::parseConfigInterface(const pugi::xml_node& interfaceNode)
{
  std::map<std::string, nmdo::InterfaceNetwork> ifaces;

  for (const auto& entryMatch :
       interfaceNode.select_nodes("loopback")) {
    const pugi::xml_node entryNode{entryMatch.node()};
    nmdo::InterfaceNetwork iface{parseConfigInterfaceEntry(entryNode)};
    iface.setMediaType("loopback");
    ifaces[iface.getName()] = iface;
  }

  for (const auto& entryMatch :
       interfaceNode.select_nodes("ethernet/entry")) {
    const pugi::xml_node entryNode{entryMatch.node()};
    nmdo::InterfaceNetwork iface{parseConfigInterfaceEntry(entryNode)};
    ifaces[iface.getName()] = iface;
  }
  for (const auto& entryMatch :
       interfaceNode.select_nodes("ethernet/entry/layer3/units/entry")) {
    const pugi::xml_node entryNode{entryMatch.node()};
    nmdo::InterfaceNetwork iface{parseConfigInterfaceEntry(entryNode)};
    ifaces[iface.getName()] = iface;
  }

  for (const auto& entryMatch :
       interfaceNode.select_nodes("tunnel/units/entry")) {
    const pugi::xml_node entryNode{entryMatch.node()};
    nmdo::InterfaceNetwork iface{parseConfigInterfaceEntry(entryNode)};
    iface.setMediaType("tunnel");
    ifaces[iface.getName()] = iface;
  }

  return ifaces;
}


nmdo::InterfaceNetwork
Parser::parseConfigInterfaceEntry(const pugi::xml_node& ifaceEntryNode)
{
  nmdo::InterfaceNetwork iface;

  std::string ifaceName{
    ifaceEntryNode.attribute("name").value()
  };
  if ("loopback" == std::string{ifaceEntryNode.name()}) {
    ifaceName = ifaceEntryNode.name();
  }
  iface.setName(ifaceName);

  for (const auto& ipEntryMatch :
       ifaceEntryNode.select_nodes(
         "(ip/entry)|"
         "(ipv6/entry)")) {
    std::string ipName{
      ipEntryMatch.node().attribute("name").value()
    };

    if (nmdp::matchString<nmdp::ParserIpAddress, nmdo::IpAddress>(ipName)) {
      nmdo::IpAddress ipAddr{ipName};
      iface.addIpAddress(ipAddr);
    }
    else {
      bool ipFound{false};
      for (const auto& ipNetmaskMatch :
           ifaceEntryNode.select_nodes(
             ("//address/entry[@name='" + ipName + "']/ip-netmask").c_str())) {
        nmdo::IpAddress ipAddr{ipNetmaskMatch.node().text().as_string()};
        iface.addIpAddress(ipAddr);
        ipFound = true;
      }
      if (!ipFound) {
        std::cerr << "Could not find IP for: " << ipName << std::endl;
      }
    }
  }

  return iface;
}


std::map<std::string, nmdo::Vrf>
Parser::parseConfigVirtualRouter(const pugi::xml_node& virtualRouterNode)
{
  std::map<std::string, nmdo::Vrf> vrfs;

  for (const auto& virtualRouterEntryMatch :
       virtualRouterNode.select_nodes("entry")) {
    const pugi::xml_node virtualRouterEntryNode{virtualRouterEntryMatch.node()};
    const std::string vrfName{
      virtualRouterEntryNode.attribute("name").value()
    };
    vrfs[vrfName].setId(vrfName);

    for (const auto& interfaceMemberMatch :
         virtualRouterEntryNode.select_nodes("interface/member")) {
      const pugi::xml_node interfaceMemberNode{interfaceMemberMatch.node()};
      const std::string ifaceName{
        interfaceMemberNode.text().as_string()
      };
      vrfs[vrfName].addIface(ifaceName);
    }

    for (const auto& staticRouteEntryMatch :
         virtualRouterEntryNode.select_nodes(
           "(routing-table/ip/static-route/entry)|"
           "(routing-table/ipv6/static-route/entry)")) {
      const pugi::xml_node staticRouteEntryNode{staticRouteEntryMatch.node()};
      const std::string staticRouteName{
        staticRouteEntryNode.attribute("name").value()
      };

      nmdo::Route route;
      route.setVrfId(vrfName);
      route.setProtocol("static");
      route.setDescription(staticRouteName);

      const auto destinationMatch{
        staticRouteEntryNode.select_node("destination")
      };
      if (destinationMatch) {
        const nmdo::IpAddress dstIpNet{destinationMatch.node().text().as_string()};
        route.setDstIpNet(dstIpNet);
      }

      const auto nextHopIpMatch{
        staticRouteEntryNode.select_node(
            "(nexthop/ip-address)|"
            "(nexthop/ipv6-address)")
      };
      if (nextHopIpMatch) {
        const nmdo::IpAddress rtrIpAddr{nextHopIpMatch.node().text().as_string()};
        route.setNextHopIpAddr(rtrIpAddr);
      }

      const auto interfaceMatch{
        staticRouteEntryNode.select_node("interface")
      };
      if (interfaceMatch) {
        route.setIfaceName(interfaceMatch.node().text().as_string());
      }

      const auto metricMatch{
        staticRouteEntryNode.select_node("metric")
      };
      if (metricMatch) {
        route.setMetric(metricMatch.node().text().as_uint());
      }

      vrfs[vrfName].addRoute(route);
    }
  }

  return vrfs;
}


std::map<std::string, nmdo::AclZone>
Parser::parseConfigZone(const pugi::xml_node& zoneNode)
{
  std::map<std::string, nmdo::AclZone> aclZones;

  for (const auto& zoneEntryMatch :
       zoneNode.select_nodes("entry")) {
    const pugi::xml_node zoneEntryNode{zoneEntryMatch.node()};
    const std::string zoneName{
      zoneEntryNode.attribute("name").value()
    };
    aclZones[zoneName].setId(zoneName);

    for (const auto& memberMatch :
         zoneEntryNode.select_nodes("network/layer3/member")) {
      const pugi::xml_node memberNode{memberMatch.node()};
      const std::string ifaceName{
        memberNode.text().as_string()
      };
      aclZones[zoneName].addIface(ifaceName);
    }
  }

  return aclZones;
}


std::map<std::string, nmdo::AclIpNetSet>
Parser::parseConfigAddress(const pugi::xml_node& addressNode)
{
  std::map<std::string, nmdo::AclIpNetSet> aclIpNetSets;

  for (const auto& entryMatch : addressNode.select_nodes("entry")) {
    const pugi::xml_node entryNode{entryMatch.node()};
    const std::string ipNetSetName{
      entryNode.attribute("name").value()
    };
    aclIpNetSets[ipNetSetName].setId(ipNetSetName);

    for (const auto& ipNetmaskMatch : entryNode.select_nodes("ip-netmask")) {
      const nmdo::IpNetwork ipNet{ipNetmaskMatch.node().text().as_string()};
      aclIpNetSets[ipNetSetName].addIpNet(ipNet);
    }

    for (const auto& fqdnMatch : entryNode.select_nodes("fqdn")) {
      const pugi::xml_node fqdnNode{fqdnMatch.node()};
      const std::string dnsName{fqdnNode.text().as_string()};
      data.observations.addNotable("FQDNs are used that must be resolved");
      aclIpNetSets[ipNetSetName].addHostname(dnsName);
    }
  }

  return aclIpNetSets;
}


std::map<std::string, nmdo::AclIpNetSet>
Parser::parseConfigAddressGroup(const pugi::xml_node& addressGroupNode)
{
  std::map<std::string, nmdo::AclIpNetSet> aclIpNetSets;

  for (const auto& entryMatch : addressGroupNode.select_nodes("entry")) {
    const pugi::xml_node entryNode{entryMatch.node()};
    const std::string ipNetSetName{
      entryNode.attribute("name").value()
    };
    aclIpNetSets[ipNetSetName].setId(ipNetSetName);

    for (const auto& memberMatch : entryNode.select_nodes("static/member")) {
      const std::string ipNetName{
        memberMatch.node().text().as_string()
      };
      aclIpNetSets[ipNetSetName].addIncludedId(ipNetName);
    }
  }

  return aclIpNetSets;
}


std::vector<nmdo::AclService>
Parser::parseConfigService(const pugi::xml_node& serviceNode)
{
  std::vector<nmdo::AclService> aclServices;

  if (true) {
    nmdo::AclService aclService;
    const nmdo::PortRange srcPortRange{0, 65535};
    const nmdo::PortRange dstPortRange{0, 65535};

    aclService.setId("any");
    aclService.setProtocol("any");
    aclService.addSrcPortRange(srcPortRange);
    aclService.addDstPortRange(dstPortRange);

    aclServices.emplace_back(aclService);
  }

  for (const auto& serviceEntryMatch :
       serviceNode.select_nodes("entry")) {
    const pugi::xml_node serviceEntryNode{serviceEntryMatch.node()};
    nmdo::AclService aclService;
    
    const std::string serviceName{
      serviceEntryNode.attribute("name").value()
    };
    aclService.setId(serviceName);

    for (const auto& protocolMatch :
         serviceEntryNode.select_nodes("protocol/child::*")) {
      const pugi::xml_node protocolNode{protocolMatch.node()};
      const std::string protocol{protocolNode.name()};
      aclService.setProtocol(protocol);

      if (("tcp" == protocol) || ("udp" == protocol) ||
          ("sctp" == protocol)) {  // Implicit default source-port range)
        const nmdo::PortRange srcPortRange{0, 65535};
        aclService.addSrcPortRange(srcPortRange);
      }

      const auto portMatch{protocolNode.select_node("port")};
      if (portMatch) {
        const nmdo::PortRange dstPortRange{
          portMatch.node().text().as_string()
        };
        aclService.addDstPortRange(dstPortRange);
      }
      else if (("tcp" == protocol) || ("udp" == protocol) ||
          ("sctp" == protocol)) {  // Implicit default destination-port range)
        const nmdo::PortRange dstPortRange{0, 65535};
        aclService.addDstPortRange(dstPortRange);
      }
    }

    aclServices.emplace_back(aclService);
  }

  return aclServices;
}


std::vector<nmdo::AclService>
Parser::parseConfigServiceGroup(const pugi::xml_node& serviceGroupNode)
{
  std::vector<nmdo::AclService> aclServices;

  for (const auto& serviceGroupEntryMatch :
       serviceGroupNode.select_nodes("entry")) {
    const pugi::xml_node serviceGroupEntryNode{serviceGroupEntryMatch.node()};
    nmdo::AclService aclService;
    
    const std::string serviceGroupName{
      serviceGroupEntryNode.attribute("name").value()
    };
    aclService.setId(serviceGroupName);

    for (const auto& memberMatch :
         serviceGroupEntryNode.select_nodes("members/member")) {
      const std::string memberName{
        memberMatch.node().text().as_string()
      };
      aclService.addIncludedId(memberName);
    }

    aclServices.emplace_back(aclService);
  }

  return aclServices;
}


std::vector<nmdo::AclRuleService>
Parser::parseConfigRulebase(const pugi::xml_node& rulebaseNode,
                            const LogicalSystem& logicalSystem)
{
  std::vector<nmdo::AclRuleService> aclRules;

  for (const auto& rulesMatch : rulebaseNode.select_nodes("security/rules")) {
    const pugi::xml_node rulesNode{rulesMatch.node()};
    for (const auto& aclRule : parseConfigRules(rulesNode, 1000000, logicalSystem)) {
      aclRules.emplace_back(aclRule);
    }
  }

  for (const auto& rulesMatch : rulebaseNode.select_nodes("default-security-rules/rules")) {
    const pugi::xml_node rulesNode{rulesMatch.node()};
    for (const auto& aclRule : parseConfigRules(rulesNode, 2000000, logicalSystem)) {
      aclRules.emplace_back(aclRule);
    }
  }

  //for (const auto& rulesMatch : rulebaseNode.select_nodes("pbf/rules")) {
  //  const pugi::xml_node rulesNode{rulesMatch.node()};
  //  for (const auto& aclRule : parseConfigRules(rulesNode, ?000000, logicalSystem)) {
  //    aclRules.emplace_back(aclRule);
  //  }
  //}

  //for (const auto& rulesMatch : rulebaseNode.select_nodes("nat/rules")) {
  //  const pugi::xml_node rulesNode{rulesMatch.node()};
  //  for (const auto& aclRule : parseConfigRules(rulesNode, ?000000, logicalSystem)) {
  //    aclRules.emplace_back(aclRule);
  //  }
  //}

  return aclRules;
}


std::vector<nmdo::AclRuleService>
Parser::parseConfigRules(const pugi::xml_node& rulesNode, const size_t ruleIdBase,
                         const LogicalSystem& logicalSystem)
{
  std::vector<nmdo::AclRuleService> aclRules;

  size_t ruleId{ruleIdBase};
  for (const auto& rulesEntryMatch : rulesNode.select_nodes("entry")) {
    const pugi::xml_node rulesEntryNode{rulesEntryMatch.node()};

    const std::string description{
      rulesEntryNode.attribute("name").value()
    };

    std::vector<std::string> ruleTypes;
    for (const auto& ruleTypeMatch :
         rulesEntryNode.select_nodes("rule-type")) {
      const std::string ruleType{
        ruleTypeMatch.node().text().as_string()
      };
      ruleTypes.emplace_back(ruleType);
    }
    if (ruleTypes.empty()) {
      if ("intrazone-default" == description) {
        ruleTypes.emplace_back("intrazone");
      }
      else if ("interzone-default" == description) {
        ruleTypes.emplace_back("interzone");
      }
      else {
        ruleTypes.emplace_back("unspecified");
      }
    }

    std::vector<std::string> incomingZoneIds;
    for (const auto& incomingZoneMatch :
         rulesEntryNode.select_nodes("from/member")) {
      const std::string incomingZoneId{
        incomingZoneMatch.node().text().as_string()
      };
      if ("any" == incomingZoneId) {
        // Expand explicit "any" zone to list of defined zone IDs
        for (const auto& [zoneId, zone] : logicalSystem.aclZones) {
          incomingZoneIds.emplace_back(zoneId);
        }
      }
      else {
        incomingZoneIds.emplace_back(incomingZoneId);
      }
    }
    if (incomingZoneIds.empty()) {
      // Expand implicit "any" zone to list of defined zone IDs
      for (const auto& [zoneId, zone] : logicalSystem.aclZones) {
        incomingZoneIds.emplace_back(zoneId);
      }
    }

    std::vector<std::string> outgoingZoneIds;
    for (const auto& outgoingZoneMatch :
         rulesEntryNode.select_nodes("to/member")) {
      const std::string outgoingZoneId {
        outgoingZoneMatch.node().text().as_string()
      };
      if ("any" == outgoingZoneId) {
        // Expand explicit "any" zone to list of defined zone IDs
        for (const auto& [zoneId, zone] : logicalSystem.aclZones) {
          outgoingZoneIds.emplace_back(zoneId);
        }
      }
      else {
        outgoingZoneIds.emplace_back(outgoingZoneId);
      }
    }
    if (outgoingZoneIds.empty()) {
      // Expand implicit "any" zone to list of defined zone IDs
      for (const auto& [zoneId, zone] : logicalSystem.aclZones) {
        outgoingZoneIds.emplace_back(zoneId);
      }
    }

    std::vector<std::string> srcIpNetSetIds;
    for (const auto& sourceMatch :
         rulesEntryNode.select_nodes("source/member")) {
      srcIpNetSetIds.emplace_back(sourceMatch.node().text().as_string());
    }
    if (srcIpNetSetIds.empty()) {
      srcIpNetSetIds.emplace_back("any");
    }

    std::vector<std::string> dstIpNetSetIds;
    for (const auto& destinationMatch :
         rulesEntryNode.select_nodes("destination/member")) {
      dstIpNetSetIds.emplace_back(destinationMatch.node().text().as_string());
    }
    if (dstIpNetSetIds.empty()) {
      dstIpNetSetIds.emplace_back("any");
    }

    std::vector<std::string> serviceIds;
    for (const auto& serviceMatch :
         rulesEntryNode.select_nodes("service/member")) {
      serviceIds.emplace_back(serviceMatch.node().text().as_string());
    }
    if (serviceIds.empty()) {
      serviceIds.emplace_back("any");
    }

    std::string action;
    auto actionMatch{rulesEntryNode.select_node("action")};
    if (actionMatch) {
      const std::string actionValue{actionMatch.node().text().as_string()};
      // Normalize Palo Alto actions to Netmeld actions.
      if (("deny" == actionValue) ||
          ("drop" == actionValue) ||
          ("reset-client" == actionValue) ||
          ("reset-server" == actionValue) ||
          ("reset-both" == actionValue)) {
        action = "block";
      }
      else {
        action = actionValue;
      }
    }

    for (const auto& incomingZoneId : incomingZoneIds) {
      for (const auto& outgoingZoneId : outgoingZoneIds) {
        for (const auto& ruleType : ruleTypes) {
          if ((("intrazone" == ruleType) && (incomingZoneId != outgoingZoneId)) ||
              (("interzone" == ruleType) && (incomingZoneId == outgoingZoneId))) {
            // Skip over invalid incomingZoneId/outgoingZoneId pairings
            // based on the intrazone/interzone ruleType.
            continue;
          }
          for (const auto& srcIpNetSetId : srcIpNetSetIds) {
            for (const auto& dstIpNetSetId : dstIpNetSetIds) {
              for (const auto& service_id : serviceIds) {
                nmdo::AclRuleService aclRule;
                aclRule.setPriority(ruleId);
                aclRule.setAction(action);
                aclRule.setIncomingZoneId(incomingZoneId);
                aclRule.setOutgoingZoneId(outgoingZoneId);
                aclRule.setSrcIpNetSetId(srcIpNetSetId);
                aclRule.setDstIpNetSetId(dstIpNetSetId);
                aclRule.setServiceId(service_id);
                aclRule.setDescription(description);
                aclRules.emplace_back(aclRule);
              }
            }
          }
        }
      }
    }
    ++ruleId;
  }

  return aclRules;
}
