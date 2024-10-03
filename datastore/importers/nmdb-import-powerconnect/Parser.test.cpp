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

namespace nmdp = netmeld::datastore::parsers;

using qi::ascii::blank;

class TestParser : public Parser
{
  public:
    using Parser::d;

    using Parser::start;
    using Parser::hostname;
    using Parser::interface;
};

BOOST_AUTO_TEST_CASE(testHostname)
{
  TestParser tp;
  const auto& parserRule {tp.hostname};

  std::vector<std::string> testsOk {
      "hostname name\n"
    , "hostname NAME\n"
    , "hostname \"name\"\n"
  };

  for (const auto& test : testsOk) {
    BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
              , "Parse rule 'hostname': " << test
              );
    const auto& out {tp.d.devInfo.toDebugString()};
    nmdp::testInString(out, "id: name,");
    nmdp::testInString(out, "vendor: ,");
  }
}

BOOST_AUTO_TEST_CASE(testInterface)
{
  TestParser tp;
  const auto& parserRule {tp.interface};

  std::string test;
  std::string out;

  {
    test = "interface vlan 123\n"
           "exit\n"
      ;
    BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
              , "Parse rule 'interface': " << test
              );
    BOOST_TEST_REQUIRE(1 == tp.d.ifaces.size());
    auto t1 {tp.d.ifaces["vlan123"]};
    BOOST_TEST(t1.isValid());
    out = t1.toDebugString();
    nmdp::testInString(out, "name: vlan123,");
    nmdp::testInString(out, "mediaType: vlan");
    nmdp::testInString(out, "isUp: true");
    nmdp::testInString(out, "mode: l2 access");
    nmdp::testInString(out, "ipAddrs: [],");
    BOOST_TEST(0 == tp.d.routes.size());
  }
  {
    test = "interface vlan 123 other\n"
           "ip address 1.2.3.4 255.255.255.0\n"
           "switchport mode spmode\n"
           "shutdown\n"
           "random other 123 data\n"
           "exit\n"
      ;
    BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
              , "Parse rule 'interface': " << test
              );
    BOOST_TEST_REQUIRE(1 == tp.d.ifaces.size());
    auto t1 {tp.d.ifaces["vlan123"]};
    BOOST_TEST(t1.isValid());
    out = t1.toDebugString();
    nmdp::testInString(out, "name: vlan123,");
    nmdp::testInString(out, "mediaType: vlan");
    nmdp::testInString(out, "isUp: false");
    nmdp::testInString(out, "mode: l2 spmode");
    nmdp::testInString(out, "ipAddress: 1.2.3.4/24,");
    BOOST_TEST(0 == tp.d.routes.size());
  }
  {
    test = "interface random 1/23/45.a\n"
           "ip address 1.2.3.4 255.255.255.0 1.2.3.1\n"
           "exit\n"
      ;
    BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
              , "Parse rule 'interface': " << test
              );
    BOOST_TEST_REQUIRE(2 == tp.d.ifaces.size());
    auto t1 {tp.d.ifaces["random1/23/45.a"]};
    BOOST_TEST(t1.isValid());
    out = t1.toDebugString();
    nmdp::testInString(out, "name: random1/23/45.a,");
    nmdp::testInString(out, "mediaType: random");
    nmdp::testInString(out, "isUp: true");
    nmdp::testInString(out, "mode: l2 access");
    nmdp::testInString(out, "ipAddress: 1.2.3.4/24,");
    BOOST_TEST_REQUIRE(1 == tp.d.routes.size());
    auto t2 {tp.d.routes["random1/23/45.a"]};
    BOOST_TEST(t2.isValid());
    out = t2.toDebugString();
    nmdp::testInString(out, "ipNetwork: 1.2.3.0/24,");
    nmdp::testInString(out, "ipAddress: 1.2.3.1/32,");
  }
}
