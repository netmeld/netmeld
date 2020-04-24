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
    (  ipAccessListStandard
     | ipAccessListExtended
//     | ipAccessList
    );

  // TODO double check vs other types
  // TODO ID ACTION PROTOCOL...

  //==========
  // ACL Types
  //==========

  /*
    ip access-list standard NAME
  */
  ipAccessListStandard =
    (qi::lit("ip access-list standard") > token > qi::eol)
    ;

  /*
    ip access-list extended NAME
      ACTION PROTOCOL SOURCE [PORTS] [ DEST [PORTS] ] ["log"]
    ---
    ACTION ( "permit" | "deny" )
    PROTOCOL ( name )
    SOURCE ( IpAddr *mask | "host" IpAddr | "any" )
      - Note: *mask (wildcard mask) takes bit representation of mask and
        compares with bits of IpAddr. (1=wild, 0=must match IpAddr)
    DEST ( IpAddr *mask | "host" IpAddr | "any" | "object-group" name )
    PORTS ( "eq" port | "range" startPort endPort )
    "log" (Couldn't find anything about options to this in this context)
  */
  ipAccessListExtended =
    (qi::lit("ip access-list extended") > token > qi::eol)
      [pnx::bind(&Acls::updateCurRuleBook, this, qi::_1)] >>
    *(indent [pnx::bind(&Acls::updateCurRule, this)] >>
      // TODO ensure this gets NXOS rules
      -(*qi::uint_) >>
      // ACTION
      token [pnx::bind(&Acls::setCurRuleAction, this, qi::_1)] >>
      // PROTOCOL
      token [pnx::bind(&Acls::curRuleProtocol, this) = qi::_1] >>
      // SOURCE
      addressArgument [pnx::bind(&Acls::setCurRuleSrc, this, qi::_1)] >>
      // SOURCE PORTS
      -(ports [pnx::bind(&Acls::curRuleSrcPort, this) = qi::_1]) >>
      // DESTINATION
      -(addressArgument [pnx::bind(&Acls::setCurRuleDst, this, qi::_1)] >>
        // DESTINATION PORTS
        -(ports [pnx::bind(&Acls::curRuleDstPort, this) = qi::_1])
      ) >> -qi::lit("log") >> qi::eol
    ) [pnx::bind(&Acls::curRuleFinalize, this)]
    ;

//  ipAccessList =
//    ;


  //========
  // Helpers
  //========

  /*
    ( IpAddr *mask | "host" IpAddr | "any" | "object-group" name )
  */
  addressArgument =
    (  (ipAddr >> ipAddr)
         [qi::_val = pnx::bind(&Acls::setWildcardMask, this, qi::_1, qi::_2)]
     | (qi::lit("host") >> ipAddr)
         [qi::_val = pnx::bind(&nmco::IpAddress::toString, &qi::_1)]
     | (qi::lit("object-group") >> token)
         [qi::_val = qi::_1]
     | (qi::string("any"))
         [qi::_val = qi::_1]
    )
    ;

  ports =
    (  (qi::lit("eq") >> token)
         [qi::_val = qi::_1]
     | (qi::lit("range") >> token >> token)
         [qi::_val = (qi::_1 + "-" + qi::_2)]
    )
    ;


  BOOST_SPIRIT_DEBUG_NODES(
      //(start)
      (config)
      (ipAccessListStandard)(ipAccessListExtended)(ipAccessList)
      (addressArgument)(ports)
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
