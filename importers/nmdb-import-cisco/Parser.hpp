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
#include <netmeld/core/objects/Service.hpp>
#include <netmeld/core/objects/ToolObservations.hpp>
#include <netmeld/core/objects/Vlan.hpp>
#include <netmeld/core/parsers/ParserDomainName.hpp>
#include <netmeld/core/parsers/ParserIpAddress.hpp>
#include <netmeld/core/parsers/ParserMacAddress.hpp>
#include <netmeld/core/tools/AbstractImportTool.hpp>
#include <netmeld/core/utils/StringUtilities.hpp>

#include "CiscoAcls.hpp"
#include "CiscoNetworkBook.hpp"
#include "CiscoServiceBook.hpp"
#include "CommonRules.hpp"

namespace nmdsic = netmeld::datastore::importers::cisco;

namespace nmco = netmeld::core::objects;
namespace nmcp = netmeld::core::parsers;
namespace nmcu = netmeld::core::utils;

using nmdsic::RuleBook;
using nmdsic::NetworkBook;
using nmdsic::ServiceBook;

// =============================================================================
// Data containers
// =============================================================================
struct Data
{
  std::string                          domainName;
  nmco::DeviceInformation              devInfo;
  std::vector<std::string>             aaas;
  nmco::ToolObservations               observations;

  std::map<std::string, nmco::InterfaceNetwork>  ifaces;

  std::vector<nmco::Route>             routes;
  std::vector<nmco::Service>           services;
  std::vector<nmco::Vlan>              vlans;

  std::map<std::string, NetworkBook>  networkBooks;
  std::map<std::string, ServiceBook>  serviceBooks;
  std::map<std::string, RuleBook>     ruleBooks;
};
typedef std::vector<Data> Result;


// =============================================================================
// Parser definition
// =============================================================================
class Parser :
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
      domainData,
      route,
      vlanDef,
      interface, switchport, spanningTree,
      accessPolicyRelated
      ;

    qi::rule<nmcp::IstreamIter, qi::ascii::blank_type, qi::locals<std::string>>
      policyMap, classMap;

    qi::rule<nmcp::IstreamIter, std::string(), qi::ascii::blank_type>
      addressArgument,
      ports;

    qi::rule<nmcp::IstreamIter, nmco::Vlan(), qi::ascii::blank_type>
      vlan;

    nmcp::ParserDomainName  domainName;
    nmcp::ParserIpAddress   ipAddr;
    nmcp::ParserMacAddress  macAddr;

    nmdsic::CiscoAcls         aclRuleBook;
    nmdsic::CiscoNetworkBook  networkBooks;
    nmdsic::CiscoServiceBook  serviceBooks;

    // Supporting data structures
    Data d;

    bool isNo {false};

    nmco::InterfaceNetwork* tgtIface;

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

    nmco::AcRule *curRule {nullptr};
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

    // Service related
    void serviceAddDhcp(const nmco::IpAddress&);
    void serviceAddNtp(const nmco::IpAddress&);
    void serviceAddSnmp(const nmco::IpAddress&);
    void serviceAddRadius(const nmco::IpAddress&);
    void serviceAddDns(const std::vector<nmco::IpAddress>&);
    void serviceAddSyslog(const nmco::IpAddress&);

    // Route related
    void routeAddIp(const nmco::IpAddress&, const nmco::IpAddress&);
    void routeAddIface(const nmco::IpAddress&, const std::string&);

    // Interface related
    void ifaceInit(const std::string&);
    void ifaceFinalize();
    void ifaceSetUpdate(std::set<std::string>* const);

    // Vlan related
    void vlanAdd(nmco::Vlan&);
    void vlanAddIfaceData();

    // Policy Related
    void createAccessGroup(const std::string&, const std::string&);
    void createServicePolicy(const std::string&, const std::string&);
    void updatePolicyMap(const std::string&, const std::string&);
    void updateClassMap(const std::string&, const std::string&);
    void aclRuleBookAdd(const std::pair<std::string, RuleBook>&);

    // Named Books Related
    void finalizeNamedBooks();

    // Unsupported
    void unsup(const std::string&);
    void addObservation(const std::string&);

    // Object return
    Result getData();
};
#endif // PARSER_HPP
