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

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include <netmeld/core/parsers/ParserTestHelper.hpp>

#include "Acls.hpp"

namespace nmco = netmeld::core::objects;
namespace nmcp = netmeld::core::parsers;
namespace nmdsic = netmeld::datastore::importers::cisco;

using qi::ascii::blank;

BOOST_AUTO_TEST_CASE(testAclRules)
{
  // TODO move protected, subclass, refactor
  nmdsic::Acls parserAcl;

  {
    const auto& parserRule = parserAcl.action;
    // OK
    std::vector<std::tuple<std::string, std::string>> testsOk {
      // {test, expected format}
      {"permit", "permit"},
      {" deny ", "deny"},
    };
    for (const auto& [test, format] : testsOk) {
      std::string out;
      BOOST_TEST(nmcp::testAttr(test.c_str(), parserRule, out, blank));
      const auto& temp = parserAcl.curRule;
      BOOST_TEST("" == out);
      BOOST_TEST(format == temp.getActions().at(0));
    }
    // BAD
    BOOST_TEST(!nmcp::test("other", parserRule, blank));
  }

  {
    const auto& parserRule = parserAcl.protocolArgument;
    // OK
    std::vector<std::tuple<std::string, std::string>> testsOk {
      // {test, expected format}
      {"0", "0"},
      {"255", "255"},
      {"ip", "ip"},
      {"TCP", "TCP"},
      {"icmp", "icmp"},
      {"object SOI", "SOI"},
      {"object-group PGI_SGI", "PGI_SGI"},
    };
    for (const auto& [test, format] : testsOk) {
      BOOST_TEST(nmcp::test(test.c_str(), parserRule, blank));
      BOOST_TEST(format == parserAcl.curRuleProtocol);
    }
    // BAD
    // -- The parser is a 'token', so no checkable bad case?
  }

  {
    const auto& parserRule = parserAcl.addressArgumentIos;
    // OK
    std::vector<std::tuple<std::string, std::string>> testsOk {
      // {test, expected format}
      {"host 1.2.3.4", "1.2.3.4/32"},
      {"host 1234::aBcD ", "1234::abcd/128"},
      {"any ", "any"},
      {" any4 ", "any4"},
      {"any6 ", "any6"},
      {"1.2.3.4/24 ", "1.2.3.4/24"},
      {" 1234::aBcD/112 ", "1234::abcd/112"},
      {"object-group NGI", "NGI"},
      {"object NOI", "NOI"},
      {"interface IF/AC.E", "IF/AC.E"},
      {"1.2.3.4 0.0.0.255", "1.2.3.4/24"},
      {"1.2.3.4 127.255.255.255", "1.2.3.4/1"},   // WILDCARD
      {"1.2.3.4 128.0.0.0", "1.2.3.4/1"},         // NETMASK
      {"1.2.3.4 0.0.0.1", "1.2.3.4/31"},          // WILDCARD
      {"1.2.3.4 255.255.255.254", "1.2.3.4/31"},  // NETMASK
      {"1::2 3::4", "1::2/100"},                  // WILDCARD
      {"1::2 3::4", "1::2/100"},                  // NETMASK
    };
    for (const auto& [test, format] : testsOk) {
      std::string out;
      BOOST_TEST(nmcp::testAttr(test.c_str(), parserRule, out, blank));
      BOOST_TEST(format == out);
    }
    // BAD
    std::vector<std::string> testsBad {
      // {test}
      {"host 1.2.3.4/24"},
      {"host 1234::aBcD/112"},
      {"1.2.3.4 1.2.3.4/24"},
      {"1234::aBcD 1234::aBcD/112"},
      {"1.2.3.4 0.0.0.0"},
      {"1.2.3.4 255.255.255.255"},
      // always needs a space or newline after
      {"any"},
      {"any4"},
      {"any6"},
    };
    for (const auto& test : testsBad) {
      std::string out;
      BOOST_TEST(!nmcp::testAttr(test.c_str(), parserRule, out, blank));
      BOOST_TEST("" == out);
    }
  }
  {
    const auto& parserRule = parserAcl.ipNoPrefix;
    // OK
    std::vector<std::tuple<std::string, std::string>> testsOk {
      // {test, expected format}
      {"1.2.3.4", "1.2.3.4"},
      {"1.2.3.4 ", "1.2.3.4"},
      {" 1234::aBcD ", "1234::aBcD"},
    };
    for (const auto& [test, format] : testsOk) {
      std::string out;
      BOOST_TEST(nmcp::testAttr(test.c_str(), parserRule, out, blank));
      BOOST_TEST(format == out);
    }
    // BAD
    BOOST_TEST(!nmcp::test("1.2.3.4/24", parserRule));
    BOOST_TEST(!nmcp::test("1.2.3.4/", parserRule));
    BOOST_TEST(!nmcp::test("1234::aBcD/112", parserRule));
    BOOST_TEST(!nmcp::test("1234::aBcD/", parserRule));
  }

  {
    const auto& parserRule = parserAcl.portArgument;
    // OK
    std::vector<std::tuple<std::string, std::string>> testsOk {
      // {test, expected format}
      {"eq 123", "123"},
      {"eq http", "http"},
      {"neq 123", "!123"},
      {"neq http", "!http"},
      {"lt 123", "<123"},
      {"lt http", "<http"},
      {"gt 123", ">123"},
      {"gt http", ">http"},
      {"range 123 456", "123-456"},
      {"range http https", "http-https"}, // Really doesn't make sense though
    };
    for (const auto& [test, format] : testsOk) {
      std::string out;
      BOOST_TEST(nmcp::testAttr(test.c_str(), parserRule, out, blank));
      BOOST_TEST(format == out);
    }
    // BAD
    BOOST_TEST(!nmcp::test("qe 123", parserRule, blank, false));
    // -- The parser is 'keyword > token', so limited checkable bad cases
  }

  {
    const auto& parserRule = parserAcl.logArgument;
    // OK
    std::vector<std::tuple<std::string, std::string>> testsOk {
      // {test, expected format}
      {"log", "log"},
      {" log ", "log"},
      {"log-input", "log-input"},
    };
    for (const auto& [test, format] : testsOk) {
      parserAcl.initCurRule();
      std::string out;
      BOOST_TEST(nmcp::testAttr(test.c_str(), parserRule, out, blank));
      BOOST_TEST("" == out);
      nmco::AcRule temp = parserAcl.curRule;
      BOOST_TEST(format == temp.getActions().at(0));
    }
    // BAD
    BOOST_TEST(!nmcp::test("gol", parserRule, blank, false));
  }
}


//=============================================================================
// IOS tests
//=============================================================================
/*
More info:
- https://www.cisco.com/c/en/us/support/docs/security/ios-firewall/23602-confaccesslists.html#standacl
  - Not a great example, CLI to enter...not show format
  - Allows for both one and multi line show format (depending on usage)

KEY: (unless otherwise stated)
  IPV46       -- ( ip | ipv6 )
  LIST        -- Access list name
  ACTION      -- ( deny | permit )
  DYNAMIC_ARG -- ( dynamic NAME [timeout MINUTES] )
  PROTO_ARG   -- ( NAME | NUMBER )
  ADDR_ARG    -- (  host IP
                  | IP WILDMASK
                  | IPv6/PREFIX
                  | any
                 )
  PORT_ARG    -- (  ( lt | gt | eq | neq ) PORT
                  | range PORT PORT
                 )
  ICMP_ARG    -- ( ICMP_TYPE [ ICMP_CODE ] | ICMP_MESSAGE )
  LOG         -- ( log | log-input )
  TIME        -- ( time-range RANGE_NAME )

*/
/* NOTE: Below is a one-liner, wrapped for clarity
   access-list LIST ACTION ADDR_ARG
---
   NOTE: Below is a multi-liner, where \ == line wrap
   IPV46 access-list standard LIST
    ACTION ADDR_ARG
---
  ADDR_ARG -- ( IP | IP WILDMASK | any )
*/
BOOST_AUTO_TEST_CASE(testIosStandardRuleLine)
{
  nmdsic::Acls parserAcl;
  const auto& parserRule = parserAcl.iosStandardRuleLine;
  {
    const std::string fullText {
      "permit 1.2.3.4 0.0.0.255\n"
    };
    BOOST_TEST(nmcp::test(fullText.c_str(), parserRule, blank));

    const std::vector<std::tuple<std::string, std::string>> replaces {
      {"1.2.3.4 0.0.0.255", "host 1.2.3.4"},
      {"1.2.3.4 0.0.0.255", "1.2.3.4/24"},
      {"1.2.3.4 0.0.0.255", "any"},
      {"permit", "deny"},
    };
    for (const auto& [toReplace, replace] : replaces) {
      std::string testReplace = fullText;
      auto locReplace {testReplace.find(toReplace)};
      if (locReplace == testReplace.npos) { continue; }
      testReplace =
        testReplace.replace(locReplace, toReplace.size(), replace);
      BOOST_TEST(nmcp::test(testReplace.c_str(), parserRule, blank));
    }
  }
}
BOOST_AUTO_TEST_CASE(testIosStandard)
{
  nmdsic::Acls parserAcl;
  const auto& parserRule = parserAcl.iosStandard;
  {
    const std::string fullText {
      "access-list TEST permit any\n"
    };

    BOOST_TEST(nmcp::test(fullText.c_str(), parserRule, blank));
  }
  {
    const std::string fullText {
      "ip access-list standard TEST\n"
      " remark crazy rule\n"
      " permit any\n"
      " remark crazy rule\n"
      " deny   any\n"
    };

    BOOST_TEST(nmcp::test(fullText.c_str(), parserRule, blank));

    nmdsic::Result result;
    BOOST_TEST(nmcp::testAttr(fullText, nmdsic::Acls(), result, blank));

    auto& aclBookName  {result.first};
    BOOST_TEST("TEST" == aclBookName);

    auto& aclBook  {result.second};
    BOOST_TEST(2 == aclBook.size());
    BOOST_TEST("permit" == aclBook.at(1).getActions().at(0));
    BOOST_TEST("deny" == aclBook.at(2).getActions().at(0));
    size_t count {1};
    BOOST_TEST(count <= aclBook.size());
    for (auto& [id, aclLine] : aclBook) {
      BOOST_TEST(count == (id));
      ++count;
      BOOST_TEST("any" == aclLine.getSrcs().at(0));
      BOOST_TEST(0 == aclLine.getDsts().size());
      BOOST_TEST(0 == aclLine.getServices().size());
    }
  }
  {
    const std::string fullText {
      "ipv6 access-list standard TEST\n"
      " remark crazy rule\n"
      " permit any\n"
      " remark crazy rule\n"
      " deny   any\n"
    };

    BOOST_TEST(nmcp::test(fullText.c_str(), parserRule, blank));
  }
}

/* NOTE: Below is a one-liner, wrapped for clarity
   access-list LIST [DYNAMIC_ARG]
    ACTION PROTO_ARG
    ADDR_ARG ADDR_ARG [ICMP_ARG]
    [precedence PRECEDENCE] [tos TOS] [LOG] [TIME]

   access-list LIST [DYNAMIC_ARG]
    ACTION PROTO_ARG
    ADDR_ARG [PORT_ARG] ADDR_ARG [PORT_ARG]
    [established] [precedence PRECEDENCE] [tos TOS] [LOG] [TIME]
---
   NOTE: Below is a multi-liner, where \ == line wrap
   IPV46 access-list extended LIST
    ACTION icmp \
    ADDR_ARG ADDR_ARG [ICMP_ARG] \
    [precedence PRECEDENCE] [tos TOS] [LOG] [TIME]

   IPV46 access-list extended LIST
    ACTION PROTO_ARG \
    ADDR_ARG [PORT_ARG] ADDR_ARG [PORT_ARG] \
    [established] [precedence PRECEDENCE] [tos TOS] [LOG] [TIME]
---
  PRECEDENCE -- 0-7 or name
  TOS        -- 0-15 or name
*/
BOOST_AUTO_TEST_CASE(testIosExtendedRuleLine)
{
  nmdsic::Acls parserAcl;
  const auto& parserRule = parserAcl.iosExtendedRuleLine;
  {
    const std::string fullText {
      "permit ip"
      " 1.2.3.4 0.0.0.255 range 123 456"
      " 1.2.3.4 0.0.0.255 range 123 456"
      " established precedence 7 tos 15"
      " log time-range RANGE_NAME\n"
    };
    BOOST_TEST(nmcp::test(fullText.c_str(), parserRule, blank));

    const std::vector<std::string> removals {
      {" established"},
      {" precedence 7"},
      {" tos 15"},
      {" time-range RANGE_NAME"},
      {" range 123 456"},
      {" range 123 456"},
    };
    const std::vector<std::tuple<std::string, std::string>> replaces {
      {"1.2.3.4 0.0.0.255", "host 1.2.3.4"},
      {"1.2.3.4 0.0.0.255", "host 1.2.3.4"},
      {"1.2.3.4 0.0.0.255", "1.2.3.4/24"},
      {"1.2.3.4 0.0.0.255", "1.2.3.4/24"},
      {"1.2.3.4 0.0.0.255", "any"},
      {"1.2.3.4 0.0.0.255", "any"},
      {"range 123 456", "eq 123"},
      {"range 123 456", "eq 123"},
    };
    std::string testRemoval = fullText;
    for (const auto& removal : removals) {
      auto locRemoval {testRemoval.find(removal)};
      BOOST_TEST(locRemoval != testRemoval.npos);
      testRemoval = testRemoval.erase(locRemoval, removal.size());
      BOOST_TEST(nmcp::test(testRemoval.c_str(), parserRule, blank));
      std::string testReplace = testRemoval;
      for (const auto& [toReplace, replace] : replaces) {
        auto locReplace {testReplace.find(toReplace)};
        if (locReplace == testReplace.npos) { continue; }
        testReplace =
          testReplace.replace(locReplace, toReplace.size(), replace);
        BOOST_TEST(nmcp::test(testReplace.c_str(), parserRule, blank));
      }
    }
    testRemoval = fullText;
    for (const auto& removal : removals) {
      auto locRemoval {testRemoval.rfind(removal)};
      BOOST_TEST(locRemoval != testRemoval.npos);
      testRemoval = testRemoval.erase(locRemoval, removal.size());
      BOOST_TEST(nmcp::test(testRemoval.c_str(), parserRule, blank));
      std::string testReplace = testRemoval;
      for (const auto& [toReplace, replace] : replaces) {
        auto locReplace {testReplace.rfind(toReplace)};
        if (locReplace == testReplace.npos) { continue; }
        testReplace =
          testReplace.replace(locReplace, toReplace.size(), replace);
        BOOST_TEST(nmcp::test(testReplace.c_str(), parserRule, blank));
      }
    }
  }
  {
    const std::string fullText {
      "permit icmp"
      " 1.2.3.4 0.0.0.255 "
      " 1.2.3.4 0.0.0.255 0 0"
      " established precedence 7 tos 15"
      " log time-range RANGE_NAME\n"
    };
    BOOST_TEST(nmcp::test(fullText.c_str(), parserRule, blank));

    const std::vector<std::string> removals {
      {" established"},
      {" precedence 7"},
      {" tos 15"},
      {" time-range RANGE_NAME"},
    };
    const std::vector<std::tuple<std::string, std::string>> replaces {
      {"0 0", "255 255"},
      {"0 0", "administratively-prohibited"},
      {"0 0", "unreachable"},
    };
    std::string testRemoval = fullText;
    for (const auto& removal : removals) {
      auto locRemoval {testRemoval.find(removal)};
      BOOST_TEST(locRemoval != testRemoval.npos);
      testRemoval = testRemoval.erase(locRemoval, removal.size());
      BOOST_TEST(nmcp::test(testRemoval.c_str(), parserRule, blank));
      std::string testReplace = testRemoval;
      for (const auto& [toReplace, replace] : replaces) {
        auto locReplace {testReplace.find(toReplace)};
        if (locReplace == testReplace.npos) { continue; }
        testReplace =
          testReplace.replace(locReplace, toReplace.size(), replace);
        BOOST_TEST(nmcp::test(testReplace.c_str(), parserRule, blank));
      }
    }
    testRemoval = fullText;
    for (const auto& removal : removals) {
      auto locRemoval {testRemoval.rfind(removal)};
      BOOST_TEST(locRemoval != testRemoval.npos);
      testRemoval = testRemoval.erase(locRemoval, removal.size());
      BOOST_TEST(nmcp::test(testRemoval.c_str(), parserRule, blank));
      std::string testReplace = testRemoval;
      for (const auto& [toReplace, replace] : replaces) {
        auto locReplace {testReplace.rfind(toReplace)};
        if (locReplace == testReplace.npos) { continue; }
        testReplace =
          testReplace.replace(locReplace, toReplace.size(), replace);
        BOOST_TEST(nmcp::test(testReplace.c_str(), parserRule, blank));
      }
    }
  }
}
BOOST_AUTO_TEST_CASE(testIosExtended)
{
  nmdsic::Acls parserAcl;
  const auto& parserRule = parserAcl.iosExtended;
  {
    const std::string fullText {
      "access-list TEST dynamic NAME timeout 5 permit ip any any\n"
    };
    BOOST_TEST(nmcp::test(fullText.c_str(), parserRule, blank));

    const std::vector<std::string> removals {
      {" timeout 5"},
      {" dynamic NAME"},
    };
    std::string testRemoval = fullText;
    for (const auto& removal : removals) {
      auto locRemoval {testRemoval.find(removal)};
      BOOST_TEST(locRemoval != testRemoval.npos);
      testRemoval = testRemoval.erase(locRemoval, removal.size());
      BOOST_TEST(nmcp::test(testRemoval.c_str(), parserRule, blank));
    }
  }
  {
    const std::string fullText {
      "ip access-list extended TEST dynamic NAME timeout 5\n"
      " remark crazy rule\n"
      " permit ip any any\n"
      " remark crazy rule\n"
      " deny   ip any any\n"
    };
    const std::vector<std::string> removals {
      {" timeout 5"},
      {" dynamic NAME"},
    };
    std::string test = fullText;
    for (const auto& removal : removals) {
      test = test.erase(test.find(removal), removal.size());
      BOOST_TEST(nmcp::test(test.c_str(), parserRule, blank));
    }

    nmdsic::Result result;
    BOOST_TEST(nmcp::testAttr(fullText, nmdsic::Acls(), result, blank));

    auto& aclBookName  {result.first};
    BOOST_TEST("TEST" == aclBookName);

    auto& aclBook  {result.second};
    BOOST_TEST(2 == aclBook.size());
    BOOST_TEST("permit" == aclBook.at(1).getActions().at(0));
    BOOST_TEST("deny" == aclBook.at(2).getActions().at(0));
    size_t count {1};
    BOOST_TEST(count <= aclBook.size());
    for (auto& [id, aclLine] : aclBook) {
      BOOST_TEST(count == (id));
      ++count;
      BOOST_TEST("any" == aclLine.getSrcs().at(0));
      BOOST_TEST("any" == aclLine.getDsts().at(0));
      BOOST_TEST("ip::" == aclLine.getServices().at(0));
    }
  }
  {
    const std::string test {
      "ipv6 access-list extended TEST dynamic NAME timeout 5\n"
      " remark crazy rule\n"
      " permit ip any any\n"
      " remark crazy rule\n"
      " deny   ip any any\n"
    };
    BOOST_TEST(nmcp::test(test.c_str(), parserRule, blank));
  }
}

/* NOTE: Below is a multi-liner
   IPV46 access-list ( standard | extended ) LIST
    remark TEXT

   NOTE: Below is a one-liner wrapped for clarity
   access-list LIST remark TEXT
*/
BOOST_AUTO_TEST_CASE(testIosRemarkRuleLine)
{
  nmdsic::Acls parserAcl;
  const auto& parserRule = parserAcl.iosRemarkRuleLine;
  {
    // OK
    BOOST_TEST(nmcp::test("remark some\n", parserRule, blank));
    BOOST_TEST(nmcp::test("remark s r t\n", parserRule, blank));
    //// BAD
    BOOST_CHECK_THROW(nmcp::test("remark \n", parserRule, blank, false),
                      std::runtime_error);
    BOOST_CHECK_THROW(nmcp::test("remark\n", parserRule, blank, false),
                      std::runtime_error);
    BOOST_TEST(!nmcp::test("\n", parserRule, blank, false));
    BOOST_TEST(!nmcp::test("", parserRule, blank, false));
  }
}
BOOST_AUTO_TEST_CASE(testIosRemark)
{
  {
    const std::string parserAcl {
      "access-list TEST remark crazy rules\n"
    };

    nmdsic::Result result;
    BOOST_TEST(nmcp::testAttr(parserAcl, nmdsic::Acls(), result, blank));

    auto& aclBookName  {result.first};
    BOOST_TEST("TEST" == aclBookName);

    auto& aclBook  {result.second};
    BOOST_TEST(0 == aclBook.size());
  }
}


//=============================================================================
// NXOS tests
//=============================================================================

/*
More info:
- Nothing really useful, but
  - https://developer.cisco.com/docs/nx-os-n3k-n9k-api-ref-7-x/#!configuring-ace/configuring-ace-to-make-vlan-based-matches
  - https://www.cisco.com/c/m/en_us/techdoc/dc/reference/cli/nxos/commands/sec/show-access-lists.html
  - https://www.cisco.com/c/en/us/td/docs/switches/datacenter/nexus9000/sw/6-x/security/configuration/guide/b_Cisco_Nexus_9000_Series_NX-OS_Security_Configuration_Guide/b_Cisco_Nexus_9000_Series_NX-OS_Security_Configuration_Guide_chapter_01010.html

KEY: (unless otherwise stated)
  IPV46       -- ( ip | ipv6 )
  LIST        -- Access list name
  ID          -- Rule number
  ACTION      -- ( deny | permit )
  PROTO_ARG   -- ( NAME | NUMBER )
  ADDR_ARG    -- (  any
                  | IP NETMASK
                  | IP WILDMASK
                  | IP/PREFIX
                 )
TODO Need to figure out how to handle/determine NET vs WILD mask
                 )
  PORT_ARG    -- (  ( lt | gt | eq | neq ) PORT
                  | range PORT PORT
                  | object-group SERVICE_GROUP_ID
                 )
  ICMP_ARG    -- (  ICMP_TYPE [ ICMP_CODE ]
                  | object-group ICMP_GROUP_ID
                 )
  USER_ARG    -- (  user ( [DOMAIN\]NAME | any | none )
                  | user-group [DOMAIN\\]GROUP
                  | object-group USER_GROUP_ID
                 )
  SEC_GRP_ARG -- (  security-group ( name NAME | tag TAG )
                  | object-group-security SECURITY_GROUP_ID
                 )
  LOG         -- ( log [[LEVEL] [interval SECS] | disable | default] )
  TIME        -- ( time-range RANGE_NAME )

*/
/* NOTE: Below is a one-liner, wrapped for clarity
   NO-EXAMPLE FOUND
---
   NOTE: Below is a multi-liner, where \ == line wrap
   NO-EXAMPLE FOUND
*/
BOOST_AUTO_TEST_CASE(testNxosStandardRuleLine)
{
  nmdsic::Acls parserAcl;
  const auto& parserRule = parserAcl.nxosStandardRuleLine;

  {
    const std::string fullText {
      "permit any\n"
    };

    BOOST_TEST(!nmcp::test(fullText.c_str(), parserRule, blank));
  }
}
BOOST_AUTO_TEST_CASE(testNxosStandard)
{
  nmdsic::Acls parserAcl;
  const auto& parserRule = parserAcl.nxosStandardRuleLine;

  {
    const std::string fullText {
      "access-list TEST permit any\n"
    };

    BOOST_TEST(!nmcp::test(fullText.c_str(), parserRule, blank));
  }
  {
    const std::string fullText {
      "ip access-list standard TEST\n"
      " 10 remark crazy rule\n"
      " 20 permit any\n"
      " 30 remark crazy rule\n"
      " 40 deny   any\n"
    };

    BOOST_TEST(!nmcp::test(fullText.c_str(), parserRule, blank));
  }
  {
    const std::string fullText {
      "ipv6 access-list standard TEST\n"
      " 10 remark crazy rule\n"
      " 20 permit any\n"
      " 30 remark crazy rule\n"
      " 40 deny   any\n"
    };

    BOOST_TEST(!nmcp::test(fullText.c_str(), parserRule, blank));
  }
}
/* NOTE: Below is a one-liner, wrapped for clarity
   NO-EXAMPLE FOUND
---
   NOTE: Below is a multi-liner, where \ == line wrap
   IPV46 access-list LIST
    ID ACTION PROTO_ARG \
    ADDR_ARG [PORT_ARG] ADDR_ARG [PORT_ARG] \
    [established] [fragments] [precedence PRECEDENCE] [tos TOS] [LOG] [TIME]

NOTE: Appears to support objects, but no documenation shows output format
      In lieu of clear documentation, assuming at least the same as IOS
*/
BOOST_AUTO_TEST_CASE(testNxosExtendedRuleLine)
{
  nmdsic::Acls parserAcl;
  const auto& parserRule = parserAcl.nxosExtendedRuleLine;
  {
    const std::string fullText {
      "10 permit ip"
      " 1.2.3.4 0.0.0.255 range 123 456"
      " 1.2.3.4 0.0.0.255 range 123 456"
      " established fragments precedence 7 tos 15"
      " log time-range RANGE_NAME\n"
    };
    BOOST_TEST(nmcp::test(fullText.c_str(), parserRule, blank));

    const std::vector<std::string> removals {
      {" established"},
      {" fragments"},
      {" precedence 7"},
      {" tos 15"},
      {" time-range RANGE_NAME"},
      {" range 123 456"},
      {" range 123 456"},
    };
    const std::vector<std::tuple<std::string, std::string>> replaces {
      {"1.2.3.4 0.0.0.255", "host 1.2.3.4"},
      {"1.2.3.4 0.0.0.255", "host 1.2.3.4"},
      {"1.2.3.4 0.0.0.255", "1.2.3.4/24"},
      {"1.2.3.4 0.0.0.255", "1.2.3.4/24"},
      {"1.2.3.4 0.0.0.255", "any"},
      {"1.2.3.4 0.0.0.255", "any"},
      {"range 123 456", "eq 123"},
      {"range 123 456", "eq 123"},
    };
    std::string testRemoval = fullText;
    for (const auto& removal : removals) {
      auto locRemoval {testRemoval.find(removal)};
      BOOST_TEST(locRemoval != testRemoval.npos);
      testRemoval = testRemoval.erase(locRemoval, removal.size());
      BOOST_TEST(nmcp::test(testRemoval.c_str(), parserRule, blank));
      std::string testReplace = testRemoval;
      for (const auto& [toReplace, replace] : replaces) {
        auto locReplace {testReplace.find(toReplace)};
        if (locReplace == testReplace.npos) { continue; }
        testReplace =
          testReplace.replace(locReplace, toReplace.size(), replace);
        BOOST_TEST(nmcp::test(testReplace.c_str(), parserRule, blank));
      }
    }
    testRemoval = fullText;
    for (const auto& removal : removals) {
      auto locRemoval {testRemoval.rfind(removal)};
      BOOST_TEST(locRemoval != testRemoval.npos);
      testRemoval = testRemoval.erase(locRemoval, removal.size());
      BOOST_TEST(nmcp::test(testRemoval.c_str(), parserRule, blank));
      std::string testReplace = testRemoval;
      for (const auto& [toReplace, replace] : replaces) {
        auto locReplace {testReplace.rfind(toReplace)};
        if (locReplace == testReplace.npos) { continue; }
        testReplace =
          testReplace.replace(locReplace, toReplace.size(), replace);
        BOOST_TEST(nmcp::test(testReplace.c_str(), parserRule, blank));
      }
    }
  }
}
BOOST_AUTO_TEST_CASE(testNxosExtended)
{
  nmdsic::Acls parserAcl;
  const auto& parserRule = parserAcl.nxosExtended;
  {
    const std::string fullText {
      "ip access-list TEST dynamic NAME timeout 5\n"
      " 10 remark crazy rule\n"
      " 20 permit ip any any\n"
      " 30 remark crazy rule\n"
      " 40 deny   ip any any\n"
    };
    const std::vector<std::string> removals {
      {" timeout 5"},
      {" dynamic NAME"},
    };
    std::string test = fullText;
    for (const auto& removal : removals) {
      test = test.erase(test.find(removal), removal.size());
      BOOST_TEST(nmcp::test(test.c_str(), parserRule, blank));
    }

    nmdsic::Result result;
    BOOST_TEST(nmcp::testAttr(fullText, nmdsic::Acls(), result, blank));

    auto& aclBookName  {result.first};
    BOOST_TEST("TEST" == aclBookName);

    auto& aclBook  {result.second};
    BOOST_TEST(2 == aclBook.size());
    BOOST_TEST("permit" == aclBook.at(1).getActions().at(0));
    BOOST_TEST("deny" == aclBook.at(2).getActions().at(0));
    size_t count {1};
    BOOST_TEST(count <= aclBook.size());
    for (auto& [id, aclLine] : aclBook) {
      BOOST_TEST(count == (id));
      ++count;
      BOOST_TEST("any" == aclLine.getSrcs().at(0));
      BOOST_TEST("any" == aclLine.getDsts().at(0));
      BOOST_TEST("ip::" == aclLine.getServices().at(0));
    }
  }
  {
    const std::string fullText {
      "ip access-list TEST\n"
      " 10 remark ignored rules\n"
      " 20 permit tcp any any http-method head\n"
      " 21 permit ip any any vlan 5\n"
      " 22 permit tcp any any tcp-option-length 36\n"
      " 23 permit ip any any udf name1 0x1 0x2 udf name2 0xff 0xee\n"
      " 100 remark captured rule\n"
      " 110 deny   ip any any\n"
      " 120 remark ignored rule\n"
      " 130 some other line rule that should not match\n"
    };

    nmdsic::Result result;
    BOOST_TEST(nmcp::testAttr(fullText, parserAcl, result, blank));

    auto& aclBookName  {result.first};
    BOOST_TEST("TEST" == aclBookName);

    auto& aclBook  {result.second};
    BOOST_TEST(1 == aclBook.size());
    BOOST_TEST("deny" == aclBook.at(1).getActions().at(0));
    const auto& ignoredRules = parserAcl.getIgnoredRuleData();
    BOOST_TEST(5 == ignoredRules.size());
  }
}
/* NOTE: Below is a one-liner, wrapped for clarity
   NO-EXAMPLE FOUND
---
   NOTE: Below is a multi-liner, where \ == line wrap

   IPV46 access-list LIST
    ID remark TEXT

   NO-EXAMPLE FOUND
*/
BOOST_AUTO_TEST_CASE(testNxosRemarkRuleLine)
{
  nmdsic::Acls parserAcl;
  const auto& parserRule = parserAcl.nxosRemarkRuleLine;
  {
    const std::vector<std::string> tests {
      {"10 remark some\n"},
      {"100 remark s r t\n"},
    };
    for (const auto& test : tests) {
      BOOST_TEST(nmcp::test(test.c_str(), parserRule, blank));
    }
  }
  {
    const std::vector<std::string> tests {
      {"10 remark \n"},
      {"10 remark\n"},
    };
    for (const auto& test : tests) {
      BOOST_CHECK_THROW(!nmcp::test(test.c_str(), parserRule, blank, false),
                        std::runtime_error);
    }
  }
  {
    const std::vector<std::string> tests {
      {"10\n"},
      {"\n"},
      {""},
    };
    for (const auto& test : tests) {
      BOOST_TEST(!nmcp::test(test.c_str(), parserRule, blank, false));
    }
  }
}
BOOST_AUTO_TEST_CASE(testNxosRemark)
{
}

//=============================================================================
// ASA tests
//=============================================================================


/*
More info:
- https://www.cisco.com/c/en/us/td/docs/security/asa/asa96/configuration/firewall/asa-96-firewall-config/access-acls.html
  - Appears to be representative of show format

KEY: (unless otherwise stated)
  *_OBJECT_ID   -- An object ID as created via the `object *` command.
  *_GROUP_ID    -- An object ID as created via the `object-group *` command.
  LIST        -- Access list name
  ACTION      -- ( deny | permit )
  PROTO_ARG   -- (  ( NAME | NUMBER )
                  | object SERVICE_OBJECT_ID
                  | object-group ( PROTOCOL_GROUP_ID | SERVICE_GROUP_ID )
                 )
  ADDR_ARG    -- (  host IP
                  | IP NETMASK
                  | IPv6/PREFIX
                  | any[46]
                  | interface IFACE_NAME
                  | object NETWORK_OBJECT_ID
                  | object-group NETWORK_GROUP_ID
                 )
  PORT_ARG    -- (  ( lt | gt | eq | neq ) PORT
                  | range PORT PORT
                  | object-group SERVICE_GROUP_ID
                 )
  ICMP_ARG    -- (  ICMP_TYPE [ ICMP_CODE ]
                  | object-group ICMP_GROUP_ID
                 )
  USER_ARG    -- (  user ( [DOMAIN\]NAME | any | none )
                  | user-group [DOMAIN\\]GROUP
                  | object-group USER_GROUP_ID
                 )
  SEC_GRP_ARG -- (  security-group ( name NAME | tag TAG )
                  | object-group-security SECURITY_GROUP_ID
                 )
  LOG         -- ( log [[LEVEL] [interval SECS] | disable | default] )
  TIME        -- ( time-range RANGE_NAME )

*/
/* NOTE: Below is a one-liner wrapped for clarity
   access-list LIST standard ACTION ADDR_ARG
   ---
   ADDR_ARG -- ( host IP | IP MASK | any4)
*/
BOOST_AUTO_TEST_CASE(testAsaStandardRuleLine)
{
  nmdsic::Acls parserAcl;
  const auto& parserRule = parserAcl.asaStandardRuleLine;
  {
    const std::string fullText {
      "permit 1.2.3.4 0.0.0.255\n"
    };
    BOOST_TEST(nmcp::test(fullText.c_str(), parserRule, blank));

    const std::vector<std::tuple<std::string, std::string>> replaces {
      {"1.2.3.4 0.0.0.255", "host 1.2.3.4"},
      {"1.2.3.4 0.0.0.255", "1.2.3.4/24"},
      {"1.2.3.4 0.0.0.255", "any"},
      {"1.2.3.4 0.0.0.255", "any4"},
      {"1.2.3.4 0.0.0.255", "any6"},
      {"permit", "deny"},
    };
    for (const auto& [toReplace, replace] : replaces) {
      std::string testReplace = fullText;
      auto locReplace {testReplace.find(toReplace)};
      if (locReplace == testReplace.npos) { continue; }
      testReplace =
        testReplace.replace(locReplace, toReplace.size(), replace);
      BOOST_TEST(nmcp::test(testReplace.c_str(), parserRule, blank));
    }
  }
}
BOOST_AUTO_TEST_CASE(testAsaStandard)
{
//  nmcu::LoggerSingleton::getInstance().setLevel(nmcu::Severity::ALL);

  nmdsic::Acls parserAcl;
  const auto& parserRule = parserAcl.asaStandard;
  {
    const std::string fullText {
      "access-list TEST standard permit any\n"
      "access-list TEST standard deny any\n"
    };
    BOOST_TEST(nmcp::test(fullText.c_str(), parserRule, blank, false));

    nmdsic::Result result;
    BOOST_TEST(nmcp::testAttr(fullText, nmdsic::Acls(), result, blank, false));

    auto& aclBookName  {result.first};
    BOOST_TEST("TEST" == aclBookName);

    auto& aclBook  {result.second};
    BOOST_TEST(1 == aclBook.size());
    BOOST_TEST("permit" == aclBook.at(1).getActions().at(0));
    size_t count {1};
    BOOST_TEST(count <= aclBook.size());
    for (auto& [id, aclLine] : aclBook) {
      BOOST_TEST(count == (id));
      ++count;
      BOOST_TEST("any" == aclLine.getSrcs().at(0));
      BOOST_TEST(0 == aclLine.getDsts().size());
      BOOST_TEST(0 == aclLine.getServices().size());
    }
  }
}
#if false
/* NOTE: Below is a one-liner wrapped for clarity
   access-list LIST extended ACTION
    PROTO_ARG
    [USER_ARG | SEC_GRP_ARG] ADDR_ARG [PORT_ARG]
    [SEC_GRP_ARG] ADDR_ARG [PORT_ARG | ICMP_ARG]
    [LOG] [TIME] [inactive]
*/
BOOST_AUTO_TEST_CASE(testAsaExtendedRuleLine)
{
  nmdsic::Acls parserAcl;
  const auto& parserRule = parserAcl.asaExtendedRuleLine;
  {
    const std::string fullText {
      "permit ip"
      " security-group name SGN1 1.2.3.4 0.0.0.255 range 123 456"
      " security-group name SGN2 1.2.3.4 0.0.0.255 range 123 456"
      " log LEVEL interval 10 time-range RANGE_NAME inactive\n"
    };
    BOOST_TEST(nmcp::test(fullText.c_str(), parserRule, blank));

    const std::vector<std::string> removals {
      {" time-range RANGE_NAME"},
      {" inactive"},
      {" log LEVEL interval 10"},
      {" security-group name SGN2"},
      {" security-group name SGN1"},
      {" range 123 456"},
      {" range 123 456"},
    };
    const std::vector<std::tuple<std::string, std::string>> replaces {
      {"security-group name SGN1", "security-group tag SGT1"},
      {"security-group name SGN2", "security-group tag SGT2"},
      {"security-group tag SGT1", "object-group-security OGSEC1"},
      {"security-group tag SGT2", "object-group-security OGSEC2"},
      {"object-group-security OGSEC1", "user domain\\name"},
      {"user domain\\name", "user any"},
      {"user any", "user none"},
      {"user none", "user-group domain\\group"},
      {"user-group domain\\group", "object-group UGI"},
      {"1.2.3.4 0.0.0.255", "host 1.2.3.4"},
      {"1.2.3.4 0.0.0.255", "1.2.3.4/24"},
      {"1.2.3.4 0.0.0.255", "1.2.3.4/24"},
      {"1.2.3.4 0.0.0.255", "any"},
      {"1.2.3.4 0.0.0.255", "any"},
      {"range 123 456", "eq 123"},
      {"range 123 456", "eq 123"},
    };
    std::string testRemoval = fullText;
    for (const auto& removal : removals) {
      auto locRemoval {testRemoval.find(removal)};
      BOOST_TEST(locRemoval != testRemoval.npos);
      testRemoval = testRemoval.erase(locRemoval, removal.size());
      BOOST_TEST(nmcp::test(testRemoval.c_str(), parserRule, blank));
      std::string testReplace = testRemoval;
      for (const auto& [toReplace, replace] : replaces) {
        auto locReplace {testReplace.find(toReplace)};
        if (locReplace == testReplace.npos) { continue; }
        testReplace =
          testReplace.replace(locReplace, toReplace.size(), replace);
        BOOST_TEST(nmcp::test(testReplace.c_str(), parserRule, blank));
      }
    }
    testRemoval = fullText;
    for (const auto& removal : removals) {
      auto locRemoval {testRemoval.rfind(removal)};
      BOOST_TEST(locRemoval != testRemoval.npos);
      testRemoval = testRemoval.erase(locRemoval, removal.size());
      BOOST_TEST(nmcp::test(testRemoval.c_str(), parserRule, blank));
      std::string testReplace = testRemoval;
      for (const auto& [toReplace, replace] : replaces) {
        auto locReplace {testReplace.rfind(toReplace)};
        if (locReplace == testReplace.npos) { continue; }
        testReplace =
          testReplace.replace(locReplace, toReplace.size(), replace);
        BOOST_TEST(nmcp::test(testReplace.c_str(), parserRule, blank));
      }
    }
  }
}
BOOST_AUTO_TEST_CASE(testAsaExtended)
{
}
#endif
/* NOTE: Below is a one-liner wrapped for clarity
   access-list LIST remark SPACED_FILLED_TEXT
*/
BOOST_AUTO_TEST_CASE(testAsaRemarkRuleLine)
{
  nmdsic::Acls parserAcl;
  const auto& parserRule = parserAcl.asaRemarkRuleLine;
  {
    // OK
    BOOST_TEST(nmcp::test("remark some\n", parserRule, blank));
    BOOST_TEST(nmcp::test("remark s r t\n", parserRule, blank));
    //// BAD
    BOOST_CHECK_THROW(nmcp::test("remark \n", parserRule, blank, false),
                      std::runtime_error);
    BOOST_CHECK_THROW(nmcp::test("remark\n", parserRule, blank, false),
                      std::runtime_error);
    BOOST_TEST(!nmcp::test("\n", parserRule, blank, false));
    BOOST_TEST(!nmcp::test("", parserRule, blank, false));
  }
}
BOOST_AUTO_TEST_CASE(testAsaRemark)
{
  {
    const std::string parserAcl {
      "access-list TEST remark - crazy rules\n"
    };

    nmdsic::Result result;
    BOOST_TEST(nmcp::testAttr(parserAcl, nmdsic::Acls(), result, blank));

    auto& aclBookName  {result.first};
    BOOST_TEST("TEST" == aclBookName);

    auto& aclBook  {result.second};
    BOOST_TEST(0 == aclBook.size());
  }
}
#if false
/* NOTE: Below is a one-liner wrapped for clarity
   access-list LIST webtype ACTION
    url ( URL_REGEX | any )
    [LOG] [TIME] [inactive]
   access-list LIST webtype ACTION
    tcp ADDR_ARG
    [PORT_ARG]
    [LOG] [TIME] [inactive]
  PORT_ARG -- (  ( lt | gt | eq | neq ) PORT
               | range PORT PORT
              )
*/
BOOST_AUTO_TEST_CASE(testAsaWebType)
{
}
/* NOTE: Below is a one-liner wrapped for clarity
   access-list LIST ethertype ACTION
    {  any
     | bpdu
     | dsap hex_address
     | ipx
     | isis
     | mpls-multicast
     | mpls-unicast
     | hex_number
    }
*/
BOOST_AUTO_TEST_CASE(testAsaEthertype)
{
  {
    const auto& parserRule = parserAcl.asaEther;
    // OK
    BOOST_TEST(nmcp::test("ethertype permit any", parserRule, blank));
    BOOST_TEST(nmcp::test("ethertype deny any", parserRule, blank));
    // Cisco note: no longer matches as intended, use dsap instead
    BOOST_TEST(nmcp::test("ethertype permit bpdu", parserRule, blank));
    BOOST_TEST(nmcp::test("ethertype permit dsap 0x42", parserRule, blank));
    BOOST_TEST(nmcp::test("ethertype permit dsap 0x01", parserRule, blank));
    BOOST_TEST(nmcp::test("ethertype permit dsap 0xff", parserRule, blank));
    BOOST_TEST(nmcp::test("ethertype permit ipx", parserRule, blank));
    BOOST_TEST(nmcp::test("ethertype permit isis", parserRule, blank));
    BOOST_TEST(nmcp::test("ethertype permit mpls-multicast", parserRule, blank));
    BOOST_TEST(nmcp::test("ethertype permit mpls-unicast", parserRule, blank));
    BOOST_TEST(nmcp::test("ethertype permit 0x600", parserRule, blank));
    BOOST_TEST(nmcp::test("ethertype permit 0xffff", parserRule, blank));
    // BAD
    BOOST_TEST(!nmcp::test("ethertype permit", parserRule, blank));
    BOOST_TEST(!nmcp::test("ethertype permit asdf", parserRule, blank));
  }

  // Full trip
  {
    const std::string fullText {
      "access-list TEST ethertype permit any\n"
      "access-list TEST ethertype deny any\n"
    };

    nmdsic::Result result;
    BOOST_TEST(nmcp::testAttr(fullText, nmdsic::Acls(), result, blank));

    auto& aclBookName  {result.first};
    BOOST_TEST("" == aclBookName);

    auto& aclBook  {result.second};
    BOOST_TEST(0 == aclBook.size());
  }
}
#endif
