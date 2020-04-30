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

#ifndef CISCO_GRAMMAR_ACLS_HPP
#define CISCO_GRAMMAR_ACLS_HPP

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

#include "CommonRules.hpp"

namespace netmeld::datastore::importers::cisco {

namespace nmco = netmeld::core::objects;
namespace nmcp = netmeld::core::parsers;
namespace nmcu = netmeld::core::utils;

typedef std::map<size_t, nmco::AcRule> RuleBook;

// =============================================================================
// Data containers
// =============================================================================
typedef std::pair<std::string, RuleBook> Result;


// =============================================================================
// Parser definition
// =============================================================================
class Acls :
  public qi::grammar<nmcp::IstreamIter, Result(), qi::ascii::blank_type>
{
  // ===========================================================================
  // Variables
  // ===========================================================================
  public:
    // Rules
    qi::rule<nmcp::IstreamIter, Result(), qi::ascii::blank_type>
      start;

    qi::rule<nmcp::IstreamIter, qi::ascii::blank_type>
      config,
      ipv46,
      iosRule,
        iosRemark,   iosRemarkRuleLine,
        iosStandard, iosStandardRuleLine,
        iosExtended, iosExtendedRuleLine,
      nxosRule,
        nxosRemark,   nxosRemarkRuleLine,
        nxosStandard, nxosStandardRuleLine,
        nxosExtended, nxosExtendedRuleLine,
      asaRule,
        asaRemark,   asaRemarkRuleLine,
        asaStandard, asaStandardRuleLine,
        asaExtended, asaExtendedRuleLine,
      dynamicArgument,
      sourceAddrIos, destinationAddrIos,
      sourcePort, destinationPort,
      icmpArgument,
        icmpTypeCode, icmpMessage,
      establishedArgument,
      fragmentsArgument,
      precedenceArgument,
      tosArgument,
      logArgument,
      timeRangeArgument,
      ipAccessListExtended, ipAccessList;

    qi::rule<nmcp::IstreamIter, std::string(), qi::ascii::blank_type>
      bookName,
      action,
      protocolArgument,
      addressArgument,
        addressArgumentIos,
        mask,
      portArgument;

    nmcp::ParserIpAddress   ipAddr;

    qi::rule<nmcp::IstreamIter, std::string()>
      addrFromIpNoPrefix,
        ipNoPrefix,
      addrFromIpMask,
      addrFromIpPrefix,
      anyTerm,
      ignoredRuleLine;


//  protected:
    nmco::AcRule curRule;
    // Supporting data structures
    const std::string ZONE  {"global"};

    std::string  ruleBookName {""};
    RuleBook     ruleBook;


    size_t       curRuleId {0};
    std::string  curRuleProtocol {""};
    std::string  curRuleSrcPort {""};
    std::string  curRuleDstPort {""};

    std::set<std::string> ignoredRuleData;

  private:

  // ===========================================================================
  // Constructors
  // ===========================================================================
  public: // Constructor is only default and must be public
    Acls();

  // ===========================================================================
  // Methods
  // ===========================================================================
  public:
    std::set<std::string> getIgnoredRuleData();

    void initCurRule();
  protected:
  private: // Methods which should be hidden from API users
    void addIgnoredRuleData(const std::string&);

    // Policy Related
    void initRuleBook(const std::string&);

    void setCurRuleAction(const std::string&);

    void setCurRuleSrc(const std::string&);
    void setCurRuleDst(const std::string&);

    std::string setWildcardMask(nmco::IpAddress&, const nmco::IpAddress&);

    void curRuleFinalize();

    // Object return
    Result getData();
};
}
#endif // CISCO_GRAMMAR_ACLS_HPP
