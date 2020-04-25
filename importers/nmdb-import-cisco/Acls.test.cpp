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

#include <netmeld/core/objects/AcRule.hpp>
#include <netmeld/core/objects/IpAddress.hpp>
#include <netmeld/core/parsers/ParserHelper.hpp>
#include <netmeld/core/parsers/ParserIpAddress.hpp>

#include "Acls.hpp"

namespace nmco = netmeld::core::objects;
namespace nmcp = netmeld::core::parsers;
namespace nmdsic = netmeld::datastore::importers::cisco;


bool parse(nmdsic::Result&, const std::string&);
bool parse(nmdsic::Result& result, const std::string& text)
{
  std::istringstream dataStream(text);
  dataStream.unsetf(std::ios::skipws);
  nmcp::IstreamIter i(dataStream), e;
  auto success =
    qi::phrase_parse(i, e, nmdsic::Acls(), qi::ascii::blank, result);

  //if ((!success) || (i != e)) {
  //  LOG_INFO << "Parser failed around:\n";
  //  std::ostringstream oss;
  //  for (size_t count {0}; (count < 20) && (i != e); ++count, ++i) {
  //    oss << *i;
  //  }
  //  LOG_INFO << oss.str() << std::endl;
  //}
  return success;
}


BOOST_AUTO_TEST_CASE(testAclStandard)
{
//  nmcu::LoggerSingleton::getInstance().setLevel(nmcu::Severity::ALL);
  const std::string acl {
    "ip access-list standard TEST\n"
    " permit 1.2.3.4 0.0.0.255\n"
  };

  nmdsic::Result result;
  BOOST_TEST(parse(result, acl));

  auto& ruleName  {result.first};
  //BOOST_TEST_EQUAL("TEST", ruleName);
  BOOST_TEST("" == ruleName);

  auto& ruleBook  {result.second};
  //BOOST_TEST(1 == ruleBook.size());
  BOOST_TEST(0 == ruleBook.size());

//  auto& rule  {ruleBook.at(0)};
//  BOOST_TEST_EQUAL("permit", rule.getActions().at(0));
//  BOOST_TEST_EQUAL("ip", rule.getServices().at(0));
//  BOOST_TEST_EQUAL("any", rule.getSrcs().at(0));
//  BOOST_TEST_EQUAL("10.52.0.0/16", rule.getDsts().at(0));
}

BOOST_AUTO_TEST_CASE(testAclExtended)
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
    BOOST_TEST(parse(result, acl));

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
    BOOST_TEST(parse(result, acl));

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
    BOOST_TEST(parse(result, acl));

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
    BOOST_TEST(parse(result, acl));

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
    BOOST_TEST(parse(result, acl));

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
    BOOST_TEST(parse(result, acl));

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
    BOOST_TEST(parse(result, acl));

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
    BOOST_TEST(parse(result, acl));

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
    BOOST_TEST(parse(result, acl));

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

BOOST_AUTO_TEST_CASE(testAcl)
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
    BOOST_TEST(parse(result, acl));

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
      " 10 permit ip 1.2.3.4 0.0.0.255 eq 123 1.2.3.4 0.0.0.255 eq 123\n"
      " 20 permit ip 1.2.3.4/24 eq 123 1.2.3.4 0.0.0.255 eq 123\n"
      " 30 permit ip 1.2.3.4 0.0.0.255 eq 123 1.2.3.4/24 eq 123\n"
      " 40 permit ip 1.2.3.4/24 eq 123 1.2.3.4/24 eq 123\n"
    };

    nmdsic::Result result;
    BOOST_TEST(parse(result, acl));

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
    BOOST_TEST(parse(result, acl));

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
    BOOST_TEST(parse(result, acl));

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
    BOOST_TEST(parse(result, acl));

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
    BOOST_TEST(parse(result, acl));

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
      "ip access-list TEST\n"
      " 10 permit ip host 1.2.3.4 1.2.3.4 0.0.0.0\n"
      " 20 permit ip 1.2.3.4 0.0.0.0 host 1.2.3.4\n"
      " 30 permit ip host 1.2.3.4 host 1.2.3.4\n"
    };

    nmdsic::Result result;
    BOOST_TEST(parse(result, acl));

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
    BOOST_TEST(parse(result, acl));

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
      "ip access-list TEST\n"
      " 20 permit ip any any\n"
    };

    nmdsic::Result result;
    BOOST_TEST(parse(result, acl));

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
