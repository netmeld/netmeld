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
  nmdsic::Acls acl;

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
    const auto& rule = acl.ports;
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

BOOST_AUTO_TEST_CASE(testIosStandard)
{
  {
    const std::string acl {
      "ip access-list standard TEST\n"
      " permit 1.2.3.4 0.0.0.255\n"
    };

    nmdsic::Result result;
    BOOST_TEST(nmcp::testAttr(acl, nmdsic::Acls(), result, blank, false));

    auto& ruleName  {result.first};
    BOOST_TEST("" == ruleName);

    auto& ruleBook  {result.second};
    BOOST_TEST(0 == ruleBook.size());
  }
}

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

/*
  NOTE: These are single lines.
    More info: https://www.cisco.com/c/en/us/td/docs/security/asa/asa96/configuration/firewall/asa-96-firewall-config/access-acls.html

  Common format?
  - access-list NAME [line NUM] extended ACTION PROTO_ARG SRC_ARG DST_ARG [log [[LVL] [interval SEC] | disable | default]] [time-range RANGE_NAME] [inactive]

  Specific ones...
  - access-list access_list_name [line line_number] extended {deny | permit} protocol_argument source_address_argument dest_address_argument [log [[level] [interval secs] | disable | default]] [time-range time_range_name] [inactive]
  - access-list access_list_name [line line_number] extended {deny | permit} {tcp | udp | sctp} source_address_argument [port_argument] dest_address_argument [port_argument] [log [[level] [interval secs] | disable | default] [time-range time-range-name] [inactive]
  - access-list access_list_name [line line_number] extended {deny | permit} {icmp | icmp6} source_address_argument dest_address_argument [icmp_argument] [log [[level] [interval secs] | disable | default]] [time-range time_range_name] [inactive]
  - access-list access_list_name [line line_number] extended {deny | permit} protocol_argument [user_argument] source_address_argument [port_argument] dest_address_argument [port_argument] [log [[level] [interval secs] | disable | default]] [time-range time_range_name] [inactive]
  - access-list access_list_name [line line_number] extended {deny | permit} protocol_argument [security_group_argument] source_address_argument [port_argument] [security_group_argument] dest_address_argument [port_argument] [log [[level] [interval secs] | disable | default]] [inactive | time-range time_range_name]
  - access-list access_list_name standard {deny | permit} {any4 | host ip_address | ip_address mask }
  - access-list access_list_name webtype {deny | permit} url {url_string | any} [log [[level] [interval secs] | disable | default]] [time_range time_range_name] [inactive]
  - access-list access_list_name webtype {deny | permit } tcp dest_address_argument [operator port] [log [[level] [interval secs] | disable | default]] [time_range time_range_name]] [inactive]]
  - access-list access_list_name ethertype {deny | permit} {any | bpdu | dsap hex_address | ipx | isis | mpls-multicast | mpls-unicast | hex_number}
  ---
  NAME
    ACL name
  ACTION
    ( deny | permit )
  PROTO_ARG
    ( NAME | NUMBER | object-group ( PROTO_GROUP | SRVC_GROUP ) | object SRVC_OBJ )
  (SRC|DST)_ARG
    ( host IP | IP MASK | IP/CIDR | any[4|6] | interface IFACE | object NW_OBJ | object-group NW_OBJ_GRP )
  LVL
    0-7

  

  port_argument ( OPERATOR PORT )
    
*/
BOOST_AUTO_TEST_CASE(testAsa)
{
}
