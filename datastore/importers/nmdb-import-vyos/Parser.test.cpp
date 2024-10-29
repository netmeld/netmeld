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

// =====
// Documenation:
// - https://docs.vyos.io/en/crux/configuration/firewall/index.html
//   - Parser was, probably, originally written for v1.2; maybe v1.3
// - https://docs.vyos.io/en/latest/configuration/index.html
//   - Breaking differences for v1.4+
//   - "latest" appears to be a dev branch
// =====
class TestParser : public Parser
{
  public:
    // variables
    using Parser::d;
    using Parser::defaultAction;

    // methods (not parser rules)
    using Parser::getData;
    using Parser::ruleAddDefault;

    // parser rules
    using Parser::token;
    using Parser::comment;
    using Parser::system;
    using Parser::login;
    //using Parser::user;
    using Parser::interfaces;
    //using Parser::interface;
    //using Parser::ifaceFirewall;
    using Parser::firewall;
    //using Parser::group;
    //using Parser::addressGroup;
    //using Parser::ruleSets;
    using Parser::rule;
    //using Parser::destination;
    //using Parser::source;
    using Parser::start;
};

BOOST_AUTO_TEST_CASE(testRuleToken)
{
  TestParser tp;
  const auto& parserRule {tp.token};

  std::vector<std::tuple<std::string, std::string>> testsOk {
      {"abc123"       , "abc123"}
    , {R"("abc123")"  , "abc123"}
    , {R"("abc 123")" , "abc 123"}
    , {R"("a1.~\|\"z")" , R"(a1.~\|z)"}
    , {R"(a"b)" , R"(a"b)"}
    };

  for (const auto& [test, expected] : testsOk) {
    std::string out;
    BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out)
              , "Parser rule 'token': " << test
              );
    BOOST_TEST(expected == out);
  }

  std::vector<std::string> testsNotFull {
      "a{"
    , "a}"
    , "a#"
    };

  for (const auto& test : testsNotFull) {
    std::string out;
    BOOST_TEST( !nmdp::testAttr(test.c_str(), parserRule, out)
              , "Parser rule 'token': " << test
              );
    BOOST_TEST("a" == out);
  }

  std::vector<std::string> testsMalformed {
      R"("a)"
    , R"("a\"b)"
    };

  for (const auto& test : testsNotFull) {
    std::string out;
    BOOST_TEST( !nmdp::testAttr(test.c_str(), parserRule, out)
              , "Parser rule 'token': " << test
              );
    BOOST_TEST("a" == out);
  }
}

BOOST_AUTO_TEST_CASE(testRuleComment)
{
  TestParser tp;
  const auto& parserRule {tp.comment};

  std::string test {"# comment"};
  BOOST_TEST( nmdp::test(test.c_str(), parserRule)
            , "Parser rule 'comment': " << test
            );
}

BOOST_AUTO_TEST_CASE(testRuleSystem)
{
  TestParser tp;
  const auto& parserRule {tp.system};

  std::string test {
      R"(system {
           host-name abc
           domain-name abc.arpa
           name-server 1.2.3.4
           # login block handled elsewhere
           # ntp block not configured
         })"
    };
  BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
            , "Parser rule 'system': " << test
            );

  BOOST_TEST(1 == tp.d.services.size());
  const auto tStr1 {tp.d.services[0].toDebugString()};
  nmdp::testInString(tStr1, "serviceName: DNS");
  nmdp::testInString(tStr1, "protocol: udp");
  nmdp::testInString(tStr1, "dstPorts: [53]");
  nmdp::testInString(tStr1, "serviceReason: vyos device config");

  const auto tStr2 {tp.d.observations.toDebugString()};
  nmdp::testInString(tStr2, "unsupportedFeatures:"
                            " [domain-name abc.arpa"
                            ", host-name abc]"
                    );
}

BOOST_AUTO_TEST_CASE(testRuleLogin)
{
  // tests user as well
  TestParser tp;
  const auto& parserRule {tp.login};

  std::string test {
      R"(login {
           user tuser {
             authentication {
               plaintext-password tplain
               encrypted-password tenc
             }
             level tlevel
           }
         })"
    };

  BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
            , "Parser rule 'login': " << test
            );

  const auto tStr1 {tp.d.observations.toDebugString()};
  nmdp::testInString(tStr1, "notables: [Creds--tlevel:tuser:tenc:tplain]");
}

BOOST_AUTO_TEST_CASE(testRuleInterfaces)
{
  // tests interfaces as well
  // tests ifaceFirewall as well
  TestParser tp;
  const auto& parserRule {tp.interfaces};

  std::string test {
      R"(interfaces {
           loopback lo {
           }
           ethernet eth0 {
             hw-id 00:11:22:33:44:55
           }
           ethernet eth1 {
             address dhcp
             hw-id 00:11:22:33:44:55
           }
           ethernet eth2 {
             address 1.2.3.4/24
             hw-id 00:11:22:33:44:55
             description "some text"
           }
           ethernet eth3 {
             firewall {
               in {
                 ipv6-name fwin6name
                 name fwinname
               }
               local {
                 name fwlocalname
               }
               out {
                 name fwoutname
               }
             }
           }
           ethernet eth4 {
             vif 100 {
               address 1.2.3.4/24
             }
           }
         })"
    };
  BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
            , "Parser rule 'interfaces': " << test
            );

  BOOST_TEST(6 == tp.d.ifaces.size());
  for (const auto& name : {"lo", "eth0", "eth1", "eth2", "eth3", "eth4"}) {
    BOOST_TEST_REQUIRE(tp.d.ifaces.contains(name));
    const auto tStr1 {tp.d.ifaces.at(name).toDebugString()};
    nmdp::testInString(tStr1, std::format("name: {}", name));
    nmdp::testInString(tStr1, "isUp: true");
    if ("lo" == name) {
      nmdp::testInString(tStr1, "mediaType: loopback");
    } else {
      nmdp::testInString(tStr1, "mediaType: ethernet");
    }
    if ("eth0" == name || "eth1" == name || "eth2" == name) {
      nmdp::testInString(tStr1, "macAddress: 00:11:22:33:44:55");
    }
    if ("eth1" == name) {
      const auto tStr2 {tp.d.observations.toDebugString()};
      nmdp::testInString(tStr2, "unsupportedFeatures: [address dhcp]");
    }
    if ("eth2" == name) {
      nmdp::testInString(tStr1, "ipAddress: 1.2.3.4/24");
      nmdp::testInString(tStr1, "description: some text");
    }
    if ("eth3" == name) {
      // interfaces come after rules, so books should be empty
      BOOST_TEST_REQUIRE(4 == tp.d.ruleBooks.size());
      BOOST_TEST(0 == tp.d.ruleBooks.at("fwin6name").size());
      BOOST_TEST(0 == tp.d.ruleBooks.at("fwinname").size());
      BOOST_TEST(0 == tp.d.ruleBooks.at("fwlocalname").size());
      BOOST_TEST(0 == tp.d.ruleBooks.at("fwoutname").size());
    }
  }
}

BOOST_AUTO_TEST_CASE(testRuleFirewall)
{
  TestParser tp;
  {
    const auto& parserRule {tp.rule};
    // tests destination as well
    // tests source as well

    std::vector<std::string> testsOk {
        R"(rule 10 {
             action raction
             description "rule text"
             protocol something
             log disable
             disable
           })"
      , R"(rule 20 {
             action raction
             state {
               established enable
               related enable
               invalid enable
             }
           })"
      , R"(rule 30 {
             action raction
             destination {
               port 123
               group {
                 address-group abc
               }
               address 1.2.3.4
             }
             source {
               port 123
               group {
                 address-group abc
               }
               address 1.2.3.4
             }
           })"
      , R"(rule 40 {
             action raction
             time {
               starttime 00:11:22
               stoptime 00:11:22
               weekdays Fri,Sat
             }
           })"
      /* TODO Not accounted for yet; may cause issues during parsing
      , R"(rule xx {
             # Below can be either destination or source
             source { mac-address 00:11:22:33:44:55 }
             source { group { domain-group abc } }
             source { group { interface-group abc } }
             source { group { ipv6-address-group abc } }
             source { group { ipv6-network-group abc } }
             source { group { mac-group abc } }
             source { group { network-group abc } }
             source { group { port-group abc } }
           })"
      */
      };

    for (const auto& test : testsOk) {
      BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
                , "Parser rule 'firewall': " << test
                );
    }

    // simulate adding default rule
    tp.defaultAction = "default_action";
    tp.ruleAddDefault();

    BOOST_TEST(1 == tp.d.ruleBooks.size());
    BOOST_TEST_REQUIRE(5 == tp.d.ruleBooks["global"].size());

    for (const auto& [pos, acr] : tp.d.ruleBooks["global"]) {
      const auto tStr1 {acr.toDebugString()};

      if (4 == pos) {
        nmdp::testInString(tStr1, "id: 1000000");
        nmdp::testInString(tStr1, "actions: [default_action]");
      } else {
        nmdp::testInString(tStr1, std::format("id: {}", ((pos*10)+10)));
        nmdp::testInString(tStr1, "actions: [raction]");
      }
      nmdp::testInString(tStr1, "srcId: global");
      nmdp::testInString(tStr1, "dstId: global");
      nmdp::testInString(tStr1, "srcIfaces: []");
      nmdp::testInString(tStr1, "dstIfaces: []");

      if (0 == pos) {
        nmdp::testInString(tStr1, "enabled: false");
        nmdp::testInString(tStr1, "services: [something::]");
        nmdp::testInString(tStr1, "description: rule text");
      } else {
        nmdp::testInString(tStr1, "enabled: true");
        nmdp::testInString(tStr1, "description: ]");
      }
      if (1 == pos) {
        nmdp::testInString(tStr1, "services: [all::,"
                                  " state:(established,related,invalid)]"
                          );
      }
      if (2 == pos) {
        nmdp::testInString(tStr1, "srcs: [abc, 1.2.3.4]");
        nmdp::testInString(tStr1, "dsts: [abc, 1.2.3.4]");
        nmdp::testInString(tStr1, "services: [:123:123]");
      } else {
        nmdp::testInString(tStr1, "srcs: []");
        nmdp::testInString(tStr1, "dsts: []");
      }
      if (3 == pos) {
        nmdp::testInString(tStr1, "services: [all::]");
      }
    }

    // Calling `getData()` alters rules
    tp.getData();
    for (const auto& [pos, acr] : tp.d.ruleBooks["global"]) {
      const auto tStr1 {acr.toDebugString()};

      nmdp::testInString(tStr1, "srcIfaces: [any]");
      nmdp::testInString(tStr1, "dstIfaces: [any]");

      if (2 == pos) {
        nmdp::testInString(tStr1, "srcs: [abc, 1.2.3.4]");
        nmdp::testInString(tStr1, "dsts: [abc, 1.2.3.4]");
      } else {
        nmdp::testInString(tStr1, "srcs: [any]");
        nmdp::testInString(tStr1, "dsts: [any]");
      }
    }
  }

  {
    const auto& parserRule {tp.firewall};
    // tests group as well
    // tests addressGroup as well
    // tests ruleSets as well

    std::vector<std::string> tests {
      R"(firewall {
           group {
             address-group agname {
               address 1.2.3.0/24
               address 4.3.2.0/24
               description text
             }
           }
           name v4rules {
             default-action daction
             description btext
             enable-default-log
             # rule tested elsewhere
           }
           ipv6-name v6rules {
             default-action daction
             description btext
             enable-default-log
             # rule tested elsewhere
           }
         })"
    // TODO These are not handled correctly; ignored instead of saved
    , R"(firewall {
           group {
             address-group agname {
               address 1.2.3.1-1.2.3.5
             }
             domain-group dgname {
               address abc.def
             }
             interface-group igname {
               interface abc
             }
             ipv6-address-group v6agname {
               address 1::2
               address 1::2-1::5
             }
             ipv6-network-group v6ngname {
               address 1::0/64
             }
             mac-group mgname {
               mac-address 00:11:22:33:44:55
             }
             network-group ngname {
               address 1.2.3.0/24
             }
             port-group pgname {
               port abc
               port 123
               port 123-456
             }
           }
         })"
    };

    for (const auto& test : tests) {
      BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
                , "Parser rule 'firewall': " << test
                );
    }

    BOOST_TEST_REQUIRE(1 == tp.d.networkBooks.size());

    for (const auto& [nbName, acnb] : tp.d.networkBooks["global"]) {
      const auto tStr1 {acnb.toDebugString()};
      nmdp::testInString(tStr1, "id: global");
      nmdp::testInString(tStr1, "name: agname");
      nmdp::testInString(tStr1, "data: [1.2.3.0/24, 4.3.2.0/24]");
    }

    // default rules are added for each ruleSet
    BOOST_TEST(3 == tp.d.ruleBooks.size());
    for (const auto& rbs : tp.d.ruleBooks) {
      if ("global" == rbs.first) {
        continue;
      }
      BOOST_TEST_REQUIRE(1 == rbs.second.size());
      for (const auto& rb : rbs.second) {
        BOOST_TEST(0 == rb.first);
        const auto tStr1 {rb.second.toDebugString()};
        nmdp::testInString( tStr1
                          , "[enabled: true, id: 1000000"
                            ", srcId: global, srcs: [], srcIfaces: []"
                            ", dstId: global, dsts: [], dstIfaces: []"
                            ", services: [all::], actions: [daction]"
                            ", description: ]"
                          );
      }
    }
  }
}
