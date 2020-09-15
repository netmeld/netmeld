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

#include <netmeld/datastore/objects/AcRule.hpp>
#include <netmeld/datastore/parsers/ParserIpAddress.hpp>
#include <netmeld/core/utils/StringUtilities.hpp>

#include "RulesCommon.hpp"

namespace netmeld::datastore::importers::cisco {

  namespace nmdo = netmeld::datastore::objects;
  namespace nmdp = netmeld::datastore::parsers;
  namespace nmcu = netmeld::core::utils;


  // ===========================================================================
  // Data containers
  // ===========================================================================
  typedef std::map<size_t, nmdo::AcRule> RuleBook;
  typedef std::pair<std::string, RuleBook> Result;


  // ===========================================================================
  // Parser definition
  // ===========================================================================
  class CiscoAcls :
    public qi::grammar<nmdp::IstreamIter, Result(), qi::ascii::blank_type>
  {
    // =========================================================================
    // Variables
    // =========================================================================
    public:
      // Rules
      qi::rule<nmdp::IstreamIter, Result(), qi::ascii::blank_type>
        start;

      qi::rule<nmdp::IstreamIter, qi::ascii::blank_type>
        ciscoAcl,
        ipv46,
        iosRule,
          iosRemark,   iosRemarkRuleLine,
          iosStandard, iosStandardRuleLine,
          iosExtended, iosExtendedRuleLine,
          iosIpv6,
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
        logArgument,
        userArgument,
        securityGroupArgument,
        remarkArgument,
        ipAccessListExtended, ipAccessList;

      qi::rule<nmdp::IstreamIter, std::string(), qi::ascii::blank_type>
        bookName,
        action,
        protocolArgument,
        addressArgument,
          addressArgumentIos,
          mask,
        portArgument;

      nmdp::ParserIpAddress   ipAddr;

      qi::rule<nmdp::IstreamIter, std::string()>
        addrIpOnly, addrIpMask, addrIpPrefix,
          ipNoPrefix,
        anyTerm,
        logArgumentString,
        ignoredRuleLine;

      qi::rule<nmdp::IstreamIter>
        untrackedArguments,
        inactiveArgument;

    protected:
      // Supporting data structures
      nmdo::AcRule curRule;
      const std::string ZONE  {"global"};

      std::string  ruleBookName {""};
      RuleBook     ruleBook;

      size_t       curRuleId       {0};
      std::string  curRuleProtocol {""};
      std::string  curRuleSrcPort  {""};
      std::string  curRuleDstPort  {""};

      std::string  curRuleOptions     {""};
      std::string  curRuleDescription {""};

      std::set<std::string> ignoredRuleData;

    private:

    // =========================================================================
    // Constructors
    // =========================================================================
    public: // Constructor is only default and must be public
      CiscoAcls();

    // =========================================================================
    // Methods
    // =========================================================================
    public:
    protected:
      void initCurRule();

    private: // Methods which should be hidden from API users
      void addIgnoredRuleData(const std::string&);

      // Policy Related
      void initRuleBook(const std::string&);

      void setCurRuleAction(const std::string&);
      void addCurRuleOption(const std::string&);

      void setCurRuleSrc(const std::string&);
      void setCurRuleDst(const std::string&);

      std::string setMask(nmdo::IpAddress&, const nmdo::IpAddress&);

      void curRuleFinalize();
      void updateRuleService();

      // Object return
      Result getData();
  };
}
#endif // CISCO_GRAMMAR_ACLS_HPP
