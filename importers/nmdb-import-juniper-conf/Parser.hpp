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

#ifndef PARSER_HPP
#define PARSER_HPP

#include <netmeld/core/objects/AcNetworkBook.hpp>
#include <netmeld/core/objects/AcRule.hpp>
#include <netmeld/core/objects/AcServiceBook.hpp>
#include <netmeld/core/objects/DeviceInformation.hpp>                           
#include <netmeld/core/objects/InterfaceNetwork.hpp>
#include <netmeld/core/objects/Route.hpp>
#include <netmeld/core/objects/ToolObservations.hpp>
#include <netmeld/core/objects/Vlan.hpp>
#include <netmeld/core/parsers/ParserDomainName.hpp>
#include <netmeld/core/parsers/ParserIpAddress.hpp>

namespace nmco = netmeld::core::objects;
namespace nmcp = netmeld::core::parsers;

typedef std::map<std::string, nmco::AcNetworkBook>  NetworkBook;
typedef std::map<std::string, nmco::AcServiceBook>  ServiceBook;
typedef std::map<size_t, nmco::AcRule>              RuleBook;


// =============================================================================
// Data containers
// =============================================================================
struct Data
{
  nmco::DeviceInformation devInfo {"Juniper"};

  std::vector<nmco::Route>  routes;

  std::map<std::string, nmco::Vlan>              vlans;
  std::map<std::string, nmco::InterfaceNetwork>  ifaces;

  std::map<std::string, NetworkBook>  networkBooks;
  std::map<std::string, ServiceBook>  serviceBooks;
  std::map<std::string, RuleBook>     ruleBooks;

  nmco::ToolObservations  observations;
};
typedef std::vector<Data> Result;


// =============================================================================
// Parser definition
// =============================================================================
class Parser:
  public qi::grammar<nmcp::IstreamIter, Result(), qi::ascii::blank_type>
{
  // ===========================================================================
  // Variables
  // ===========================================================================
  private:
    // Rules
    qi::rule<nmcp::IstreamIter, Result(), qi::ascii::blank_type>
      start;

    qi::rule<nmcp::IstreamIter, qi::ascii::blank_type>
      config,
      system,
      applications, application, applicationSet, appMultiLine, appSingleLine,
      interfaces, interface, unit, family, ifaceVlan,
      security,
      policies, policyFromTo, policy, policyMatch, policyThen,
      zones, zone, zoneIface,
      addressBookData,
      routingOptions, routeStatic,
      groups, group,
      vlans,
      logicalSystems, logicalSystem,
      routingInstances, routingInstance,
      ignoredBlock, startBlock, stopBlock;

    qi::rule<nmcp::IstreamIter, nmco::Route(), qi::ascii::blank_type>
      route;

    qi::rule<nmcp::IstreamIter, qi::ascii::blank_type,
             qi::locals<std::string>>
      address, addressSet, addressBook,
      vlan;

    qi::rule<nmcp::IstreamIter, std::vector<std::string>(),
             qi::ascii::blank_type>
      tokenList, logBlock;

    qi::rule<nmcp::IstreamIter, std::string()>
      token;

    qi::rule<nmcp::IstreamIter>
      typeSlot,
      comment,
      semicolon,
      garbageLine;

    nmcp::ParserIpAddress  ipAddr;
    nmcp::ParserDomainName fqdn;

    // Supporting data structures
    Result devices  {{}};
    Data*  d        {&devices.front()};

    std::map<std::string, Data> deviceMetadata;

    std::string tgtIfaceType;
    std::string tgtIfaceSlot;
    size_t      tgtIfaceUnit;
    std::string tgtIfaceName;
    bool        tgtIfaceUp {true};

    const std::string DEFAULT_ZONE {"global"};

    std::string bookName;
    std::string tgtZone   {DEFAULT_ZONE};
    std::string srcZone   {DEFAULT_ZONE};
    std::string dstZone   {DEFAULT_ZONE};
    std::string proto;
    std::string srcPort;
    std::string dstPort;

    size_t                         curRuleId;
    std::map<std::string, size_t>  ruleIds;

    size_t depthCount {0};

    std::multimap<std::string, std::string> ifaceVlanMembers;

  // ===========================================================================
  // Constructors
  // ===========================================================================
  public: // Constructor is only default and must be public
    Parser();

  // ===========================================================================
  // Methods
  // ===========================================================================
  private:
    void addDevice(const std::string&, const std::string&);
    void addGroup(const std::string&, const std::string&);
    void updateDeviceId(const std::string&);
    void resetDevice();
    void addRoute(nmco::Route&);
    void addVlan(const std::string&, unsigned short);
    void updateVlanDescription(const std::string&, const std::string&);

    void updateIfaceNameTypeState();
    void updateIfaceTypeSlot(const std::string&, const std::string&);
    void addInvalidIface(const std::string&);
    void updateIfaceUnit(const size_t);
    void addIfaceIpAddr(nmco::IpAddress&);
    void addIfaceVlan(const uint16_t);
    void addIfaceVlanRange(const uint16_t, const uint16_t);
    void addIfaceVlanMembers(const std::string&);
    void updateIfaceMode(const std::string&);
    void addZoneIface(const std::string&);

    void updateTgtZone(const std::string&);
    void updateZones(const std::string&, const std::string&);
    void updateCurRuleId(const std::string&);

    void updateBookName(const std::string&);
    void updateSrvcData(const std::string&, const std::string&);
    void updateSrvcBook();
    void updateSrvcBookGroup(const std::string&);

    void updateNetBookIp(const std::string&, const nmco::IpAddress&);
    void updateNetBookStr(const std::string&, const std::string&);
    void updateNetBookGroup(const std::string&, const std::string&);

    void updateRule(const std::string&, const std::vector<std::string>&);

    // Unsupported
    void unsup(const std::string&);

    Result getData();
};
#endif // PARSER_HPP
