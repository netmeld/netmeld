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
#include <boost/format.hpp>

#include <netmeld/datastore/parsers/ParserTestHelper.hpp>

#include "Parser.hpp"

namespace nmdo = netmeld::datastore::objects;
namespace nmdp = netmeld::datastore::parsers;

using qi::ascii::blank;

class TestParser : public Parser
{
  public:
    //using Parser::codesLegend;
    //using Parser::csvToken;
    //using Parser::distance;
    //using Parser::dstIpv4NetBlank;
    //using Parser::ifaceName;
    //using Parser::ignoredLine;
    //using Parser::ipv4Addr;
    //using Parser::ipv6Addr;
    //using Parser::metric;
    //using Parser::rtrIpv4Addr;
    //using Parser::rtrIpv6Addr;
    //using Parser::token;
    //using Parser::typeCodeBlank;
    //using Parser::vrf;
    using Parser::ipv4Route;
    using Parser::ipv6Route;
    using Parser::start;
    using Parser::typeCode;
    using Parser::typeCodeLookups;
    using Parser::uptime;
    using Parser::curObservations;
    using Parser::vrfHeader;

    using Parser::isIos;
    using Parser::isIosOld;
    using Parser::isNxos;
};

BOOST_AUTO_TEST_CASE(testRulesUptime)
{
  TestParser tp;
  const auto& parserRule {tp.uptime};

  std::vector<std::string> testsOk {
    // hh:mm:ss     -- unknown if hours has max value
      R"(01:23:45)"
    , R"(1234:12:12)"
    // 0d0h | 0w0d  -- 0 can be any positive numeral
    , R"(1d2h)"
    , R"(365d23h)"
    , R"(1w2d)"
    , R"(56w6d)"
    // 0.000000
    , R"(0.000000)"
    };
  for (const auto& test : testsOk) {
    BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
              , "Rule 'uptime': " << test
              );
  }

  std::vector<std::string> testsBad {
      R"(12:34:56:78)"
    , R"(12:34)"
    , R"(1a2b)"
    , R"(1d2b)"
    , R"(1w2b)"
    , R"(1.000000)"
    , R"(0.123456)"
    };
  for (const auto& test : testsBad) {
    BOOST_TEST(!nmdp::test(test.c_str(), parserRule, blank)
              , "Rule '!uptime': " << test
              );
  }
}

BOOST_AUTO_TEST_CASE(testVrfHeader)
{
  TestParser tp;
  const auto& parserRule {tp.vrfHeader};

  {// IOS/ASA
    std::vector<std::string> testsOk {
        "Codes: A - some1\n"
      , "Codes: A - some1, B - some2\n C - some3\n"
      , "VRF: default\nCodes: A - some1\n"
      , R"(VRF name: default
           Displaying 0 of 0 IPv6 routing table entries
           IPv6 Routing Table - 0 entries
           Codes: A - some1, B - some2
                  C - some3

           Gateway of last resort is not set
        )"
      };

    for (const auto& test : testsOk) {
      std::string out;
      BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
                , "Rule 'vrfHeader': " << test
                );
      BOOST_TEST("" == out);
      BOOST_TEST(tp.isIos);
      BOOST_TEST(!tp.isIosOld);
      BOOST_TEST(!tp.isNxos);
    }
    tp.isIos = false;
  }

  {// IOS old
    std::vector<std::string> testsOk {
        R"(Default gateway is 1.2.3.4

      Host               Gateway           Last Use    Total Uses  Interface
      )"
      };

    for (const auto& test : testsOk) {
      std::string out;
      BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
                , "Rule 'vrfHeader': " << test
                );
      BOOST_TEST("" == out);
      BOOST_TEST(!tp.isIos);
      BOOST_TEST(tp.isIosOld);
      BOOST_TEST(!tp.isNxos);
    }
    tp.isIosOld = false;
  }

  {// NXOS
    std::vector<std::string> testsOk {
        R"(IP Route Table for VRF "default"
           '*' denotes best ucast next-hop
           '**' denotes best mcast next-hop
           '[x/y]' denotes [preference/metric]
           '%<string>' in via output denotes VRF <string>
        )"
      , R"(IPv6 Routing Table for VRF "default"
           '*' denotes best ucast next-hop
           '**' denotes best mcast next-hop
           '[x/y]' denotes [preference/metric]
        )"
      };

    for (const auto& test : testsOk) {
      std::string out;
      BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
                , "Rule 'vrfHeader': " << test
                );
      BOOST_TEST("" == out);
      BOOST_TEST(!tp.isIos);
      BOOST_TEST(!tp.isIosOld);
      BOOST_TEST(tp.isNxos);
    }
    tp.isNxos = false;
  }
}

BOOST_AUTO_TEST_CASE(testRulesIpRouteBad)
{
  TestParser tp;

  // turn everything on, should all fail
  tp.isIos = true;
  tp.isIosOld = true;
  tp.isNxos = true;

  const auto& parserRule {tp.ipv4Route};

  std::vector<std::string> testsOkInvalidRoute {
      R"(1.2.3.0/24 is subnetted, junk, data)"
    , R"(     1.2.3.0/24 is variably subnetted, junk, data)"
    };

  for (const auto& test : testsOkInvalidRoute) {
    nmdo::Route out;
    BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
              , "Rule 'ipv4Route': " << test
              );
    BOOST_TEST(!out.isValid());
  }
}

BOOST_AUTO_TEST_CASE(testRulesIpRouteIos)
{
  TestParser tp;
  tp.isIos = true;

  { // IPv4
    const auto& parserRule {tp.ipv4Route};

    std::vector<std::string> testsOk {
      // IOS/ASA/ASR like
        R"(C    1.2.3.0/24 is directly connected, nic)"
      , R"(C    1.2.3.0/24 is directly connected, nic.1)"
      , R"(C    1.2.3.0 is directly connected, nic)"
      , R"( C    1.2.3.0/24 is directly connected, nic)"
      , R"(C    1.2.3.0/24 is directly connected, Null0)"
      , R"(C    1.2.3.0/24 is directly connected (source VRF n1), nic (egress VRF n2))"
      , R"(C    1.2.3.0 255.255.255.0 is directly connected, nic)"
      , R"(D    1.2.3.0/18 is a summary, 1w2d, Null0)"

      , R"(S*   0.0.0.0/0 [1/2] via 1.2.3.4)"
      , R"(B    0.0.0.0/0 [1/2] via 1.2.3.4, nic)"
      , R"(B    0.0.0.0/0 [1/2] via 1.2.3.4, 01:02:03)"
      , R"(B    0.0.0.0/0 [1/2] via 1.2.3.4, 01:02:03, nic.1)"
      , R"(B    0.0.0.0/0 [1/2] via 1.2.3.4, 1d2h)"
      , R"(B    0.0.0.0/0 [1/2] via 1.2.3.4, 1d2h, nic)"
      , R"(B    0.0.0.0/0 [1/2] via 1.2.3.4, 1w2d)"
      , R"(B    0.0.0.0/0 [1/2] via 1.2.3.4, 1w2d, nic.1)"
      , R"(B    1.0.0.0 255.255.255.0 [1/2] via 1.2.3.4, nic)"

      , R"(D EX 1.2.3.4 255.255.255.255 [1/2] via 4.3.2.1, 01:02:03, nic)"
      , R"(D EX 1.2.3.4 255.255.255.255
                                        [1/2] via 4.3.2.1, 01:02:03, nic)"
      // "one" test, two lines
      , R"(D EX 1.2.3.4 255.255.255.255 [1/2] via 4.3.2.1, 01:02:03, nic)"
      , R"(                             [1/2] via 4.3.2.2, 01:02:03, nic)"
      // "one" test, three lines
      , R"(D EX 1.2.3.4 255.255.255.255
                                        [1/2] via 4.3.2.1, 01:02:03, nic)"
      , R"(                             [1/2] via 4.3.2.2, 01:02:03, nic)"

      , R"(B L  0.0.0.0/0 [0/0] (source VRF n1) via 1.2.3.4, nic (egress VRF n2))"

      // ASR null route
      , R"(B    0.0.0.0/0 [1/2], 1w1d, Null0)"
      };

    for (const auto& test : testsOk) {
      nmdo::Route out;
      BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
                , "Rule 'ipv4Route': " << test
                );
      const auto debugStr {out.toDebugString()};
      BOOST_TEST(out.isValid()
                , "Validity failure: " + test + "\n\tYielded: " + debugStr
                );
      if (std::string::npos != debugStr.find("outIfaceName: null0")) {
        nmdp::testInString(debugStr, "isNullRoute: true");
      }
      if (std::string::npos != test.find("%default,")) {
        nmdp::testInString(debugStr, "nextVrfId: default,");
      }
    }

    // ------------------------------------------------------------------------
    std::vector<std::string> testsMultiRoute {
      // "one" test, two lines
        R"(B L  0.0.0.0 255.255.255.255 [1/2] via 1.2.3.4, 01:02:03, nic1)"
      , R"(                             [1/2] via 1.2.3.5, 01:02:03, nic2)"
      // "one" test, two lines
      , R"(B L  0.0.0.0/0 [1/2] via 1.2.3.4, nic1)"
      , R"(               [1/2] via 1.2.3.5, nic2)"
        // "one" test, two lines
      , R"(B L  0.0.0.0/0 [1/2] (source VRF n1) via 1.2.3.4, nic1 (egress VRF n2))"
      , R"(                                     via 1.2.3.5, nic2 (egress VRF n3))"
      };
    bool isFirst {true};
    for (const auto& test : testsMultiRoute) {
      nmdo::Route out;
      BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
                , "Rule 'ipv4Route': " << test
                );
      BOOST_TEST(out.isValid());
      const auto debugStr {out.toDebugString()};
      if (isFirst) {
        nmdp::testInString(debugStr, R"(nextHopIpAddr: [ipAddress: 1.2.3.4/32)");
        nmdp::testInString(debugStr, R"(outIfaceName: nic1)");
      } else {
        nmdp::testInString(debugStr, R"(nextHopIpAddr: [ipAddress: 1.2.3.5/32)");
        nmdp::testInString(debugStr, R"(outIfaceName: nic2)");
      }
      isFirst = !isFirst;
      nmdp::testInString(debugStr, R"(adminDistance: 1)");
      nmdp::testInString(debugStr, R"(metric: 2)");
    }

    // ------------------------------------------------------------------------
    std::vector<std::string> testsNextVrf {
        R"(B L  0.0.0.0/0 [1/2] (source VRF n1) via 1.2.3.4, nic1 (egress VRF n2))"
      , R"(C    0.0.0.0/0 is directly connected (source VRF n1), nic2 (egress VRF n3))"
        // "one" test, two lines
      , R"(B L  0.0.0.0/0 [1/2] (source VRF n1) via 1.2.3.4, nic1 (egress VRF n2))"
      , R"(                                     via 1.2.3.5, nic2 (egress VRF n3))"
      };
    isFirst = true;
    for (const auto& test : testsNextVrf) {
      nmdo::Route out;
      BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
                , "Rule 'ipv4Route': " << test
                );
      BOOST_TEST(out.isValid());
      const auto debugStr {out.toDebugString()};
      if (isFirst) {
        nmdp::testInString(debugStr, R"(nextVrfId: n2)");
      } else {
        nmdp::testInString(debugStr, R"(nextVrfId: n3)");
      }
      isFirst = !isFirst;
    }
  }

  {// IPv6
    const auto& parserRule {tp.ipv6Route};

    std::vector<std::string> testsOk {
        R"(C    1::/64 [1/2]
                  via nic, directly connected)"
      , R"(C    1::/64 [1/2]
                  via Null0, directly connected)"
      , R"(B    1::/62 [1/2]
                  via 1::1, nic)"
      , R"(B    1::/96 [1/2]
                  via 1::1)"
      , R"(S    ::/0 [1/2]
                  via 1::1, nic)"
      , R"(B L  1::/64 [1/2] (source VRF A)
                  via nic (egress VRF A), directly connected)"
      , R"(B L  1::/64 [1/2] (source VRF A)
                  via 1::1, nic (egress VRF A))"
      };

    for (const auto& test : testsOk) {
      nmdo::Route out;
      BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
                , "Rule 'ipv6Route': " << test
                );
      const auto debugStr {out.toDebugString()};
      BOOST_TEST(out.isValid()
                , "Validity failure: " << test
                  << "\nGot: " << debugStr
                );
      if (std::string::npos != debugStr.find("outIfaceName: null0")) {
        nmdp::testInString(debugStr, "isNullRoute: true");
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(testRulesIpRouteIosOld)
{
  TestParser tp;
  tp.isIosOld = true;

  {// IPv4
    const auto& parserRule {tp.ipv4Route};

    std::vector<std::string> testsOk {
        "1.2.3.4\t1.2.3.5\t1:00\t12345\tnic1"
      , "1.2.3.4  1.2.3.5  1:00  12345  nic1"
      , " 1.2.3.4  1.2.3.5  1:00  12345  nic1   "
      , "1.2.3.4  1.2.3.5  0:00  0  nic1"
      };

    for (const auto& test : testsOk) {
      nmdo::Route out;
      BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
                , "Rule 'ipv4Route': " << test
                );
      const auto debugStr {out.toDebugString()};
      BOOST_TEST(out.isValid()
                , "Validity failure: " + test + "\n\tYielded: " + debugStr
                );
    }
  }

  {// IPv6
    const auto& parserRule {tp.ipv6Route};

    std::vector<std::string> testsOk {
        "C   1::/64 [0/0]\n\t\tvia nic1, directly connected"
      , "L   1::/64 [0/0]\n\t\tvia nic1, receive"
      , "L   1::/64 [0/0]\n\t\tvia 1::3"
      , "L   1::/64 [0/0]\n\t\tvia 2::3, nic1"
      , "L   1::/64 [0/0], tag 123\n\t\tvia 1::3"
      , "L   1::/64 [0/0]\n\t\tvia Null0"
      , "L   1::/64 [0/0]\n\t\tvia Null0, receive"
      // "one" test, two lines
      , "L   1::/64 [0/0]\n\t\tvia 1::3"
      , "      via 1::4"
      , "L   1::/64 [0/0]\n\t\tvia 2::3, nic1"
      , "      via 2::4, nic2"
      };

    for (const auto& test : testsOk) {
      nmdo::Route out;
      BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
                , "Rule 'ipv6Route': " << test
                );
      const auto debugStr {out.toDebugString()};
      BOOST_TEST(out.isValid()
                , "Validity failure: " + test + "\n\tYielded: " + debugStr
                );

      if (std::string::npos != test.find("via nic1")) {
        nmdp::testInString(debugStr, "outIfaceName: nic1,");
      }
      if (std::string::npos != test.find("via nic2")) {
        nmdp::testInString(debugStr, "outIfaceName: nic2,");
      }
      if (std::string::npos != test.find("via Null0")) {
        nmdp::testInString(debugStr, "outIfaceName: null0,");
      }
      if (std::string::npos != test.find("via 1::")) {
        nmdp::testInString(debugStr, "outIfaceName: ,");
      }
      BOOST_TEST(nmdo::ToolObservations() == tp.curObservations);
    }
  }
}

BOOST_AUTO_TEST_CASE(testRulesIpRouteNxos)
{
  TestParser tp;
  tp.isNxos = true;

  {// IPv4
    const auto& parserRule {tp.ipv4Route};

    std::vector<std::string> testsOk {
        R"(1.2.3.0/24, ubest/mbest: 1/0
              *via 1.2.3.1, [1/2], 1w2d, static)"
      , R"(1.2.3.0/24, ubest/mbest: 1/0
              *via 1.2.3.1, nic, [1/2], 1w2d, static)"
      , R"(1.2.3.0/24, ubest/mbest: 1/0, attached
              *via 1.2.3.1, nic, [1/2], 1w2d, direct)"
      , R"(1.2.3.0/24, ubest/mbest: 1/0
              *via 1.2.3.1, nic, [1/2], 1w2d, eigrp-100, external, tag 1)"
      // "one" test, three lines
      , R"(1.2.3.255/32, ubest/mbest: 2/0, attached
              *via 1.2.3.1, nic, [0/0], 1w2d, local)"
      , R"(   *via 1.2.3.2, nic, [0/0], 1w2d, direct)"
      , R"(1.2.3.0/24, ubest/mbest: 1/0
              *via Null0, [1/2], 1w2d, ospfv3-100, discard)"
      , R"(1.2.3.0/24, ubest/mbest: 1/0
              *via nic, [1/2], 1w2d, ospfv3-1, intra)"
      // routes via internal route; same config
      , R"(1.2.3.0/24, ubest/mbest: 1/0
              *via 1.2.3.4, [1/2], 1w2d, bgp-123, internal, tag 123)"
      // routes via internal route; possible other config
      , R"(1.2.3.0/24, ubest/mbest: 1/0
              *via 1.2.3.4%default, [1/2], 1d2h, bgp-123, external, tag 123 (mpls-vpn))"
      };
    for (const auto& test : testsOk) {
      nmdo::Route out;
      BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
                , "Rule 'ipv4Route': " << test
                );
      const auto debugStr {out.toDebugString()};
      BOOST_TEST(out.isValid()
                , "Validity failure: " + test + "\n\tYielded: " + debugStr
                );
      if (std::string::npos != debugStr.find("outIfaceName: null0")) {
        nmdp::testInString(debugStr, "isNullRoute: true");
      }
      if (std::string::npos != test.find("%default,")) {
        nmdp::testInString(debugStr, "nextVrfId: default,");
      }
    }
  }

  {// IPv6
    const auto& parserRule {tp.ipv6Route};

    std::vector<std::string> testsOk {
        R"(1::/64, ubest/mbest: 1/0
              *via 1::1/64, nic, [1/2], 1w2d, static)"
      , R"(1::/64, ubest/mbest: 1/0, attached
              *via 1::1/64, nic, [1/2], 1w2d, direct)"
      , R"(1::/64, ubest/mbest: 1/0
              *via 1::1/64, nic, [1/2], 1w2d, eigrp-100, external, tag 1)"
      , R"(1::/48, ubest/mbest: 1/0
              *via Null0, [1/2], 1w2d, ospfv3-100, discard)"
      , R"(1::/48, ubest/mbest: 1/0
              *via nic, [1/2], 1w2d, ospfv3-1, intra)"
      // routes via internal route; same config
      , R"(1::/128, ubest/mbest: 1/0
              *via 1::1/128, [1/2], 1w2d, bgp-123, internal, tag 123)"
      // routes via internal route; other config
      , R"(1::/128, ubest/mbest: 1/0
              *via ::f:1.2.3.4%default:IPv4, [1/2], 1d2h, bgp-123, external, tag 123 (mpls-vpn))"
      };

    for (const auto& test : testsOk) {
      nmdo::Route out;
      BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
                , "Rule 'ipv6Route': " << test
                );
      const auto debugStr {out.toDebugString()};
      BOOST_TEST(out.isValid()
                , "Validity failure: " << test
                  << "\nGot: " << debugStr
                );
      if (std::string::npos != debugStr.find("outIfaceName: null0")) {
        nmdp::testInString(debugStr, "isNullRoute: true");
      }
      if (std::string::npos != test.find("%default,")) {
        nmdp::testInString(debugStr, "nextVrfId: default,");
      }
      if (std::string::npos != test.find("%default:IPv4,")) {
        nmdp::testInString(debugStr, "nextVrfId: default,");
        nmdp::testInString(debugStr, "nextTableId: IPv4,");
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(testUnsupported)
{
  TestParser tp;

  tp.isIos = true;
  { // IPv4

    const auto& parserRule {tp.ipv4Route};

    std::vector<std::string> testsUnsupportedInvalid {
        R"(C    net-list 255.255.255.0 is directly connected, nic)"
      , R"(B    net-list 255.255.255.0 [1/2] via 1.2.3.4, 01:02:03, nic)"
      , R"(B    net-list 255.255.255.0 [1/2] via rtr-alias, nic)"
      };

    for (const auto& test : testsUnsupportedInvalid) {
      nmdo::Route out;
      BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
                , "Rule 'ipv4Route': " << test
                );
      const auto debugStr {out.toDebugString()};
      BOOST_TEST( !out.isValid()
                , "Expected invalid Route from: " << test
                  << "\nGot: " << debugStr);
    }

    std::vector<std::string> testsUnsupportedValid {
        R"(B    1.2.3.4 255.255.255.0 [1/2] via rtr-alias, nic)"
      };

    for (const auto& test : testsUnsupportedValid) {
      nmdo::Route out;
      BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
                , "Rule 'ipv4Route': " << test
                );
      const auto debugStr {out.toDebugString()};
      BOOST_TEST( out.isValid()
                , "Expected valid Route from: " << test
                  << "\nGot: " << debugStr);
    }

    const auto debugStr {tp.curObservations.toDebugString()};
    nmdp::testInString(debugStr, "Subnet alias -- net-list");
    nmdp::testInString(debugStr, "Router alias 'rtr-alias' on interface 'nic'");
  }
  tp.isIos = false;
}

BOOST_AUTO_TEST_CASE(testWhole)
{
  { // IOS like
    TestParser tp;
    const auto& parserRule {tp.start};

    std::string test;
    {// IOS IPv4
      test = R"(
Codes: A - some1, B - some2
       C - some3

Gateway of last resort ABC to network 0.0.0.0

A    1.2.3.0 255.255.255.0 [1/0] via 1.2.3.1, out
A    1.2.4.0/24 [1/0] via 1.2.4.1, out
C    0.0.0.0 0.0.0.0 [255/0] via ABC
B    1.2.5.0 255.255.0.0
        [1/0] via 1.2.5.1, 12:34:56, in
        [1/0] via 1.2.5.2, 12:34:56, in
A    1.2.6.0 255.255.255.0 [1/0] via 1.2.6.1, 12:34:56, in
                         [1/0] via 1.2.6.2, 12:34:56, in
A    net-123 255.255.255.0 is directly connected, unsup
)";

      Result out;
      BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
                , "Rule 'testWhole': " << test
                );
      BOOST_TEST(1 == out.size());
      for (const auto& vrf : out) {
        BOOST_TEST(8 == vrf.routes.size());
        for (const auto& route : vrf.routes) {
          const auto debugStr {route.toDebugString()};
          if (std::string::npos != debugStr.find("outIfaceName: unsup")) {
            BOOST_TEST(!route.isValid()
                      , "Incorrect valid route: " << debugStr);
          } else {
            BOOST_TEST(route.isValid()
                      , "Incorrect invalid route: " << debugStr);
          }
        }
        const auto debugStr {vrf.observations.toDebugString()};
        nmdp::testInString(debugStr, "Subnet alias -- net-123");
      }
    }
    {// IOS old IPv4
      test = R"(
Default gateway is 1.2.3.4

Host               Gateway           Last Use    Total Uses  Interface
1.2.3.5            1.2.3.6           0:00        12345       nic1
)";

      Result out;
      BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
                , "Rule 'testWhole': " << test
                );
      BOOST_TEST(1 == out.size());
      for (const auto& vrf : out) {
        BOOST_TEST(1 == vrf.routes.size());
        for (const auto& route : vrf.routes) {
          const auto debugStr {route.toDebugString()};
          BOOST_TEST(route.isValid()
                    , "Incorrect invalid route: " << debugStr);
        }
      }
    }
    {// IOS old IPv4
      test = R"(
Default gateway is 1.2.3.4

Host               Gateway           Last Use    Total Uses  Interface
ICMP redirect cache is empty
)";

      Result out;
      BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
                , "Rule 'testWhole': " << test
                );
      BOOST_TEST_REQUIRE(1 == out.size());
      BOOST_TEST_REQUIRE(0 == out[0].routes.size());
      BOOST_TEST(nmdo::ToolObservations() == out[0].observations);
    }
    {// IOS old IPv6
      // same as IOS
      test = R"(
IPv6 Routing Table - default - 3 entries
Codes: C - Connected, L - Local, S - Static, U - Per-user Static route
       R - RIP, ND - ND Default, NDp - ND Prefix, DCE - Destination
       NDr - Redirect
C   0::/127 [0/0]
     via nic0, directly connected
L   1::/64 [0/0]
     via nic1, receive
B   2::/64 [0/0]
     via 1::1
     via 1::2
B   2::/64 [0/0]
     via 1::1, nic1
     via 1::2, nic2
L   3::/8 [0/0]
     via Null0, receive
)";
      Result out;
      BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
                , "Rule 'testWhole': " << test
                );
      BOOST_TEST(1 == out.size());
      for (const auto& vrf : out) {
        BOOST_TEST(7 == vrf.routes.size());
        for (const auto& route : vrf.routes) {
          BOOST_TEST(route.isValid());
        }
      }
      BOOST_TEST(nmdo::ToolObservations() == out[0].observations);
    }
  }
  { // NXOS like
    TestParser tp;
    const auto& parserRule {tp.start};

    std::string test;
    { // IPv4
      test =
        R"(IP Route Table for VRF "default"
           '*' denotes best ucast next-hop
           '**' denotes best mcast next-hop
           '[x/y]' denotes [preference/metric]
           '%<string>' in via output denotes VRF <string>

           0.0.0.0/0, ubest/mbest: 1/0
               *via 1.2.3.1, nic, [1/2], 1w2d, eigrp-100, external, tag 1
           1.2.3.0/24, ubest/mbest: 1/0, attached
               *via 1.2.3.1, Null0, [1/2], 1w2d, discard, discard
           1.2.4.1/32, ubest/mbest: 1/0
               *via 1.2.4.1, nic, [1/2], 1w2d, static
           1.2.4.2/32, ubest/mbest: 1/0
               *via 1.2.4.2, nic, [1/2], 1w2d, static
               *via 1.2.4.2, nic, [1/2], 1w2d, static
        )";

      Result out;
      BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
                , "Rule 'testWhole': " << test
                );
      BOOST_TEST(1 == out.size());
      for (const auto& vrf : out) {
        BOOST_TEST(5 == vrf.routes.size());
        for (const auto& route : vrf.routes) {
          BOOST_TEST(route.isValid());
        }
      }
    }
    { // IPv6
      test =
R"(IPv6 Routing Table for VRF "default"
   '*' denotes best ucast next-hop
   '**' denotes best mcast next-hop
   '[x/y]' denotes [preference/metric]
   0::/127, ubest/mbest: 1/0
       *via 0::, Null0, [1/2], 1w2d, discard, discard
   1::/64, ubest/mbest: 1/0
       *via 1::1, nic, [1/2], 12:34:56, direct
   2::/64, ubest/mbest: 1/0
       *via 2::1, nic, [1/2], 1w2d, eigrp-100, external, tag 1
   2::/64, ubest/mbest: 1/0
       *via 2::1, nic1, [1/2], 1w2d, eigrp-100, external, tag 1
       *via 2::1, nic2, [1/2], 1w2d, eigrp-100, external, tag 1
)";

      Result out;
      BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
                , "Rule 'testWhole': " << test
                );
      BOOST_TEST(1 == out.size());
      for (const auto& vrf : out) {
        BOOST_TEST(5 == vrf.routes.size());
        for (const auto& route : vrf.routes) {
          BOOST_TEST(route.isValid());
        }
      }
    }
  }
  {
    TestParser tp;
    const auto& parserRule {tp.start};

    std::string test;

    // Multi-VRF and garbage
    test = R"(garbage at the start

VRF name: default
Displaying some other stuff
Codes: A - some1, B - some2
       C - some3

Gateway of last resort ABC to network 0.0.0.0

C    0.0.0.0/0 [255/0] via 1.2.3.1, out

VRF: no-space-and-space
Codes: A - some1, B - some2
       C - some3

Gateway of last resort ABC to network 0.0.0.0
C    0.0.0.0/0 [255/0] via 1.2.3.1, out

C    0.0.0.0/0 [255/0] via 1.2.3.1, out

garbage in the middle

VRF name: empty
Codes: A - some1, B - some2
       C - some3

Gateway of last resort ABC to network 0.0.0.0


VRF name: ipv6
IPv6 Routing Table some other stuff
Codes: A - some1, B - some2
       C - some3

Gateway of last resort ABC to network 0.0.0.0

C    ::/0 [255/0] via 1::2, out

garbage at the end
)";

    Result out;
    std::string debugStr;
    BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
              , "Rule 'testWhole': " << test
              );
    BOOST_TEST_REQUIRE(4 == out.size());
    BOOST_TEST_REQUIRE(1 == out[0].routes.size());
    debugStr = out[0].routes[0].toDebugString();
    nmdp::testInString(debugStr, "vrfId: ,");
    BOOST_TEST_REQUIRE(2 == out[1].routes.size());
    debugStr = out[1].routes[0].toDebugString();
    nmdp::testInString(debugStr, "vrfId: no-space-and-space,");
    BOOST_TEST(0 == out[2].routes.size());
    BOOST_TEST_REQUIRE(1 == out[3].routes.size());
    debugStr = out[3].routes[0].toDebugString();
    nmdp::testInString(debugStr, "vrfId: ipv6,");
    for (const auto& vrf : out) {
      for (const auto& route : vrf.routes) {
        BOOST_TEST(route.isValid(), route.toDebugString());
      }
    }
  }
}
