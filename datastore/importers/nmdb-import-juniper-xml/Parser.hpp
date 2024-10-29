// =============================================================================
// Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC
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

#ifndef PARSER_HPP
#define PARSER_HPP

#include <netmeld/datastore/objects/AclIpNetSet.hpp>
#include <netmeld/datastore/objects/AclRuleService.hpp>
#include <netmeld/datastore/objects/AclService.hpp>
#include <netmeld/datastore/objects/AclZone.hpp>
#include <netmeld/datastore/objects/DnsResolver.hpp>
#include <netmeld/datastore/objects/InterfaceNetwork.hpp>
#include <netmeld/datastore/objects/IpAddress.hpp>
#include <netmeld/datastore/objects/MacAddress.hpp>
#include <netmeld/datastore/objects/OperatingSystem.hpp>
#include <netmeld/datastore/objects/Port.hpp>
#include <netmeld/datastore/objects/PortRange.hpp>
#include <netmeld/datastore/objects/Route.hpp>
#include <netmeld/datastore/objects/Service.hpp>
#include <netmeld/datastore/objects/ToolObservations.hpp>
#include <netmeld/datastore/objects/Vrf.hpp>

#include <pugixml.hpp>

#include <map>
#include <string>
#include <tuple>
#include <vector>


namespace nmdo = netmeld::datastore::objects;

typedef std::tuple<std::string, std::string> InterfaceHierarchy;


struct LogicalSystem
{
  std::map<std::string, nmdo::InterfaceNetwork> ifaces;
  std::vector<InterfaceHierarchy> ifaceHierarchies;
  std::map<std::string, nmdo::Vrf> vrfs;
  std::vector<nmdo::Service> services;
  std::vector<nmdo::DnsResolver> dnsResolvers;
  std::vector<std::string> dnsSearchDomains;

  std::map<std::string, nmdo::AclZone> aclZones;
  std::map<std::string, std::map<std::string, nmdo::AclIpNetSet>> aclIpNetSets;
  std::vector<nmdo::AclService> aclServices;
  std::vector<nmdo::AclRuleService> aclRules;

  auto operator<=>(const LogicalSystem&) const = default;
  bool operator==(const LogicalSystem&) const = default;
};


struct Data
{
  std::map<std::string, LogicalSystem> logicalSystems;
  nmdo::ToolObservations observations;

  auto operator<=>(const Data&) const = default;
  bool operator==(const Data&) const = default;
};

typedef std::vector<Data> Results;


class Parser
{
  // Variables
  private:
  protected:
    Data data;

    const std::string DEFAULT_VRF_ID {""};//{"master"};

  public:

  // Functions
  public:
    Results getData();

    void handleXML(const pugi::xml_document& doc);
    void parseConfig(const pugi::xml_node& configNode);
    void parseRouteInfo(const pugi::xml_node& routeInfoNode);
    void parseArpTableInfo(const pugi::xml_node& arpTableInfoNode);
    void parseIpv6NeighborInfo(const pugi::xml_node& ipv6NeighborInfoNode);
    void parseLldpNeighborInfo(const pugi::xml_node& lldpNeighborInfoNode);
    void parseEthernetSwitching(const pugi::xml_node& l2ngNode);
    void parseUnsupported(const pugi::xml_node& unsupportedNode);
    void parseError(const pugi::xml_node& errorNode);
    void parseWarning(const pugi::xml_node& warningNode);

  protected:
    std::tuple<std::map<std::string, nmdo::InterfaceNetwork>,
               std::vector<InterfaceHierarchy>>
    parseConfigInterfaces(const pugi::xml_node& interfacesNode);

    std::map<std::string, nmdo::Vrf>
    parseConfigRoutingInstances(const pugi::xml_node& routingInstancesNode,
        const std::map<std::string, nmdo::InterfaceNetwork>& ifaces);

    nmdo::RoutingTable
    parseConfigRoutingOptions(const pugi::xml_node& routingOptionsNode);

    std::map<std::string, nmdo::AclZone>
    parseConfigZones(const pugi::xml_node& zonesNode);

    std::map<std::string, std::map<std::string, nmdo::AclIpNetSet>>
    parseConfigAddressBook(const pugi::xml_node& addressBookNode);

    std::vector<nmdo::AclService>
    parseConfigApplications(const pugi::xml_node& applicationsNode);

    std::vector<nmdo::AclService>
    parseConfigApplicationOrTerm(const pugi::xml_node& applicationNode);

    std::vector<nmdo::AclRuleService>
    parseConfigPolicies(const pugi::xml_node& policiesNode);

    std::vector<nmdo::AclRuleService>
    parseConfigPolicy(const pugi::xml_node& policyNode, const size_t ruleId);

    std::string
    parseRouteLogicalSystemName(const std::string& s);

    std::map<std::string, nmdo::Vrf>
    parseRouteTable(const pugi::xml_node& routeTableNode);

    nmdo::RoutingTable
    parseRoute(const pugi::xml_node& routeNode);

    std::tuple<std::string, std::string>
    extractVrfIdTableId(const std::string&);

  private:
};
#endif  /* PARSER_HPP */
