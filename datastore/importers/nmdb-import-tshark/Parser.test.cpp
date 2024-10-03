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

class TestParser : public Parser<nmdp::IstreamIter>
{
  public:
    // variables
    using Parser::pd;
    using Parser::processedPacketCount;
    using Parser::parsedPacketCount;

    // rules
    using Parser::start;
    using Parser::packetBlock;
    using Parser::packetArray;

    // functions
    using Parser::setPacketData;


    // helpers
    DataContainerSingleton& dcs = DataContainerSingleton::getInstance();

    void
    testStringValueAtKey(const std::string& expected, const std::string& key)
    {
      testKeyExists(key);

      const auto& actual {std::any_cast<std::string&>(pd.at(key))};
      BOOST_TEST(expected == actual);
    }

    void
    testVtsstValueAtKey(const std::string& expected, const std::string& key)
    {
      const auto& actual {std::any_cast<VecTupStrStrType&>(pd.at(key))};
      BOOST_TEST(1 == actual.size());
      for (const auto& [k,v] : actual) {
        BOOST_TEST(key == k);
        BOOST_TEST(expected == v);
      }
    }

    void
    testKeyExists(const std::string& key)
    {
      BOOST_TEST_REQUIRE( pd.contains(key)
                        , std::format("Key '{}' not found\n", key)
                        );
    }

    void
    testInTupleVector( const std::tuple<std::string, std::string>& expected
                     , const std::string& key
                     )
    {
      bool found {false};
      const auto& vec = std::any_cast<VecTupStrStrType&>(pd.at(key));
      for (const auto& i : vec) {
        if (expected == i) {
          found = true;
          break;
        }
      }
      BOOST_TEST( found
                , std::format( "Key '{}' does not contain tuple '{}','{}'\n"
                             , key
                             , std::get<0>(expected)
                             , std::get<1>(expected)
                             )
                );
    }
    void
    testInStringVector( const std::string& expected
                      , const std::string& key
                      )
    {
      bool found {false};
      const auto& vec = std::any_cast<VecStrType&>(pd.at(key));
      for (const auto& i : vec) {
        if (expected == i) {
          found = true;
          break;
        }
      }
      BOOST_TEST( found
                , std::format( "Key '{}' does not contain string '{}'\n"
                             , key
                             , expected
                             )
                );
    }
};


BOOST_AUTO_TEST_CASE(testRulePacketBlock)
{
  TestParser tp;
  const auto& parserRule {tp.packetBlock};

  const std::string test {R"({
        "abc": 123,
        "eth.src": "00:11:22:33:44:55",
        "123": "abc",
        "ip.src": "1.2.3.4",
        "ab c": "1 23"
      }
    )"};

  BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
            , "Rule 'packetBlock': " << test
            );

  BOOST_TEST(2 == tp.pd.size());
  tp.testStringValueAtKey("00:11:22:33:44:55", R"("eth.src": )");
  tp.testStringValueAtKey("1.2.3.4", R"("ip.src": )");

  BOOST_TEST(!tp.dcs.hasData());
}

// NOTE: These are intermediate in the chain, we only really care about the
//       data to save.
BOOST_AUTO_TEST_CASE(testSetPacketData)
{
  TestParser tp;

  { // if block; dns; k to tuple vector
    const std::string kDrn {R"("dns.resp.name": )"};
    const std::string vDrn {"example.com"};
    tp.setPacketData(kDrn, vDrn);
    BOOST_TEST(0 == tp.pd.size());

    const std::vector<std::tuple<std::string, std::string>> tests {
        {R"("dns.some.other": )", "value"}
      , {R"("dns.a": )", "v1"}
      };

    size_t i {0};
    for (const auto& [k,v] : tests) {
      tp.setPacketData(k, v);
      BOOST_TEST(++i == tp.pd.size());
      tp.testKeyExists(k);
      tp.testInTupleVector(std::make_tuple(vDrn, v), k);
    }

    BOOST_TEST(!tp.dcs.hasData());
    tp.pd.clear();
  }

  { // if block; dhcp, cdp; simple k,v pair
    const std::vector<std::tuple<std::string, std::string>> tests {
        {R"("dhcp.hw.mac_addr": )", "00:11:22:33:44:55"}
      , {R"("cdp.nrgyz.ip_address": )", "1.2.3.4"}
        // test overwrite
      , {R"("dhcp.hw.mac_addr": )", "00:11:22:33:44:56"}
      };

    for (const auto& [k,v] : tests) {
      tp.setPacketData(k, v);
      tp.testStringValueAtKey(v, k);
    }
    BOOST_TEST(2 == tp.pd.size());

    BOOST_TEST(!tp.dcs.hasData());
    tp.pd.clear();
  }

  { // if block; vlan, dhcp options; k to string vector
    const std::vector<std::tuple<std::string, std::string>> tests {
        {R"("vlan.id": )", "12"}
      , {R"("vlan.id": )", "12"}
      , {R"("vlan.id": )", "123"}
      , {R"("dhcp.option.domain_name_server": )", "1.2.3.4"}
      , {R"("dhcp.option.domain_name_server": )", "1.2.3.5"}
      , {R"("dhcp.option.domain_name_server": )", "1.2.3.5"}
      };

    for (const auto& [k,v] : tests) {
      tp.setPacketData(k, v);
      tp.testKeyExists(k);
      tp.testInStringVector(v, k);
    }
    BOOST_TEST(2 == tp.pd.size());
    for (const auto& [k, _] : tp.pd) {
      const auto& vec = std::any_cast<VecStrType&>(tp.pd.at(k));
      BOOST_TEST(3 == vec.size());
    }

    BOOST_TEST(!tp.dcs.hasData());
    tp.pd.clear();
  }

  { // if block; cdp version; aggregated string

    const std::vector<std::string> tests {
        "v1.0"
      , "v2.0"
      , "abc"
      };

    const std::string key {R"("cdp.software_version": )"};
    std::string value {""};
    for (const auto& test : tests) {
      tp.setPacketData(key, test);
      value += test;
      tp.testStringValueAtKey(value, key);
      value += " ";
    }

    BOOST_TEST(!tp.dcs.hasData());
    tp.pd.clear();
  }

  { // if block; sllSrc; use different key

    const std::vector<std::string> tests {
        "00:11:22:33:44:55"
      , "00:11:22:33:44:56"
      , "00:11:22:33:44:55"
      };

    const std::string key1 {R"("eth.src": )"};
    const std::string key2 {R"("sll.src.eth": )"};
    for (const auto& test : tests) {
      tp.setPacketData(key2, test);
      BOOST_TEST(1 == tp.pd.size());
      tp.testStringValueAtKey(test, key1);
    }

    BOOST_TEST(!tp.dcs.hasData());
    tp.pd.clear();
  }
}

/******
  NOTE: The following tests use "full packet logic" to get targeted data.
        Tests do not contain ALL possible fields, just relevant ones.
******/
BOOST_AUTO_TEST_CASE(testProcessPacketVtp)
{
  TestParser tp;
  const auto& parserRule {tp.start};

  const std::string test {R"([
      {
        "eth.src": "00:11:22:33:44:55",
        "vtp": {
        }
      }
    ])"};

  BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
            , "Rule 'start': " << test
            );

  BOOST_TEST(0 == tp.pd.size());
  BOOST_TEST(tp.processedPacketCount == tp.parsedPacketCount);

  BOOST_TEST(tp.dcs.hasData());

  const auto& out {tp.dcs.getData()};
  BOOST_TEST_REQUIRE(1 == out.size());
  for (const auto& entry : out) {
    // empties
    BOOST_TEST(0 == entry.ipAddrs.size());
    BOOST_TEST(0 == entry.vlans.size());
    BOOST_TEST(0 == entry.ifaces.size());
    BOOST_TEST(0 == entry.services.size());

    // populated
    BOOST_TEST_REQUIRE(1 == entry.macAddrs.size());
    const auto& mac {entry.macAddrs.at("00:11:22:33:44:55")};
    const auto& obs {entry.observations};
    BOOST_TEST(mac.isValid());
    BOOST_TEST(obs.isValid());

    std::string dbgStr;
    dbgStr = mac.toDebugString();
    nmdp::testInString(dbgStr, "macAddress: 00:11:22:33:44:55,");
    nmdp::testInString(dbgStr, "isResponding: true]");
    dbgStr = obs.toDebugString();
    nmdp::testInString( dbgStr
                      , "notables: [Probable VTP from MAC: 00:11:22:33:44:55]"
                      );
  }
}

BOOST_AUTO_TEST_CASE(testProcessPacketDtp)
{
  TestParser tp;
  const auto& parserRule {tp.start};

  const std::string test {R"([
      {
        "eth.src": "00:11:22:33:44:55",
        "dtp.senderid": "00:11:22:33:44:56"
      }
    ])"};

  BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
            , "Rule 'start': " << test
            );

  BOOST_TEST(0 == tp.pd.size());
  BOOST_TEST(tp.processedPacketCount == tp.parsedPacketCount);

  BOOST_TEST(tp.dcs.hasData());

  const auto& out {tp.dcs.getData()};
  BOOST_TEST_REQUIRE(1 == out.size());
  for (const auto& entry : out) {
    // empties
    BOOST_TEST(0 == entry.ipAddrs.size());
    BOOST_TEST(0 == entry.vlans.size());
    BOOST_TEST(0 == entry.ifaces.size());
    BOOST_TEST(0 == entry.services.size());

    // populated
    BOOST_TEST_REQUIRE(1 == entry.macAddrs.size());
    const auto& mac {entry.macAddrs.at("00:11:22:33:44:55")};
    const auto& obs {entry.observations};
    BOOST_TEST(mac.isValid());
    BOOST_TEST(obs.isValid());

    std::string dbgStr;
    dbgStr = mac.toDebugString();
    nmdp::testInString(dbgStr, "macAddress: 00:11:22:33:44:55,");
    nmdp::testInString(dbgStr, "isResponding: true]");
    dbgStr = obs.toDebugString();
    nmdp::testInString( dbgStr
                      , "notables: [Probable DTP from MAC: 00:11:22:33:44:55"
                        " (Sender ID: 00:11:22:33:44:56)]"
                      );
  }
}

BOOST_AUTO_TEST_CASE(testProcessPacketStp)
{
  TestParser tp;
  const auto& parserRule {tp.start};

  const std::string test {R"([
      {
        "eth.src": "00:11:22:33:44:55",
        "stp.root.hw": "00:11:22:33:44:56"
        "stp.bridge.hw": "00:11:22:33:44:57"
      }
    ])"};

  BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
            , "Rule 'start': " << test
            );

  BOOST_TEST(0 == tp.pd.size());
  BOOST_TEST(tp.processedPacketCount == tp.parsedPacketCount);

  BOOST_TEST(tp.dcs.hasData());

  const auto& out {tp.dcs.getData()};
  BOOST_TEST_REQUIRE(1 == out.size());
  for (const auto& entry : out) {
    // empties
    BOOST_TEST(0 == entry.ipAddrs.size());
    BOOST_TEST(0 == entry.vlans.size());
    BOOST_TEST(0 == entry.ifaces.size());
    BOOST_TEST(0 == entry.services.size());

    // populated
    BOOST_TEST_REQUIRE(1 == entry.macAddrs.size());
    const auto& mac {entry.macAddrs.at("00:11:22:33:44:55")};
    const auto& obs {entry.observations};
    BOOST_TEST(mac.isValid());
    BOOST_TEST(obs.isValid());

    std::string dbgStr;
    dbgStr = mac.toDebugString();
    nmdp::testInString(dbgStr, "macAddress: 00:11:22:33:44:55,");
    nmdp::testInString(dbgStr, "isResponding: true]");
    dbgStr = obs.toDebugString();
    nmdp::testInString( dbgStr
                      , "notables: [Probable STP from MAC: 00:11:22:33:44:55"
                        " (Root HW: 00:11:22:33:44:56,"
                        " Bridge HW: 00:11:22:33:44:57)]"
                      );
  }
}

BOOST_AUTO_TEST_CASE(testProcessPacketArp)
{
  TestParser tp;
  const auto& parserRule {tp.start};

  const std::string test {R"([
      {
        "arp.src.hw_mac": "00:11:22:33:44:55",
        "arp.src.proto_ipv4": "1.2.3.4"
      }
    ])"};

  BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
            , "Rule 'start': " << test
            );

  BOOST_TEST(0 == tp.pd.size());
  BOOST_TEST(tp.processedPacketCount == tp.parsedPacketCount);

  BOOST_TEST(tp.dcs.hasData());

  const auto& out {tp.dcs.getData()};
  BOOST_TEST_REQUIRE(1 == out.size());
  for (const auto& entry : out) {
    // empties
    BOOST_TEST(0 == entry.ipAddrs.size());
    BOOST_TEST(0 == entry.vlans.size());
    BOOST_TEST(0 == entry.ifaces.size());
    BOOST_TEST(0 == entry.services.size());
    BOOST_TEST(!entry.observations.isValid());

    // populated
    BOOST_TEST_REQUIRE(1 == entry.macAddrs.size());
    const auto& mac {entry.macAddrs.at("00:11:22:33:44:55")};
    BOOST_TEST(mac.isValid());

    const auto& dbgStr {mac.toDebugString()};
    nmdp::testInString(dbgStr, "macAddress: 00:11:22:33:44:55,");
    nmdp::testInString(dbgStr, "isResponding: true]");
    nmdp::testInString(dbgStr, "[ipAddress: 1.2.3.4/32, isResponding: true,");
  }
}

BOOST_AUTO_TEST_CASE(testProcessPacketDns)
{
  TestParser tp;
  const auto& parserRule {tp.start};

  // alias will only be set after 'dns.resp.name' is processed
  const std::string test {R"([
      {
        "dns.a": "1.2.3.4",
        "dns.resp.name": "n1.dom",
        "dns.aaaa": "1::2",
        "dns.cname": "n2.dom",
        "dns.ns": "n3.dom",
        "dns.soa.mname": "n4.dom",
        "dns.soa.rname": "n5.dom"
      }
    ])"};

  BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
            , "Rule 'start': " << test
            );

  BOOST_TEST(0 == tp.pd.size());
  BOOST_TEST(tp.processedPacketCount == tp.parsedPacketCount);

  BOOST_TEST(tp.dcs.hasData());

  const auto& out {tp.dcs.getData()};
  BOOST_TEST_REQUIRE(1 == out.size());
  for (const auto& entry : out) {
    // empties
    BOOST_TEST(0 == entry.macAddrs.size());
    BOOST_TEST(0 == entry.vlans.size());
    BOOST_TEST(0 == entry.ifaces.size());
    BOOST_TEST(0 == entry.services.size());
    BOOST_TEST(!entry.observations.isValid());

    // populated
    BOOST_TEST_REQUIRE(2 == entry.ipAddrs.size());
    const auto& ip1 {entry.ipAddrs.at("1.2.3.4")};
    BOOST_TEST(ip1.isValid());
    {
      const auto& dbgStr {ip1.toDebugString()};
      nmdp::testInString(dbgStr, "[ipAddress: 1.2.3.4/32,");
      nmdp::testInString(dbgStr, "isResponding: false,");
      nmdp::testInString(dbgStr, "aliases: []]");
    }

    const auto& ip2 {entry.ipAddrs.at("1::2")};
    BOOST_TEST(ip2.isValid());
    {
      const auto& dbgStr {ip2.toDebugString()};
      nmdp::testInString(dbgStr, "[ipAddress: 1::2/128,");
      nmdp::testInString(dbgStr, "isResponding: false,");
      nmdp::testInString(dbgStr, "aliases: [n1.dom]]");
    }
  }
}

BOOST_AUTO_TEST_CASE(testProcessPacketDhcp)
{
  TestParser tp;
  const auto& parserRule {tp.start};

  const std::string test {R"([
      {
        "dhcp.option.subnet_mask": "255.255.255.0",
        "dhcp.option.domain_name_server": "1.2.3.4",
        "dhcp.option.dhcp_server_id": "1.2.3.5",
        "dhcp.option.router": "1.2.3.1",
        "dhcp.option.domain_name": "n1.dom",
        "dhcp.option.tftp_server_address": "1.2.3.6",
        "dhcp.option.sip_server.address": "1.2.3.7",
        "dhcp.option.hostname": "host1",
        "dhcp.ip.your": "1.2.3.8",
        "dhcp.ip.relay": "1.2.3.9",
        "dhcp.hw.mac_addr": "00:11:22:33:44:55",
      }
    ])"};

  BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
            , "Rule 'start': " << test
            );

  BOOST_TEST(0 == tp.pd.size());
  BOOST_TEST(tp.processedPacketCount == tp.parsedPacketCount);

  BOOST_TEST(tp.dcs.hasData());

  const auto& out {tp.dcs.getData()};
  BOOST_TEST_REQUIRE(1 == out.size());
  for (const auto& entry : out) {
    // empties
    BOOST_TEST(0 == entry.vlans.size());
    BOOST_TEST(0 == entry.ifaces.size());
    BOOST_TEST(!entry.observations.isValid());

    // populated
    BOOST_TEST_REQUIRE(1 == entry.ipAddrs.size());
    const auto& ip {entry.ipAddrs.at("1.2.3.8")};
    BOOST_TEST(ip.isValid());
    {
      const auto& dbgStr {ip.toDebugString()};
      nmdp::testInString(dbgStr, "[ipAddress: 1.2.3.8/24,");
      nmdp::testInString(dbgStr, "isResponding: true,");
      nmdp::testInString(dbgStr, "aliases: [host1]]");
    }

    BOOST_TEST_REQUIRE(1 == entry.macAddrs.size());
    const auto& mac {entry.macAddrs.at("00:11:22:33:44:55")};
    BOOST_TEST(mac.isValid());
    {
      const auto& dbgStr {mac.toDebugString()};
      nmdp::testInString(dbgStr, "[macAddress: 00:11:22:33:44:55,");
      nmdp::testInString(dbgStr, "ipAddrs: [[ipAddress: 1.2.3.5/32,");
      nmdp::testInString(dbgStr, "isResponding: true,");
      nmdp::testInString(dbgStr, "aliases: []]");
    }

    BOOST_TEST_REQUIRE(2 == entry.services.size());
    for (const auto& s : entry.services) {
      BOOST_TEST(s.isValid());
      const auto& dbgStr {s.toDebugString()};
      if ("dns" == s.getServiceName()) {
        nmdp::testInString(dbgStr, "dstAddress: [ipAddress: 1.2.3.4/32,");
        nmdp::testInString(dbgStr, "protocol: udp,");
        nmdp::testInString(dbgStr, "dstPorts: [53],");
      } else if ("tftp" == s.getServiceName()) {
        nmdp::testInString(dbgStr, "dstAddress: [ipAddress: 1.2.3.6/32,");
        nmdp::testInString(dbgStr, "protocol: udp,");
        nmdp::testInString(dbgStr, "dstPorts: [69],");
      } else {
        BOOST_TEST(false, "Unhandled service: " << s.getServiceName());
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(testProcessPacketDhcpSkip)
{
  TestParser tp;
  const auto& parserRule {tp.start};

  const std::string test {R"([
      {
        "dhcp.option.subnet_mask": "255.255.255.0",
        "dhcp.ip.your": "0.0.0.0",
      }
    ])"};

  BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
            , "Rule 'start': " << test
            );

  BOOST_TEST(0 == tp.pd.size());
  BOOST_TEST(1 == tp.processedPacketCount);
  BOOST_TEST(0 == tp.parsedPacketCount);


  BOOST_TEST(!tp.dcs.hasData());
}

BOOST_AUTO_TEST_CASE(testProcessPacketNtp)
{
//  nmcu::LoggerSingleton::getInstance().setLevel(nmcu::Severity::DEBUG_SPIRIT);

  TestParser tp;
  const auto& parserRule {tp.start};

  const std::string test {R"([
      {
        "ip.src": "1.2.3.4",
        "ntp.flags.mode": "1",
      },
      {
        "ip.src": "1.2.3.4",
        "ntp.flags.mode": "2",
      },
      {
        "ip.src": "1.2.3.4",
        "ntp.flags.mode": "4",
      },
      {
        "ip.src": "1.2.3.4",
        "ntp.flags.mode": "5",
      },
      {
        "frame.number": "12345",
        "ip.src": "1.2.3.4",
        "ntp.flags.mode": "6",
        "ntp.ctrl.flags2.r": "1",
      },
    ])"};

  BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
            , "Rule 'start': " << test
            );

  BOOST_TEST(0 == tp.pd.size());
  BOOST_TEST(tp.processedPacketCount == tp.parsedPacketCount);

  BOOST_TEST(tp.dcs.hasData());
  const auto& out {tp.dcs.getData()};
  BOOST_TEST(5 == out.size());
  for (const auto& entry : out) {
    // empties
    BOOST_TEST(0 == entry.ipAddrs.size());
    BOOST_TEST(0 == entry.macAddrs.size());
    BOOST_TEST(0 == entry.vlans.size());
    BOOST_TEST(0 == entry.ifaces.size());

    // populated
    BOOST_TEST_REQUIRE(1 == entry.services.size());
    for (const auto& s : entry.services) {
      BOOST_TEST(s.isValid());
      const auto& dbgStr {s.toDebugString()};
      if ("ntp" == s.getServiceName()) {
        nmdp::testInString(dbgStr, "[dstAddress: [ipAddress: 1.2.3.4/32,");
        nmdp::testInString(dbgStr, "protocol: udp,");
        nmdp::testInString(dbgStr, "dstPorts: [123],");
      } else {
        BOOST_TEST(false, "Unhandled service: " << s.getServiceName());
      }
    }
  }

  // populated, one-off(s)
  BOOST_TEST(out[4].observations.isValid());
  const auto& dbgStr {out[4].observations.toDebugString()};
  nmdp::testInString( dbgStr
                    , "[notables: [NTP control data present in frame: 12345],"
                    );
}

BOOST_AUTO_TEST_CASE(testProcessPacketNtpSkip)
{
  TestParser tp;
  const auto& parserRule {tp.start};

  const std::string test {R"([
      {
        "ntp.flags.mode": "3",
        "ntp.ctrl.flags2.r": "0",
      },
      {
        "ntp.flags.mode": "3",
        "ntp.ctrl.flags2.r": "1",
      },
      {
        "ntp.flags.mode": "6",
        "ntp.ctrl.flags2.r": "0",
      },
      {
        "ip.src": "1.2.3.4",
        "ntp.flags.mode": "6",
      },
    ])"};

  BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
            , "Rule 'start': " << test
            );
  BOOST_TEST(0 == tp.pd.size());
  BOOST_TEST(4 == tp.processedPacketCount);
  BOOST_TEST(0 == tp.parsedPacketCount);

  BOOST_TEST(!tp.dcs.hasData());
}

BOOST_AUTO_TEST_CASE(testProcessPacketCdp)
{
  TestParser tp;
  const auto& parserRule {tp.start};

  const std::string test {R"([
      {
        "eth.src": "00:11:22:33:44:55",
        "cdp.deviceid": "abc123",
        "cdp.platform": "abc 123",
        "cdp.software_version": "v1.2",
        "cdp.nrgyz.ip_address": "1.2.3.4",
        "cdp.portid": "ether",
        "cdp.native_vlan": "1234",
      },
    ])"};

  BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
            , "Rule 'start': " << test
            );

  BOOST_TEST(0 == tp.pd.size());
  BOOST_TEST(tp.processedPacketCount == tp.parsedPacketCount);

  BOOST_TEST(tp.dcs.hasData());
  const auto& out {tp.dcs.getData()};
  BOOST_TEST(1 == out.size());
  for (const auto& entry : out) {
    // empties
    BOOST_TEST(0 == entry.ipAddrs.size());
    BOOST_TEST(0 == entry.vlans.size());
    BOOST_TEST(0 == entry.services.size());
    BOOST_TEST(!entry.observations.isValid());

    // populated
    BOOST_TEST(1 == entry.macAddrs.size());
    for (const auto& [_, macAddr] : entry.macAddrs) {
      BOOST_TEST(macAddr.isValid());
      const auto& dbgStr {macAddr.toDebugString()};
      nmdp::testInString(dbgStr, "[macAddress: 00:11:22:33:44:55,");
      nmdp::testInString(dbgStr, "isResponding: true]");
    }
    BOOST_TEST_REQUIRE(1 == entry.ifaces.size());
    for (const auto& [_, iface] : entry.ifaces) {
      BOOST_TEST(iface.isValid());
      const auto& dbgStr {iface.toDebugString()};
      nmdp::testInString(dbgStr, "[name: ether,");
      nmdp::testInString(dbgStr, "description: abc 123 -- v1.2,");
      nmdp::testInString(dbgStr, "mediaType: ether,");
      nmdp::testInString(dbgStr, "isUp: true,");
      nmdp::testInString(dbgStr, "isDiscoveryProtocolEnabled: true,");
      nmdp::testInString(dbgStr, "macAddr: [macAddress: 00:11:22:33:44:55");
      nmdp::testInString(dbgStr, "ipAddrs: [[ipAddress: 1.2.3.4/32,");
    }
  }
}

BOOST_AUTO_TEST_CASE(testProcessPacketVlan)
{
  TestParser tp;
  const auto& parserRule {tp.start};

  const std::string test {R"([
      {
        "vlan.id": "1234",
      },
    ])"};

  BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
            , "Rule 'start': " << test
            );

  BOOST_TEST(0 == tp.pd.size());
  BOOST_TEST(tp.processedPacketCount == tp.parsedPacketCount);

  BOOST_TEST(tp.dcs.hasData());
  const auto& out {tp.dcs.getData()};
  BOOST_TEST(1 == out.size());
  for (const auto& entry : out) {
    // empties
    BOOST_TEST(0 == entry.ipAddrs.size());
    BOOST_TEST(0 == entry.ifaces.size());
    BOOST_TEST(0 == entry.macAddrs.size());
    BOOST_TEST(0 == entry.services.size());
    BOOST_TEST(!entry.observations.isValid());

    // populated
    BOOST_TEST(1 == entry.vlans.size());
    for (const auto& [_, vlan] : entry.vlans) {
      BOOST_TEST(vlan.isValid());
      const auto& dbgStr {vlan.toDebugString()};
      nmdp::testInString(dbgStr, "[vlanId: 1234,");
    }
  }
}

BOOST_AUTO_TEST_CASE(testProcessPacketEthernet)
{
  TestParser tp;
  const auto& parserRule {tp.start};

  const std::string test {R"([
      {
        "eth.src": "00:11:22:33:44:55",
      },
    ])"};

  BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
            , "Rule 'start': " << test
            );

  BOOST_TEST(0 == tp.pd.size());
  BOOST_TEST(tp.processedPacketCount == tp.parsedPacketCount);

  BOOST_TEST(tp.dcs.hasData());
  const auto& out {tp.dcs.getData()};
  BOOST_TEST(1 == out.size());
  for (const auto& entry : out) {
    // empties
    BOOST_TEST(0 == entry.ipAddrs.size());
    BOOST_TEST(0 == entry.ifaces.size());
    BOOST_TEST(0 == entry.vlans.size());
    BOOST_TEST(0 == entry.services.size());
    BOOST_TEST(!entry.observations.isValid());

    // populated
    BOOST_TEST(1 == entry.macAddrs.size());
    for (const auto& [_, macAddr] : entry.macAddrs) {
      BOOST_TEST(macAddr.isValid());
      const auto& dbgStr {macAddr.toDebugString()};
      nmdp::testInString(dbgStr, "[macAddress: 00:11:22:33:44:55,");
      nmdp::testInString(dbgStr, "isResponding: true]");
    }
  }
}
