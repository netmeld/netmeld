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
  nmco::AcRule temp;
  acl.curRule = &temp;

  {
    const auto& rule = acl.action;
    // OK
    BOOST_TEST(nmcp::test("permit", rule, blank));
    BOOST_TEST(nmcp::test("deny", rule, blank));
    BOOST_TEST(nmcp::test(" permit ", rule, blank));
    BOOST_TEST("permit" == temp.getActions().at(0));
    // BAD
    BOOST_TEST(!nmcp::test("other", rule, blank));
  }

  // TODO protocolArgument

  {
    const auto& rule = acl.addressArgumentIos;
    // OK
    BOOST_TEST(nmcp::test("host 1.2.3.4", rule, blank));
    BOOST_TEST(nmcp::test("host 1234::aBcD", rule, blank));
    BOOST_TEST(nmcp::test("any", rule, blank));
    BOOST_TEST(nmcp::test("1.2.3.4 0.0.0.255", rule, blank));
    BOOST_TEST(nmcp::test("1.2.3.4/24", rule, blank));
    BOOST_TEST(nmcp::test("1234::aBcD/112", rule, blank));
    // BAD
    BOOST_TEST(!nmcp::test("host 1.2.3.4/24", rule, blank));
    BOOST_TEST(!nmcp::test("host 1234::aBcD/112", rule, blank));
    BOOST_TEST(!nmcp::test("any4", rule, blank));
    BOOST_TEST(!nmcp::test("any6", rule, blank));
    BOOST_TEST(!nmcp::test("1.2.3.4/24 1.2.3.4/24", rule, blank));
    BOOST_TEST(!nmcp::test("1234::aBcD/112 1234::aBcD/112", rule, blank));
  }
  {
    const auto& rule = acl.ipLikeNoCidr;
    // OK
    BOOST_TEST(nmcp::test("1.2.3.4", rule));
    BOOST_TEST(nmcp::test("1234::aBcD", rule));
    BOOST_TEST(nmcp::test("1.2.3.4 ", rule, false));
    BOOST_TEST(nmcp::test("1234::aBcD ", rule, false));
    // BAD
    BOOST_TEST(!nmcp::test("1.2.3.4/24", rule));
    BOOST_TEST(!nmcp::test("1.2.3.4/", rule));
    BOOST_TEST(!nmcp::test("1234::aBcD/112", rule));
    BOOST_TEST(!nmcp::test("1234::aBcD/", rule));
    // FORMAT
    std::string out;
    BOOST_TEST(nmcp::testAttr("1.2.3.4", rule, out));
    BOOST_TEST("1.2.3.4" == out);
    out.clear();
    BOOST_TEST(nmcp::testAttr("1234::aBcD ", rule, out, false));
    BOOST_TEST("1234::aBcD" == out);
  }

  {
    const auto& rule = acl.portArgument;
    // OK
    BOOST_TEST(nmcp::test("eq 123", rule, blank));
    BOOST_TEST(nmcp::test("eq http", rule, blank));
    BOOST_TEST(nmcp::test("neq 123", rule, blank));
    BOOST_TEST(nmcp::test("neq http", rule, blank));
    BOOST_TEST(nmcp::test("lt 123", rule, blank));
    BOOST_TEST(nmcp::test("lt http", rule, blank));
    BOOST_TEST(nmcp::test("gt 123", rule, blank));
    BOOST_TEST(nmcp::test("gt http", rule, blank));
    BOOST_TEST(nmcp::test("range 123 456", rule, blank));
    // BAD
    // FORMAT
    std::string out;
    BOOST_TEST(nmcp::testAttr("eq 123", rule, out, blank));
    BOOST_TEST("123" == out);
    out.clear();
    BOOST_TEST(nmcp::testAttr("neq 123", rule, out, blank));
    BOOST_TEST("!123" == out);
    out.clear();
    BOOST_TEST(nmcp::testAttr("lt 123", rule, out, blank));
    BOOST_TEST("<123" == out);
    out.clear();
    BOOST_TEST(nmcp::testAttr("gt 123", rule, out, blank));
    BOOST_TEST(">123" == out);
    out.clear();
    BOOST_TEST(nmcp::testAttr("range 123 456", rule, out, blank));
    BOOST_TEST("123-456" == out);
    out.clear();
    BOOST_TEST(nmcp::testAttr("eq http", rule, out, blank));
    BOOST_TEST("http" == out);
    out.clear();
    BOOST_TEST(nmcp::testAttr("neq http", rule, out, blank));
    BOOST_TEST("!http" == out);
    out.clear();
    BOOST_TEST(nmcp::testAttr("lt http", rule, out, blank));
    BOOST_TEST("<http" == out);
    out.clear();
    BOOST_TEST(nmcp::testAttr("gt http", rule, out, blank));
    BOOST_TEST(">http" == out);
    out.clear();
    BOOST_TEST(nmcp::testAttr("range http https", rule, out, blank));
    BOOST_TEST("http-https" == out);
    out.clear();
  }

  // TODO logArgument

  {
    const auto& rule = acl.addressArgument;
    // OK
    BOOST_TEST(nmcp::test("host 1.2.3.4", rule, blank));
    BOOST_TEST(nmcp::test("host 1234::aBcD", rule, blank));
    BOOST_TEST(nmcp::test("object-group group", rule, blank));
    BOOST_TEST(nmcp::test("object object", rule, blank));
    BOOST_TEST(nmcp::test("interface iface", rule, blank));
    BOOST_TEST(nmcp::test("any", rule, blank));
    BOOST_TEST(nmcp::test("any4", rule, blank));
    BOOST_TEST(nmcp::test("any6", rule, blank));
    BOOST_TEST(nmcp::test("1.2.3.4 0.0.0.255", rule, blank));
    BOOST_TEST(nmcp::test("1.2.3.4/24", rule, blank));
    BOOST_TEST(nmcp::test("1234::aBcD/112", rule, blank));
    // BAD
    BOOST_TEST(!nmcp::test("1.2.3.4/24 1.2.3.4/24", rule, blank));
    BOOST_TEST(!nmcp::test("1234::aBcD/112 1234::aBcD/112", rule, blank));
    BOOST_TEST(!nmcp::test("host 1.2.3.4/24", rule, blank));
    BOOST_TEST(!nmcp::test("host 1234::aBcD/112", rule, blank));
  }
}

/*
BOOST_AUTO_TEST_CASE(testIosExtended)
{
  {
    const std::string acl {
      "ip access-list extended TEST\n"
      " permit ip 1.2.3.4 0.0.0.255 eq 123 1.2.3.4 0.0.0.255 eq 123 log\n"
      " permit ip 1.2.3.4/24 eq 123 1.2.3.4 0.0.0.255 eq 123 log\n"
      " permit ip 1.2.3.4 0.0.0.255 eq 123 1.2.3.4/24 eq 123 log\n"
      " permit ip 1.2.3.4/24 eq 123 1.2.3.4/24 eq 123 log\n"
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
      "ip access-list extended TEST\n"
      " permit ip 1.2.3.4 0.0.0.255 eq 123 1.2.3.4 0.0.0.255 eq 123\n"
      " permit ip 1.2.3.4/24 eq 123 1.2.3.4 0.0.0.255 eq 123\n"
      " permit ip 1.2.3.4 0.0.0.255 eq 123 1.2.3.4/24 eq 123\n"
      " permit ip 1.2.3.4/24 eq 123 1.2.3.4/24 eq 123\n"
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
      "ip access-list extended TEST\n"
      " permit ip 1.2.3.4 0.0.0.255 eq 123 1.2.3.4 0.0.0.255\n"
      " permit ip 1.2.3.4/24 eq 123 1.2.3.4 0.0.0.255\n"
      " permit ip 1.2.3.4 0.0.0.255 eq 123 1.2.3.4/24\n"
      " permit ip 1.2.3.4/24 eq 123 1.2.3.4/24\n"
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
      "ip access-list extended TEST\n"
      " permit ip 1.2.3.4 0.0.0.255 1.2.3.4 0.0.0.255 eq 123\n"
      " permit ip 1.2.3.4/24 1.2.3.4 0.0.0.255 eq 123\n"
      " permit ip 1.2.3.4 0.0.0.255 1.2.3.4/24 eq 123\n"
      " permit ip 1.2.3.4/24 1.2.3.4/24 eq 123\n"
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
      "ip access-list extended TEST\n"
      " permit ip 1.2.3.4 0.0.0.255 1.2.3.4 0.0.0.255\n"
      " permit ip 1.2.3.4/24 1.2.3.4 0.0.0.255\n"
      " permit ip 1.2.3.4 0.0.0.255 1.2.3.4/24\n"
      " permit ip 1.2.3.4/24 1.2.3.4/24\n"
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
      "ip access-list extended TEST\n"
      " permit ip host 1.2.3.4 eq 123 1.2.3.4 0.0.0.0 eq 123 log\n"
      " permit ip 1.2.3.4 0.0.0.0 eq 123 host 1.2.3.4 eq 123 log\n"
      " permit ip host 1.2.3.4 eq 123 host 1.2.3.4 eq 123 log\n"
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
      BOOST_TEST("1.2.3.4/32" == rule.getSrcs().at(0));
      BOOST_TEST("1.2.3.4/32" == rule.getDsts().at(0));
    }
  }
  {
    const std::string acl {
      "ip access-list extended TEST\n"
      " permit ip host 1.2.3.4 1.2.3.4 0.0.0.0\n"
      " permit ip 1.2.3.4 0.0.0.0 host 1.2.3.4\n"
      " permit ip host 1.2.3.4 host 1.2.3.4\n"
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
      "ip access-list extended TEST\n"
      " permit ip any eq 123 any eq 123 log\n"
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
      BOOST_TEST("any" == rule.getSrcs().at(0));
      BOOST_TEST("any" == rule.getDsts().at(0));
    }
  }
  {
    const std::string acl {
      "ip access-list extended TEST\n"
      " permit ip any any\n"
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
}

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

/* NOTE: Below is a multi-liner, as defined in spec
   ip access-list standard LIST
    ACTION ADDR_ARG 

   NOTE: Below is a one-liner wrapped for clarity
   access-list LIST ACTION ADDR_ARG

   ---
   ADDR_ARG -- (  IP
                | IP WILDMASK
                | any
               )
*/
BOOST_AUTO_TEST_CASE(testIosStandard)
{
  {
    nmdsic::Acls acl;
    const auto& rule = acl.iosStandard;
    // OK
    const std::string prefix {"access-list TEST permit"};
    std::string test;

    test = prefix + " 1.2.3.4 0.0.0.255\n";
    BOOST_TEST(nmcp::test(test.c_str(), rule, blank));
    test = prefix + " 1.2.3.4\n";
    BOOST_TEST(nmcp::test(test.c_str(), rule, blank));
    test = prefix + " 1234::aBcD\n";
    BOOST_TEST(nmcp::test(test.c_str(), rule, blank));
    test = prefix + " any\n";
    BOOST_TEST(nmcp::test(test.c_str(), rule, blank));
    // BAD
    BOOST_TEST(!nmcp::test(prefix.c_str(), rule, blank));
    test = "access-list standard TEST permit any\n";
    BOOST_TEST(!nmcp::test(test.c_str(), rule, blank, false));
  }

  // Full trip
  {
    const std::string acl {
      "ip access-list standard TEST\n"
      " permit 1.2.3.4 0.0.0.255\n"
      " permit 1.2.3.4\n"
      " permit 1234::aBcD\n"
      " deny   any\n"
    };

    nmdsic::Result result;
    BOOST_TEST(nmcp::testAttr(acl, nmdsic::Acls(), result, blank, false));

    auto& ruleName  {result.first};
    BOOST_TEST("TEST" == ruleName);

    // TODO update

    auto& ruleBook  {result.second};
    size_t count {1};
    BOOST_TEST(count <= ruleBook.size());
    for (auto& [id, rule] : ruleBook) {
      BOOST_TEST(count == (id));
      ++count;
      BOOST_TEST("permit" == rule.getActions().at(0));
      BOOST_TEST(0 == rule.getServices().size());
      BOOST_TEST(0 == rule.getSrcs().size());
      BOOST_TEST(0 == rule.getDsts().size());
    }
  }
}

/* NOTE: Below is a multi-liner
   ip access-list ( standard | extended ) LIST
    remark TEXT

   NOTE: Below is a one-liner wrapped for clarity
   access-list LIST remark TEXT
*/
BOOST_AUTO_TEST_CASE(testIosRemark)
{
  {
    nmdsic::Acls acl;
    const auto& rule = acl.iosRemark;
    // OK
    BOOST_TEST(nmcp::test("access-list TEST remark some\n", rule, blank));
    BOOST_TEST(nmcp::test("access-list TEST remark s r t\n", rule, blank));
    //// BAD
    BOOST_CHECK_THROW(
          nmcp::test("access-list TEST remark \n", rule, blank, false),
          std::runtime_error
        );
    BOOST_CHECK_THROW(
          nmcp::test("access-list TEST remark\n", rule, blank, false),
          std::runtime_error
        );
    BOOST_TEST(!nmcp::test("\n", rule, blank, false));
    BOOST_TEST(!nmcp::test("", rule, blank, false));
  }

  // Full trip
  {
    const std::string acl {
      "ip access-list standard TEST\n"
      " remark some random text\n"
      " permit any\n"
    };

    nmdsic::Result result;
    BOOST_TEST(nmcp::testAttr(acl, nmdsic::Acls(), result, blank));

    auto& ruleName  {result.first};
    BOOST_TEST("TEST" == ruleName);

    auto& ruleBook  {result.second};
    BOOST_TEST(2 == ruleBook.size());

    // TODO update
  }
//  {
//    const std::string acl {
//      "ip access-list extended TEST\n"
//      " remark some random text\n"
//      " permit ip any any\n"
//    };
//
//    nmdsic::Result result;
//    BOOST_TEST(nmcp::testAttr(acl, nmdsic::Acls(), result, blank));
//
//    auto& ruleName  {result.first};
//    BOOST_TEST("TEST" == ruleName);
//
//    auto& ruleBook  {result.second};
//    size_t count {1};
//    BOOST_TEST(count <= ruleBook.size());
//    for (auto& [id, rule] : ruleBook) {
//      BOOST_TEST(count == (id));
//      ++count;
//      BOOST_TEST("permit" == rule.getActions().at(0));
//      BOOST_TEST("ip::" == rule.getServices().at(0));
//      BOOST_TEST("any" == rule.getSrcs().at(0));
//      BOOST_TEST("any" == rule.getDsts().at(0));
//    }
//  }
}

/* NOTE: Below is a multi-liner, as defined in spec
   ip access-list extended LIST

   NOTE: Below is a one-liner wrapped for clarity
   access-list LIST [DYNAMIC_ARG]
    ACTION PROTO_ARG
    ADDR_ARG ADDR_ARG
    [precedence PRECEDENCE] [tos TOS] [LOG] [TIME]

   access-list LIST [DYNAMIC_ARG]
    ACTION PROTO_ARG
    ADDR_ARG ADDR_ARG
    [ICMP_ARG]
    [precedence PRECEDENCE] [tos TOS] [LOG] [TIME]

   access-list LIST [DYNAMIC_ARG]
    ACTION PROTO_ARG
    ADDR_ARG [PORT_ARG] ADDR_ARG [PORT_ARG]
    [established] [precedence PRECEDENCE] [tos TOS] [LOG] [TIME]

   access-list LIST [DYNAMIC_ARG]
    ACTION PROTO_ARG
    ADDR_ARG [PORT_ARG] ADDR_ARG [PORT_ARG]
    [precedence PRECEDENCE] [tos TOS] [LOG] [TIME]
*/
#if false
BOOST_AUTO_TEST_CASE(testIosExtended)
{
}
/*
*/
BOOST_AUTO_TEST_CASE(testIos)
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
