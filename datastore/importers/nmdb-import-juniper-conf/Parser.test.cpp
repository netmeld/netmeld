// =============================================================================
// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
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

namespace nmdp = netmeld::datastore::parsers;

using qi::ascii::blank;

class TestParser : public Parser
{
  public:
    using Parser::d;
    using Parser::DEFAULT_ZONE;
    using Parser::tgtZone;

    using Parser::start;
    using Parser::config;
    using Parser::system;
    using Parser::applications;
    using Parser::application;
    using Parser::applicationSet;
    using Parser::appMultiLine;
    using Parser::appSingleLine;
    using Parser::interfaces;
    using Parser::interface;
    using Parser::unit;
    using Parser::family;
    using Parser::ifaceVlan;
    using Parser::security;
    using Parser::policies;
    using Parser::policyFromTo;
    using Parser::policy;
    using Parser::policyMatch;
    using Parser::policyThen;
    using Parser::zones;
    using Parser::zone;
    using Parser::zoneIface;
    using Parser::addressBookData;
    using Parser::routingOptions;
    using Parser::routeStatic;
    using Parser::groups;
    using Parser::group;
    using Parser::vlans;
    using Parser::logicalSystems;
    using Parser::logicalSystem;
    using Parser::routingInstances;
    using Parser::routingInstance;
    using Parser::ignoredBlock;
    using Parser::startBlock;
    using Parser::stopBlock;
    using Parser::route;
    using Parser::address;
    using Parser::addressSet;
    using Parser::addressBook;
    using Parser::vlan;
    using Parser::tokenList;
    using Parser::logBlock;
    using Parser::token;
    using Parser::typeSlot;
    using Parser::comment;
    using Parser::semicolon;
    using Parser::garbageLine;
};

BOOST_AUTO_TEST_CASE(testRuleTokenTokenList)
{
  TestParser tp;
  {
    const auto& parserRule {tp.token};

    const std::vector<std::tuple<std::string, std::string>> testsOk {
        {"abc", "abc"}
      , {"abc123;", "abc123"}
      , {R"("abc 123")", "abc 123"}
      , {R"("abc";)", "abc"}
      , {R"("a\"b\"c")", "abc"}
      , {R"("a\"b\"c;";)", "abc;"}
      };

    for (const auto& [test, expected] : testsOk) {
      std::string out;
      BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out)
                , "Parse rule 'token': " << test
                );
      BOOST_TEST(expected == out);
    }

    const std::vector<std::tuple<std::string, std::string>> testsPartial {
        {"abc{", "abc"}
      ,  {"abc}", "abc"}
      ,  {"abc#", "abc"}
      ,  {"abc#", "abc"}
      };

    for (const auto& [test, expected] : testsPartial) {
      std::string out;
      BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, false)
                , "Parse rule 'token': " << test
                );
      BOOST_TEST(expected == out);
    }
  }

  {
    const auto& parserRule {tp.tokenList};

    std::string test {
      R"([abc "abc" "a\"b\"c"];)"
      };

    std::vector<std::string> out;
    BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
              , "Parse rule 'tokenList': " << test
              );
    for (const auto& token : out) {
      BOOST_TEST("abc" == token);
    }
  }
}

BOOST_AUTO_TEST_CASE(testRuleSystem)
{
  TestParser tp;
  const auto& parserRule {tp.system};
  // also tests ignoredBlock

  const std::string test {
      R"(system { # ignored comment
        host-name abc123;
        ignored line;
        ignored {
          block;
          blocks {
            ignored as well;
          }
        }
        # ignored comment

        { ignored; }
      }
      )"
    };

  BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
            , "Parse rule 'system': " << test
            );
}

BOOST_AUTO_TEST_CASE(testRuleApplications)
{
  TestParser tp;
  const auto& parserRule {tp.applications};
  // also tests application
  // also tests applicationSet
  // also tests appMultiLine
  // also tests appSingleLine

  const std::string test {
      R"(applications {
        # application SingleLine
        application b1 protocol p-only;
        application b1 source-port sp-only;
        application b1 destination-port dp-only;
        application b2 protocol p-a source-port sp-a destination-port dp-a;
        application b3 protocol p-a uuid uuid-a;
        application b3 protocol p-a rpc-program-number rpc-a;
        # application MultiLine
        application b4 {
          protocol p-a1;
          source-port sp-a1;
          destination-port dp-a1;
        }
        application b4 {
          protocol p-a2;
          source-port sp-a2;
          destination-port dp-a2;
        }
        # applicationSet
        application-set b5 {
          application b1;
          ignored line;
          application b6
        }
      }
      )"
    };

  // parser should add applications to default zone
  tp.tgtZone = "not-global";

  BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
            , "Parse rule 'applications': " << test
            );

  BOOST_TEST_REQUIRE(tp.d->serviceBooks.contains(tp.DEFAULT_ZONE));

  const auto& tServiceBook {tp.d->serviceBooks[tp.DEFAULT_ZONE]};
  //for (const auto& [k,v] : tServiceBook) {
  //  LOG_INFO << std::format("k: {}, v: {}\n", k, v.toDebugString());
  //}
  BOOST_TEST_REQUIRE(6 == tServiceBook.size());
  BOOST_TEST_REQUIRE(tServiceBook.contains("b1"));
  nmdp::testInString( tServiceBook.at("b1").toDebugString()
                    , "data: [::dp-only, :sp-only:, p-only::]"
                    );
  BOOST_TEST_REQUIRE(tServiceBook.contains("b2"));
  nmdp::testInString( tServiceBook.at("b2").toDebugString()
                    , "data: [p-a:sp-a:dp-a]"
                    );
  BOOST_TEST_REQUIRE(tServiceBook.contains("b3"));
  nmdp::testInString( tServiceBook.at("b3").toDebugString()
                    , "data: [p-a-ms-rpc::uuid-a, p-a-sun-rpc::rpc-a]"
                    );
  BOOST_TEST_REQUIRE(tServiceBook.contains("b4"));
  nmdp::testInString( tServiceBook.at("b4").toDebugString()
                    , "data: [p-a1:sp-a1:dp-a1, p-a2:sp-a2:dp-a2]"
                    );
  BOOST_TEST_REQUIRE(tServiceBook.contains("b5"));
  nmdp::testInString( tServiceBook.at("b5").toDebugString()
                    , "data: [::dp-only, :sp-only:, b6, p-only::]"
                    );
  BOOST_TEST_REQUIRE(tServiceBook.contains("b6"));
  nmdp::testInString( tServiceBook.at("b6").toDebugString()
                    , "data: []"
                    );
}

BOOST_AUTO_TEST_CASE(testRuleInterfaces)
{
  TestParser tp;
  const auto& parserRule {tp.interfaces};
  // also tests interface
  // also tests typeSlot
  // also tests unit
  // also tests family
  // also tests ifaceVlan

  const std::string test {
      R"(interfaces {
        # unsupported
        interface-range range-name {
          member abc;
          member-range abd-def;
          ignored line;
        }
        # supported
        ignored line;
        type-slot1 {
          ignored line;
          unit 0 {
            ignored line;
            family abc {
              ignored line;
            }
          }
          unit 1 {
            disable;
          }
          unit 2 {
            family 2 {
              address 1.2.3.4/24;
            }
          }
          unit 3 {
            family 3 {
              address 1.2.3.5 {
                ignored line;
              }
              port-mode abc;
            }
          }
          unit 4 {
            family 4 {
              native-vlan-id 123;
              vlan {
                members 123;
              }
            }
            vlan-id 123;
          }
          unit 5 {
            family 5 {
              vlan {
                members [all 1-5 789 other];
              }
            }
          }
        }
        type-slot2 {
          unit 0 {
            family 0 {
              address 1.2.3.4/25;
            }
          }
          disable;
        }
      }
      )"
    };

  BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
            , "Parse rule 'interfaces': " << test
            );

  const auto& tIfaces {tp.d->ifaces};
//  for (const auto& [k,v] : tIfaces) {
//    LOG_INFO << std::format("k: {}, v: {}\n", k, v.toDebugString());
//  }
  BOOST_TEST_REQUIRE(7 == tIfaces.size());
  BOOST_TEST_REQUIRE(tIfaces.contains("type-slot1.0"));
  auto tStr {tIfaces.at("type-slot1.0").toDebugString()};
  nmdp::testInString(tStr , "name: type-slot1.0");
  nmdp::testInString(tStr , "mediaType: type");
  nmdp::testInString(tStr , "isUp: true");
  BOOST_TEST_REQUIRE(tIfaces.contains("type-slot1.1"));
  tStr = tIfaces.at("type-slot1.1").toDebugString();
  // TODO this should pass, not fail
  //nmdp::testInString(tStr , "isUp: false");
  BOOST_TEST_REQUIRE(tIfaces.contains("type-slot1.2"));
  tStr = tIfaces.at("type-slot1.2").toDebugString();
  nmdp::testInString(tStr , "ipAddress: 1.2.3.4/24");
  BOOST_TEST_REQUIRE(tIfaces.contains("type-slot1.3"));
  tStr = tIfaces.at("type-slot1.3").toDebugString();
  nmdp::testInString(tStr , "ipAddress: 1.2.3.5/32");
  nmdp::testInString(tStr , "mode: l2 abc");
  BOOST_TEST_REQUIRE(tIfaces.contains("type-slot1.4"));
  tStr = tIfaces.at("type-slot1.4").toDebugString();
  nmdp::testInString(tStr , "vlanId: 123");
  BOOST_TEST_REQUIRE(tIfaces.contains("type-slot1.5"));
  tStr = tIfaces.at("type-slot1.5").toDebugString();
  nmdp::testInString(tStr , "vlanId: 1");
  nmdp::testInString(tStr , "vlanId: 2");
  nmdp::testInString(tStr , "vlanId: 3");
  nmdp::testInString(tStr , "vlanId: 4");
  nmdp::testInString(tStr , "vlanId: 5");
  nmdp::testInString(tStr , "vlanId: 789");
  BOOST_TEST_REQUIRE(tIfaces.contains("type-slot2.0"));
  tStr = tIfaces.at("type-slot2.0").toDebugString();
  nmdp::testInString(tStr , "isUp: false");
  nmdp::testInString(tStr , "ipAddress: 1.2.3.4/25");
}

BOOST_AUTO_TEST_CASE(testRuleRoutingOptions)
{
  TestParser tp;
  const auto& parserRule {tp.routingOptions};
  // also tests routeStatic
  // also tests route

  const std::string test {
      R"(routing-options {
        ignored line;
        static {
          ignored line;
          route 1.2.3.0/24 next-hop 1.2.3.1;
          route 1.2.3.0/23 next-hop nic01;
        }
      }
      )"
    };
  BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
            , "Parse rule 'routingOptions': " << test
            );
  BOOST_TEST_REQUIRE(2 == tp.d->routes.size());
  const auto& tStr1 {tp.d->routes[0].toDebugString()};
  nmdp::testInString(tStr1 , "vrfId: ,");
  nmdp::testInString(tStr1 , "dstIpNet: [ipNetwork: 1.2.3.0/24,");
  nmdp::testInString(tStr1 , "nextHopIpAddr: [ipAddress: 1.2.3.1/32,");
  const auto& tStr2 {tp.d->routes[1].toDebugString()};
  nmdp::testInString(tStr1 , "vrfId: ,");
  nmdp::testInString(tStr2 , "dstIpNet: [ipNetwork: 1.2.2.0/23,");
  nmdp::testInString(tStr2 , "nextHopIpAddr: [ipAddress: 0.0.0.0/0,");
  nmdp::testInString(tStr2 , "outIfaceName: nic01,");
}

BOOST_AUTO_TEST_CASE(testRuleSecurity)
{
  TestParser tp;
  const auto& parserRule {tp.security};
  // also tests policies
  // also tests policyFromTo
  // also tests policy
  // also tests policyMatch
  // also tests policyThen
  // also tests logBlock
  // also tests zones
  // also tests zone
  // also tests addressBook
  // also tests addressBookData
  // also tests address
  // also tests addressSet
  // also tests zoneIface

  const std::string test {
      R"(security {
        ignored line;
        policies {
          ignored line;
          from-zone abc to-zone def {
            ignored line;
            policy abc_def {
              ignored line;
              match {
                ignored line;
                category1 condition;
                category2 [ space separated condition list ];
                category3 multi condition;
              }
              then {
                ignored line;
                action;
                log {
                  condition1;
                  condition2;
                }
                another block {
                  ignored block;
                }
              }
            }
          }
        }
        zones {
          ignored line;
          security-zone abc {
            ignored line;
            interfaces {
              ignored line;
              abc-0/1.234 {
                ignored block;
              }
            }
            address-book {
              attach def {
                logic is security's address-book;
              }
            }
          }
        }
        address-book {
          ignored line;
        }
      }
      )"
    };
  BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
            , "Parse rule 'security': " << test
            );
  const auto& ruleBooks {tp.d->ruleBooks};
  BOOST_TEST_REQUIRE(1 == ruleBooks.size());
  BOOST_TEST_REQUIRE(ruleBooks.contains("abc->def"));
  const auto& ruleBook {ruleBooks.at("abc->def")};
  BOOST_TEST_REQUIRE(1 == ruleBook.size());
  BOOST_TEST_REQUIRE(ruleBook.contains(0));
  const auto& tStr1 {ruleBook.at(0).toDebugString()};
  nmdp::testInString(tStr1 , "enabled: true,");
  nmdp::testInString(tStr1 , "id: 0,");
  // TODO below should: not contain 'ignored, line'
  nmdp::testInString(tStr1 , "srcIfaces: [ignored, line, abc-0/1.234]");
  // TODO below should:
  //    1) not contain 'ignored line'
  //    2) contain 'another block { ignored block; }'
  nmdp::testInString(tStr1 , "actions: [ignored line"
                                      ", action"
                                      ", log condition1 condition2"
                                      ", another block"
                                      "]"
                    );
  nmdp::testInString(tStr1 , "description: abc_def");
  const auto& networkBooks {tp.d->networkBooks};
  BOOST_TEST_REQUIRE(0 == networkBooks.size());
  const auto& serviceBooks {tp.d->serviceBooks};
  BOOST_TEST_REQUIRE(0 == serviceBooks.size());
}
