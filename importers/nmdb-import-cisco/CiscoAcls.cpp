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

#include "CiscoAcls.hpp"

namespace nmdsic = netmeld::datastore::importers::cisco;

namespace netmeld::datastore::importers::cisco {
  // ===========================================================================
  // Parser logic
  // ===========================================================================
  CiscoAcls::CiscoAcls() : CiscoAcls::base_type(start)
  {
    using nmdsic::token;
    using nmdsic::tokens;
    using nmdsic::indent;

    start =
      ciscoAcl [qi::_val = pnx::bind(&CiscoAcls::getData, this)]
      ;

    ciscoAcl =
      (  iosRule
       | nxosRule
       | asaRule
      );


    //====
    // IOS
    //====
    iosRule =
      (iosRemark | iosStandard | iosExtended | iosIpv6)
      ;

    iosRemark =
      qi::lit("access-list ") >> bookName >> iosRemarkRuleLine
      // multi-liner - see iosStandard or iosExtended
      ;
    iosRemarkRuleLine =
      qi::lit("remark ") > remarkArgument > qi::eol
      ;

    iosStandard =
      (  (ipv46 >> qi::lit("access-list standard ")
          > bookName > qi::eol
          > *(indent > (  iosRemarkRuleLine
                        | iosStandardRuleLine
                        | ignoredRuleLine
                       )))
       | (qi::lit("access-list ") >> bookName >> iosStandardRuleLine)
      )
      ;
    iosStandardRuleLine =
      action
      >> sourceAddrIos
      >> *(logArgument)
      > qi::eol [pnx::bind(&CiscoAcls::curRuleFinalize, this)]
      ;

    iosExtended =
      (  (ipv46 >> qi::lit("access-list extended ")
          > bookName >> -dynamicArgument > qi::eol
          >> *(indent > (  iosRemarkRuleLine
                         | iosExtendedRuleLine
                         | ignoredRuleLine
                        )))
       | (qi::lit("access-list ")
          >> bookName >> -dynamicArgument >> iosExtendedRuleLine)
      )
      ;
    iosExtendedRuleLine =
      action
      >> protocolArgument
      >> sourceAddrIos >> -sourcePort
      >> -(destinationAddrIos >> -(destinationPort | icmpArgument))
        // Not exactly right, but we don't care about order
      >> *(  establishedArgument
           | logArgument
           | untrackedArguments
          )
      >> qi::eol [pnx::bind(&CiscoAcls::curRuleFinalize, this)]
      ;

    iosIpv6 =
      qi::lit("ipv6 access-list") > bookName > qi::eol
      >> *( indent > (  iosRemarkRuleLine
                      | iosExtendedRuleLine
                      | nxosExtendedRuleLine
                      | ignoredRuleLine
                     ))
      ;


    //====
    // NXOS
    //====
    nxosRule =
      (nxosRemark | nxosStandard | nxosExtended)
      ;
    
    nxosRemark =
      !qi::attr("")
      ;
    nxosRemarkRuleLine =
      qi::uint_ >> iosRemarkRuleLine
      ;

    nxosStandard =
      !qi::attr("")
      ;
    nxosStandardRuleLine =
      !qi::attr("")
      ;

    nxosExtended =
      ipv46 >> qi::lit("access-list ")
      > bookName >> -dynamicArgument > qi::eol
      >> *(indent > (  nxosRemarkRuleLine
                     | nxosExtendedRuleLine
                     | ignoredRuleLine
                    ))
      ;
    nxosExtendedRuleLine =
      qi::uint_ >> iosExtendedRuleLine
      ;

    //==========
    // ASA
    //==========
    asaRule =
      (asaRemark | asaStandard | asaExtended)
      ;

    asaRemark =
      iosRemark
      ;
    asaRemarkRuleLine =
      iosRemarkRuleLine
      ;

    asaStandard =
      qi::lit("access-list ") >> bookName >> qi::lit("standard ")
      >> asaStandardRuleLine
      ;
    asaStandardRuleLine =
      iosStandardRuleLine
      ;

    asaExtended =
      qi::lit("access-list ") >> bookName >> qi::lit("extended ")
      >> asaExtendedRuleLine
      ;
    asaExtendedRuleLine =
      action
      >> protocolArgument
      >> -(securityGroupArgument | userArgument)
      >> sourceAddrIos >> -sourcePort
      >> -(securityGroupArgument)
      >> -(destinationAddrIos >> -(destinationPort | icmpArgument))
        // Not exactly right, but we don't care about order
      >> *(  establishedArgument
           | logArgument
           | inactiveArgument
           | untrackedArguments
          )
      >> qi::eol [pnx::bind(&CiscoAcls::curRuleFinalize, this)]
      ;

  //  asaWebType =
  //    qi::lit("access-list ") >> bookName >> qi::lit("webtype ")
  //    >> asaWebtypeRuleLine
  //    ;
  //  asaWebtypeRuleLine =
  //    action
  //    >> (url | address | addressPort)
  //      // Not exactly right, but we don't care about order
  //    >> *(
  //         | logArgument
  //         | inactive
  //        )
  //    >> qi::eol [pnx::bind(&CiscoAcls::curRuleFinalize, this)]
  //    ;
  //  asaEther =
  //    qi::lit("ethertype ")
  //    // ACTION
  //    > token
  //    // TYPE
  //    > etherArgument
  //    ;


    //========
    // Helper piece-wise rules
    //========
    ipv46 =
      (qi::lit("ipv6") | qi::lit("ip"))
      ;
    bookName =
      token [pnx::bind(&CiscoAcls::initRuleBook, this, qi::_1)]
      ;

    action =
      (qi::string("permit") | qi::string("deny"))
        [pnx::bind(&CiscoAcls::initCurRule, this),
         pnx::bind(&CiscoAcls::setCurRuleAction, this, qi::_1)]
      ;

    dynamicArgument =
      qi::lit("dynamic ") > token >> -(qi::lit("timeout ") > qi::uint_)
      ;

    protocolArgument =
      -(qi::lit("object") >> -qi::lit("-group "))
      >> token [pnx::bind(&CiscoAcls::curRuleProtocol, this) = qi::_1]
      ;

    sourceAddrIos =
      addressArgumentIos [pnx::bind(&CiscoAcls::setCurRuleSrc, this, qi::_1)]
      ;
    destinationAddrIos =
      addressArgumentIos [pnx::bind(&CiscoAcls::setCurRuleDst, this, qi::_1)]
      ;
    addressArgumentIos =
      (  (qi::lit("host ") >> addrIpOnly)
       | (qi::lit("object") >> -qi::lit("-group ") > token)
       | (qi::lit("interface ") > token)
       | (anyTerm)
       | (addrIpMask)
       | (addrIpPrefix)
      )
      ;
    addrIpOnly =
      (&ipNoPrefix > ipAddr)
        [qi::_val = pnx::bind(&nmco::IpAddress::toString, &qi::_1)]
      ;
    ipNoPrefix =
      (ipAddr.ipv4 | ipAddr.ipv6) >> !(qi::lit('/') >> ipAddr.prefix)
      ;
    addrIpPrefix =
       (&((ipNoPrefix >> qi::eol) | !ipNoPrefix) >> ipAddr)
         [qi::_val = pnx::bind(&nmco::IpAddress::toString, &qi::_1)]
      ;
    addrIpMask =
       (&ipNoPrefix >> ipAddr >> qi::omit[+qi::blank]
        >> !(&(qi::lit("0.0.0.0") | qi::lit("255.255.255.255")))
        >> &ipNoPrefix >> ipAddr)
         [qi::_val = pnx::bind(&CiscoAcls::setMask, this, qi::_1, qi::_2)]
      ;
    anyTerm =
      qi::as_string[qi::string("any") >> -qi::char_("46") >> &qi::space]
        [qi::_val = qi::_1] // Needed as partial match returns itself
      ;

    sourcePort =
      portArgument [pnx::bind(&CiscoAcls::curRuleSrcPort, this) = qi::_1]
      ;
    destinationPort =
      portArgument [pnx::bind(&CiscoAcls::curRuleDstPort, this) = qi::_1]
      ;
    portArgument =
      (  (qi::lit("eq ") > token) [qi::_val = qi::_1]
       | (qi::lit("neq ") > token) [qi::_val = "!" + qi::_1]
       | (qi::lit("lt ") > token) [qi::_val = "<" + qi::_1]
       | (qi::lit("gt ") > token) [qi::_val = ">" + qi::_1]
       | (qi::lit("range ") > token > token) [qi::_val = (qi::_1+"-"+qi::_2)]
      )
      ;
    icmpArgument =
      (  (qi::lit("object-group ") > token)
          [pnx::bind(&CiscoAcls::curRuleDstPort, this) = qi::_1]
       | icmpTypeCode
       | icmpMessage
      )
      ;
    icmpTypeCode =
      qi::as_string[+qi::digit]
        [pnx::bind(&CiscoAcls::curRuleSrcPort, this) = qi::_1]
      > -(+qi::blank > -qi::as_string[+qi::digit]
        [pnx::bind(&CiscoAcls::curRuleDstPort, this) = qi::_1])
      ;
    icmpMessage = // A token is too greedy, so...
      (  qi::string("administratively-prohibited")
       | qi::string("alternate-address")
       | qi::string("conversion-error")
       | qi::string("dod-host-prohibited")
       | qi::string("dod-net-prohibited")
       | qi::string("echo-reply")
       | qi::string("echo")
       | qi::string("general-parameter-problem")
       | qi::string("host-isolated")
       | qi::string("host-precedence-unreachable")
       | qi::string("host-redirect")
       | qi::string("host-tos-redirect")
       | qi::string("host-tos-unreachable")
       | qi::string("host-unknown")
       | qi::string("host-unreachable")
       | qi::string("information-reply")
       | qi::string("information-request")
       | qi::string("mask-reply")
       | qi::string("mask-request")
       | qi::string("mobile-redirect")
       | qi::string("net-redirect")
       | qi::string("net-tos-redirect")
       | qi::string("net-tos-unreachable")
       | qi::string("net-unreachable")
       | qi::string("network-unknown")
       | qi::string("no-room-for-option")
       | qi::string("option-missing")
       | qi::string("packet-too-big")
       | qi::string("parameter-problem")
       | qi::string("port-unreachable")
       | qi::string("precedence-unreachable")
       | qi::string("protocol-unreachable")
       | qi::string("reassembly-timeout")
       | qi::string("redirect")
       | qi::string("router-advertisement")
       | qi::string("router-solicitation")
       | qi::string("source-quench")
       | qi::string("source-route-failed")
       | qi::string("time-exceeded")
       | qi::string("timestamp-reply")
       | qi::string("timestamp-request")
       | qi::string("traceroute")
       | qi::string("ttl-exceeded")
       | qi::string("unreachable")
      ) [pnx::bind(&CiscoAcls::curRuleDstPort, this) = qi::_1]
      ;

    untrackedArguments =
      (  (qi::lit("dest-option-type") > +qi::blank > token)
       | (qi::lit("dscp") > +qi::blank > token)
       | (qi::lit("flow-label") > +qi::blank > token)
       | (qi::lit("fragments") >> &(qi::blank | qi::eol))
       | (qi::lit("mobility") >> &(qi::blank | qi::eol))
       | (qi::lit("mobility-type") > +qi::blank > token)
       | (qi::lit("precedence") > +qi::blank > token)
       | (qi::lit("routing") >> &(qi::blank | qi::eol))
       | (qi::lit("routing-type") > +qi::blank > token)
       | (qi::lit("sequence") > +qi::blank > token)
       | (qi::lit("time-range") > +qi::blank > token)
       | (qi::lit("tos") > +qi::blank > token)
       | (qi::lit("undetermined-transport") >> &(qi::blank | qi::eol))
      )
      ;

    establishedArgument =
      (  qi::string("established")
       | qi::string("tracked") /* arista equivalent */
      ) [pnx::bind(&CiscoAcls::addCurRuleOption, this, qi::_1)]
      ;

    logArgument =
      logArgumentString [pnx::bind(&CiscoAcls::setCurRuleAction, this, qi::_1)]
      ;
    logArgumentString =
      qi::string("log") >> -qi::string("-input")
      >> *qi::hold[+qi::blank
        >> (!&(qi::lit("time-range") | qi::lit("inactive")))
        >> token
      ]
      ;

    userArgument =
      (  (qi::lit("object-group-user ") > token)
       | (qi::lit("user") >> -qi::lit("-group") > token)
      )
      ;

    securityGroupArgument =
      (  (qi::lit("object-group-security ") > token)
       | (qi::lit("security-group ") > (qi::lit("name ") | qi::lit("tag "))
          > token)
      )
      ;
    
    inactiveArgument =
      qi::lit("inactive") [pnx::bind([&](){curRule.disable();})]
      ;

    remarkArgument =
      tokens [pnx::bind(&CiscoAcls::curRuleDescription, this) = qi::_1]
      ;

    ignoredRuleLine =
      tokens [pnx::bind(&CiscoAcls::addIgnoredRuleData, this, qi::_1)]
      >> qi::eol
      ;

    BOOST_SPIRIT_DEBUG_NODES(
        //(start)
        (ciscoAcl)
        (iosRule)
          (iosRemark)   (iosRemarkRuleLine)
          (iosStandard) (iosStandardRuleLine)
          (iosExtended) (iosExtendedRuleLine)
        (nxosRule)
          (nxosRemark)   (nxosRemarkRuleLine)
          (nxosStandard) (nxosStandardRuleLine)
          (nxosExtended) (nxosExtendedRuleLine)
        (asaRule)
          (asaRemark)   (asaRemarkRuleLine)
          (asaStandard) (asaStandardRuleLine)
          (asaExtended) (asaExtendedRuleLine)
        (dynamicArgument)
        (sourceAddrIos) (destinationAddrIos)
        (sourcePort) (destinationPort)
        (icmpArgument)
          (icmpTypeCode) (icmpMessage)
        (establishedArgument)
        (untrackedArguments)
        (userArgument)
        (securityGroupArgument)
        (inactiveArgument)
        (ipAccessListExtended)(ipAccessList)
        (bookName)
        (action)
        (protocolArgument)
        (addressArgument) (addressArgumentIos) (mask)
        (portArgument)
        (addrIpOnly) (addrIpMask) (addrIpPrefix)
          (ipNoPrefix)
        (anyTerm)
        (logArgument)
        (remarkArgument)
        (ignoredRuleLine)
        //(token)(tokens)(indent)
        );
  }

  // ===========================================================================
  // Parser helper methods
  // ===========================================================================

  // Policy Related
  void
  CiscoAcls::initRuleBook(const std::string& name)
  {
    ruleBookName = name;
    curRuleId = 1;
    ruleBook.clear();
  }

  void
  CiscoAcls::initCurRule()
  {
    curRuleProtocol.clear();
    curRuleSrcPort.clear();
    curRuleDstPort.clear();
    curRuleOptions.clear();

    curRule = {};
  }

  void
  CiscoAcls::setCurRuleAction(const std::string& action)
  {
    curRule.addAction(action);
  }

  void
  CiscoAcls::addCurRuleOption(const std::string& option)
  {
    std::ostringstream oss(curRuleOptions, std::ios_base::ate);

    if (!curRuleOptions.empty()) {
      oss << ',';
    }
    oss << option;
    curRuleOptions = oss.str();
  }

  void
  CiscoAcls::setCurRuleSrc(const std::string& addr)
  {
    curRule.setSrcId(ZONE);
    curRule.addSrc(addr);
  }

  void
  CiscoAcls::setCurRuleDst(const std::string& addr)
  {
    curRule.setDstId(ZONE);
    curRule.addDst(addr);
  }

  std::string
  CiscoAcls::setMask(nmco::IpAddress& ipAddr, const nmco::IpAddress& mask)
  {
    bool isContiguous {ipAddr.setMask(mask)};
    if (!isContiguous) {
      std::ostringstream oss;
      oss << "IpAddress (" << ipAddr
          << ") set with non-contiguous wildcard netmask (" << mask << ")";
    }

    return ipAddr.toString();
  }

  void
  CiscoAcls::curRuleFinalize()
  {
    curRule.setRuleId(curRuleId);
    if (curRuleDescription.empty()) {
      curRuleDescription = ruleBookName;
    }
    curRule.setRuleDescription(curRuleDescription);
    curRuleDescription.clear();

    updateRuleService();

    ruleBook.emplace(curRuleId, curRule);
    ++curRuleId;
  }

  void
  CiscoAcls::updateRuleService()
  {
    if (curRuleProtocol.empty()) { return; }
    std::ostringstream oss;

    if (curRuleSrcPort.empty() && curRuleDstPort.empty()) {
      oss << curRuleProtocol;
    } else {
      oss << nmcu::getSrvcString(curRuleProtocol,
                                 curRuleSrcPort,
                                 curRuleDstPort);
    }

    if (!curRuleOptions.empty()) {
      oss << "--" << curRuleOptions;
    }

    curRule.addService(oss.str());
  }

  void
  CiscoAcls::addIgnoredRuleData(const std::string& _data)
  {
    ignoredRuleData.emplace(_data);
  }

  std::set<std::string>
  CiscoAcls::getIgnoredRuleData()
  {
    return ignoredRuleData;
  }

  // Object return
  Result
  CiscoAcls::getData()
  {
    Result r(ruleBookName, ruleBook);
    return r;
  }
}
