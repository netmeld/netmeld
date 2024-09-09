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
#include <vector>

namespace nmdo = netmeld::datastore::objects;

struct LogicalSystem
{
  std::string name;
  std::map<std::string, nmdo::InterfaceNetwork> ifaces;
  std::map<std::string, nmdo::Vrf> vrfs;
  std::vector<nmdo::Service> services;
  std::vector<nmdo::DnsResolver> dnsResolvers;
  std::vector<std::string> dnsSearchDomains;

  std::map<std::string, nmdo::AclZone> aclZones;
  std::map<std::string, nmdo::AclIpNetSet> aclIpNetSets;
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


class Parser
{
  public:
    Data
    getData();

    void
    parseConfig(const pugi::xml_node&);

    void
    parseConfigRouteInfo(const pugi::xml_node&);

  protected:
    void
    parseConfigDeviceconfig(const pugi::xml_node&, LogicalSystem&);

    void
    parseConfigVsys(const pugi::xml_node&);

    std::map<std::string, nmdo::InterfaceNetwork>
    parseConfigInterface(const pugi::xml_node&);

    nmdo::InterfaceNetwork
    parseConfigInterfaceEntry(const pugi::xml_node&);

    std::map<std::string, nmdo::Vrf>
    parseConfigVirtualRouter(const pugi::xml_node&);

    std::map<std::string, nmdo::AclZone>
    parseConfigZone(const pugi::xml_node&);

    std::map<std::string, nmdo::AclIpNetSet>
    parseConfigAddress(const pugi::xml_node&);

    std::map<std::string, nmdo::AclIpNetSet>
    parseConfigAddressGroup(const pugi::xml_node&);

    std::vector<nmdo::AclService>
    parseConfigService(const pugi::xml_node&);

    std::vector<nmdo::AclService>
    parseConfigServiceGroup(const pugi::xml_node&);

    std::vector<nmdo::AclRuleService>
    parseConfigRulebase(const pugi::xml_node&, const LogicalSystem&);

    std::vector<nmdo::AclRuleService>
    parseConfigRules(const pugi::xml_node&, const size_t, const LogicalSystem&);

  private:
    Data data;
};

#endif  /* PARSER_HPP */
