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
#include <netmeld/datastore/parsers/ParserMacAddress.hpp>
#include <netmeld/datastore/utils/ServiceFactory.hpp>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <cctype>
#include <regex>
#include <algorithm>
#include <iterator>

extern "C" {
#include <netdb.h>
}


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
  for (const auto& groupsMatch :
       configNode.select_nodes("groups")) {
    const pugi::xml_node groupsNode{groupsMatch.node()};
    const std::string groupName{
      groupsNode.select_node("name").node().text().as_string()
    };

    if ("junos-defaults" == groupName) {
      // Parse the junos-defaults configuration.
      parseConfig(groupsNode);
    }
    else {
      // Notify when groups are present.
      // Recommend to user to collect data in a way that applies the groups.
      data.observations.addNotable(
          "Groups are present."
          " Recommend collecting config with"
          " `show configuration | display inheritance | display xml | no-more`"
          );
    }
  }

  // Initialize logical system.
  std::string logicalSystemName;
  const auto nameMatch{configNode.select_node("name")};
  if (nameMatch) {
    logicalSystemName = nmcu::toLower(nameMatch.node().text().as_string());
  }
  if ("junos-defaults" == logicalSystemName) {
    logicalSystemName = "";
  }

  auto& logicalSystem{data.logicalSystems[logicalSystemName]};
  logicalSystem.name = logicalSystemName;

  // Parse system settings. Only copy is at global system scope:
  for (const auto& nameServerMatch :
       configNode.select_nodes(
         "/rpc-reply/configuration/system/name-server[not(@inactive='inactive')]")) {
    const pugi::xml_node nameServerNode{nameServerMatch.node()};
    nmdo::IpAddress dnsResolverIpAddr{
      nameServerNode.select_node("name").node().text().as_string()
    };
    // Service version
    auto dnsService{nmdu::ServiceFactory::makeDns()};
    dnsService.setDstAddress(dnsResolverIpAddr);
    logicalSystem.services.emplace_back(dnsService);
    // DnsResolver version
    nmdo::DnsResolver dnsResolver;
    dnsResolver.setDstAddress(dnsResolverIpAddr);
    logicalSystem.dnsResolvers.emplace_back(dnsResolver);
  }

  for (const auto& domainSearchMatch :
       configNode.select_nodes(
         "/rpc-reply/configuration/system/domain-search[not(@inactive='inactive')]")) {
    const pugi::xml_node domainSearchNode{domainSearchMatch.node()};
    const std::string dnsSearchDomain{domainSearchNode.text().as_string()};
    logicalSystem.dnsSearchDomains.emplace_back(dnsSearchDomain);
  }

  for (const auto& tacplusServerMatch :
       configNode.select_nodes(
         "/rpc-reply/configuration/system/tacplus-server[not(@inactive='inactive')]")) {
    const pugi::xml_node tacplusServerNode{tacplusServerMatch.node()};
    nmdo::IpAddress tacplusServerIpAddr{
      tacplusServerNode.select_node("name").node().text().as_string()
    };
    auto tacplusService{nmdu::ServiceFactory::makeTacacsPlus()};
    tacplusService.setDstAddress(tacplusServerIpAddr);
    logicalSystem.services.emplace_back(tacplusService);
  }

  for (const auto& ntpMatch :
       configNode.select_nodes(
         "/rpc-reply/configuration/system/ntp[not(@inactive='inactive')]")) {
    const pugi::xml_node ntpNode{ntpMatch.node()};
    const auto ntpSrcAddrMatch{
      ntpNode.select_node("source-address[not(@inactive='inactive')]")
    };
    for (const auto& ntpServerMatch :
         ntpNode.select_nodes("server[not(@inactive='inactive')]")) {
      const pugi::xml_node ntpServerNode{ntpServerMatch.node()};
      nmdo::IpAddress ntpServerIpAddr{
        ntpServerNode.select_node("name").node().text().as_string()
      };
      auto ntpService{nmdu::ServiceFactory::makeNtp()};
      ntpService.setDstAddress(ntpServerIpAddr);
      if (ntpSrcAddrMatch) {
        const pugi::xml_node ntpSrcAddrNode{ntpSrcAddrMatch.node()};
        nmdo::IpAddress ntpSrcIpAddr{
          ntpSrcAddrNode.select_node("name").node().text().as_string()
        };
        ntpService.setSrcAddress(ntpSrcIpAddr);
      }
      logicalSystem.services.emplace_back(ntpService);
    }
  }


  // Parse networking settings:
  logicalSystem.ifaces["_self_"].setName("_self_");
  for (const auto& interfacesMatch :
       configNode.select_nodes("interfaces[not(@inactive='inactive')]")) {
    const pugi::xml_node interfacesNode{interfacesMatch.node()};
    auto parsedIfacesTuple{parseConfigInterfaces(interfacesNode)};
    auto parsedIfaces{std::get<0>(parsedIfacesTuple)};
    logicalSystem.ifaces.merge(parsedIfaces);
    for (const auto& [conflictName, conflict] : parsedIfaces) {
      std::cerr << "ifaces merge conflict: " << conflictName << std::endl;
    }
    auto parsedIfaceHierarchies{std::get<1>(parsedIfacesTuple)};
    for (const auto& ifaceHierarchy : parsedIfaceHierarchies) {
      logicalSystem.ifaceHierarchies.emplace_back(ifaceHierarchy);
    }
  }

  for (const auto& routingInstancesMatch :
       configNode.select_nodes("routing-instances[not(@inactive='inactive')]")) {
    const pugi::xml_node routingInstancesNode{routingInstancesMatch.node()};
    auto parsedVrfs{parseConfigRoutingInstances(routingInstancesNode, logicalSystem.ifaces)};
    logicalSystem.vrfs.merge(parsedVrfs);
    for (const auto& [vrfId, vrfConflict] : parsedVrfs) {
      logicalSystem.vrfs[vrfId].merge(vrfConflict);
    }
  }

  for (const auto& routingOptionsMatch :
       configNode.select_nodes("routing-options[not(@inactive='inactive')]")) {
    const pugi::xml_node routingOptionsNode{routingOptionsMatch.node()};
    for (const auto& route : parseConfigRoutingOptions(routingOptionsNode)) {
      logicalSystem.vrfs[""].addRoute(route);
    }
  }

  // Parse firewall settings:
  for (const auto& zonesMatch :
       configNode.select_nodes("security/zones[not(@inactive='inactive')]")) {
    const pugi::xml_node zonesNode{zonesMatch.node()};
    auto parsedAclZones{parseConfigZones(zonesNode)};
    logicalSystem.aclZones.merge(parsedAclZones);
    for (const auto& [conflictName, conflict] : parsedAclZones) {
      std::cerr << "aclZones merge conflict: " << conflictName << std::endl;
    }
  }
  logicalSystem.aclZones["junos-host"].setId("junos-host");
  logicalSystem.aclZones["junos-host"].addIface("_self_");
  logicalSystem.aclZones["any"].setId("any");
  for (const auto& [aclZoneName, aclZone] : logicalSystem.aclZones) {
    if (("any" != aclZoneName) && ("junos-host" != aclZoneName)) {
      logicalSystem.aclZones["any"].addIncludedId(aclZoneName);
    }
  }

  for (const auto& addressBookMatch :
       configNode.select_nodes
       ("(security/address-book[not(@inactive='inactive')])|"
        "(security/zones[not(@inactive='inactive')]/security-zone[not(@inactive='inactive')]/address-book[not(@inactive='inactive')])")) {
    const pugi::xml_node addressBookNode{addressBookMatch.node()};
    auto parsedAclIpNetSets{parseConfigAddressBook(addressBookNode)};
    logicalSystem.aclIpNetSets.merge(parsedAclIpNetSets);
    for (const auto& [conflictName, conflict] : parsedAclIpNetSets) {
      std::cerr << "aclIpNetSets merge conflict: " << conflictName << std::endl;
    }
  }
  logicalSystem.aclIpNetSets["global"];
  for (auto& [aclIpNetSetsNamespace, aclIpNetSetsValue] : logicalSystem.aclIpNetSets) {
    aclIpNetSetsValue["any-ipv4"].setId("any-ipv4", aclIpNetSetsNamespace);
    aclIpNetSetsValue["any-ipv4"].addIpNet(nmdo::IpNetwork("0.0.0.0/0"));
    aclIpNetSetsValue["any-ipv6"].setId("any-ipv6", aclIpNetSetsNamespace);
    aclIpNetSetsValue["any-ipv6"].addIpNet(nmdo::IpNetwork("::/0"));
    aclIpNetSetsValue["any"].setId("any", aclIpNetSetsNamespace);
    aclIpNetSetsValue["any"].addIncludedId("any-ipv4");
    aclIpNetSetsValue["any"].addIncludedId("any-ipv6");
  }

  for (const auto& applicationsMatch :
       configNode.select_nodes("applications[not(@inactive='inactive')]")) {
    const pugi::xml_node applicationsNode{applicationsMatch.node()};
    for (const auto& aclService :
         parseConfigApplications(applicationsNode)) {
      logicalSystem.aclServices.emplace_back(aclService);
    }
  }

  for (const auto& policiesMatch :
       configNode.select_nodes("security/policies[not(@inactive='inactive')]")) {
    const pugi::xml_node policiesNode{policiesMatch.node()};
    for (const auto& aclRule : parseConfigPolicies(policiesNode)) {
      logicalSystem.aclRules.emplace_back(aclRule);
    }
  }

  // Parse logical-systems:
  for (const auto& logicalSystemMatch :
       configNode.select_nodes("logical-systems[not(@inactive='inactive')]")) {
    const pugi::xml_node logicalSystemNode{logicalSystemMatch.node()};
    parseConfig(logicalSystemNode);
  }
}


std::tuple<std::map<std::string, nmdo::InterfaceNetwork>,
           std::vector<InterfaceHierarchy>>
Parser::parseConfigInterfaces(const pugi::xml_node& interfacesNode)
{
  std::map<std::string, nmdo::InterfaceNetwork> ifaces;
  std::vector<InterfaceHierarchy> ifaceHierarchies;

  const std::regex mediaTypeRegex{"^([a-zA-Z]+)\\S*$"};
  std::smatch mediaTypeMatch;

  for (const auto& interfaceRangeMatch :
       interfacesNode.select_nodes("interface-range[not(@inactive='inactive')]")) {
    const pugi::xml_node ifaceRangeNode{interfaceRangeMatch.node()};
    const std::string ifaceRangeName{
      ifaceRangeNode.select_node("name").node().text().as_string()
    };
    for (const auto& memberMatch :
         ifaceRangeNode.select_nodes("member[not(@inactive='inactive')]")) {
      const pugi::xml_node memberNode{memberMatch.node()};
      const std::string ifaceName{
        memberNode.select_node("name").node().text().as_string()
      };
      ifaces[ifaceName].setName(ifaceName);
      ifaces[ifaceName].setDescription(ifaceRangeName);
      if (std::regex_match(ifaceName, mediaTypeMatch, mediaTypeRegex)) {
        ifaces[ifaceName].setMediaType(mediaTypeMatch[1]);
      }

      for (const auto optionsMatch : memberNode.select_nodes(
             "(../gigether-options[not(@inactive='inactive')])|"
             "(../ether-options[not(@inactive='inactive')])")) {
        const pugi::xml_node optionsNode{optionsMatch.node()};
        for (const auto virtualIfaceMatch : optionsNode.select_nodes(
               "(redundant-parent[not(@inactive='inactive')]/parent[not(@inactive='inactive')])|"
               "(ieee-802.3ad[not(@inactive='inactive')]/bundle[not(@inactive='inactive')])")) {
          const std::string virtualIfaceName{
            virtualIfaceMatch.node().text().as_string()
          };
          ifaces[virtualIfaceName].setName(virtualIfaceName);
          ifaceHierarchies.emplace_back(ifaceName, virtualIfaceName);
        }
      }
    }
  }

  for (const auto& interfaceMatch :
       interfacesNode.select_nodes("interface[not(@inactive='inactive')]")) {
    const pugi::xml_node ifaceNode{interfaceMatch.node()};
    const std::string ifaceName{
      ifaceNode.select_node("name").node().text().as_string()
    };

    // Physical interface
    if (true) {
      ifaces[ifaceName].setName(ifaceName);
      if (std::regex_match(ifaceName, mediaTypeMatch, mediaTypeRegex)) {
        ifaces[ifaceName].setMediaType(mediaTypeMatch[1]);
      }

      const auto descriptionMatch{ifaceNode.select_node("description")};
      if (descriptionMatch) {
        ifaces[ifaceName].setDescription(descriptionMatch.node().text().as_string());
      }
      const auto disableMatch{ifaceNode.select_node("disable")};
      if (disableMatch) {
        ifaces[ifaceName].setState(false);
      }

      for (const auto optionsMatch : ifaceNode.select_nodes(
             "(gigether-options[not(@inactive='inactive')])|"
             "(ether-options[not(@inactive='inactive')])")) {
        const pugi::xml_node optionsNode{optionsMatch.node()};
        for (const auto virtualIfaceMatch : optionsNode.select_nodes(
               "(redundant-parent[not(@inactive='inactive')]/parent[not(@inactive='inactive')])|"
               "(ieee-802.3ad[not(@inactive='inactive')]/bundle[not(@inactive='inactive')])")) {
          const std::string virtualIfaceName{
            virtualIfaceMatch.node().text().as_string()
          };
          ifaces[virtualIfaceName].setName(virtualIfaceName);
          ifaceHierarchies.emplace_back(ifaceName, virtualIfaceName);
        }
      }

      for (const auto optionsMatch : ifaceNode.select_nodes(
             "fabric-options[not(@inactive='inactive')]")) {
        const pugi::xml_node optionsNode{optionsMatch.node()};
        for (const auto underlyingIfaceMatch : optionsNode.select_nodes(
             "member-interfaces[not(@inactive='inactive')]/name")) {
          const std::string underlyingIfaceName{
            underlyingIfaceMatch.node().text().as_string()
          };
          ifaces[underlyingIfaceName].setName(underlyingIfaceName);
          ifaceHierarchies.emplace_back(underlyingIfaceName, ifaceName);
        }
      }
    }

    // Logical interface units
    for (const auto& unitMatch :
         ifaceNode.select_nodes("unit[not(@inactive='inactive')]")) {
      const pugi::xml_node unitNode{unitMatch.node()};
      const std::string unitName{
        unitNode.select_node("name").node().text().as_string()
      };
      const std::string ifaceUnitId{ifaceName + "." + unitName};

      ifaceHierarchies.emplace_back(ifaceName, ifaceUnitId);

      ifaces[ifaceUnitId].setName(ifaceUnitId);
      if (std::regex_match(ifaceName, mediaTypeMatch, mediaTypeRegex)) {
        ifaces[ifaceUnitId].setMediaType(mediaTypeMatch[1]);
      }

      const auto descriptionMatch{unitNode.select_node("description")};
      if (descriptionMatch) {
        ifaces[ifaceUnitId].setDescription(descriptionMatch.node().text().as_string());
      }
      const auto disableMatch{unitNode.select_node("(disable)|(../disable)")};
      if (disableMatch) {
        ifaces[ifaceUnitId].setState(false);
      }
      const auto vlanMatch{unitNode.select_node("vlan-id[not(@inactive='inactive')]")};
      if (vlanMatch) {
        const uint16_t vlanId{
          static_cast<uint16_t>(vlanMatch.node().text().as_uint())
        };
        ifaces[ifaceUnitId].addVlan(vlanId);
      }

      for (const auto& addressMatch :
           unitNode.select_nodes(
             "(family[not(@inactive='inactive')]/inet[not(@inactive='inactive')]/address[not(@inactive='inactive')])|"
             "(family[not(@inactive='inactive')]/inet6[not(@inactive='inactive')]/address[not(@inactive='inactive')])")) {
        const pugi::xml_node addressNode{addressMatch.node()};
        nmdo::IpAddress ipAddr{
          addressNode.select_node("name").node().text().as_string()
        };
        ifaces[ifaceUnitId].addIpAddress(ipAddr);

        for (const auto& arpMatch :
             addressNode.select_nodes("arp[not(@inactive='inactive')]")) {
          const pugi::xml_node arpNode{arpMatch.node()};
          const auto peerMacAddrMatch{
            arpNode.select_node("mac[not(@inactive='inactive')]")
          };
          if (peerMacAddrMatch &&
              nmdp::matchString<nmdp::ParserMacAddress, nmdo::MacAddress>
              (peerMacAddrMatch.node().text().as_string())) {
            nmdo::MacAddress peerMacAddr{
              peerMacAddrMatch.node().text().as_string()
            };
            peerMacAddr.setResponding(true);

            const auto peerIpAddrMatch{
              arpNode.select_node("name[not(@inactive='inactive')]")
            };
            if (peerIpAddrMatch &&
                nmdp::matchString<nmdp::ParserIpAddress, nmdo::IpAddress>
                (peerIpAddrMatch.node().text().as_string())) {
              nmdo::IpAddress peerIpAddr{
                peerIpAddrMatch.node().text().as_string()
              };
              peerMacAddr.addIpAddress(peerIpAddr);
            }

            ifaces[ifaceUnitId].addReachableMac(peerMacAddr);
          }
        }
      }
    }
  }

  return std::tuple{ifaces, ifaceHierarchies};
}


std::map<std::string, nmdo::Vrf>
Parser::parseConfigRoutingInstances(const pugi::xml_node& routingInstancesNode,
    const std::map<std::string, netmeld::datastore::objects::InterfaceNetwork>& ifaces)
{
  std::map<std::string, nmdo::Vrf> vrfs;

  for (const auto& routingInstanceMatch :
       routingInstancesNode.select_nodes("instance[not(@inactive='inactive')]")) {
    const pugi::xml_node routingInstanceNode{routingInstanceMatch.node()};
    const std::string vrfId{
      routingInstanceNode.select_node("name").node().text().as_string()
    };
    vrfs[vrfId].setId(vrfId);

    for (const auto& interfaceMatch :
         routingInstanceNode.select_nodes("interface[not(@inactive='inactive')]")) {
      const pugi::xml_node interfaceNode{interfaceMatch.node()};
      const std::string ifaceName{
        interfaceNode.select_node("name").node().text().as_string()
      };
      if (ifaces.end() != ifaces.find(ifaceName)) {
        vrfs[vrfId].addIface(ifaceName);
      }
      else {
        data.observations.addNotable(
            (boost::format("routing-instance %1% contains undefined interface %2%")
             % vrfId
             % ifaceName).str());
      }
    }

    for (const auto& routingOptionsMatch :
         routingInstanceNode.select_nodes("routing-options[not(@inactive='inactive')]")) {
      const pugi::xml_node routingOptionsNode{routingOptionsMatch.node()};
      for (auto& route : parseConfigRoutingOptions(routingOptionsNode)) {
        route.setVrfId(vrfId);
        vrfs[vrfId].addRoute(route);
      }
    }
  }

  return vrfs;
}


nmdo::RoutingTable
Parser::parseConfigRoutingOptions(const pugi::xml_node& routingOptionsNode)
{
  nmdo::RoutingTable routes;

  for (const auto& routeMatch :
       routingOptionsNode.select_nodes(
         "static[not(@inactive='inactive')]/route[not(@inactive='inactive')]")) {
    const pugi::xml_node routeNode{routeMatch.node()};
    nmdo::Route route;
    route.setProtocol("static");
    route.setAdminDistance(5);  // Default for static routes

    const auto nameMatch{routeNode.select_node("name")};
    if (nameMatch) {
      const nmdo::IpAddress dstIpNet{nameMatch.node().text().as_string()};
      route.setDstIpNet(dstIpNet);
    }

    const auto discardMatch{routeNode.select_node("discard[not(@inactive='inactive')]")};
    if (discardMatch) {
      const std::string outgoingIfaceName{discardMatch.node().name()};
      route.setIfaceName(outgoingIfaceName);
    }

    const auto nextHopMatch{routeNode.select_node("next-hop[not(@inactive='inactive')]")};
    if (nextHopMatch) {
      const nmdo::IpAddress nextHopIpAddr{nextHopMatch.node().text().as_string()};
      route.setNextHopIpAddr(nextHopIpAddr);
    }

    const auto nextTableMatch{routeNode.select_node("next-table[not(@inactive='inactive')]")};
    if (nextTableMatch) {
      const std::string nextRouteTableName{
        nextTableMatch.node().text().as_string()
      };
      const auto [nextVrfId, nextTableId]{
        extractVrfIdTableId(nextRouteTableName)
      };
      route.setNextVrfId(nextVrfId);
      route.setNextTableId(nextTableId);
    }

    if (route.isV4()) {
      route.setTableId("inet.0");
    }
    else if (route.isV6()) {
      route.setTableId("inet6.0");
    }

    routes.emplace_back(route);
  }

  return routes;
}


std::map<std::string, nmdo::AclZone>
Parser::parseConfigZones(const pugi::xml_node& zonesNode)
{
  std::map<std::string, nmdo::AclZone> aclZones;

  for (const auto& zoneMatch :
       zonesNode.select_nodes("security-zone[not(@inactive='inactive')]")) {
    const pugi::xml_node zoneNode{zoneMatch.node()};
    const std::string zoneName{
      zoneNode.select_node("name").node().text().as_string()
    };
    aclZones[zoneName].setId(zoneName);

    for (const auto& interfacesMatch :
         zoneNode.select_nodes("interfaces[not(@inactive='inactive')]")) {
      const pugi::xml_node interfacesNode{interfacesMatch.node()};
      const std::string ifaceName{
        interfacesNode.select_node("name").node().text().as_string()
      };
      aclZones[zoneName].addIface(ifaceName);
    }
  }

  return aclZones;
}


std::map<std::string, std::map<std::string, nmdo::AclIpNetSet>>
Parser::parseConfigAddressBook(const pugi::xml_node& addressBookNode)
{
  std::map<std::string, std::map<std::string, nmdo::AclIpNetSet>> aclIpNetSets;

  std::string addressBookNamespace;
  const auto& securityZoneMatch{
    addressBookNode.select_node("parent::security-zone[not(@inactive='inactive')]")
  };
  if (securityZoneMatch) {
    const pugi::xml_node securityZoneNode{securityZoneMatch.node()};
    addressBookNamespace = securityZoneNode.select_node("name").node().text().as_string();
  }
  else {
    addressBookNamespace = addressBookNode.select_node("name").node().text().as_string();
  }
  aclIpNetSets[addressBookNamespace];

  for (const auto& addressMatch :
       addressBookNode.select_nodes("address[not(@inactive='inactive')]")) {
    const pugi::xml_node addressNode{addressMatch.node()};
    const std::string ipNetName{
      addressNode.select_node("name").node().text().as_string()
    };
    aclIpNetSets[addressBookNamespace][ipNetName].setId(ipNetName, addressBookNamespace);

    for (const auto& ipPrefixMatch :
         addressNode.select_nodes("ip-prefix[not(@inactive='inactive')]")) {
      const nmdo::IpNetwork ipNet{ipPrefixMatch.node().text().as_string()};
      aclIpNetSets[addressBookNamespace][ipNetName].addIpNet(ipNet);
    }

    for (const auto& dnsNameMatch :
         addressNode.select_nodes("dns-name[not(@inactive='inactive')]")) {
      const pugi::xml_node dnsNameNode{dnsNameMatch.node()};
      const std::string dnsName{
        dnsNameNode.select_node("name").node().text().as_string()
      };
      data.observations.addNotable("FQDNs are used that must be resolved");
      aclIpNetSets[addressBookNamespace][ipNetName].addHostname(dnsName);
    }
  }

  for (const auto& addressSetMatch :
       addressBookNode.select_nodes("address-set[not(@inactive='inactive')]")) {
    const pugi::xml_node addressSetNode{addressSetMatch.node()};
    const std::string ipNetSetName{
      addressSetNode.select_node("name").node().text().as_string()
    };
    aclIpNetSets[addressBookNamespace][ipNetSetName].setId(ipNetSetName, addressBookNamespace);

    for (const auto& addressMatch :
         addressSetNode.select_nodes("address[not(@inactive='inactive')]")) {
      const pugi::xml_node addressNode{addressMatch.node()};
      const std::string ipNetName{
        addressNode.select_node("name").node().text().as_string()
      };
      aclIpNetSets[addressBookNamespace][ipNetSetName].addIncludedId(ipNetName);
    }
  }

  return aclIpNetSets;
}


std::vector<nmdo::AclService>
Parser::parseConfigApplications(const pugi::xml_node& applicationsNode)
{
  std::vector<nmdo::AclService> aclServices;

  for (const auto& applicationMatch :
       applicationsNode.select_nodes("application[not(@inactive='inactive')]")) {
    const pugi::xml_node applicationNode{applicationMatch.node()};

    auto aclServicesToAdd =
        parseConfigApplicationOrTerm(applicationNode);
    std::copy(
        aclServicesToAdd.begin(),
        aclServicesToAdd.end(), 
        std::back_inserter(aclServices)
        );
  }

  for (const auto& applicationSetMatch :
       applicationsNode.select_nodes("application-set[not(@inactive='inactive')]")) {
    const pugi::xml_node applicationSetNode{applicationSetMatch.node()};
    nmdo::AclService aclService;

    const std::string applicationSetName{
      applicationSetNode.select_node("name").node().text().as_string()
    };
    aclService.setId(applicationSetName);

    for (const auto& applicationMatch :
         applicationSetNode.select_nodes("application[not(@inactive='inactive')]")) {
      const pugi::xml_node applicationNode{applicationMatch.node()};
      const std::string applicationName{
        applicationNode.select_node("name").node().text().as_string()
      };
      aclService.addIncludedId(applicationName);
    }

    aclServices.emplace_back(aclService);
  }

  return aclServices;
}


std::vector<nmdo::AclService>
Parser::parseConfigApplicationOrTerm(const pugi::xml_node& applicationNode)
{
  std::vector<nmdo::AclService> aclServices;

  const std::string applicationName{
    applicationNode.select_node("name").node().text().as_string()
  };

  const auto protocolMatch{
    applicationNode.select_node("protocol[not(@inactive='inactive')]")
  };
  if (protocolMatch) {
    nmdo::AclService aclService;
    aclService.setId(applicationName);

    std::string protocol{
      protocolMatch.node().text().as_string()
    };
    if (!protocol.empty() && std::isdigit(protocol.at(0))) {
      try {
        // If the protocol is a numeric string, look up the corresponding protocol name.
        const int protoNumber{boost::lexical_cast<int>(protocol)};
        const protoent* protoEntity{getprotobynumber(protoNumber)};
        if (protoEntity) {
          protocol = protoEntity->p_name;
        }
      }
      catch (boost::bad_lexical_cast&) {
        // Consume the exception and continue with the non-numeric protocol string.
      }
    }
    aclService.setProtocol(protocol);

    const auto srcPortMatch{
      applicationNode.select_node("source-port[not(@inactive='inactive')]")
    };
    if (srcPortMatch) {
      const nmdo::PortRange srcPortRange{
        srcPortMatch.node().text().as_string()
      };
      aclService.addSrcPortRange(srcPortRange);
    }
    else if (("tcp" == protocol) || ("udp" == protocol) ||
        ("sctp" == protocol)) {  // Implicit default source-port range
      const nmdo::PortRange srcPortRange{0, 65535};
      aclService.addSrcPortRange(srcPortRange);
    }

    const auto dstPortMatch{
      applicationNode.select_node("destination-port[not(@inactive='inactive')]")
    };
    if (dstPortMatch) {
      const nmdo::PortRange dstPortRange{
        dstPortMatch.node().text().as_string()
      };
      aclService.addDstPortRange(dstPortRange);
    }
    else if (("tcp" == protocol) || ("udp" == protocol) ||
        ("sctp" == protocol)) {  // Implicit default destination-port range
      const nmdo::PortRange dstPortRange{0, 65535};
      aclService.addDstPortRange(dstPortRange);
    }

    aclServices.emplace_back(aclService);
  }

  for (const auto& termMatch :
       applicationNode.select_nodes("term[not(@inactive='inactive')]")) {
    const pugi::xml_node termNode{termMatch.node()};
    for (auto& aclService : parseConfigApplicationOrTerm(termNode)) {
      aclService.setId(applicationName);
      aclServices.emplace_back(aclService);
    }
  }

  return aclServices;
}


std::vector<nmdo::AclRuleService>
Parser::parseConfigPolicies(const pugi::xml_node& policiesNode)
{
  std::vector<nmdo::AclRuleService> aclRules;
  size_t ruleId{0};

  ruleId = 0;
  for (const auto& policyMatch :
       policiesNode.select_nodes(
         "policy[not(@inactive='inactive')]/policy[not(@inactive='inactive')]")) {
    const pugi::xml_node policyNode{policyMatch.node()};
    auto aclRulesToAdd = parseConfigPolicy(policyNode, ruleId);
    std::copy(
        aclRulesToAdd.begin(),
        aclRulesToAdd.end(), 
        std::back_inserter(aclRules)
        );
    ++ruleId;
  }

  ruleId = 0;
  for (const auto& policyMatch :
       policiesNode.select_nodes("global/policy[not(@inactive='inactive')]")) {
    const pugi::xml_node policyNode{policyMatch.node()};
    auto aclRulesToAdd = parseConfigPolicy(policyNode, ruleId);
    std::copy(
        aclRulesToAdd.begin(),
        aclRulesToAdd.end(), 
        std::back_inserter(aclRules)
        );
    ++ruleId;
  }

  return aclRules;
}


std::vector<nmdo::AclRuleService>
Parser::parseConfigPolicy(const pugi::xml_node& policyNode, const size_t ruleId)
{
  std::vector<nmdo::AclRuleService> aclRules;

  const std::string description{
    policyNode.select_node("name").node().text().as_string()
  };

  std::vector<std::string> incomingZoneIds;

  const auto &incomingZoneNodes = policyNode.select_nodes
       ("(../from-zone-name[not(@inactive='inactive')])|"
        "(match[not(@inactive='inactive')]/from-zone[not(@inactive='inactive')])");
  std::transform(incomingZoneNodes.begin(), incomingZoneNodes.end(),
      std::back_inserter(incomingZoneIds),
      [](auto& incomingZoneMatch){return incomingZoneMatch.node().text().as_string();});
  if (incomingZoneIds.empty()) {
    incomingZoneIds.emplace_back("any");
  }

  std::vector<std::string> outgoingZoneIds;
  const auto &outgoingZoneNodes = policyNode.select_nodes
       ("(../to-zone-name[not(@inactive='inactive')])|"
        "(match[not(@inactive='inactive')]/to-zone[not(@inactive='inactive')])");
  std::transform(outgoingZoneNodes.begin(), outgoingZoneNodes.end(),
      std::back_inserter(outgoingZoneIds),
      [](auto& outgoingZoneMatch){return outgoingZoneMatch.node().text().as_string();});
  if (outgoingZoneIds.empty()) {
    outgoingZoneIds.emplace_back("any");
  }

  std::vector<std::string> srcIpNetSetIds;
  const auto &srcAddressNodes = policyNode.select_nodes(
      "match[not(@inactive='inactive')]/source-address[not(@inactive='inactive')]");
  std::transform(srcAddressNodes.begin(), srcAddressNodes.end(),
      std::back_inserter(srcIpNetSetIds),
      [](auto& srcAddressMatch){return srcAddressMatch.node().text().as_string();});
  if (srcIpNetSetIds.empty()) {
    srcIpNetSetIds.emplace_back("any");
  }

  std::vector<std::string> dstIpNetSetIds;
  const auto &dstAddressNodes = policyNode.select_nodes(
         "match[not(@inactive='inactive')]/destination-address[not(@inactive='inactive')]");
  std::transform(dstAddressNodes.begin(), dstAddressNodes.end(),
      std::back_inserter(dstIpNetSetIds),
      [](auto& dstAddressMatch){return dstAddressMatch.node().text().as_string();});
  if (dstIpNetSetIds.empty()) {
    dstIpNetSetIds.emplace_back("any");
  }

  std::vector<std::string> serviceIds;
  const auto &serviceIdNodes = policyNode.select_nodes(
         "match[not(@inactive='inactive')]/application[not(@inactive='inactive')]");
  std::transform(serviceIdNodes.begin(), serviceIdNodes.end(),
      std::back_inserter(serviceIds),
      [](auto& serviceIdMatch){return serviceIdMatch.node().text().as_string();});

  if (serviceIds.empty()) {
    serviceIds.emplace_back("any");
  }

  std::string action;
  if (policyNode.select_node(
        "then[not(@inactive='inactive')]/permit[not(@inactive='inactive')]")) {
    action = "allow";
  }
  else if (policyNode.select_node(
        "(then[not(@inactive='inactive')]/deny[not(@inactive='inactive')])|"
        "(then[not(@inactive='inactive')]/reject[not(@inactive='inactive')])")) {
    action = "block";
  }

  for (const auto& incomingZoneId : incomingZoneIds) {
    for (const auto& outgoingZoneId : outgoingZoneIds) {
      // Initially default to global address-book.
      std::string srcIpNetSetNamespace{"global"};
      std::string dstIpNetSetNamespace{"global"};
      if (policyNode.select_node("../../../zones/security-zone/address-book")) {
        // Use per-zone address-books instead of global address-book.
        // However, leave "any" zones on the global address-book.
        if ("any" != incomingZoneId) {
          srcIpNetSetNamespace = incomingZoneId;
        }
        if ("any" != outgoingZoneId) {
          dstIpNetSetNamespace = outgoingZoneId;
        }
      }

      size_t ruleIdBase{0};
      if (false) {
        ruleIdBase = 5000000;  // Default Policies
      }
      else if (policyNode.select_node("parent::global")) {
        ruleIdBase = 4000000;  // Global Policies
      }
      else if (incomingZoneId != outgoingZoneId) {
        ruleIdBase = 3000000;  // Inter-Zone Policies
      }
      else {
        ruleIdBase = 2000000;  // Intra-Zone Policies
      }
      for (const auto& srcIpNetSetId : srcIpNetSetIds) {
        for (const auto& dstIpNetSetId : dstIpNetSetIds) {
          for (const auto& serviceId : serviceIds) {
            nmdo::AclRuleService aclRule;
            aclRule.setPriority(ruleIdBase + ruleId);
            aclRule.setAction(action);
            aclRule.setIncomingZoneId(incomingZoneId);
            aclRule.setOutgoingZoneId(outgoingZoneId);
            aclRule.setSrcIpNetSetId(srcIpNetSetId, srcIpNetSetNamespace);
            aclRule.setDstIpNetSetId(dstIpNetSetId, dstIpNetSetNamespace);
            aclRule.setServiceId(serviceId);
            aclRule.setDescription(description);
            aclRules.emplace_back(aclRule);
          }
        }
      }
    }
  }

  return aclRules;
}


void
Parser::parseRouteInfo(const pugi::xml_node& routeInfoNode)
{
  std::string logicalSystemName;
  const pugi::xml_node outputNode{routeInfoNode.previous_sibling("output")};
  if (outputNode) {
    logicalSystemName =
      parseRouteLogicalSystemName(outputNode.text().as_string());
  }

  auto& logicalSystem{data.logicalSystems[logicalSystemName]};

  for (const auto& routeTableMatch :
       routeInfoNode.select_nodes("route-table[not(@inactive='inactive')]")) {
    const pugi::xml_node routeTableNode{routeTableMatch.node()};
    auto parsedVrfs{parseRouteTable(routeTableNode)};
    logicalSystem.vrfs.merge(parsedVrfs);
    for (const auto& [vrfId, vrfConflict] : parsedVrfs) {
      logicalSystem.vrfs[vrfId].merge(vrfConflict);
    }
  }
}


std::string
Parser::parseRouteLogicalSystemName(const std::string& s)
{
  std::string logicalSystemName{"unknown"};

  std::regex r{"^logical-system:\\s+(\\S+)\\s*$"};
  std::smatch m;
  if (std::regex_match(s, m, r)) {
    logicalSystemName = m[1];
  }
  if ("default" == logicalSystemName) {
    logicalSystemName.clear();
  }

  return nmcu::toLower(logicalSystemName);
}


std::map<std::string, nmdo::Vrf>
Parser::parseRouteTable(const pugi::xml_node& routeTableNode)
{
  const std::string routeTableName{
    routeTableNode.select_node("table-name").node().text().as_string()
  };
  const auto [vrfId, tableId]{
    extractVrfIdTableId(routeTableName)
  };
  std::map<std::string, nmdo::Vrf> vrfs;
  bool ignoredRouteTable{false};

  // Only "inet" and "inet6" route tables are useful here.
  // Other route table types contain non-IP destinations such as
  // BGP group IDs and encapsulated VPN (evpn) IDs.
  if ((0 != tableId.find("inet.")) && (0 != tableId.find("inet6."))) {
    ignoredRouteTable = true;
  }

  if (ignoredRouteTable) {
    return vrfs;
  }

  vrfs[vrfId].setId(vrfId);

  for (const auto& routeMatch : routeTableNode.select_nodes("rt[not(@inactive='inactive')]")) {
    const pugi::xml_node routeNode{routeMatch.node()};
    for (auto& route : parseRoute(routeNode)) {
      route.setVrfId(vrfId);
      route.setTableId(tableId);
      vrfs[vrfId].addRoute(route);
    }
  }

  return vrfs;
}


nmdo::RoutingTable
Parser::parseRoute(const pugi::xml_node& routeNode)
{
  nmdo::RoutingTable routes;
  bool ignoreRoute{false};

  const std::string dstIpString{
    routeNode.select_node("rt-destination").node().text().as_string()
  };
  const std::string prefixString{
    routeNode.select_node("rt-prefix-length").node().text().as_string()
  };

  // Ignore ephemeral multicast routes where the destination
  // is of the form "multicaseIp, unicastIp".
  // There isn't a clean way to collapse these into standard routes
  // and they are so short lived that they aren't useful for
  // analysis of periodic data snapshots.
  if (std::string::npos != dstIpString.find(",")) {
    ignoreRoute = true;
  }

  if (ignoreRoute) {
    return routes;  // Return empty set of routes.
  }

  const nmdo::IpAddress dstIpNet{
    dstIpString + "/" + prefixString
  };

  for (const auto& routeEntryMatch :
       routeNode.select_nodes("rt-entry[not(@inactive='inactive')]")) {
    const pugi::xml_node routeEntryNode{routeEntryMatch.node()};

    nmdo::Route route;
    route.setDstIpNet(dstIpNet);

    const std::string activeTag{
      routeEntryNode.select_node("active-tag").node().text().as_string()
    };
    if ("*" != activeTag) {
      route.setActive(false);
    }

    const auto protocolMatch{
      routeEntryNode.select_node("protocol-name[not(@inactive='inactive')]")
    };
    if (protocolMatch) {
      const std::string protocol{
        protocolMatch.node().text().as_string()
      };
      route.setProtocol(protocol);
    }

    const auto preferenceMatch{
      routeEntryNode.select_node("preference[not(@inactive='inactive')]")
    };
    if (preferenceMatch) {
      const size_t adminDistance{
        preferenceMatch.node().text().as_uint()
      };
      route.setAdminDistance(adminDistance);
    }

    const auto metricMatch{
      routeEntryNode.select_node("metric[not(@inactive='inactive')]")
    };
    if (metricMatch) {
      const size_t metric{
        metricMatch.node().text().as_uint()
      };
      route.setMetric(metric);
    }

    const auto nhTypeMatch{
      routeEntryNode.select_node("nh-type[not(@inactive='inactive')]")
    };
    if (nhTypeMatch) {
      const std::string nhType{
        nhTypeMatch.node().text().as_string()
      };
      if ("Discard" == nhType) {
        route.setIfaceName("discard");
      }
    }

    const auto nextHopTableMatch{
      routeEntryNode.select_node(
          "nh[not(@inactive='inactive')]/nh-table[not(@inactive='inactive')]")
    };
    if (nextHopTableMatch) {
      const std::string nextRouteTableName{
        nextHopTableMatch.node().text().as_string()
      };
      const auto [nextVrfId, nextTableId]{
        extractVrfIdTableId(nextRouteTableName)
      };
      route.setNextVrfId(nextVrfId);
      route.setNextTableId(nextTableId);
    }

    const auto nextHopRtrMatch{
      routeEntryNode.select_node(
          "nh[not(@inactive='inactive')]/to[not(@inactive='inactive')]")
    };
    if (nextHopRtrMatch) {
      const nmdo::IpAddress nextHopIpAddr{
        nextHopRtrMatch.node().text().as_string()
      };
      route.setNextHopIpAddr(nextHopIpAddr);
    }

    const auto nextHopViaMatch{
      routeEntryNode.select_node
        ("(nh[not(@inactive='inactive')]/via[not(@inactive='inactive')])|"
         "(nh[not(@inactive='inactive')]/nh-local-interface[not(@inactive='inactive')])")
    };
    if (nextHopViaMatch) {
      const std::string ifaceName{
        nextHopViaMatch.node().text().as_string()
      };
      route.setIfaceName(ifaceName);
    }

    routes.emplace_back(route);
  }

  return routes;
}


void
Parser::parseArpTableInfo(const pugi::xml_node& arpTableInfoNode)
{
  const std::string logicalSystemId;
  auto& ifaces{data.logicalSystems[logicalSystemId].ifaces};

  for (const auto& arpTableEntryMatch :
       arpTableInfoNode.select_nodes("arp-table-entry")) {
    const pugi::xml_node arpTableEntryNode{arpTableEntryMatch.node()};

    const auto ifaceNameMatch{
      arpTableEntryNode.select_node("interface-name")
    };
    if (ifaceNameMatch) {
      std::string ifaceName{
        ifaceNameMatch.node().text().as_string()
      };
      ifaceName.resize(ifaceName.find(" ["));
      ifaces[ifaceName].setName(ifaceName);

      const auto peerMacAddrMatch{
        arpTableEntryNode.select_node("mac-address")
      };
      if (peerMacAddrMatch &&
          nmdp::matchString<nmdp::ParserMacAddress, nmdo::MacAddress>
          (peerMacAddrMatch.node().text().as_string())) {
        nmdo::MacAddress peerMacAddr{
          peerMacAddrMatch.node().text().as_string()
        };
        peerMacAddr.setResponding(true);

        const auto peerIpAddrMatch{
          arpTableEntryNode.select_node("ip-address")
        };
        if (peerIpAddrMatch &&
            nmdp::matchString<nmdp::ParserIpAddress, nmdo::IpAddress>
            (peerIpAddrMatch.node().text().as_string())) {
          nmdo::IpAddress peerIpAddr{
            peerIpAddrMatch.node().text().as_string()
          };
          peerMacAddr.addIpAddress(peerIpAddr);
        }

        ifaces[ifaceName].addReachableMac(peerMacAddr);
      };
    }
  }
}


void
Parser::parseIpv6NeighborInfo(const pugi::xml_node& ipv6NeighborInfoNode)
{
  const std::string logicalSystemId;
  auto& ifaces{data.logicalSystems[logicalSystemId].ifaces};

  for (const auto& ipv6NdEntryMatch :
       ipv6NeighborInfoNode.select_nodes("ipv6-nd-entry")) {
    const pugi::xml_node ipv6NdEntryNode{ipv6NdEntryMatch.node()};

    const auto ifaceNameMatch{
      ipv6NdEntryNode.select_node("ipv6-nd-interface-name")
    };
    if (ifaceNameMatch) {
      std::string ifaceName{
        ifaceNameMatch.node().text().as_string()
      };
      ifaceName.resize(ifaceName.find(" ["));
      ifaces[ifaceName].setName(ifaceName);

      const auto peerMacAddrMatch{
        ipv6NdEntryNode.select_node("ipv6-nd-neighbor-l2-address")
      };
      if (peerMacAddrMatch &&
          nmdp::matchString<nmdp::ParserMacAddress, nmdo::MacAddress>
          (peerMacAddrMatch.node().text().as_string())) {
        nmdo::MacAddress peerMacAddr{
          peerMacAddrMatch.node().text().as_string()
        };
        peerMacAddr.setResponding(true);

        const auto peerIpAddrMatch{
          ipv6NdEntryNode.select_node("ipv6-nd-neighbor-address")
        };
        if (peerIpAddrMatch &&
            nmdp::matchString<nmdp::ParserIpAddress, nmdo::IpAddress>
            (peerIpAddrMatch.node().text().as_string())) {
          nmdo::IpAddress peerIpAddr{
            peerIpAddrMatch.node().text().as_string()
          };
          peerMacAddr.addIpAddress(peerIpAddr);
        }

        ifaces[ifaceName].addReachableMac(peerMacAddr);
      };
    }
  }
}


void
Parser::parseLldpNeighborInfo(const pugi::xml_node& lldpNeighborInfoNode)
{
  const std::string logicalSystemId;
  auto& ifaces{data.logicalSystems[logicalSystemId].ifaces};

  for (const auto& infoMatch :
       lldpNeighborInfoNode.select_nodes("lldp-neighbor-information[not(@inactive='inactive')]")) {
    const pugi::xml_node infoNode{infoMatch.node()};

    const auto localPortIdMatch{
      infoNode.select_node("lldp-local-port-id")
    };
    std::string localIfaceName;
    if (localPortIdMatch) {
      const std::string ifaceName{
        localPortIdMatch.node().text().as_string()
      };
      if (!ifaceName.empty() && ("-" != ifaceName)) {
        localIfaceName = ifaceName;
        ifaces[localIfaceName].setName(localIfaceName);
      }
    }

    const auto localParentIfaceMatch{
      infoNode.select_node("lldp-local-parent-interface-name")
    };
    std::string localParentIfaceName;
    if (localParentIfaceMatch) {
      const std::string ifaceName{
        localParentIfaceMatch.node().text().as_string()
      };
      if (!ifaceName.empty() && ("-" != ifaceName)) {
        localParentIfaceName = ifaceName;
        ifaces[localParentIfaceName].setName(localParentIfaceName);
      }
    }

    const auto remoteChassisIdSubtypeMatch{
      infoNode.select_node("lldp-remote-chassis-id-subtype")
    };
    if (remoteChassisIdSubtypeMatch) {
      const std::string remoteChassisIdSubtype{
        remoteChassisIdSubtypeMatch.node().text().as_string()
      };
      const std::string remoteChassisId{
        infoNode.select_node("lldp-remote-chassis-id").node().text().as_string()
      };
      if (("Mac address" == remoteChassisIdSubtype) &&
          nmdp::matchString<nmdp::ParserMacAddress, nmdo::MacAddress>
          (remoteChassisId)) {
        nmdo::MacAddress macAddr{remoteChassisId};
        if (!localIfaceName.empty()) {
          ifaces[localIfaceName].addReachableMac(macAddr);
        }
        if (!localParentIfaceName.empty()) {
          ifaces[localParentIfaceName].addReachableMac(macAddr);
        }
      }
    }

    const auto remotePortIdSubtypeMatch{
      infoNode.select_node("lldp-remote-port-id-subtype")
    };
    if (remotePortIdSubtypeMatch) {
      const std::string remotePortIdSubtype{
        remotePortIdSubtypeMatch.node().text().as_string()
      };
      const std::string remotePortId{
        infoNode.select_node("lldp-remote-port-id").node().text().as_string()
      };
      if (("Mac address" == remotePortIdSubtype) &&
          nmdp::matchString<nmdp::ParserMacAddress, nmdo::MacAddress>
          (remotePortId)) {
        nmdo::MacAddress macAddr{remotePortId};
        if (!localIfaceName.empty()) {
          ifaces[localIfaceName].addReachableMac(macAddr);
        }
        if (!localParentIfaceName.empty()) {
          ifaces[localParentIfaceName].addReachableMac(macAddr);
        }
      }
    }
  }
}


void
Parser::parseEthernetSwitching(const pugi::xml_node& l2ngNode)
{
  const std::string logicalSystemId;
  auto& ifaces{data.logicalSystems[logicalSystemId].ifaces};

  for (const auto& l2ngEntryMatch :
       l2ngNode.select_nodes(
         "l2ng-l2rtb-evpn-arp-entry|"
         "l2ng-l2rtb-evpn-nd-entry|"
         "l2ng-l2ald-mac-entry-vlan|"
         "l2ng-l2ald-mac-ip-entry")) {
    const pugi::xml_node l2ngEntryNode{l2ngEntryMatch.node()};

    const auto ifaceMatch{
      l2ngEntryNode.select_node("l2ng-l2-mac-logical-interface")
    };
    const std::string ifaceName{
      ifaceMatch.node().text().as_string()
    };
    if (!ifaceName.empty()) {
      ifaces[ifaceName].setName(ifaceName);

      const auto vlanMatch{
        l2ngEntryNode.select_node("l2ng-l2-vlan-id")
      };
      if (vlanMatch &&
          (std::string("none") != vlanMatch.node().text().as_string())) {
        const uint16_t vlanId{
          static_cast<uint16_t>(vlanMatch.node().text().as_uint())
        };
        ifaces[ifaceName].addVlan(vlanId);
      }

      const auto macAddrMatch{
        l2ngEntryNode.select_node("l2ng-l2-mac-address")
      };
      if (macAddrMatch &&
          nmdp::matchString<nmdp::ParserMacAddress, nmdo::MacAddress>
          (macAddrMatch.node().text().as_string())) {
        nmdo::MacAddress macAddr{
          macAddrMatch.node().text().as_string()
        };

        const auto ipAddrMatch{
          l2ngEntryNode.select_node(
              "l2ng-l2-ip-address|"
              "l2ng-l2-evpn-arp-inet-address|"
              "l2ng-l2-evpn-nd-inet6-address")
        };
        if (ipAddrMatch &&
            nmdp::matchString<nmdp::ParserIpAddress, nmdo::IpAddress>
            (ipAddrMatch.node().text().as_string())) {
          nmdo::IpAddress ipAddr{
            ipAddrMatch.node().text().as_string()
          };
          macAddr.addIpAddress(ipAddr);
        }

        ifaces[ifaceName].addReachableMac(macAddr);
      }
    }
  }
}


void
Parser::parseError(const pugi::xml_node& errorNode)
{
  const std::string message{
    errorNode.select_node("message").node().text().as_string()
  };
  data.observations.addNotable(message);
}


void
Parser::parseWarning(const pugi::xml_node& warningNode)
{
  const std::string message{
    warningNode.select_node("message").node().text().as_string()
  };
  data.observations.addNotable(message);
}


void
Parser::parseUnsupported(const pugi::xml_node& unsupportedNode)
{
  data.observations.addUnsupportedFeature(unsupportedNode.path());
}


std::tuple<std::string, std::string>
Parser::extractVrfIdTableId(const std::string& _vrfIdTableId)
{
  std::string vrfId;
  std::string tableId;

  // TODO: Need a more robust solution.
  // Should factor out of the Boost.Spirit parser to a micro-parser
  // that is used both there and here.
  const std::regex r{
    "^(([\\w_-]+)\\.)?((inet6|inet)(\\.\\d+)?)$"
  };
  std::smatch m;
  if (std::regex_match(_vrfIdTableId, m, r)) {
    vrfId = m[2];
    tableId = m[3];
  }

  return std::make_tuple(vrfId, tableId);
}

