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

#include <netmeld/datastore/objects/AcRule.hpp>
#include <netmeld/datastore/objects/DeviceInformation.hpp>
#include <netmeld/datastore/objects/InterfaceNetwork.hpp>
#include <netmeld/datastore/objects/Route.hpp>
#include <netmeld/datastore/objects/Service.hpp>
#include <netmeld/datastore/objects/ToolObservations.hpp>
#include <netmeld/datastore/objects/Vlan.hpp>
#include <netmeld/datastore/parsers/ParserDomainName.hpp>
#include <netmeld/datastore/parsers/ParserIpAddress.hpp>
#include <netmeld/datastore/parsers/ParserMacAddress.hpp>
#include <netmeld/datastore/tools/AbstractImportSpiritTool.hpp>

#include "CiscoAcls.hpp"
#include "CiscoNetworkBook.hpp"
#include "CiscoServiceBook.hpp"
#include "RulesCommon.hpp"

namespace nmdo = netmeld::datastore::objects;
namespace nmdp = netmeld::datastore::parsers;

namespace nmdsic = netmeld::datastore::importers::cisco;

using nmdsic::RuleBook;
using nmdsic::NetworkBook;
using nmdsic::ServiceBook;

// =============================================================================
// Data containers
// =============================================================================
struct Data
{
  std::string                          domainName;
  nmdo::DeviceInformation              devInfo;
  std::vector<std::string>             dnsSearchDomains;
  std::vector<std::string>             aaas;
  nmdo::ToolObservations               observations;

  std::map<std::string, nmdo::InterfaceNetwork>  ifaces;
  std::map<uint16_t, std::set<std::string>> portChannels;

  std::vector<nmdo::Route>             routes;
  std::vector<nmdo::Service>           services;
  std::vector<nmdo::Vlan>              vlans;

  std::map<std::string, NetworkBook>  networkBooks;
  std::map<std::string, ServiceBook>  serviceBooks;
  std::map<std::string, RuleBook>     ruleBooks;
};
typedef std::vector<Data> Result;


// =============================================================================
// Parser definition
// =============================================================================
class Parser :
  public qi::grammar<nmdp::IstreamIter, Result(), qi::ascii::blank_type>
{
  // ===========================================================================
  // Variables
  // ===========================================================================
  private:
    // Rules
    qi::rule<nmdp::IstreamIter, Result(), qi::ascii::blank_type>
      start;

    qi::rule<nmdp::IstreamIter, qi::ascii::blank_type>
      config,
      domainData,
      globalServices,
      routerId,
      route,
      vlanDef,
      channelGroup,
      encapsulation,
      switchport,
        switchportPortSecurity,
          vlanRange,
          vlanId,
        spanningTree,
      accessPolicyRelated;

    qi::rule<nmdp::IstreamIter, qi::ascii::blank_type,
             qi::locals<uint8_t>>
      interface
      ;

    qi::rule<nmdp::IstreamIter, qi::ascii::blank_type,
             qi::locals<std::string, std::string>>
      switchportVlan;

    qi::rule<nmdp::IstreamIter, qi::ascii::blank_type, qi::locals<std::string>>
      policyMap, classMap;

    qi::rule<nmdp::IstreamIter, std::string(), qi::ascii::blank_type>
      addressArgument,
      ports;

    qi::rule<nmdp::IstreamIter, std::tuple<uint16_t, uint16_t>(), qi::ascii::blank_type>
      vlanNumberRange;

    qi::rule<nmdp::IstreamIter, std::vector<std::tuple<uint16_t, uint16_t>>(), qi::ascii::blank_type>
      vlanNumberRangeList;

    qi::rule<nmdp::IstreamIter, std::vector<nmdo::Vlan>(), qi::ascii::blank_type,
             qi::locals<std::vector<std::tuple<uint16_t, uint16_t>>, nmdo::Vlan>>
      vlan;

    qi::rule<nmdp::IstreamIter, nmdo::IpAddress()>
      ipMask;

    nmdp::ParserDomainName  domainName;
    nmdp::ParserIpAddress   ipAddr;
    nmdp::ParserMacAddress  macAddr;

    nmdsic::CiscoAcls         aclRuleBook;
    nmdsic::CiscoNetworkBook  networkBooks;
    nmdsic::CiscoServiceBook  serviceBooks;

    // Supporting data structures
    Data d;

    bool isNo {false};

    nmdo::InterfaceNetwork* tgtIface;
    std::map<std::string, nmdo::InterfaceNetwork*> ifaceAliases;
    std::vector<std::tuple<nmdo::InterfaceNetwork*,
                           std::string,
                           nmdo::IpAddress>>
      postIfaceAliasIpData;

    bool globalCdpEnabled         {true};
    bool globalBpduGuardEnabled   {false};
    bool globalBpduFilterEnabled  {false};

    std::set<std::string>  ifaceSpecificCdp;
    std::set<std::string>  ifaceSpecificBpduGuard;
    std::set<std::string>  ifaceSpecificBpduFilter;

    const std::string ZONE  {"global"};

    std::map<std::string, std::pair<std::string, std::string>> appliedRuleSets;

    std::map<std::string, std::set<std::pair<std::string, std::string>>>
      servicePolicies;
    std::map<std::string, std::set<std::string>> policies;
    std::map<std::string, std::set<std::string>> classes;

    nmdo::AcRule *curRule {nullptr};
    std::string  curRuleBook {""};
    size_t       curRuleId {0};
    std::string  curRuleProtocol {""};
    std::string  curRuleSrcPort {""};
    std::string  curRuleDstPort {""};

  // ===========================================================================
  // Constructors
  // ===========================================================================
  public: // Constructor is only default and must be public
    Parser();

  // ===========================================================================
  // Methods
  // ===========================================================================
  private: // Methods which should be hidden from API users
    // Device related
    void deviceAaaAdd(const std::string&);
    void deviceAddDnsSearchDomain(const std::string&);

    // Service related
    void serviceAddDhcp(const nmdo::IpAddress&);
    void serviceAddNtp(const nmdo::IpAddress&);
    void serviceAddSnmp(const nmdo::IpAddress&);
    void serviceAddRadius(const nmdo::IpAddress&);
    void serviceAddDns(const nmdo::IpAddress&);
    void serviceAddSyslog(const nmdo::IpAddress&);

    // Route related
    void routeAddIp(const nmdo::IpAddress&, const nmdo::IpAddress&);
    void routeAddIface(const nmdo::IpAddress&, const std::string&);
    void routeSetAdminDistance(const size_t);

    // Interface related
    void ifaceInit(const std::string&);
    void ifaceSetUpdate(std::set<std::string>* const);
    void ifaceAddAlias(const std::string&, const nmdo::IpAddress&);

    // Port-channel related
    void portChannelAddIface(uint16_t);

    // Encapsulation related
    void encapsulationDot1qAddVlan(uint16_t);

    // Vlan related
    std::vector<nmdo::Vlan> expandVlanNumberRangeList(
        const std::vector<std::tuple<uint16_t, uint16_t>>&,
        const nmdo::Vlan&) const;
    void vlansAdd(const std::vector<nmdo::Vlan>&);
    void vlanAdd(const nmdo::Vlan&);
    void vlanAddIfaceData();

    // Policy Related
    void createAccessGroup(const std::string&, const std::string&,
                           const std::string& = "");
    void createServicePolicy(const std::string&, const std::string&);
    void updatePolicyMap(const std::string&, const std::string&);
    void updateClassMap(const std::string&, const std::string&);
    void aclRuleBookAdd(std::pair<std::string, RuleBook>&);

    // Named Books Related
    void finalizeNamedBooks();

    // Unsupported
    void unsup(const std::string&);
    void addObservation(const std::string&);

    // Object return
    void setRuleTargetIface(nmdo::AcRule&, const std::string&,
                            void (nmdo::AcRule::*x)(const std::string&));
    Result getData();
};
#endif // PARSER_HPP
