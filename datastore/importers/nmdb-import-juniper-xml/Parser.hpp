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


typedef std::tuple<std::string, std::string> InterfaceHierarchy;


struct LogicalSystem
{
  std::string name;
  std::map<std::string, netmeld::datastore::objects::InterfaceNetwork> ifaces;
  std::vector<InterfaceHierarchy> ifaceHierarchies;
  std::map<std::string, netmeld::datastore::objects::Vrf> vrfs;
  std::vector<netmeld::datastore::objects::Service> services;
  std::vector<netmeld::datastore::objects::DnsResolver> dnsResolvers;
  std::vector<std::string> dnsSearchDomains;

  std::map<std::string, netmeld::datastore::objects::AclZone> aclZones;
  std::map<std::string, std::map<std::string, netmeld::datastore::objects::AclIpNetSet>> aclIpNetSets;
  std::vector<netmeld::datastore::objects::AclService> aclServices;
  std::vector<netmeld::datastore::objects::AclRuleService> aclRules;
};


struct Data
{
  std::map<std::string, LogicalSystem> logicalSystems;
  netmeld::datastore::objects::ToolObservations observations;
};


class Parser
{
  public:
    Data getData();

    void parseConfig(const pugi::xml_node&);
    void parseRouteInfo(const pugi::xml_node&);
    void parseArpTableInfo(const pugi::xml_node&);
    void parseIpv6NeighborInfo(const pugi::xml_node&);
    void parseLldpNeighborInfo(const pugi::xml_node&);
    void parseEthernetSwitching(const pugi::xml_node&);
    void parseUnsupported(const pugi::xml_node&);
    void parseError(const pugi::xml_node&);
    void parseWarning(const pugi::xml_node&);

  protected:
    std::tuple<std::map<std::string, netmeld::datastore::objects::InterfaceNetwork>,
               std::vector<InterfaceHierarchy>>
    parseConfigInterfaces(const pugi::xml_node& interfacesNode);

    std::map<std::string, netmeld::datastore::objects::Vrf>
    parseConfigRoutingInstances(const pugi::xml_node& routingInstancesNode,
        const std::map<std::string, netmeld::datastore::objects::InterfaceNetwork>& ifaces);

    netmeld::datastore::objects::RoutingTable
    parseConfigRoutingOptions(const pugi::xml_node& routingOptionsNode);

    std::map<std::string, netmeld::datastore::objects::AclZone>
    parseConfigZones(const pugi::xml_node& zonesNode);

    std::map<std::string, std::map<std::string, netmeld::datastore::objects::AclIpNetSet>>
    parseConfigAddressBook(const pugi::xml_node& addressBookNode);

    std::vector<netmeld::datastore::objects::AclService>
    parseConfigApplications(const pugi::xml_node& applicationsNode);

    std::vector<netmeld::datastore::objects::AclService>
    parseConfigApplicationOrTerm(const pugi::xml_node& applicationNode);

    std::vector<netmeld::datastore::objects::AclRuleService>
    parseConfigPolicies(const pugi::xml_node& policiesNode);

    std::vector<netmeld::datastore::objects::AclRuleService>
    parseConfigPolicy(const pugi::xml_node& policyNode, const size_t ruleId);

    std::string
    parseRouteLogicalSystemName(const std::string& s);

    std::map<std::string, netmeld::datastore::objects::Vrf>
    parseRouteTable(const pugi::xml_node& routeTableNode);

    netmeld::datastore::objects::RoutingTable
    parseRoute(const pugi::xml_node& routeNode);

    std::tuple<std::string, std::string>
    extractVrfIdTableId(const std::string&);

  public:

  private:
    Data data;
};

#endif  /* PARSER_HPP */
