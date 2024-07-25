// =============================================================================
// Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC
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

#include <netmeld/datastore/parsers/ParserTestHelper.hpp>

#include "Parser.hpp"

namespace nmdo = netmeld::datastore::objects;
namespace nmdp = netmeld::datastore::parsers;

using qi::ascii::blank;

class TestParser : public Parser
{
  public:
    // Variables
    using Parser::d;

    // Rules
    using Parser::counts;
    using Parser::sportModule;
    using Parser::dportModule;
    using Parser::icmpModule;
    using Parser::rule;
    using Parser::chain;
    using Parser::start;
};

BOOST_AUTO_TEST_CASE(testRuleCounts)
{
  TestParser tp;
  const auto& parserRule {tp.counts};

  std::vector<std::string> testsOk {
      "[0:0]"
    , "[123:456]"
    , "[1234567890:9876543210]"
    };

  for (const auto& test : testsOk) {
    BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
              , "Parser rule 'counts': " << test
              );
  }

  std::vector<std::string> testsBad {
      "123:456]"
    , "[123:456"
    , "[12a:456]"
    , "[123:45a]"
    };

  for (const auto& test : testsBad) {
    BOOST_TEST( !nmdp::test(test.c_str(), parserRule, blank)
              , "Parser rule 'counts': " << test
              );
  }
}

BOOST_AUTO_TEST_CASE(testRuleSportDportIcmpModule)
{
  TestParser tp;

  {
    const auto& parserRule {tp.sportModule};

    std::vector<std::tuple<std::string, std::string>> testsOk {
        {"! --sport 123"  , "!123"}
      , {"--sport 123"    , "123"}
      , {""               , ""}
      };

    for (const auto& [test, expected] : testsOk) {
      std::string out;
      BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
                , "Parser rule 'sportModule': " << test
                );
      BOOST_TEST(expected == out);
    }

    std::string out;
    BOOST_TEST( nmdp::testAttr("abc", parserRule, out, blank, false)
              , "Parser rule 'sportModule': abc"
              );
    BOOST_TEST("" == out);
  }
  {
    const auto& parserRule {tp.dportModule};

    std::vector<std::tuple<std::string, std::string>> testsOk {
        {"! --dport 123"  , "!123"}
      , {"--dport 123"    , "123"}
      , {""               , ""}
      };

    for (const auto& [test, expected] : testsOk) {
      std::string out;
      BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
                , "Parser rule 'dportModule': " << test
                );
      BOOST_TEST(expected == out);
    }

    std::string out;
    BOOST_TEST( nmdp::testAttr("abc", parserRule, out, blank, false)
              , "Parser rule 'dportModule': abc"
              );
    BOOST_TEST("" == out);
  }
  {
    const auto& parserRule {tp.icmpModule};

    std::vector<std::tuple<std::string, std::string>> testsOk {
        {"! --icmp-type 123"    , "!123"}
      , {"--icmp-type 123"      , "123"}
      , {"! --icmpv6-type 123"  , "!123"}
      , {"--icmpv6-type 123"    , "123"}
      };

    for (const auto& [test, expected] : testsOk) {
      std::string out;
      BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
                , "Parser rule 'icmpModule': " << test
                );
      BOOST_TEST(expected == out);
    }
  }
}

BOOST_AUTO_TEST_CASE(testRuleRule)
{
  // tests optionSwitch as well
  // tests optionValue as well

  {
    TestParser tp;
    const auto& parserRule {tp.rule};

    std::vector<std::string> testsOk {
        "[0:0] -A ABC\n"
      , "[0:0] -A ABC -p tcp -m tcp --sport 123 --dport 456\n"
      , "[0:0] -A ABC -p udp -m udp --sport 123 --dport 456\n"
      , "[0:0] -A ABC -p icmp -m icmp --icmp-type 123\n"
      // unknown options
      , "[0:0] -A ABC -a abc\n"
      , "[0:0] -A ABC ! -a abc\n"
      };

    for (const auto& test : testsOk) {
      BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
                , "Parser rule 'rule': " << test
                );
    }

    BOOST_TEST_REQUIRE(1 == tp.d.ruleBooks.size());
    for (const auto& [bname, mrb] : tp.d.ruleBooks) {
      BOOST_TEST_REQUIRE(6 == mrb.size());
      for (const auto& [id, acr] : mrb) {
        const auto& tStr1 {acr.toDebugString()};
        nmdp::testInString(tStr1, "enabled: true,");
        nmdp::testInString(tStr1, "srcId: :ABC,");
        nmdp::testInString(tStr1, "srcs: [any],");
        nmdp::testInString(tStr1, "srcIfaces: [any],");
        nmdp::testInString(tStr1, "dstId: :ABC,");
        nmdp::testInString(tStr1, "dsts: [any],");
        nmdp::testInString(tStr1, "dstIfaces: [any],");
        nmdp::testInString(tStr1, "actions: [none]");
        nmdp::testInString(tStr1, "description: ]");

        if (1 == id) {
          nmdp::testInString(tStr1, "services: [tcp:123:456]");
        } else if (2 == id) {
          nmdp::testInString(tStr1, "services: [udp:123:456]");
        } else if (3 == id) {
          nmdp::testInString(tStr1, "services: [icmp::123]");
        } else {
          nmdp::testInString(tStr1, "services: [any]");
        }
      }
    }

    const std::string id {":ABC"};

    const auto& tgt1 {tp.d.serviceBooks};
    BOOST_TEST_REQUIRE(tgt1.contains(id));
    BOOST_TEST(3 == tgt1.at(id).size());
    BOOST_TEST(tgt1.at(id).contains("tcp:123:456"));
    BOOST_TEST(tgt1.at(id).contains("udp:123:456"));
    BOOST_TEST(tgt1.at(id).contains("icmp::123"));
  }
  {
    TestParser tp;
    const auto& parserRule {tp.rule};

    std::vector<std::string> testsOk {
        "[0:0] -A ABC -s 1.2.3.4/32 -d 1.2.3.0/24\n"
      , "[0:0] -A ABC -i a0 -o b0\n"
      , "[0:0] -A ABC -p abc -m abc\n"
      , "[0:0] -A ABC -j abc\n"
      , "[0:0] -A ABC -j abc def\n"
      , "[0:0] -A ABC -j abc --def 123 -i a0 -o b0\n"
      , "[0:0] -A ABC -f\n"
      , "[0:0] -A ABC -g abc\n"
      };

    for (const auto& test : testsOk) {
      BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
                , "Parser rule 'rule': " << test
                );
    }

    for (const auto& [bname, mrb] : tp.d.ruleBooks) {
      for (const auto& [id, acr] : mrb) {
        const auto& tStr1 {acr.toDebugString()};
        nmdp::testInString(tStr1, "enabled: true,");
        nmdp::testInString(tStr1, "srcId: :ABC,");
        nmdp::testInString(tStr1, "dstId: :ABC,");
        nmdp::testInString(tStr1, "description: ]");

        if (0 == id) {
          nmdp::testInString(tStr1, "srcs: [1.2.3.4/32],");
          nmdp::testInString(tStr1, "dsts: [1.2.3.0/24],");
        } else {
          nmdp::testInString(tStr1, "srcs: [any],");
          nmdp::testInString(tStr1, "dsts: [any],");
        }
        if (1 == id || 5 == id) {
          nmdp::testInString(tStr1, "srcIfaces: [a0],");
          nmdp::testInString(tStr1, "dstIfaces: [b0],");
        } else {
          nmdp::testInString(tStr1, "srcIfaces: [any],");
          nmdp::testInString(tStr1, "dstIfaces: [any],");
        }
        if (2 == id) {
          nmdp::testInString(tStr1, "services: [abc]");
        } else if (6 == id) {
          nmdp::testInString(tStr1, "services: [fragment]");
        } else {
          nmdp::testInString(tStr1, "services: [any]");
        }
        if (3 == id) {
          nmdp::testInString(tStr1, "actions: [jump abc]");
        } else if (4 == id) {
          nmdp::testInString(tStr1, "actions: [jump abc def]");
        } else if (5 == id) {
          nmdp::testInString(tStr1, "actions: [jump abc --def 123]");
        } else if (7 == id) {
          nmdp::testInString(tStr1, "actions: [goto abc]");
        } else {
          nmdp::testInString(tStr1, "actions: [none]");
        }
      }
    }

    const std::string id {":ABC"};

    const auto& tgt1 {tp.d.serviceBooks};
    BOOST_TEST_REQUIRE(tgt1.contains(id));
    BOOST_TEST(2 == tgt1.at(id).size());
    BOOST_TEST(tgt1.at(id).contains("abc"));
    BOOST_TEST(tgt1.at(id).contains("fragment"));
    
    const auto& tgt2 {tp.d.networkBooks};
    BOOST_TEST_REQUIRE(tgt2.contains(id));
    BOOST_TEST(2 == tgt2.at(id).size());
    BOOST_TEST(tgt2.at(id).contains("1.2.3.4/32"));
    BOOST_TEST(tgt2.at(id).contains("1.2.3.0/24"));
  }
}

BOOST_AUTO_TEST_CASE(testRuleChain)
{
  TestParser tp;
  const auto& parserRule {tp.chain};

  const auto initAcRule = [&](const auto& id, const auto& tgt) {
      nmdo::AcRule acr;
      acr.setSrcId(id);
      acr.setDstId(id);
      acr.setRuleId(SIZE_MAX);
      acr.addSrc("any");
      acr.addSrcIface("any");
      acr.addDst("any");
      acr.addDstIface("any");
      acr.addService("any");
      acr.addAction(tgt);
      return acr;
    };

  std::string test1 {": CHAIN1 TGT1 [0:0]\n"};
  BOOST_TEST( nmdp::test(test1.c_str(), parserRule, blank)
            , "Parser rule 'rule': " << test1
            );

  const std::string id {":CHAIN1"};
  const auto& expected = initAcRule(id, "TGT1");

  const auto& tgt1 {tp.d.ruleBooks};
  BOOST_TEST_REQUIRE(1 == tgt1.size());
  BOOST_TEST_REQUIRE(tgt1.contains(id));
  const auto& tgt2 {tgt1.at(id)};
  BOOST_TEST_REQUIRE(1 == tgt2.size());
  BOOST_TEST_REQUIRE(tgt2.contains(SIZE_MAX));
  BOOST_TEST(expected == tgt2.at(SIZE_MAX));

  std::string test2 {": CHAIN2 - [0:0]\n"};
  BOOST_TEST( nmdp::test(test2.c_str(), parserRule, blank)
            , "Parser rule 'rule': " << test2
            );

  BOOST_TEST_REQUIRE(1 == tgt1.size());
}

BOOST_AUTO_TEST_CASE(testWhole)
{
  TestParser tp;
  const auto& parserRule {tp.start};

  std::string test {
      "# comment\n"
      "*table\n"
      ": chain1 target1 [0:0]\n"
      ": chain2 target2 [0:0]\n"
      ": chain3 - [0:0]\n"
      "[0:0] -A chain1\n"
      "[0:0] -A chain3 -j chain2\n"
      "COMMIT\n"
      "# comment\n"
    };

  BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
            , "Parser rule 'rule': " << test
            );
}
