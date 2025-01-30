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

struct TestParser : public Parser
{
  public:
    using Parser::d;

    using Parser::phyIface;
    using Parser::config;
    using Parser::start;
};

BOOST_AUTO_TEST_CASE(testPhyIface)
{
  TestParser tp;
  const auto& parserRule {tp.phyIface};

  const std::vector<std::string> testsOk {
      "interface address abc123 1.2.3.4 255.255.255.0"
    , "interface address abc123 1.2.3.4 255.255.255.0 1.2.3.1"
    , "interface address dynamic-interface abc123 1.2.3.4 255.255.255.0 1.2.3.1"
    };

  for (const auto& test : testsOk) {
    BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
              , "Parse rule 'phyIface': " << test
              );
  }

  BOOST_TEST_REQUIRE(3 == tp.d.ifaces.size());
  for (const auto& iface : tp.d.ifaces) {
    BOOST_TEST(iface.isValid());

    const auto& dbgStr {iface.toDebugString()};
    nmdp::testInString(dbgStr, "name: abc123,");
    nmdp::testInString(dbgStr, "ipAddress: 1.2.3.4/24,");
    nmdp::testInString(dbgStr, "isDiscoveryProtocolEnabled: true,");
    nmdp::testInString(dbgStr, "isUp: true,");
  }

  BOOST_TEST_REQUIRE(2 == tp.d.routes.size());
  for (const auto& route : tp.d.routes) {
    BOOST_TEST(route.isValid());

    const auto& dbgStr {route.toDebugString()};
    nmdp::testInString(dbgStr, "outIfaceName: abc123,");
    nmdp::testInString(dbgStr, "dstIpNet: [ipNetwork: 1.2.3.0/24,");
    nmdp::testInString(dbgStr, "nextHopIpAddr: [ipAddress: 1.2.3.1/32,");
    nmdp::testInString(dbgStr, "vrfId: ,");
    nmdp::testInString(dbgStr, "isNullRoute: false,");
  }
}

BOOST_AUTO_TEST_CASE(testConfig)
{
  TestParser tp;
  const auto& parserRule {tp.config};

  const std::string test {R"(
      sysname fqdn.domain
      abc 123
      time ntp server 123 1.2.3.1
      time abc 123
    )"};

  BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
            , "Parse rule 'config': " << test
            );

  auto dbgStr {tp.d.devInfo.toDebugString()};
  nmdp::testInString(dbgStr, "id: fqdn.domain,");

  BOOST_TEST_REQUIRE(1 == tp.d.services.size());
  dbgStr = tp.d.services[0].toDebugString();
  nmdp::testInString(dbgStr, "serviceName: ntp,");
  nmdp::testInString(dbgStr, "dstAddress: [ipAddress: 1.2.3.1/32,");
  nmdp::testInString(dbgStr, "serviceReason: fqdn.domain's config,");
}

BOOST_AUTO_TEST_CASE(testWhole)
{
  TestParser tp;
  const auto& parserRule {tp.start};

  const std::string test {R"()"};

  BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
            , "Parse rule 'start': " << test
            );

  auto dbgStr {tp.d.devInfo.toDebugString()};
  nmdp::testInString(dbgStr, "vendor: cisco,");

  BOOST_TEST(0 == tp.d.ifaces.size());
  BOOST_TEST(0 == tp.d.routes.size());
  BOOST_TEST(0 == tp.d.services.size());
}
