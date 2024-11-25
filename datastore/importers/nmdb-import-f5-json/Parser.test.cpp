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
    using Parser::data;

    using Parser::fromJson;
    using Parser::parseIpAddrVrfStr;
    using Parser::parseLtmVirtualAddress;
    using Parser::parseNetArpNdp;
    using Parser::parseNetRoute;
    using Parser::parseNetSelf;
    using Parser::parseUnsupported;
};

BOOST_AUTO_TEST_CASE(testParseIpAddrVrfStr)
{
  TestParser tp;

  const std::vector<std::tuple<std::string, std::string, std::string>> tests {
      {"1.2.3.4", "1.2.3.4", ""}
    , {"1.2.3.4%123", "1.2.3.4", "123"}
    , {"1.2.3.4%abc", "1.2.3.4", "abc"}
    , {"1.2.3.4%abc/24", "1.2.3.4/24", "abc"}
    , {"::1%123", "::1", "123"}
    , {"default", "0.0.0.0/0", ""}
    , {"default-inet6", "::/0", ""}
    , {"any", "0.0.0.0", ""}
    , {"any6", "::", ""}
    };

  for (const auto& [test, eIp, eVrf] : tests) {
    const auto [oIp, oVrf] {tp.parseIpAddrVrfStr(test)};

    nmdo::IpAddress temp {eIp};
    BOOST_TEST(temp == oIp);
    if (eIp.starts_with("0.0.0.0") || eIp.starts_with("::")) {
      BOOST_TEST(!temp.isValid(), "valid IpAddress -- " << eIp);
    } else {
      BOOST_TEST(temp.isValid(), "invalid IpAddress -- " << eIp);
    }
    BOOST_TEST(eVrf == oVrf);
  }
}

BOOST_AUTO_TEST_CASE(testParseNetRoute)
{
  TestParser tp;

  /* Example known fields, not collected
      - "kind":"tm:net:route:routestate"
      - "generation":1
      - "selfLink":"https://<ip-name>/mgmt/tm/net/route/<path>?ver=1.2"
      - "mtu":0
  */
  const auto test = json::parse(R"(
      {
        "items": [
          { "name": "rt-1"
          , "partition": "abc"
          , "fullPath": "/abc/rt-1"
          , "network": "1.2.3.0/24"
          , "gw": "1.2.3.1"
          }
        ]
      }
    )");

  tp.parseNetRoute(test);

  BOOST_TEST_REQUIRE(tp.data.logicalSystems.contains("abc"));
  const auto& out {tp.data.logicalSystems.at("abc")};

  BOOST_TEST_REQUIRE(out.vrfs.contains(""));
  BOOST_TEST(!out.vrfs.at("").isValid());
  auto dbgStr {out.vrfs.at("").toDebugString()};
  nmdp::testInString(dbgStr, "tableId: rt-1,");
  nmdp::testInString(dbgStr, "description: /abc/rt-1]");
  nmdp::testInString(dbgStr, "dstIpNet: [ipNetwork: 1.2.3.0/24,");
  nmdp::testInString(dbgStr, "nextHopIpAddr: [ipAddress: 1.2.3.1/32,");
  nmdp::testInString(dbgStr, "routes: [[vrfId: ,");
  nmdp::testInString(dbgStr, "[vrfId: , ifaces:");
}

BOOST_AUTO_TEST_CASE(testParseNetSelf)
{
  TestParser tp;

  /* Example known fields, not collected
      - "kind":"tm:net:self:selfstate"
      - "generation":177
      - "selfLink":"https://<ip-name>/mgmt/tm/net/self/<path>?ver=1.2"
      - "floating":"disabled"
      - "inheritedTrafficGroup":"false"
      - "trafficGroup":"<path>"
      - "unit":0
      - "vlan":"<path>"
      - "allowService":["<name>"]
  */
  const auto test = json::parse(R"(
      {
        "items": [
          { "name": "iface-1"
          , "partition": "abc"
          , "fullPath": "/abc/iface-1"
          , "address": "1.2.3.4/24"
          }
        ]
      }
    )");

  tp.parseNetSelf(test);

  BOOST_TEST_REQUIRE(tp.data.logicalSystems.contains("abc"));
  const auto& out {tp.data.logicalSystems.at("abc")};

  BOOST_TEST_REQUIRE(out.ifaces.contains("iface-1"));
  BOOST_TEST(out.ifaces.at("iface-1").isValid());
  auto dbgStr {out.ifaces.at("iface-1").toDebugString()};
  nmdp::testInString(dbgStr, "name: iface-1,");
  nmdp::testInString(dbgStr, "description: /abc/iface-1,");
  nmdp::testInString(dbgStr, "ipAddress: 1.2.3.4/24,");

  BOOST_TEST_REQUIRE(out.vrfs.contains(""));
  BOOST_TEST(!out.vrfs.at("").isValid());
  dbgStr = out.vrfs.at("").toDebugString();
  nmdp::testInString(dbgStr, "[vrfId: , ifaces: [iface-1]");
}

BOOST_AUTO_TEST_CASE(testParseNetArpNdp)
{
  TestParser tp;

  /* Example known fields, not collected
      - "kind":"tm:net:arp:arpstate"
      - "kind":"tm:net:ndp:ndpstate"
      - "generation":177
      - "selfLink":"https://<ip-name>/mgmt/tm/net/arp/<path>?ver=1.2"
      - "selfLink":"https://<ip-name>/mgmt/tm/net/ndp/<path>?ver=1.2"
  */
  const auto test = json::parse(R"(
      {
        "items": [
          { "name": "iface-1"
          , "partition": "abc"
          , "fullPath": "/abc/iface-1"
          , "ipAddress": "1.2.3.4"
          , "macAddress": "00:11:22:33:44:55"
          }
        ]
      }
    )");

  tp.parseNetArpNdp(test);

  BOOST_TEST_REQUIRE(tp.data.logicalSystems.contains("abc"));
  const auto& out {tp.data.logicalSystems.at("abc")};

  BOOST_TEST_REQUIRE(out.ifaces.contains("iface-1"));
  BOOST_TEST(out.ifaces.at("iface-1").isValid());
  auto dbgStr {out.ifaces.at("iface-1").toDebugString()};
  nmdp::testInString(dbgStr, "name: iface-1,");
  nmdp::testInString(dbgStr, "description: /abc/iface-1,");
  nmdp::testInString(dbgStr, "[[macAddress: 00:11:22:33:44:55");
  nmdp::testInString(dbgStr, "isResponding: true");
  nmdp::testInString(dbgStr, "[[ipAddress: 1.2.3.4/32,");

  BOOST_TEST_REQUIRE(out.vrfs.contains(""));
  BOOST_TEST(!out.vrfs.at("").isValid());
  dbgStr = out.vrfs.at("").toDebugString();
  nmdp::testInString(dbgStr, "[vrfId: , ifaces: [iface-1]");
}

BOOST_AUTO_TEST_CASE(testParseLtmVirtualAddress)
{
  TestParser tp;

  const auto test = json::parse(R"(
      {
        "items": [
          { "name": "iface-1"
          , "partition": "abc"
          , "fullPath": "/abc/iface-1"
          , "mask": "255.255.255.0"
          , "address": "1.2.3.4"
          }
        ]
      }
    )");

  tp.parseLtmVirtualAddress(test);

  BOOST_TEST_REQUIRE(tp.data.logicalSystems.contains("abc"));
  const auto& out {tp.data.logicalSystems.at("abc")};

  BOOST_TEST_REQUIRE(out.ifaces.contains("iface-1"));
  BOOST_TEST(out.ifaces.at("iface-1").isValid());
  auto dbgStr {out.ifaces.at("iface-1").toDebugString()};
  nmdp::testInString(dbgStr, "name: iface-1,");
  nmdp::testInString(dbgStr, "description: /abc/iface-1,");
  nmdp::testInString(dbgStr, "ipAddress: 1.2.3.4/24,");

  BOOST_TEST_REQUIRE(out.vrfs.contains(""));
  BOOST_TEST(!out.vrfs.at("").isValid());
  dbgStr = out.vrfs.at("").toDebugString();
  nmdp::testInString(dbgStr, "[vrfId: , ifaces: [iface-1]");
}

BOOST_AUTO_TEST_CASE(testWhole)
{
  TestParser tp;

  const std::vector<std::string> tests {
      "tm::abc"
    , "tm:ltm:virtual-address:virtual-addresscollectionstate"
    , "tm:net:arp:arpcollectionstate"
    , "tm:net:ndp:ndpcollectionstate"
    , "tm:net:self:selfcollectionstate"
    , "tm:net:route:routecollectionstate"
    };

  json j;

  // add targeted data so sub-parsing does not error out
  json item;
  item["partition"] = "a1";
  item["name"] = "a1";
  item["fullPath"] = "/a1/a1";
  item["mask"] = "255.255.255.0";
  item["address"] = "1.2.3.4";
  item["ipAddress"] = "1.2.3.4/24";
  item["macAddress"] = "00:11:22:33:44:55";
  item["network"] = "1.2.3.0/24";
  item["gw"] = "1.2.3.1";
//  item[] = "";
  j["items"].push_back(item);

  for (const auto& test : tests) {
    j["kind"] = test;
    tp.fromJson(j);
  }

  BOOST_TEST(1 == tp.data.logicalSystems.size());
  BOOST_TEST_REQUIRE(tp.data.logicalSystems.contains("a1"));
  const auto& out {tp.data.logicalSystems.at("a1")};

  // not checking everything, that is done in other unit tests
  BOOST_TEST(1 == out.ifaces.size());
  for (const auto& [_, iface] : out.ifaces) {
    nmdp::testInString( iface.toDebugString()
                      , "[name: a1, description: /a1/a1,"
                      );
  }
  BOOST_TEST(1 == out.vrfs.size());
  for (const auto& [_, vrf] : out.vrfs) {
    nmdp::testInString( vrf.toDebugString()
                      , "routes: [[vrfId: , tableId: a1,"
                      );
  }
  nmdp::testInString( tp.data.observations.toDebugString()
                    , "unsupportedFeatures: [tm::abc]]"
                    );
}
