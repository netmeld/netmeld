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
  nmdsic::Acls acl;

  {
    const auto& rule = acl.action;
    // OK
    std::vector<std::tuple<std::string, std::string>> testsOk {
      // {test, expected format}
      {"permit", "permit"},
      {" deny ", "deny"},
    };
    for (const auto& [test, format] : testsOk) {
      std::string out;
      BOOST_TEST(nmcp::testAttr(test.c_str(), rule, out, blank));
      const auto& temp = acl.curRule;
      BOOST_TEST("" == out);
      BOOST_TEST(format == temp->getActions().at(0));
    }
    // BAD
    BOOST_TEST(!nmcp::test("other", rule, blank));
  }

  {
    const auto& rule = acl.protocolArgument;
    // OK
    BOOST_TEST(nmcp::test("tcp", rule, blank));
    BOOST_TEST(nmcp::test("IP", rule, blank));
    BOOST_TEST(nmcp::test("0", rule, blank));
    BOOST_TEST(nmcp::test("255", rule, blank));
    std::string out;
    BOOST_TEST(nmcp::testAttr("tcp", rule, out, blank));
    BOOST_TEST("" == out);
    // BAD
    // -- The parser is a 'token', so no checkable bad case?
  }

  {
    const auto& rule = acl.addressArgumentIos;
    // OK
    std::vector<std::tuple<std::string, std::string>> testsOk {
      // {test, expected format}
      {"host 1.2.3.4", "1.2.3.4/32"},
      {"host 1234::aBcD ", "1234::abcd/128"},
      {" any ", "any"},
      {" 1.2.3.4 0.0.0.255", "1.2.3.4/24"},
      {"1.2.3.4/24 ", "1.2.3.4/24"},
      {" 1234::aBcD/112 ", "1234::abcd/112"},
    };
    for (const auto& [test, format] : testsOk) {
      std::string out;
      BOOST_TEST(nmcp::testAttr(test.c_str(), rule, out, blank));
      BOOST_TEST(format == out);
    }
    // BAD
    std::vector<std::string> testsBad {
      // {test, expected format}
      {"host 1.2.3.4/24"},
      {"host 1234::aBcD/112"},
      {"any4"},
      {"any6"},
      {"1.2.3.4 1.2.3.4/24"},
      {"1234::aBcD 1234::aBcD/112"},
      {"object-group group"},
      {"object object"},
      {"interface iface"},
    };
    for (const auto& test : testsBad) {
      std::string out;
      BOOST_TEST(!nmcp::testAttr(test.c_str(), rule, out, blank));
      BOOST_TEST("" == out);
    }
  }
  {
    const auto& rule = acl.ipLikeNoCidr;
    // OK
    std::vector<std::tuple<std::string, std::string>> testsOk {
      // {test, expected format}
      {"1.2.3.4", "1.2.3.4"},
      {"1.2.3.4 ", "1.2.3.4"},
      {" 1234::aBcD ", "1234::aBcD"},
    };
    for (const auto& [test, format] : testsOk) {
      std::string out;
      BOOST_TEST(nmcp::testAttr(test.c_str(), rule, out, blank));
      BOOST_TEST(format == out);
    }
    // BAD
    BOOST_TEST(!nmcp::test("1.2.3.4/24", rule));
    BOOST_TEST(!nmcp::test("1.2.3.4/", rule));
    BOOST_TEST(!nmcp::test("1234::aBcD/112", rule));
    BOOST_TEST(!nmcp::test("1234::aBcD/", rule));
  }

  {
    const auto& rule = acl.portArgument;
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
      BOOST_TEST(nmcp::testAttr(test.c_str(), rule, out, blank));
      BOOST_TEST(format == out);
    }
    // BAD
    BOOST_TEST(!nmcp::test("qe 123", rule, blank, false));
    // -- The parser is 'keyword > token', so limited checkable bad cases
  }

  {
    const auto& rule = acl.logArgument;
    // OK
    std::vector<std::tuple<std::string, std::string>> testsOk {
      // {test, expected format}
      {"log", "log"},
      {" log ", "log"},
      {"log-input", "log-input"},
    };
    for (const auto& [test, format] : testsOk) {
      nmco::AcRule temp;
      acl.curRule = &temp;
      std::string out;
      BOOST_TEST(nmcp::testAttr(test.c_str(), rule, out, blank));
      BOOST_TEST("" == out);
      BOOST_TEST(format == temp.getActions().at(0));
    }
    // BAD
    BOOST_TEST(!nmcp::test("gol", rule, blank, false));
  }
}

/*
BOOST_AUTO_TEST_CASE(testNxos)
{
  {
    const std::string acl {
      "ip access-list TEST\n"
      " 10 permit ip 1.2.3.4 0.0.0.255 eq 123 1.2.3.4 0.0.0.255 eq 123 log\n"
      " 20 permit ip 1.2.3.4/24 eq 123 1.2.3.4 0.0.0.255 eq 123 log\n"
      " 30 permit ip 1.2.3.4 0.0.0.255 eq 123 1.2.3.4/24 eq 123 log\n"
      " 40 permit ip 1.2.3.4/24 eq 123 1.2.3.4/24 eq 123 log\n"
    };

    nmdsic::Result result;
    BOOST_TEST(nmcp::testAttr(acl, nmdsic::Acls(), result, blank));

    auto& ruleName  {result.first};
    BOOST_TEST("TEST" == ruleName);

    auto& ruleBook  {result.second};
    size_t count {1};
    BOOST_TEST(count <= ruleBook.size());
    for (auto& [id, rule] : ruleBook) {
      BOOST_TEST(count == (id));
      ++count;
      BOOST_TEST("permit" == rule.getActions().at(0));
      BOOST_TEST("log" == rule.getActions().at(1));
      BOOST_TEST("ip:123:123" == rule.getServices().at(0));
      BOOST_TEST("1.2.3.4/24" == rule.getSrcs().at(0));
      BOOST_TEST("1.2.3.4/24" == rule.getDsts().at(0));
    }
  }
  {
    const std::string acl {
      "ip access-list TEST\n"
      " 10 permit ip 1.2.3.4 0.0.0.255 eq 123 1.2.3.4 0.0.0.255 eq 123\n"
      " 20 permit ip 1.2.3.4/24 eq 123 1.2.3.4 0.0.0.255 eq 123\n"
      " 30 permit ip 1.2.3.4 0.0.0.255 eq 123 1.2.3.4/24 eq 123\n"
      " 40 permit ip 1.2.3.4/24 eq 123 1.2.3.4/24 eq 123\n"
    };

    nmdsic::Result result;
    BOOST_TEST(nmcp::testAttr(acl, nmdsic::Acls(), result, blank));

    auto& ruleName  {result.first};
    BOOST_TEST("TEST" == ruleName);

    auto& ruleBook  {result.second};
    size_t count {1};
    BOOST_TEST(count <= ruleBook.size());
    for (auto& [id, rule] : ruleBook) {
      BOOST_TEST(count == (id));
      ++count;
      BOOST_TEST("permit" == rule.getActions().at(0));
      BOOST_TEST("ip:123:123" == rule.getServices().at(0));
      BOOST_TEST("1.2.3.4/24" == rule.getSrcs().at(0));
      BOOST_TEST("1.2.3.4/24" == rule.getDsts().at(0));
    }
  }
  {
    const std::string acl {
      "ip access-list TEST\n"
      " 10 permit ip 1.2.3.4 0.0.0.255 eq 123 1.2.3.4 0.0.0.255\n"
      " 20 permit ip 1.2.3.4/24 eq 123 1.2.3.4 0.0.0.255\n"
      " 30 permit ip 1.2.3.4 0.0.0.255 eq 123 1.2.3.4/24\n"
      " 40 permit ip 1.2.3.4/24 eq 123 1.2.3.4/24\n"
    };

    nmdsic::Result result;
    BOOST_TEST(nmcp::testAttr(acl, nmdsic::Acls(), result, blank));

    auto& ruleName  {result.first};
    BOOST_TEST("TEST" == ruleName);

    auto& ruleBook  {result.second};
    size_t count {1};
    BOOST_TEST(count <= ruleBook.size());
    for (auto& [id, rule] : ruleBook) {
      BOOST_TEST(count == (id));
      ++count;
      BOOST_TEST("permit" == rule.getActions().at(0));
      BOOST_TEST("ip:123:" == rule.getServices().at(0));
      BOOST_TEST("1.2.3.4/24" == rule.getSrcs().at(0));
      BOOST_TEST("1.2.3.4/24" == rule.getDsts().at(0));
    }
  }
  {
    const std::string acl {
      "ip access-list TEST\n"
      " 10 permit ip 1.2.3.4 0.0.0.255 1.2.3.4 0.0.0.255 eq 123\n"
      " 20 permit ip 1.2.3.4/24 1.2.3.4 0.0.0.255 eq 123\n"
      " 30 permit ip 1.2.3.4 0.0.0.255 1.2.3.4/24 eq 123\n"
      " 40 permit ip 1.2.3.4/24 1.2.3.4/24 eq 123\n"
    };

    nmdsic::Result result;
    BOOST_TEST(nmcp::testAttr(acl, nmdsic::Acls(), result, blank));

    auto& ruleName  {result.first};
    BOOST_TEST("TEST" == ruleName);

    auto& ruleBook  {result.second};
    size_t count {1};
    BOOST_TEST(count <= ruleBook.size());
    for (auto& [id, rule] : ruleBook) {
      BOOST_TEST(count == (id));
      ++count;
      BOOST_TEST("permit" == rule.getActions().at(0));
      BOOST_TEST("ip::123" == rule.getServices().at(0));
      BOOST_TEST("1.2.3.4/24" == rule.getSrcs().at(0));
      BOOST_TEST("1.2.3.4/24" == rule.getDsts().at(0));
    }
  }
  {
    const std::string acl {
      "ip access-list TEST\n"
      " 10 permit ip 1.2.3.4 0.0.0.255 1.2.3.4 0.0.0.255\n"
      " 20 permit ip 1.2.3.4/24 1.2.3.4 0.0.0.255\n"
      " 30 permit ip 1.2.3.4 0.0.0.255 1.2.3.4/24\n"
      " 40 permit ip 1.2.3.4/24 1.2.3.4/24\n"
    };

    nmdsic::Result result;
    BOOST_TEST(nmcp::testAttr(acl, nmdsic::Acls(), result, blank));

    auto& ruleName  {result.first};
    BOOST_TEST("TEST" == ruleName);

    auto& ruleBook  {result.second};
    size_t count {1};
    BOOST_TEST(count <= ruleBook.size());
    for (auto& [id, rule] : ruleBook) {
      BOOST_TEST(count == (id));
      ++count;
      BOOST_TEST("permit" == rule.getActions().at(0));
      BOOST_TEST("ip::" == rule.getServices().at(0));
      BOOST_TEST("1.2.3.4/24" == rule.getSrcs().at(0));
      BOOST_TEST("1.2.3.4/24" == rule.getDsts().at(0));
    }
  }

  // Given above, should not need exhaustive permutation testing for the rest
  {
    const std::string acl {
      "ip access-list TEST\n"
      " 10 permit ip host 1.2.3.4 eq 123 1.2.3.4 0.0.0.0 eq 123 log\n"
      " 20 permit ip 1.2.3.4 0.0.0.0 eq 123 host 1.2.3.4 eq 123 log\n"
      " 30 permit ip host 1.2.3.4 eq 123 host 1.2.3.4 eq 123 log\n"
    };   

    nmdsic::Result result;
    BOOST_TEST(nmcp::testAttr(acl, nmdsic::Acls(), result, blank));

    auto& ruleName  {result.first};
    BOOST_TEST("TEST" == ruleName);

    auto& ruleBook  {result.second};
    size_t count {1};
    BOOST_TEST(count <= ruleBook.size());
    for (auto& [id, rule] : ruleBook) {
      BOOST_TEST(count == (id));
      ++count;
      BOOST_TEST("permit" == rule.getActions().at(0));
      BOOST_TEST("log" == rule.getActions().at(1));
      BOOST_TEST("ip:123:123" == rule.getServices().at(0));
      BOOST_TEST("1.2.3.4/32" == rule.getSrcs().at(0));
      BOOST_TEST("1.2.3.4/32" == rule.getDsts().at(0));
    }
  }
  {
    const std::string acl {
      "ip access-list TEST\n"
      " 10 permit ip host 1.2.3.4 1.2.3.4 0.0.0.0\n"
      " 20 permit ip 1.2.3.4 0.0.0.0 host 1.2.3.4\n"
      " 30 permit ip host 1.2.3.4 host 1.2.3.4\n"
    };

    nmdsic::Result result;
    BOOST_TEST(nmcp::testAttr(acl, nmdsic::Acls(), result, blank));

    auto& ruleName  {result.first};
    BOOST_TEST("TEST" == ruleName);

    auto& ruleBook  {result.second};
    size_t count {1};
    BOOST_TEST(count <= ruleBook.size());
    for (auto& [id, rule] : ruleBook) {
      BOOST_TEST(count == (id));
      ++count;
      BOOST_TEST("permit" == rule.getActions().at(0));
      BOOST_TEST("ip::" == rule.getServices().at(0));
      BOOST_TEST("1.2.3.4/32" == rule.getSrcs().at(0));
      BOOST_TEST("1.2.3.4/32" == rule.getDsts().at(0));
    }
  }
  {
    const std::string acl {
      "ip access-list TEST\n"
      " 10 permit ip any eq 123 any eq 123 log\n"
    };

    nmdsic::Result result;
    BOOST_TEST(nmcp::testAttr(acl, nmdsic::Acls(), result, blank));

    auto& ruleName  {result.first};
    BOOST_TEST("TEST" == ruleName);

    auto& ruleBook  {result.second};
    size_t count {1};
    BOOST_TEST(count <= ruleBook.size());
    for (auto& [id, rule] : ruleBook) {
      BOOST_TEST(count == (id));
      ++count;
      BOOST_TEST("permit" == rule.getActions().at(0));
      BOOST_TEST("log" == rule.getActions().at(1));
      BOOST_TEST("ip:123:123" == rule.getServices().at(0));
      BOOST_TEST("any" == rule.getSrcs().at(0));
      BOOST_TEST("any" == rule.getDsts().at(0));
    }
  }
  {
    const std::string acl {
      "ip access-list TEST\n"
      " 20 permit ip any any\n"
    };

    nmdsic::Result result;
    BOOST_TEST(nmcp::testAttr(acl, nmdsic::Acls(), result, blank));

    auto& ruleName  {result.first};
    BOOST_TEST("TEST" == ruleName);

    auto& ruleBook  {result.second};
    size_t count {1};
    BOOST_TEST(count <= ruleBook.size());
    for (auto& [id, rule] : ruleBook) {
      BOOST_TEST(count == (id));
      ++count;
      BOOST_TEST("permit" == rule.getActions().at(0));
      BOOST_TEST("ip::" == rule.getServices().at(0));
      BOOST_TEST("any" == rule.getSrcs().at(0));
      BOOST_TEST("any" == rule.getDsts().at(0));
    }
  }
  {
    const std::string acl {
      "ip access-list TEST\n"
      " 20 permit ip any any log tracked\n"
    };

    nmdsic::Result result;
    BOOST_TEST(nmcp::testAttr(acl, nmdsic::Acls(), result, blank));

    auto& ruleName  {result.first};
    BOOST_TEST("TEST" == ruleName);

    auto& ruleBook  {result.second};
    size_t count {1};
    BOOST_TEST(count <= ruleBook.size());
    for (auto& [id, rule] : ruleBook) {
      BOOST_TEST(count == (id));
      ++count;
      BOOST_TEST("permit" == rule.getActions().at(0));
      BOOST_TEST("log" == rule.getActions().at(1));
      BOOST_TEST("tracked" == rule.getActions().at(2));
      BOOST_TEST("ip::" == rule.getServices().at(0));
      BOOST_TEST("any" == rule.getSrcs().at(0));
      BOOST_TEST("any" == rule.getDsts().at(0));
    }
  }
}
*/

//=============================================================================
// IOS tests
//=============================================================================
/*
More info:
- https://www.cisco.com/c/en/us/support/docs/security/ios-firewall/23602-confaccesslists.html#standacl
  - Not a great example, CLI to enter...not show format
  - Allows for both one and multi line show format (depending on usage)

KEY: (unless otherwise stated)
  LIST        -- Access list name
  ACTION      -- ( deny | permit )
  DYNAMIC_ARG -- ( dynamic NAME [timeout MINUTES] )
  ADDR_ARG    -- (  host IP
                  | IP WILDMASK
                  | IPv6/PREFIX
                  | any
                 )
  PROTO_ARG   -- ( NAME | NUMBER )
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
   ip access-list standard LIST
    ACTION ADDR_ARG 
---
  ADDR_ARG -- ( IP | IP WILDMASK | any )
*/
BOOST_AUTO_TEST_CASE(testIosStandardRuleLine)
{
  {
    nmdsic::Acls acl;
    const auto& rule = acl.iosExtendedRuleLine;
  
    const std::string full { 
      "permit ip 1.2.3.4 0.0.0.255 1.2.3.4 0.0.0.255\n"
    };
    BOOST_TEST(nmcp::test(full.c_str(), rule, blank));

    const std::vector<std::tuple<std::string, std::string>> replaces {
      {"1.2.3.4 0.0.0.255", "host 1.2.3.4"},
      {"1.2.3.4 0.0.0.255", "host 1.2.3.4"},
      {"1.2.3.4 0.0.0.255", "1.2.3.4/24"},
      {"1.2.3.4 0.0.0.255", "1.2.3.4/24"},
      {"1.2.3.4 0.0.0.255", "any"},
      {"1.2.3.4 0.0.0.255", "any"},
    };
    for (const auto& [toReplace, replace] : replaces) {
      std::string testReplace = full;
      auto locReplace {testReplace.find(toReplace)};
      if (locReplace == testReplace.npos) { continue; }
      testReplace =
        testReplace.replace(locReplace, toReplace.size(), replace);
      BOOST_TEST(nmcp::test(testReplace.c_str(), rule, blank));
    }
    for (const auto& [toReplace, replace] : replaces) {
      std::string testReplace = full;
      auto locReplace {testReplace.rfind(toReplace)};
      if (locReplace == testReplace.npos) { continue; }
      testReplace =
        testReplace.replace(locReplace, toReplace.size(), replace);
      BOOST_TEST(nmcp::test(testReplace.c_str(), rule, blank));
    }
  }
}
BOOST_AUTO_TEST_CASE(testIosStandard)
{
  {
    nmdsic::Acls acl;
    const auto& rule = acl.iosStandard;
  
    const std::string full { 
      "access-list TEST permit any\n"
    };

    BOOST_TEST(nmcp::test(full.c_str(), rule, blank));
  }
  {
    nmdsic::Acls acl;
    const auto& rule = acl.iosStandard;
  
    const std::string full { 
      "ip access-list standard TEST\n"
      " remark crazy rule\n"
      " permit any\n"
      " remark crazy rule\n"
      " deny   any\n"
    };

    BOOST_TEST(nmcp::test(full.c_str(), rule, blank));

    nmdsic::Result result;
    BOOST_TEST(nmcp::testAttr(full, nmdsic::Acls(), result, blank));

    auto& ruleName  {result.first};
    BOOST_TEST("TEST" == ruleName);

    auto& ruleBook  {result.second};
    BOOST_TEST(2 == ruleBook.size());
    BOOST_TEST("permit" == ruleBook.at(1).getActions().at(0));
    BOOST_TEST("deny" == ruleBook.at(2).getActions().at(0));
    size_t count {1};
    BOOST_TEST(count <= ruleBook.size());
    for (auto& [id, rule] : ruleBook) {
      BOOST_TEST(count == (id));
      ++count;
      BOOST_TEST("any" == rule.getSrcs().at(0));
      BOOST_TEST(0 == rule.getDsts().size());
      BOOST_TEST(0 == rule.getServices().size());
    }
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
   ip access-list extended LIST
    ACTION PROTO_ARG \
    ADDR_ARG ADDR_ARG [ICMP_ARG] \
    [precedence PRECEDENCE] [tos TOS] [LOG] [TIME]

   ip access-list extended LIST
    ACTION PROTO_ARG \
    ADDR_ARG [PORT_ARG] ADDR_ARG [PORT_ARG] \
    [established] [precedence PRECEDENCE] [tos TOS] [LOG] [TIME]
---
  PRECEDENCE -- 0-7 or name
  TOS        -- 0-15 or name
*/
BOOST_AUTO_TEST_CASE(testIosExtendedRuleLine)
{
  {
    nmdsic::Acls acl;
    const auto& rule = acl.iosExtendedRuleLine;
  
    const std::string full { 
      "permit ip"
      " 1.2.3.4 0.0.0.255 range 123 456"
      " 1.2.3.4 0.0.0.255 range 123 456"
      " established precedence 7 tos 15"
      " log time-range RANGE_NAME\n"
    };
    BOOST_TEST(nmcp::test(full.c_str(), rule, blank));

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
    std::string testRemoval = full;
    for (const auto& removal : removals) {
      auto locRemoval {testRemoval.find(removal)};
      BOOST_TEST(locRemoval != testRemoval.npos);
      testRemoval = testRemoval.erase(locRemoval, removal.size());
      BOOST_TEST(nmcp::test(testRemoval.c_str(), rule, blank));
      std::string testReplace = testRemoval;
      for (const auto& [toReplace, replace] : replaces) {
        auto locReplace {testReplace.find(toReplace)};
        if (locReplace == testReplace.npos) { continue; }
        testReplace =
          testReplace.replace(locReplace, toReplace.size(), replace);
        BOOST_TEST(nmcp::test(testReplace.c_str(), rule, blank));
      }
    }
    testRemoval = full;
    for (const auto& removal : removals) {
      auto locRemoval {testRemoval.rfind(removal)};
      BOOST_TEST(locRemoval != testRemoval.npos);
      testRemoval = testRemoval.erase(locRemoval, removal.size());
      BOOST_TEST(nmcp::test(testRemoval.c_str(), rule, blank));
      std::string testReplace = testRemoval;
      for (const auto& [toReplace, replace] : replaces) {
        auto locReplace {testReplace.rfind(toReplace)};
        if (locReplace == testReplace.npos) { continue; }
        testReplace =
          testReplace.replace(locReplace, toReplace.size(), replace);
        BOOST_TEST(nmcp::test(testReplace.c_str(), rule, blank));
      }
    }
  }
}
BOOST_AUTO_TEST_CASE(testIosExtended)
{
  {
    nmdsic::Acls acl;
    const auto& rule = acl.iosExtended;
  
    const std::string full { 
      "access-list TEST dynamic NAME timeout 5 permit ip any any\n"
    };
    BOOST_TEST(nmcp::test(full.c_str(), rule, blank));

    const std::vector<std::string> removals {
      {" timeout 5"},
      {" dynamic NAME"},
    };
    std::string testRemoval = full;
    for (const auto& removal : removals) {
      auto locRemoval {testRemoval.find(removal)};
      BOOST_TEST(locRemoval != testRemoval.npos);
      testRemoval = testRemoval.erase(locRemoval, removal.size());
      BOOST_TEST(nmcp::test(testRemoval.c_str(), rule, blank));
    }
  }
  {
    nmdsic::Acls acl;
    const auto& rule = acl.iosExtended;
  
    const std::string full { 
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
    std::string test = full;
    for (const auto& removal : removals) {
      test = test.erase(test.find(removal), removal.size());
      BOOST_TEST(nmcp::test(test.c_str(), rule, blank));
    }

    nmdsic::Result result;
    BOOST_TEST(nmcp::testAttr(full, nmdsic::Acls(), result, blank));

    auto& ruleName  {result.first};
    BOOST_TEST("TEST" == ruleName);

    auto& ruleBook  {result.second};
    BOOST_TEST(2 == ruleBook.size());
    BOOST_TEST("permit" == ruleBook.at(1).getActions().at(0));
    BOOST_TEST("deny" == ruleBook.at(2).getActions().at(0));
    size_t count {1};
    BOOST_TEST(count <= ruleBook.size());
    for (auto& [id, rule] : ruleBook) {
      BOOST_TEST(count == (id));
      ++count;
      BOOST_TEST("any" == rule.getSrcs().at(0));
      BOOST_TEST("any" == rule.getDsts().at(0));
      BOOST_TEST("ip::" == rule.getServices().at(0));
    }
  }
}

/* NOTE: Below is a multi-liner
   ip access-list ( standard | extended ) LIST
    remark TEXT

   NOTE: Below is a one-liner wrapped for clarity
   access-list LIST remark TEXT
*/
BOOST_AUTO_TEST_CASE(testIosRemarkRuleLine)
{
  {
    nmdsic::Acls acl;
    const auto& rule = acl.iosRemarkRuleLine;
    // OK
    BOOST_TEST(nmcp::test("remark some\n", rule, blank));
    BOOST_TEST(nmcp::test("remark s r t\n", rule, blank));
    //// BAD
    BOOST_CHECK_THROW(nmcp::test("remark \n", rule, blank, false),
                      std::runtime_error);
    BOOST_CHECK_THROW(nmcp::test("remark\n", rule, blank, false),
                      std::runtime_error);
    BOOST_TEST(!nmcp::test("\n", rule, blank, false));
    BOOST_TEST(!nmcp::test("", rule, blank, false));
  }
}
BOOST_AUTO_TEST_CASE(testIosRemark)
{
  {
    const std::string acl {
      "access-list TEST remark crazy rules\n"
    };

    nmdsic::Result result;
    BOOST_TEST(nmcp::testAttr(acl, nmdsic::Acls(), result, blank));

    auto& ruleName  {result.first};
    BOOST_TEST("TEST" == ruleName);

    auto& ruleBook  {result.second};
    BOOST_TEST(0 == ruleBook.size());
  }
}


#if false

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
  ADDR_ARG    -- (  host IP
                  | IP NETMASK
                  | IPv6/PREFIX
                  | any[46]
                  | interface IFACE_NAME
                  | object NETWORK_OBJECT_ID
                  | object-group NETWORK_GROUP_ID
                 )
  PROTO_ARG   -- (  ( NAME | NUMBER )
                  | object SERVICE_OBJECT_ID
                  | object-group ( PROTOCOL_GROUP_ID | SERVICE_GROUP_ID )
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
   access-list LIST remark SPACED_FILLED_TEXT
*/
BOOST_AUTO_TEST_CASE(testAsaRemark)
{
}
/* NOTE: Below is a one-liner wrapped for clarity
   access-list LIST standard ACTION ADDR_ARG
   ---
   ADDR_ARG -- ( host IP | IP MASK | any4)
*/
BOOST_AUTO_TEST_CASE(testAsaStandard)
{
}
/* NOTE: Below is a one-liner wrapped for clarity
   access-list LIST extended ACTION
    PROTO_ARG
    ADDR_ARG ADDR_ARG
    [LOG] [TIME] [inactive]
   access-list LIST extended ACTION
    {tcp | udp | sctp}
    ADDR_ARG [PORT_ARG] ADDR_ARG [PORT_ARG]
    [LOG] [TIME] [inactive]
   access-list LIST extended ACTION
    {icmp | icmp6}
    ADDR_ARG ADDR_ARG
    [ICMP_ARG]
    [LOG] [TIME] [inactive]
   access-list LIST extended ACTION
    PROTO_ARG [USER_ARG]
    ADDR_ARG [PORT_ARG] ADDR_ARG [PORT_ARG]
    [LOG] [TIME] [inactive]
   access-list LIST extended ACTION
    PROTO_ARG
    [SEC_GRP_ARG] ADDR_ARG [PORT_ARG]
    [SEC_GRP_ARG] ADDR_ARG [PORT_ARG]
    [LOG] [TIME] [inactive]
*/
BOOST_AUTO_TEST_CASE(testAsaExtended)
{
}
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
    const auto& rule = acl.asaEther;
    // OK
    BOOST_TEST(nmcp::test("ethertype permit any", rule, blank));
    BOOST_TEST(nmcp::test("ethertype deny any", rule, blank));
    // Cisco note: no longer matches as intended, use dsap instead
    BOOST_TEST(nmcp::test("ethertype permit bpdu", rule, blank));
    BOOST_TEST(nmcp::test("ethertype permit dsap 0x42", rule, blank));
    BOOST_TEST(nmcp::test("ethertype permit dsap 0x01", rule, blank));
    BOOST_TEST(nmcp::test("ethertype permit dsap 0xff", rule, blank));
    BOOST_TEST(nmcp::test("ethertype permit ipx", rule, blank));
    BOOST_TEST(nmcp::test("ethertype permit isis", rule, blank));
    BOOST_TEST(nmcp::test("ethertype permit mpls-multicast", rule, blank));
    BOOST_TEST(nmcp::test("ethertype permit mpls-unicast", rule, blank));
    BOOST_TEST(nmcp::test("ethertype permit 0x600", rule, blank));
    BOOST_TEST(nmcp::test("ethertype permit 0xffff", rule, blank));
    // BAD
    BOOST_TEST(!nmcp::test("ethertype permit", rule, blank));
    BOOST_TEST(!nmcp::test("ethertype permit asdf", rule, blank));
  }

  // Full trip
  {
    const std::string acl {
      "access-list TEST ethertype permit any\n"
      "access-list TEST ethertype deny any\n"
    };

    nmdsic::Result result;
    BOOST_TEST(nmcp::testAttr(acl, nmdsic::Acls(), result, blank));

    auto& ruleName  {result.first};
    BOOST_TEST("" == ruleName);

    auto& ruleBook  {result.second};
    BOOST_TEST(0 == ruleBook.size());
  }
}
#endif
