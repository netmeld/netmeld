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

#include <netmeld/datastore/tools/AbstractImportTool.hpp>
#include <netmeld/datastore/parsers/ParserDomainName.hpp>
#include <netmeld/datastore/parsers/ParserIpAddress.hpp>
#include <netmeld/datastore/parsers/ParserMacAddress.hpp>

#include <netmeld/datastore/objects/AcRule.hpp>
#include <netmeld/datastore/objects/AcNetworkBook.hpp>
#include <netmeld/datastore/objects/AcServiceBook.hpp>
#include <netmeld/datastore/objects/DeviceInformation.hpp>
#include <netmeld/datastore/objects/InterfaceNetwork.hpp>
#include <netmeld/datastore/objects/ToolObservations.hpp>
#include <netmeld/datastore/objects/Route.hpp>
#include <netmeld/datastore/objects/Service.hpp>
#include <netmeld/datastore/objects/Vlan.hpp>

#include <netmeld/core/utils/StringUtilities.hpp>

namespace nmcu = netmeld::core::utils;
namespace nmdo = netmeld::datastore::objects;
namespace nmdp = netmeld::datastore::parsers;
namespace nmdu = netmeld::datastore::utils;

typedef std::map<std::string, nmdo::AcNetworkBook> NetworkBook;
typedef std::map<std::string, nmdo::AcServiceBook> ServiceBook;
typedef std::map<size_t, nmdo::AcRule> RuleBook;

// =============================================================================
// Data containers
// =============================================================================
struct Data
{
  nmdo::DeviceInformation              devInfo;
  std::vector<std::string>             aaas;
  nmdo::ToolObservations               observations;
  std::vector<nmdo::Service>           services;
  std::vector<nmdo::Route>             routes;
  std::vector<nmdo::Vlan>              vlans;

  std::map<std::string, nmdo::InterfaceNetwork>  ifaces;

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
      policy, indent,
      interface, switchport, spanningTree;

    qi::rule<nmdp::IstreamIter, qi::ascii::blank_type, qi::locals<std::string>>
      policyMap, classMap;

    qi::rule<nmdp::IstreamIter, std::string(), qi::ascii::blank_type>
      addressArgument,
      ports;

    qi::rule<nmdp::IstreamIter, nmdo::Vlan(), qi::ascii::blank_type>
      vlan;

    qi::rule<nmdp::IstreamIter, std::string()>
      tokens,
      token;

    nmdp::ParserDomainName  domainName;
    nmdp::ParserIpAddress   ipAddr;
    nmdp::ParserMacAddress  macAddr;

    // Supporting data structures
    Data d;

    nmdo::InterfaceNetwork *tgtIface;

    bool isNo {false};

    bool globalCdpEnabled         {true};
    bool globalBpduGuardEnabled   {false};
    bool globalBpduFilterEnabled  {false};

    std::set<std::string>  ifacesCdpManuallySet;
    std::set<std::string>  ifacesBpduGuardManuallySet;
    std::set<std::string>  ifacesBpduFilterManuallySet;

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
    // Global Cdp/Bpdu related
    void addManuallySetCdpIface();
    void addManuallySetBpduGuardIface();
    void addManuallySetBpduFilterIface();

    // Device related
    void setVendor(const std::string&);
    void setDevId(const std::string&);
    void addAaa(const std::string&);
    void addObservation(const std::string&);

    // Service related
    void addDhcpService(const nmdo::IpAddress&);
    void addNtpService(const nmdo::IpAddress&);
    void addSnmpService(const nmdo::IpAddress&);

    // Route related
    void addRouteIp(const nmdo::IpAddress&, const nmdo::IpAddress&);
    void addRouteIface(const nmdo::IpAddress&, const std::string&);

    // Interface related
    void ifaceInit(const std::string&);
    void ifaceFinalize();

    // Vlan related
    void addVlan(nmdo::Vlan&);

    // Policy Related
    void createAccessGroup(const std::string&, const std::string&);
    void createServicePolicy(const std::string&, const std::string&);
    void updatePolicyMap(const std::string&, const std::string&);
    void updateClassMap(const std::string&, const std::string&);

    void updateCurRuleBook(const std::string&);
    void updateCurRule();

    void setCurRuleAction(const std::string&);

    void setCurRuleSrc(const std::string&);
    void setCurRuleDst(const std::string&);

    std::string setWildcardMask(nmdo::IpAddress&, const nmdo::IpAddress&);

    void curRuleFinalize();

    // Unsupported
    void unsup(const std::string&);

    // Object return
    Result getData();
};
#endif // PARSER_HPP

