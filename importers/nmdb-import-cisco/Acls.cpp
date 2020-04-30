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

#include "Acls.hpp"

namespace nmdsic = netmeld::datastore::importers::cisco;

namespace netmeld::datastore::importers::cisco {
// =============================================================================
// Parser logic
// =============================================================================
Acls::Acls() : Acls::base_type(start)
{
  using nmdsic::token;
  using nmdsic::tokens;
  using nmdsic::indent;

  start =
    config [qi::_val = pnx::bind(&Acls::getData, this)]
    ;

  config =
    (  iosRule
     | nxosRule
     | asaRule
    );


  //====
  // IOS
  //====
  iosRule =
    (iosRemark | iosStandard | iosExtended)
    ;

  iosRemark =
    qi::lit("access-list") >> bookName >> iosRemarkRuleLine
    // multi-liner - see iosStandard or iosExtended
    ;
  iosRemarkRuleLine =
    qi::lit("remark") > tokens > qi::eol
    ;

  iosStandard =
    (  (ipv46 >> qi::lit("access-list standard")
        > bookName > qi::eol
        > *(indent > (  iosRemarkRuleLine
                      | iosStandardRuleLine
                      | ignoredRuleLine
                     )))
     | (qi::lit("access-list") >> bookName >> iosStandardRuleLine)
    )
    ;
  iosStandardRuleLine =
    action
    >> sourceAddrIos
    > qi::eol [pnx::bind(&Acls::curRuleFinalize, this)]
    ;

  iosExtended =
    (  (ipv46 >> qi::lit("access-list extended")
        > bookName >> -dynamicArgument > qi::eol
        >> *(indent > (  iosRemarkRuleLine
                       | iosExtendedRuleLine
                       | ignoredRuleLine
                      )))
     | (qi::lit("access-list")
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
         | fragmentsArgument
         | precedenceArgument
         | tosArgument
         | logArgument
         | timeRangeArgument
        )
    >> qi::eol [pnx::bind(&Acls::curRuleFinalize, this)]
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
    ipv46 >> qi::lit("access-list")
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
    qi::lit("access-list") >> bookName >> qi::lit("standard")
    >> asaStandardRuleLine
    ;
  asaStandardRuleLine =
    iosStandardRuleLine
    ;

  asaExtended =
    !qi::attr("")
    ;
  asaExtendedRuleLine =
    !qi::attr("")
    ;
//  asaRule =
//    qi::lit("access-list") >> token
//    >> (  asaRemark
//        | asaStandard
//        | asaExtended
//        | asaWeb
//        | asaEther
//       ) > qi::eol
//    ;
//
//  asaRemark =
//    qi::lit("remark")
//    // DESCRIPTION
//    > tokens
//    ;
//
//  asaExtended =
//    qi::lit("extended")
//    // ACTION
//    > token
//    // PROTOCOL
//    > protocolArgument
//    // USER
//    >> -(  userArgument
//    // SOURCE SECURITY GROUP
//         | securityArgument
//        )
//    // SROUCE
//    > addressArgument
//    // SROUCE PORTS
//    >> -(portArgument)
//    // DESTINATION SECURITY GROUP
//    >> -(securityArgument)
//    // DESTINATION
//    > addressArgument
//    // DESTINATION PORTS
//    >> -(portArgument)
//    // LOG
//    >> -(logArgument)
//    // TIME RANGE
//    >> -(timeRangeArgument)
//    // STATE
//    >> -(inactive)
//    ;
//
//  asaWeb =
//    qi::lit("webtype")
//    // ACTION
//    > token
//    // URL
//    > (  (qi::lit("url") >> token)
//    // ADDRESS
//       | (qi::lit("tcp") > addressArgument
//    // ADDRESS PORT
//          >> -(qi::lit("operator") > token)
//         )
//      )
//    // LOG
//    >> -(logArgument)
//    // TIME RANGE
//    >> -(timeRangeArgument)
//    // STATE
//    >> -(inactive)
//    ;
//
//  asaEther =
//    qi::lit("ethertype")
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
    token [pnx::bind(&Acls::initRuleBook, this, qi::_1)]
    ;

  action =
    (qi::string("permit") | qi::string("deny"))
      [pnx::bind(&Acls::initCurRule, this),
       pnx::bind(&Acls::setCurRuleAction, this, qi::_1)]
    ;

  dynamicArgument =
    qi::lit("dynamic") > token >> -(qi::lit("timeout") > qi::uint_)
    ;

  protocolArgument =
    -(qi::lit("object") >> -qi::lit("-group"))
    >> token [pnx::bind(&Acls::curRuleProtocol, this) = qi::_1]
    ;

  sourceAddrIos =
    addressArgumentIos [pnx::bind(&Acls::setCurRuleSrc, this, qi::_1)]
    ;
  destinationAddrIos =
    addressArgumentIos [pnx::bind(&Acls::setCurRuleDst, this, qi::_1)]
    ;
  // TODO Need to handle wildcard or netmask...
  addressArgumentIos =
    (
       (qi::lit("host") >> (&ipNoPrefix > ipAddr))
         [qi::_val = pnx::bind(&nmco::IpAddress::toString, &qi::_1)]
     | (qi::lit("object") >> -qi::lit("-group") > token)
         [qi::_val = qi::_1]
     | (qi::lit("interface") > token)
         [qi::_val = qi::_1]
     | (anyTerm)
         [qi::_val = qi::_1]
       // TODO needs to be ipAddr.ipv4
     | (&ipNoPrefix >> ipAddr >> &ipNoPrefix >> ipAddr)
         [qi::_val = pnx::bind(&Acls::setWildcardMask, this, qi::_1, qi::_2)]
     | (&((ipNoPrefix >> qi::eol) | !ipNoPrefix) >> ipAddr)
         [qi::_val = pnx::bind(&nmco::IpAddress::toString, &qi::_1)]
    )
    ;
  ipNoPrefix =
    (ipAddr.ipv4 | ipAddr.ipv6) >> !(qi::lit('/') >> ipAddr.cidr)
    ;
  anyTerm =
    qi::string("any") >> -qi::char_("46") >> &qi::space
    ;
  mask =
    (!(qi::lit("0.0.0.0") | qi::lit("255.255.255.255"))
    >> ipAddr)
      [qi::_val = pnx::bind(&nmco::IpAddress::toString, &qi::_1)]
    ;

  sourcePort =
    portArgument [pnx::bind(&Acls::curRuleSrcPort, this) = qi::_1]
    ;
  destinationPort =
    portArgument [pnx::bind(&Acls::curRuleDstPort, this) = qi::_1]
    ;
  portArgument =
    (  (qi::lit("eq") > token) [qi::_val = qi::_1]
     | (qi::lit("neq") > token) [qi::_val = "!" + qi::_1]
     | (qi::lit("lt") > token) [qi::_val = "<" + qi::_1]
     | (qi::lit("gt") > token) [qi::_val = ">" + qi::_1]
     | (qi::lit("range") > token > token) [qi::_val = (qi::_1 + "-" + qi::_2)]
    )
    ;
  icmpArgument = 
    (icmpTypeCode | icmpMessage)
    ;
  icmpTypeCode =
    qi::ushort_ [qi::_pass = (0 <= qi::_1 && qi::_1 <= 255)]
    >> -qi::short_ [qi::_pass = (qi::_1 >= 0 && qi::_1 <= 255)]
    ;
  icmpMessage = // A token is too greedy, so...
    (  qi::lit("administratively-prohibited")
     | qi::lit("alternate-address")
     | qi::lit("conversion-error")
     | qi::lit("dod-host-prohibited")
     | qi::lit("dod-net-prohibited")
     | qi::lit("echo")
     | qi::lit("echo-reply")
     | qi::lit("general-parameter-problem")
     | qi::lit("host-isolated")
     | qi::lit("host-precedence-unreachable")
     | qi::lit("host-redirect")
     | qi::lit("host-tos-redirect")
     | qi::lit("host-tos-unreachable")
     | qi::lit("host-unknown")
     | qi::lit("host-unreachable")
     | qi::lit("information-reply")
     | qi::lit("information-request")
     | qi::lit("mask-reply")
     | qi::lit("mask-request")
     | qi::lit("mobile-redirect")
     | qi::lit("net-redirect")
     | qi::lit("net-tos-redirect")
     | qi::lit("net-tos-unreachable")
     | qi::lit("net-unreachable")
     | qi::lit("network-unknown")
     | qi::lit("no-room-for-option")
     | qi::lit("option-missing")
     | qi::lit("packet-too-big")
     | qi::lit("parameter-problem")
     | qi::lit("port-unreachable")
     | qi::lit("precedence-unreachable")
     | qi::lit("protocol-unreachable")
     | qi::lit("reassembly-timeout")
     | qi::lit("redirect")
     | qi::lit("router-advertisement")
     | qi::lit("router-solicitation")
     | qi::lit("source-quench")
     | qi::lit("source-route-failed")
     | qi::lit("time-exceeded")
     | qi::lit("timestamp-reply")
     | qi::lit("timestamp-request")
     | qi::lit("traceroute")
     | qi::lit("ttl-exceeded")
     | qi::lit("unreachable")
    )
    ;

  establishedArgument =
    qi::string("established") | qi::string("tracked") /* arista equivalent */
    ;

  fragmentsArgument =
    qi::string("fragments")
    ;

  precedenceArgument =
    qi::lit("precedence") > token
    ;

  tosArgument =
    qi::lit("tos") > token
    ;

  logArgument =
    (qi::string("log-input") | qi::string("log"))
      [pnx::bind(&Acls::setCurRuleAction, this, qi::_1)]
    ;

  timeRangeArgument =
    qi::lit("time-range") > token
    ;

//  addressArgument =
//    (
//     // object-group NETWORK_GROUP_ID
//       (qi::lit("object-group") > token)
//         [qi::_val = qi::_1]
//     // object NETWORK_OBJECT_ID
//     | (qi::lit("object") > token)
//         [qi::_val = qi::_1]
//     // interface IFACE_NAME
//     | (qi::lit("interface") > token)
//         [qi::_val = qi::_1]
//     // host IP
//     | (qi::lit("host") >> (&ipNoPrefix > ipAddr))
//         [qi::_val = pnx::bind(&nmco::IpAddress::toString, &qi::_1)]
//     // any[46]
//     | (qi::lexeme[qi::as_string[qi::string("any") >> -qi::char_("46")]])
//         [qi::_val = qi::_1]
//     // IP *MASK
//     | (&ipNoPrefix > ipAddr > ipAddr)
//         [qi::_val = pnx::bind(&Acls::setWildcardMask, this, qi::_1, qi::_2)]
//     // TODO IP NETMASK
//     // IP/CIDR
//     | (ipAddr)
//         [qi::_val = pnx::bind(&nmco::IpAddress::toString, &qi::_1)]
//    )
//    ;

//  protocolArgument =
//    (  (qi::lit("object-group") > token)
//     | (qi::lit("object") > token)
//     // NAME | NUMBER
//     | token
//    )
//    ;
//
//  icmpArgument =
//    (  (qi::lit("object-group") > token)
//     // ICMP_TYPE [ ICMP_CODE ]
//     | (token >> -token)
//    )
//    ;
//
//  userArgument =
//    (  (qi::lit("object-group") > token)
//     | (qi::lit("user-group") > token)
//     | (qi::lit("user") > token)
//    )
//    ;
//
//  securityGroupArgument =
//    (  (qi::lit("object-group-security") > token)
//     | (qi::lit("security-group") > (qi::lit("name") | qi::lit("tag")) > token)
//    )
//    ;
//
//  logArgument =
//    qi::lit("log")
//    // LEVEL
//    >> -qi::uint_ /* 0-7*/
//    // INTERVAL BETWEEN MESSAGES
//    >> -(  (qi::lit("interval") > qi::uint_ /* 1-600 */)
//    // DISABLE LOGGING
//         | (qi::lit("disable"))
//    // SAME AS NOT INCLUDING "log"
//         | (qi::lit("default"))
//        )
//    >>
//    ;
//
//
//  inactive =
//    // RULE STATE
//    qi::lit("inactive")
//    ;

  ignoredRuleLine =
    tokens [pnx::bind(&Acls::addIgnoredRuleData, this, qi::_1)]
    >> qi::eol
    ;

  BOOST_SPIRIT_DEBUG_NODES(
      //(start)
      (config)
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
      (fragmentsArgument)
      (precedenceArgument)
      (tosArgument)
      (logArgument)
      (timeRangeArgument)
      (ipAccessListExtended)(ipAccessList)
      (bookName)
      (action)
      (protocolArgument)
      (addressArgument) (addressArgumentIos)
      (portArgument)
      (ipNoPrefix)
      (ignoredRuleLine)
      //(token)(tokens)(indent)
      );
}

// =============================================================================
// Parser helper methods
// =============================================================================

// Policy Related
void
Acls::initRuleBook(const std::string& name)
{
  ruleBookName = name;
  curRuleId = 1;
  ruleBook.clear();
}

void
Acls::initCurRule()
{
  curRuleProtocol = "";
  curRuleSrcPort = "";
  curRuleDstPort = "";

  curRule = {};
}

void
Acls::setCurRuleAction(const std::string& action)
{
  curRule.addAction(action);
}

void
Acls::setCurRuleSrc(const std::string& addr)
{
  curRule.setSrcId(ZONE);
  curRule.addSrc(addr);
}

void
Acls::setCurRuleDst(const std::string& addr)
{
  curRule.setDstId(ZONE);
  curRule.addDst(addr);
}

std::string
Acls::setWildcardMask(nmco::IpAddress& ipAddr, const nmco::IpAddress& mask)
{
  bool isContiguous {ipAddr.setWildcardMask(mask)};
  if (!isContiguous) {
    std::ostringstream oss;
    oss << "IpAddress (" << ipAddr
        << ") set with non-contiguous wildcard netmask (" << mask << ")";
  }

  return ipAddr.toString();
}

void
Acls::curRuleFinalize()
{
  curRule.setRuleId(curRuleId);
  curRule.setRuleDescription(ruleBookName);

  if (!curRuleProtocol.empty()) {
    const std::string serviceString
      {nmcu::getSrvcString(curRuleProtocol, curRuleSrcPort, curRuleDstPort)};
    curRule.addService(serviceString);
  }

  ruleBook.emplace(curRuleId, curRule);
  ++curRuleId;
}

void
Acls::addIgnoredRuleData(const std::string& _data)
{
  ignoredRuleData.emplace(_data);
}

std::set<std::string>
Acls::getIgnoredRuleData()
{
  return ignoredRuleData;
}

// Object return
Result
Acls::getData()
{
  Result r(ruleBookName, ruleBook);
  return r;
}
}
