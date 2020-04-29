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
     | ipAccessListExtended
//     | ipAccessList
//     | asaRule
    );

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
    (  (qi::lit("ip access-list standard")
        > bookName > qi::eol
        > *(indent > (iosRemarkRuleLine | iosStandardRuleLine)))
     | (qi::lit("access-list") >> bookName >> iosStandardRuleLine)
    )
    ;
  iosStandardRuleLine =
    action
    >> sourceAddrIos
    > qi::eol [pnx::bind(&Acls::curRuleFinalize, this)]
    ;

  iosExtended =
    (  (qi::lit("ip access-list extended")
        > bookName >> -dynamicArgument > qi::eol
        >> *(indent > (iosRemarkRuleLine | iosExtendedRuleLine)))
     | (qi::lit("access-list")
        >> bookName >> -dynamicArgument >> iosExtendedRuleLine)
    )
    ;
  iosExtendedRuleLine =
    action
    >> protocolArgument
    >> sourceAddrIos >> -sourcePort
    >> -(destinationAddrIos >> -destinationPort)
    >> -establishedArgument
    >> -precedenceArgument
    >> -tosArgument
    >> -logArgument
    >> -timeRangeArgument
    > qi::eol [pnx::bind(&Acls::curRuleFinalize, this)]
    ;

  //==========
  // ACL Types
  //==========

  /*
     NXOS
     (ip | ipv6) access-list NAME
      ID ACTION PROTOCOL SOURCE [PORTS] [DEST [PORTS]] [ACTIONS]
    ---
    ID (it is the sequence number)
    ACTION ( permit | deny )
    PROTOCOL ( name )
    SOURCE ( IP *MASK | IP/CIDR | any )
    DEST   ( IP *MASK | IP/CIDR | any )
    PORTS ( eq PORT | range START END )
    log
  */
  ipAccessList =
    ((qi::lit("ipv6") | qi::lit("ip")) >> qi::lit("access-list") > token > qi::eol)
      [pnx::bind(&Acls::updateCurRuleBook, this, qi::_1)] >>
    *(indent [pnx::bind(&Acls::updateCurRule, this)] >>
      // ID
      +qi::uint_ >>
      // ACTION
      token [pnx::bind(&Acls::setCurRuleAction, this, qi::_1)] >>
      // PROTOCOL
      token [pnx::bind(&Acls::curRuleProtocol, this) = qi::_1] >>
      // SOURCE
      addressArgument [pnx::bind(&Acls::setCurRuleSrc, this, qi::_1)] >>
      // SOURCE PORTS
      -(portArgument [pnx::bind(&Acls::curRuleSrcPort, this) = qi::_1]) >>
      // DESTINATION
      -(addressArgument [pnx::bind(&Acls::setCurRuleDst, this, qi::_1)] >>
        // DESTINATION PORTS
        -(portArgument [pnx::bind(&Acls::curRuleDstPort, this) = qi::_1])
      ) >> -(+token [pnx::bind(&Acls::setCurRuleAction, this, qi::_1)]) >>
      qi::eol
    ) [pnx::bind(&Acls::curRuleFinalize, this)]
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
//  asaStandard =
//    qi::lit("standard")
//    // ACTION
//    > token
//    // TARGET
//    > addressArgument
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
  // Helpers
  //========
  bookName =
    token [pnx::bind(&Acls::updateCurRuleBook, this, qi::_1)]
    ;

  action =
    (qi::string("permit") | qi::string("deny"))
      [pnx::bind(&Acls::updateCurRule, this),
       pnx::bind(&Acls::setCurRuleAction, this, qi::_1)]
    ;

  // IOS specific
  dynamicArgument =
    qi::lit("dynamic") > token >> -(qi::lit("timeout") > qi::uint_)
    ;

  protocolArgument =
    token [pnx::bind(&Acls::curRuleProtocol, this) = qi::_1]
    ;

  sourceAddrIos =
    addressArgumentIos [pnx::bind(&Acls::setCurRuleSrc, this, qi::_1)]
    ;
  destinationAddrIos =
    addressArgumentIos [pnx::bind(&Acls::setCurRuleDst, this, qi::_1)]
    ;
  addressArgumentIos =
    (
       (qi::lit("host") >> (&ipLikeNoCidr > ipAddr))
         [qi::_val = pnx::bind(&nmco::IpAddress::toString, &qi::_1)]
     | (qi::lexeme[qi::string("any") >> &qi::space])
         [qi::_val = qi::_1]
       // TODO needs to be ipAddr.ipv4
     | (&ipLikeNoCidr >> ipAddr >> &ipLikeNoCidr >> ipAddr)
         [qi::_val = pnx::bind(&Acls::setWildcardMask, this, qi::_1, qi::_2)]
     | (&((ipLikeNoCidr >> qi::eol) | !ipLikeNoCidr) >> ipAddr)
         [qi::_val = pnx::bind(&nmco::IpAddress::toString, &qi::_1)]
    )
    ;
  ipLikeNoCidr =
    (ipAddr.ipv4 | ipAddr.ipv6) >> !(qi::lit('/') >> ipAddr.cidr)
    ;

  sourcePort =
    portArgument [pnx::bind(&Acls::curRuleSrcPort, this) = qi::_1]
    ;
  destinationPort =
    portArgument [pnx::bind(&Acls::curRuleDstPort, this) = qi::_1]
    ;
  portArgument =
    (  (qi::lit("eq") > token)
         [qi::_val = qi::_1]
     | (qi::lit("neq") > token)
         [qi::_val = "!" + qi::_1]
     | (qi::lit("lt") > token)
         [qi::_val = "<" + qi::_1]
     | (qi::lit("gt") > token)
         [qi::_val = ">" + qi::_1]
     | (qi::lit("range") > token > token)
         [qi::_val = (qi::_1 + "-" + qi::_2)]
    )
    ;

  establishedArgument =
    qi::string("established")
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
//     | (qi::lit("host") >> (&ipLikeNoCidr > ipAddr))
//         [qi::_val = pnx::bind(&nmco::IpAddress::toString, &qi::_1)]
//     // any[46]
//     | (qi::lexeme[qi::as_string[qi::string("any") >> -qi::char_("46")]])
//         [qi::_val = qi::_1]
//     // IP *MASK
//     | (&ipLikeNoCidr > ipAddr > ipAddr)
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

  BOOST_SPIRIT_DEBUG_NODES(
      //(start)
      (config)
      (ipAccessListExtended)(ipAccessList)
      (addressArgument)(ipLikeNoCidr)(portArgument)
      //(token)(tokens)(indent)
      );
}

// =============================================================================
// Parser helper methods
// =============================================================================

// Policy Related
void
Acls::updateCurRuleBook(const std::string& name)
{
  ruleBookName = name;
  curRuleId = 0;
  ruleBook.clear();
}

void
Acls::updateCurRule()
{
  ++curRuleId;
  curRuleProtocol = "";
  curRuleSrcPort = "";
  curRuleDstPort = "";

  curRule = &(ruleBook[curRuleId]);

  curRule->setRuleId(curRuleId);
  curRule->setRuleDescription(ruleBookName);
}

void
Acls::setCurRuleAction(const std::string& action)
{
  curRule->addAction(action);
}

void
Acls::setCurRuleSrc(const std::string& addr)
{
  curRule->setSrcId(ZONE);
  curRule->addSrc(addr);
}

void
Acls::setCurRuleDst(const std::string& addr)
{
  curRule->setDstId(ZONE);
  curRule->addDst(addr);
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
  if (curRuleProtocol.empty()) {
    return;
  }
  const std::string serviceString
    {nmcu::getSrvcString(curRuleProtocol, curRuleSrcPort, curRuleDstPort)};

  curRule->addService(serviceString);
}

// Object return
Result
Acls::getData()
{
  Result r(ruleBookName, ruleBook);
  return r;
}
}
