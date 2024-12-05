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
    using Parser::start;
    using Parser::distanceMetric;
    using Parser::ifaceName;
    using Parser::ipv4Route;
    using Parser::ipv6Route;
    using Parser::typeCode;

    // Not checking as flow only or simplistic and checked with other rule
    //using Parser::rowNumber;
    //using Parser::uptime;
    //using Parser::srcVrf;
    //using Parser::ignoredLine;
    //using Parser::dstIpv4Net;
    //using Parser::dstIpv6Net;
    //using Parser::rtrIpv4Addr;
    //using Parser::rtrIpv6Addr;
};

BOOST_AUTO_TEST_CASE(testRuleIfaceName)
{
  TestParser tp;
  const auto& parserRule {tp.ifaceName};

  std::vector<std::string> testsOk {
      "dev1"
    , "dev 1"
    , "dev\t1"
    , "dev 1/23/456.789"
    };

  for (const auto& test : testsOk) {
    nmdo::Route ctrl
              , out
              ;
    ctrl.setOutIfaceName(test);

    BOOST_TEST(ctrl != out);
    // NOTE: pnx::ref is needed to handle ref binding/semantic action logic
    BOOST_TEST( nmdp::test(test.c_str(), parserRule(pnx::ref(out)), blank)
              , "Rule 'ifaceName': " << test
              );
    BOOST_TEST(ctrl == out);
  }
}

BOOST_AUTO_TEST_CASE(testRuleDistanceMetric)
{
  TestParser tp;
  const auto& parserRule {tp.distanceMetric};

  std::vector<std::tuple<std::string, size_t, size_t>> testsOk {
      {"1/1", 1, 1}
    , {"123456/78901234", 123456, 78901234}
    };

  for (const auto& [ test, c1, c2 ] : testsOk) {
    nmdo::Route ctrl
              , out
              ;
    ctrl.setAdminDistance(c1);
    ctrl.setMetric(c2);

    BOOST_TEST(ctrl != out);
    // NOTE: pnx::ref is needed to handle ref binding/semantic action logic
    BOOST_TEST( nmdp::test(test.c_str(), parserRule(pnx::ref(out)), blank)
              , "Rule 'distanceMetric': " << test
              );
    BOOST_TEST(ctrl == out);
  }
}

BOOST_AUTO_TEST_CASE(testRuleTypeCode)
{
  TestParser tp;
  const auto& parserRule {tp.typeCode};

  std::vector<std::tuple<std::string, std::string>> testsOk {
      {"B", "bgp"}
    , {"C", "direct"} // connected IPv6
    , {"D", "direct"} // connected IPv4
    , {"I", "is-is"}
    , {"L", "local"}
    , {"O", "ospf"}
    , {"R", "rip"}
    , {"S", "static"}
    , {"a1b2", "a1b2"}
    };

  for (const auto& [ test, c1 ] : testsOk) {
    nmdo::Route ctrl
              , out
              ;
    ctrl.setProtocol(c1);

    BOOST_TEST(ctrl != out);
    // NOTE: pnx::ref is needed to handle ref binding/semantic action logic
    BOOST_TEST( nmdp::test(test.c_str(), parserRule(pnx::ref(out)), blank)
              , "Rule 'ifaceName': " << test
              );
    BOOST_TEST(ctrl == out);
  }
}

BOOST_AUTO_TEST_CASE(testRuleIpv4Route)
{
  TestParser tp;
  const auto& parserRule {tp.ipv4Route};
  // also tests dstIpv4Net
  // also tests rtrIpv4Addr

  std::vector<std::string> tests {
      "0.0.0.0/0\t1.2.3.1\tdev 1\t1/1\tS\t43d21h\n"
    , "0.0.0.0/0  1.2.3.1  dev 1  1/1  S  43d21h\n"
    , "123  0.0.0.0/0  1.2.3.1  dev 1  1/1  S  43d21h\n"
    , "0.0.0.0/0  1.2.3.1  dev 1  1/1  S  43d21h vrfId\n"
    };

  for (const auto& test : tests) {
    nmdo::Route out;
    BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
              , "Rule 'ipv4Route': " << test
              );
    BOOST_TEST_REQUIRE(out.isValid());
    auto dbgStr {out.toDebugString()};
    nmdp::testInString(dbgStr, "ipNetwork: 0.0.0.0/0");
    nmdp::testInString(dbgStr, "ipAddress: 1.2.3.1/32");
    nmdp::testInString(dbgStr, "outIfaceName: dev 1,");
    nmdp::testInString(dbgStr, "adminDistance: 1,");
    nmdp::testInString(dbgStr, "metric: 1,");
    nmdp::testInString(dbgStr, "protocol: static,");
    nmdp::testInString(dbgStr, "vrfId: ,");
  }

  std::string test;
  {
    test = "1.2.3.0/23  1.2.3.1  dev1  1/1  prot  43d21h\n";
    nmdo::Route out;
    BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
              , "Rule 'ipv4Route': " << test
              );
    BOOST_TEST_REQUIRE(out.isValid());
    auto dbgStr {out.toDebugString()};
    nmdp::testInString(dbgStr, "ipNetwork: 1.2.2.0/23");
    nmdp::testInString(dbgStr, "ipAddress: 1.2.3.1/32");
    nmdp::testInString(dbgStr, "outIfaceName: dev1,");
    nmdp::testInString(dbgStr, "adminDistance: 1,");
    nmdp::testInString(dbgStr, "metric: 1,");
    nmdp::testInString(dbgStr, "protocol: prot,");
  }
  {
    test = "1.2.3.0/24  DIRECT  dev1  1/1  C  43d21h\n";
    nmdo::Route out;
    BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
              , "Rule 'ipv4Route': " << test
              );
    BOOST_TEST_REQUIRE(out.isValid());
    auto dbgStr {out.toDebugString()};
    nmdp::testInString(dbgStr, "ipNetwork: 1.2.3.0/24");
    nmdp::testInString(dbgStr, "ipAddress: 0.0.0.0/255");
    nmdp::testInString(dbgStr, "outIfaceName: dev1,");
    nmdp::testInString(dbgStr, "adminDistance: 1,");
    nmdp::testInString(dbgStr, "metric: 1,");
    nmdp::testInString(dbgStr, "protocol: direct,");
  }
}

BOOST_AUTO_TEST_CASE(testRuleIpv6Route)
{
  TestParser tp;
  const auto& parserRule {tp.ipv6Route};
  // also tests dstIpv6Net
  // also tests rtrIpv6Addr

  std::vector<std::string> tests {
      "S\t::/0\t2::1\tdev 1\t1/1\t43d21h\n"
    , "S  ::/0  2::1  dev 1  1/1  43d21h\n"
    , "S  ::/0\n2::1\ndev 1  1/1  43d21h\n"
    , "S  ::/0\n  2::1\n  dev 1  1/1  43d21h\n"
    };

  for (const auto& test : tests) {
    nmdo::Route out;
    BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
              , "Rule 'ipv6Route': " << test
              );
    BOOST_TEST_REQUIRE(out.isValid());
    auto dbgStr {out.toDebugString()};
    nmdp::testInString(dbgStr, "ipNetwork: ::/0");
    nmdp::testInString(dbgStr, "ipAddress: 2::1/128");
    nmdp::testInString(dbgStr, "outIfaceName: dev 1,");
    nmdp::testInString(dbgStr, "adminDistance: 1,");
    nmdp::testInString(dbgStr, "metric: 1,");
    nmdp::testInString(dbgStr, "protocol: static,");
    nmdp::testInString(dbgStr, "vrfId: ,");
  }

  std::string test;
  {
    test = "prot  1::3:0/111  1::1  dev1  1/1  43d21h\n";
    nmdo::Route out;
    BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
              , "Rule 'ipv6Route': " << test
              );
    BOOST_TEST_REQUIRE(out.isValid());
    auto dbgStr {out.toDebugString()};
    nmdp::testInString(dbgStr, "ipNetwork: 1::2:0/111");
    nmdp::testInString(dbgStr, "ipAddress: 1::1/128");
    nmdp::testInString(dbgStr, "outIfaceName: dev1,");
    nmdp::testInString(dbgStr, "adminDistance: 1,");
    nmdp::testInString(dbgStr, "metric: 1,");
    nmdp::testInString(dbgStr, "protocol: prot,");
  }
  {
    test = "D  1::0/64  ::  dev1  1/1  43d21h\n";
    nmdo::Route out;
    BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
              , "Rule 'ipv6Route': " << test
              );
    BOOST_TEST_REQUIRE(out.isValid());
    auto dbgStr {out.toDebugString()};
    nmdp::testInString(dbgStr, "ipNetwork: 1::/64");
    nmdp::testInString(dbgStr, "ipAddress: 0.0.0.0/255");
    nmdp::testInString(dbgStr, "outIfaceName: dev1,");
    nmdp::testInString(dbgStr, "adminDistance: 1,");
    nmdp::testInString(dbgStr, "metric: 1,");
    nmdp::testInString(dbgStr, "protocol: direct,");
  }
}

BOOST_AUTO_TEST_CASE(testGrammar)
{
  TestParser tp;
  const auto& parserRule {tp.start};

  { // IPv4
    std::string test {
      "Total number of IP routes: 3\n"
      "Type Codes - B:BGP D:Connected O:OSPF S:Static; Cost - Dist/Metric\n"
      "BGP Codes - i:iBGP e:eBGP\n"
      "OSPF Codes - i:Inter Area 1:External Type 1 2:External Type 2 s:Sham Link\n"
      "        Destination        Gateway         Port           Cost          Type Uptime\n"
      "        0.0.0.0/0          1.2.3.4         mgmt 1         1/1           S    1d12h\n"
      "        1.2.3.0/24         DIRECT          mgmt 1         0/0           D    1d23h\n"
      "        1.2.4.1/32         DIRECT          mgmt 1         0/0           D    1d01h\n"
      };

    Result out;
    BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
              , "Grammar: " << test
              );
    BOOST_TEST_REQUIRE(3 == out.size());
  }

  { // IPv6
    // no examples
  }
}
