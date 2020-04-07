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

#include <netmeld/core/tools/AbstractImportTool.hpp>
#include <netmeld/core/parsers/ParserDomainName.hpp>
#include <netmeld/core/parsers/ParserIpAddress.hpp>
#include <netmeld/core/parsers/ParserMacAddress.hpp>

#include <netmeld/core/objects/AcRule.hpp>
#include <netmeld/core/objects/AcNetworkBook.hpp>
#include <netmeld/core/objects/AcServiceBook.hpp>
#include <netmeld/core/objects/DeviceInformation.hpp>
#include <netmeld/core/objects/InterfaceNetwork.hpp>
#include <netmeld/core/objects/ToolObservations.hpp>
#include <netmeld/core/objects/Route.hpp>
#include <netmeld/core/objects/Service.hpp>
#include <netmeld/core/objects/Vlan.hpp>

#include <netmeld/core/utils/StringUtilities.hpp>

namespace utils = netmeld::core::utils;
namespace nmco  = netmeld::core::objects;
namespace nmcp  = netmeld::core::parsers;
namespace nmcu  = netmeld::core::utils;

typedef std::map<std::string, nmco::AcNetworkBook> NetworkBook;
typedef std::map<std::string, nmco::AcServiceBook> ServiceBook;
typedef std::map<size_t, nmco::AcRule> RuleBook;

// =============================================================================
// Data containers
// =============================================================================
struct Data
{
  nmco::DeviceInformation              devInfo;
  std::vector<std::string>             aaas;
  nmco::ToolObservations               observations;
  std::vector<nmco::Service>           services;
  std::vector<nmco::Route>             routes;
  std::vector<nmco::Vlan>              vlans;

  std::map<std::string, nmco::InterfaceNetwork>  ifaces;

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
      policy, source, destination, indent,
      interface, switchport, spanningTree;

    qi::rule<nmcp::IstreamIter, std::string(), qi::ascii::blank_type>
      ports;

    qi::rule<nmcp::IstreamIter, nmco::Vlan(), qi::ascii::blank_type>
      vlan;

    qi::rule<nmcp::IstreamIter, std::string()>
      tokens,
      token;

    nmcp::ParserDomainName  domainName;
    nmcp::ParserIpAddress   ipAddr;
    nmcp::ParserMacAddress  macAddr;

    // Supporting data structures
    Data d;

    nmco::InterfaceNetwork *tgtIface;

    bool isNo {false};

    bool globalCdpEnabled         {true};
    bool globalBpduGuardEnabled   {false};
    bool globalBpduFilterEnabled  {false};

    std::set<std::string>  ifacesCdpManuallySet;
    std::set<std::string>  ifacesBpduGuardManuallySet;
    std::set<std::string>  ifacesBpduFilterManuallySet;

    const std::string ZONE  {"global"};

    std::map<std::string, std::pair<std::string, std::string>> usedRuleBooks;

    std::string  curRuleBook {""};
    size_t       curRuleId {0};
    std::string  curRuleProtocol {""};
    std::string  curRuleSourcePort {""};
    std::string  curRuleDestinationPort {""};

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
    void addDhcpService(const nmco::IpAddress&);
    void addNtpService(const nmco::IpAddress&);
    void addSnmpService(const nmco::IpAddress&);

    // Route related
    void addRouteIp(const nmco::IpAddress&, const nmco::IpAddress&);
    void addRouteIface(const nmco::IpAddress&, const std::string&);

    // Interface related
    void ifaceInit(const std::string&);
    void ifaceFinalize();

    // Vlan related
    void addVlan(nmco::Vlan&);

    // Policy Related
    void createAccessGroup(nmco::InterfaceNetwork*,
                           const std::string&,
                           const std::string&);

    void updateCurrentRuleBook(const std::string&);
    void updateCurrentRule();

    void setCurrentRuleAction(const std::string&);
    void setCurrentRuleProtocol(const std::string&);
    void setCurrentRuleSourcePorts(const std::string&);
    void setCurrentRuleDestinationPorts(const std::string&);

    void setCurrentRuleSourceIpMask(nmco::IpAddress, const nmco::IpAddress&);
    void setCurrentRuleSourceHostIp(const nmco::IpAddress&);
    void setCurrentRuleSourceAny();

    void setCurrentRuleDestinationIpMask(nmco::IpAddress, const nmco::IpAddress&);
    void setCurrentRuleDestinationHostIp(const nmco::IpAddress&);
    void setCurrentRuleDestinationAny();
    void setCurrentRuleDestinationObjectGroup(const std::string&);

    void finalizeCurrentRule();

    // Unsupported
    void unsup(const std::string&);

    // Object return
    Result getData();
};
#endif // PARSER_HPP

