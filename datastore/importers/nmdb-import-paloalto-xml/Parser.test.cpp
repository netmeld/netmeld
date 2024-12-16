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
#include <pugixml.hpp>

#include <netmeld/datastore/parsers/ParserTestHelper.hpp>
#include "Parser.hpp"

namespace nmdp = netmeld::datastore::parsers;

struct TestParser : public Parser
{
  public:
    pugi::xml_document doc;
    LogicalSystem ls;

    using Parser::data;

    using Parser::parseConfig;
    using Parser::parseConfigRouteInfo;

    using Parser::parseConfigAddress;
    using Parser::parseConfigAddressGroup;
    using Parser::parseConfigDeviceconfig;
    using Parser::parseConfigInterface;
    using Parser::parseConfigInterfaceEntry;
    using Parser::parseConfigRulebase;
    using Parser::parseConfigRules;
    using Parser::parseConfigService;
    using Parser::parseConfigServiceGroup;
    using Parser::parseConfigVirtualRouter;
    using Parser::parseConfigVsys;
    using Parser::parseConfigZone;

  pugi::xml_node
  getFirstNode(const char* xmlData)
  {
    pugi::xml_parse_result result = doc.load_string(xmlData);
    BOOST_REQUIRE_MESSAGE( result
                         , std::format("Failed to load XML: {}", xmlData)
                         );

    return doc.document_element();
  }
};

BOOST_AUTO_TEST_CASE(testParseConfigRules)
{
  TestParser tp;

  // add known zones (prior parse step that impacts results)
  tp.ls.aclZones["zone1"].setId("Zone 1");
  tp.ls.aclZones["zone2"].setId("Zone 2");
  tp.ls.aclZones["zone3"].setId("Zone 3");

  const std::string xml {R"(
      <rules>
        <entry name="explicit-rule-type">
          <rule-type>intrazone</rule-type>
          <from>        <member>zone1</member>    </from>
          <to>          <member>zone1</member>    </to>
          <source>      <member>src1</member>     </source>
          <destination> <member>dst1</member>     </destination>
          <service>     <member>service1</member> </service>
          <action>allow</action>
        </entry>
        <entry name="intrazone-default">
          <from>        <member>zone2</member>    </from>
          <to>          <member>zone2</member>    </to>
          <source>      <member>src2</member>     </source>
          <destination> <member>dst2</member>     </destination>
          <service>     <member>service2</member> </service>
          <action>deny</action>
        </entry>
        <entry name="interzone-default">
          <from>        <member>zone1</member>    </from>
          <to>          <member>zone2</member>    </to>
          <source>      <member>src3</member>     </source>
          <destination> <member>dst3</member>     </destination>
          <service>     <member>service3</member> </service>
          <action>drop</action>
        </entry>
        <entry name="unspecified1">
          <from>        <member>zone1</member>    </from>
          <to>          <member>zone2</member>    </to>
          <source>      <member>src4</member>     </source>
          <destination> <member>dst4</member>     </destination>
          <service>     <member>service4</member> </service>
          <action>reset-client</action>
        </entry>
        <entry name="unspecified2">
          <from>        <member>zone2</member>    </from>
          <to>          <member>zone1</member>    </to>
          <source>      <member>src5</member>     </source>
          <destination> <member>dst5</member>     </destination>
          <service>     <member>service5</member> </service>
          <action>reset-server</action>
        </entry>
        <entry name="any-zones">
          <from>        <member>any</member>  </from>
          <to>          <member>any</member>  </to>
          <source>      <member>any</member>  </source>
          <destination> <member>any</member>  </destination>
          <service>     <member>any</member>  </service>
          <action>reset-both</action>
        </entry>
        <entry name="any-implicit">
          <action>log drop</action>
        </entry>
      </rules>
    )"};

  const auto& test = tp.getFirstNode(xml.c_str());
  const auto& out {tp.parseConfigRules(test, 0, tp.ls)};

  BOOST_TEST_REQUIRE(23 == out.size());
  for (const auto& rule : out) {
    BOOST_TEST(rule.isValid());
  }

  auto dbgStr = out[0].toDebugString();
  nmdp::testInString(dbgStr, "priority: 0,");
  nmdp::testInString(dbgStr, "action: allow,");
  nmdp::testInString(dbgStr, "incomingZoneId: zone1,");
  nmdp::testInString(dbgStr, "outgoingZoneId: zone1,");
  nmdp::testInString(dbgStr, "srcIpNetSetId: src1,");
  nmdp::testInString(dbgStr, "dstIpNetSetId: dst1,");
  nmdp::testInString(dbgStr, "description: explicit-rule-type],");
  nmdp::testInString(dbgStr, "serviceId: service1]");

  dbgStr = out[1].toDebugString();
  nmdp::testInString(dbgStr, "priority: 1,");
  nmdp::testInString(dbgStr, "action: block,");
  nmdp::testInString(dbgStr, "incomingZoneId: zone2,");
  nmdp::testInString(dbgStr, "outgoingZoneId: zone2,");
  nmdp::testInString(dbgStr, "srcIpNetSetId: src2,");
  nmdp::testInString(dbgStr, "dstIpNetSetId: dst2,");
  nmdp::testInString(dbgStr, "description: intrazone-default],");
  nmdp::testInString(dbgStr, "serviceId: service2]");

  dbgStr = out[2].toDebugString();
  nmdp::testInString(dbgStr, "priority: 2,");
  nmdp::testInString(dbgStr, "action: block,");
  nmdp::testInString(dbgStr, "incomingZoneId: zone1,");
  nmdp::testInString(dbgStr, "outgoingZoneId: zone2,");
  nmdp::testInString(dbgStr, "srcIpNetSetId: src3,");
  nmdp::testInString(dbgStr, "dstIpNetSetId: dst3,");
  nmdp::testInString(dbgStr, "description: interzone-default],");
  nmdp::testInString(dbgStr, "serviceId: service3]");

  dbgStr = out[3].toDebugString();
  nmdp::testInString(dbgStr, "priority: 3,");
  nmdp::testInString(dbgStr, "action: block,");
  nmdp::testInString(dbgStr, "incomingZoneId: zone1,");
  nmdp::testInString(dbgStr, "outgoingZoneId: zone2,");
  nmdp::testInString(dbgStr, "srcIpNetSetId: src4,");
  nmdp::testInString(dbgStr, "dstIpNetSetId: dst4,");
  nmdp::testInString(dbgStr, "description: unspecified1],");
  nmdp::testInString(dbgStr, "serviceId: service4]");

  dbgStr = out[4].toDebugString();
  nmdp::testInString(dbgStr, "priority: 4,");
  nmdp::testInString(dbgStr, "action: block,");
  nmdp::testInString(dbgStr, "incomingZoneId: zone2,");
  nmdp::testInString(dbgStr, "outgoingZoneId: zone1,");
  nmdp::testInString(dbgStr, "srcIpNetSetId: src5,");
  nmdp::testInString(dbgStr, "dstIpNetSetId: dst5,");
  nmdp::testInString(dbgStr, "description: unspecified2],");
  nmdp::testInString(dbgStr, "serviceId: service5]");

  dbgStr = out[5].toDebugString();
  nmdp::testInString(dbgStr, "priority: 5,");
  nmdp::testInString(dbgStr, "action: block,");
  nmdp::testInString(dbgStr, "incomingZoneId: zone1,");
  nmdp::testInString(dbgStr, "outgoingZoneId: zone1,");
  nmdp::testInString(dbgStr, "srcIpNetSetId: any,");
  nmdp::testInString(dbgStr, "dstIpNetSetId: any,");
  nmdp::testInString(dbgStr, "description: any-zones],");
  nmdp::testInString(dbgStr, "serviceId: any]");

  // expanded any-zones
  for (size_t i {6} ; i < 14; ++i) {
    dbgStr = out[i].toDebugString();
    nmdp::testInString(dbgStr, "priority: 5,"); // same priority as expanded
    nmdp::testInString(dbgStr, "action: block,");
    nmdp::testInString(dbgStr, "srcIpNetSetId: any,");
    nmdp::testInString(dbgStr, "dstIpNetSetId: any,");
    nmdp::testInString(dbgStr, "description: any-zones],");
    nmdp::testInString(dbgStr, "serviceId: any]");
    // incoming
    switch(i) {
      case 6:
      case 7:
        nmdp::testInString(dbgStr, "incomingZoneId: zone1,");
        break;
      case 8:
      case 9:
      case 10:
        nmdp::testInString(dbgStr, "incomingZoneId: zone2,");
        break;
      case 11:
      case 12:
      case 13:
        nmdp::testInString(dbgStr, "incomingZoneId: zone3,");
        break;
      default:
        BOOST_TEST(false);
    }
    // outgoing
    switch(i) {
      case 8:
      case 11:
        nmdp::testInString(dbgStr, "outgoingZoneId: zone1,");
        break;
      case 6:
      case 9:
      case 12:
        nmdp::testInString(dbgStr, "outgoingZoneId: zone2,");
        break;
      case 7:
      case 10:
      case 13:
        nmdp::testInString(dbgStr, "outgoingZoneId: zone3,");
        break;
      default:
        BOOST_TEST(false);
    }
  }

  dbgStr = out[14].toDebugString();
  nmdp::testInString(dbgStr, "priority: 6,");
  nmdp::testInString(dbgStr, "action: log drop,");
  nmdp::testInString(dbgStr, "incomingZoneId: zone1,");
  nmdp::testInString(dbgStr, "outgoingZoneId: zone1,");
  nmdp::testInString(dbgStr, "srcIpNetSetId: any,");
  nmdp::testInString(dbgStr, "dstIpNetSetId: any,");
  nmdp::testInString(dbgStr, "description: any-implicit],");
  nmdp::testInString(dbgStr, "serviceId: any]");

  // expanded any-implicit
  for (size_t i {15} ; i < 23; ++i) {
    dbgStr = out[i].toDebugString();
    nmdp::testInString(dbgStr, "priority: 6,"); // same priority as expanded
    nmdp::testInString(dbgStr, "action: log drop,");
    nmdp::testInString(dbgStr, "srcIpNetSetId: any,");
    nmdp::testInString(dbgStr, "dstIpNetSetId: any,");
    nmdp::testInString(dbgStr, "description: any-implicit],");
    nmdp::testInString(dbgStr, "serviceId: any]");
    // assume prior any-zone check correct and sufficient for:
    // - incomingZoneId
    // - outgoingZoneId
  }
}

BOOST_AUTO_TEST_CASE(testParseConfigRulebase)
{
  TestParser tp;

  // add known zones (prior parse step that impacts results)
  tp.ls.aclZones["any"].setId("any");

  const std::string xml {R"(
      <rulebase>
        <security>
          <rules>
            <entry> <action>drop</action> </entry>
            <entry> <action>drop</action> </entry>
          </rules>
        </security>
        <default-security-rules>
          <rules>
            <entry> <action>drop</action> </entry>
            <entry> <action>drop</action> </entry>
          </rules>
        </default-security-rules>
        <pbf>
          <rules>
            <entry> <action>drop</action> </entry>
            <entry> <action>drop</action> </entry>
          </rules>
        </pbf>
        <nat>
          <rules>
            <entry> <action>drop</action> </entry>
            <entry> <action>drop</action> </entry>
          </rules>
        </nat>
      </rulebase>
    )"};

  const auto& test = tp.getFirstNode(xml.c_str());
  const auto& out {tp.parseConfigRulebase(test, tp.ls)};

  BOOST_TEST_REQUIRE(4 == out.size());
  for (const auto& rule : out) {
    BOOST_TEST(rule.isValid());
  }

  nmdp::testInString(out[0].toDebugString(), "priority: 1000000,");
  nmdp::testInString(out[1].toDebugString(), "priority: 1000001,");
  nmdp::testInString(out[2].toDebugString(), "priority: 2000000,");
  nmdp::testInString(out[3].toDebugString(), "priority: 2000001,");

  for (size_t i {0}; i < out.size(); ++i) {
    const auto& dbgStr {out[i].toDebugString()};
    nmdp::testInString(dbgStr, "action: block,");
    nmdp::testInString(dbgStr, "incomingZoneId: any,");
    nmdp::testInString(dbgStr, "outgoingZoneId: any,");
    nmdp::testInString(dbgStr, "srcIpNetSetId: any,");
    nmdp::testInString(dbgStr, "dstIpNetSetId: any,");
    nmdp::testInString(dbgStr, "description: ],");
    nmdp::testInString(dbgStr, "serviceId: any]");
  }
}

BOOST_AUTO_TEST_CASE(testParseConfigServiceGroup)
{
  TestParser tp;

  const std::string xml {R"(
      <service-group>
        <entry name="service1">
          <members>
            <member>member1</member>
            <member>member2</member>
          </members>
        </entry>
        <entry name="service2">
          <members>
            <member>member3</member>
          </members>
        </entry>
        <entry> </entry>
      </service-group>
    )"};

  const auto& test = tp.getFirstNode(xml.c_str());
  const auto& out {tp.parseConfigServiceGroup(test)};

  BOOST_TEST_REQUIRE(3 == out.size());
  BOOST_TEST(out[0].isValid());
  BOOST_TEST(out[1].isValid());
  BOOST_TEST(!out[2].isValid());

  auto dbgStr {out[0].toDebugString()};
  nmdp::testInString(dbgStr, "id: service1,");
  nmdp::testInString(dbgStr, "includedIds: [member1, member2]]");

  dbgStr = out[1].toDebugString();
  nmdp::testInString(dbgStr, "id: service2,");
  nmdp::testInString(dbgStr, "includedIds: [member3]]");

  dbgStr = out[2].toDebugString();
  nmdp::testInString(dbgStr, "id: ,");
  nmdp::testInString(dbgStr, "includedIds: []]");
}

BOOST_AUTO_TEST_CASE(testParseConfigService)
{
  TestParser tp;

  const std::string xml {R"(
      <service>
        <entry name="service1">
          <protocol> <tcp> <port>80</port> </tcp> </protocol>
        </entry>
        <entry name="service2">
          <protocol> <udp> <port>53</port> </udp> </protocol>
        </entry>
        <entry name="service3">
          <protocol> <icmp/> </protocol>
        </entry>
        <entry name="service4">
          <protocol> <sctp> <port>123-456</port> </sctp> </protocol>
        </entry>
      </service>
    )"};
//        <entry name="service5">
//          <protocol> <tcp>
//            <source-port>123-456</source-port>
//            <port>456-789</port>
//          </tcp> </protocol>
//        </entry>

  const auto& test = tp.getFirstNode(xml.c_str());
  const auto& out {tp.parseConfigService(test)};

  BOOST_TEST_REQUIRE(5 == out.size());
  for (const auto& service : out) {
    BOOST_TEST(service.isValid());
  }

  // default "any" service always added first
  auto dbgStr = out[0].toDebugString();
  nmdp::testInString(dbgStr, "id: any,");
  nmdp::testInString(dbgStr, "protocol: any,");
  nmdp::testInString(dbgStr, "srcPortRanges: [[min: 0, max: 65535]]");
  nmdp::testInString(dbgStr, "dstPortRanges: [[min: 0, max: 65535]]");
  nmdp::testInString(dbgStr, "includedIds: []]");

  dbgStr = out[1].toDebugString();
  nmdp::testInString(dbgStr, "id: service1,");
  nmdp::testInString(dbgStr, "protocol: tcp,");
  nmdp::testInString(dbgStr, "srcPortRanges: [[min: 0, max: 65535]]");
  nmdp::testInString(dbgStr, "dstPortRanges: [[min: 80, max: 80]]");
  nmdp::testInString(dbgStr, "includedIds: []]");

  dbgStr = out[2].toDebugString();
  nmdp::testInString(dbgStr, "id: service2,");
  nmdp::testInString(dbgStr, "protocol: udp,");
  nmdp::testInString(dbgStr, "srcPortRanges: [[min: 0, max: 65535]]");
  nmdp::testInString(dbgStr, "dstPortRanges: [[min: 53, max: 53]]");
  nmdp::testInString(dbgStr, "includedIds: []]");

  dbgStr = out[3].toDebugString();
  nmdp::testInString(dbgStr, "id: service3,");
  nmdp::testInString(dbgStr, "protocol: icmp,");
  nmdp::testInString(dbgStr, "srcPortRanges: []");
  nmdp::testInString(dbgStr, "dstPortRanges: []");
  nmdp::testInString(dbgStr, "includedIds: []]");

  dbgStr = out[4].toDebugString();
  nmdp::testInString(dbgStr, "id: service4,");
  nmdp::testInString(dbgStr, "protocol: sctp,");
  nmdp::testInString(dbgStr, "srcPortRanges: [[min: 0, max: 65535]]");
  nmdp::testInString(dbgStr, "dstPortRanges: [[min: 123, max: 456]]");
  nmdp::testInString(dbgStr, "includedIds: []]");

// TODO: Parser not configured to support different source-port
//  dbgStr = out[5].toDebugString();
//  nmdp::testInString(dbgStr, "id: service5,");
//  nmdp::testInString(dbgStr, "protocol: tcp,");
//  nmdp::testInString(dbgStr, "srcPortRanges: [[min: 123, max: 456]]");
//  nmdp::testInString(dbgStr, "dstPortRanges: [[min: 456, max: 789]]");
//  nmdp::testInString(dbgStr, "includedIds: []]");
}

BOOST_AUTO_TEST_CASE(testParseConfigAddressGroup)
{
  TestParser tp;

  const std::string xml {R"(
      <address-group>
        <entry name="group1">
          <static>
            <member>1.2.1.0/24</member>
          </static>
        </entry>
        <entry name="group2">
          <static>
            <member>1.2.1.0/24</member>
            <member>2.3.4.5/32</member>
          </static>
        </entry>
        <entry name="group3"> </entry>
      </address-group>
    )"};

  const auto& test = tp.getFirstNode(xml.c_str());
  const auto& out {tp.parseConfigAddressGroup(test)};

  BOOST_TEST_REQUIRE(3 == out.size());
  for (const auto& [_, ipNetSet] : out) {
    BOOST_TEST(ipNetSet.isValid());
  }

  auto dbgStr = out.at("group1").toDebugString();
  nmdp::testInString(dbgStr, "id: group1,");
  nmdp::testInString(dbgStr, "includedIds: [[, 1.2.1.0/24]]");

  dbgStr = out.at("group2").toDebugString();
  nmdp::testInString(dbgStr, "id: group2,");
  nmdp::testInString( dbgStr
                    , "includedIds: [[, 1.2.1.0/24], [, 2.3.4.5/32]]]"
                    );

  dbgStr = out.at("group3").toDebugString();
  nmdp::testInString(dbgStr, "id: group3,");
  nmdp::testInString(dbgStr, "includedIds: []]");
}

BOOST_AUTO_TEST_CASE(testParseConfigAddress)
{
  TestParser tp;

  const std::string xml {R"(
      <address>
        <entry name="address1">
          <ip-netmask>1.2.1.0/24</ip-netmask>
        </entry>
        <entry name="address2">
          <ip-netmask>1.2.3.4/32</ip-netmask>
          <ip-netmask>2.2.3.4/32</ip-netmask>
          <fqdn>some.fqdn.1</fqdn>
          <fqdn>some.fqdn.2</fqdn>
        </entry>
        <entry name="address3">
          <fqdn>some.other.fqdn</fqdn>
        </entry>
      </address>
    )"};

  const auto& test = tp.getFirstNode(xml.c_str());
  const auto& out {tp.parseConfigAddress(test)};

  BOOST_TEST_REQUIRE(3 == out.size());
  for (const auto& [_, ipNetSet] : out) {
    BOOST_TEST(ipNetSet.isValid());
  }

  auto dbgStr = out.at("address1").toDebugString();
  nmdp::testInString(dbgStr, "id: address1,");
  nmdp::testInString(dbgStr, "ipNetwork: 1.2.1.0/24,");
  nmdp::testInString(dbgStr, "hostnames: []");

  dbgStr = out.at("address2").toDebugString();
  nmdp::testInString(dbgStr, "id: address2,");
  nmdp::testInString(dbgStr, "ipNetwork: 1.2.3.4/32,");
  nmdp::testInString(dbgStr, "ipNetwork: 2.2.3.4/32,");
  nmdp::testInString(dbgStr, "hostnames: [some.fqdn.1, some.fqdn.2]");

  dbgStr = out.at("address3").toDebugString();
  nmdp::testInString(dbgStr, "id: address3,");
  nmdp::testInString(dbgStr, "ipNets: []");
  nmdp::testInString(dbgStr, "hostnames: [some.other.fqdn]");

  dbgStr = tp.data.observations.toDebugString();
  nmdp::testInString(dbgStr, "notables: [FQDNs are used that must be resolved],");
}

BOOST_AUTO_TEST_CASE(testParseConfigZone)
{
  TestParser tp;

  const std::string xml {R"(
      <zone>
        <entry name="zone1">
          <network>
            <layer3>
              <member>eth0</member>
              <member>Eth 1/1</member>
            </layer3>
          </network>
        </entry>
        <entry name="zone2">
          <network>
            <layer3>
              <member>eth2</member>
            </layer3>
          </network>
        </entry>
        <entry name="zone3">
          <network>
            <layer3>
              <member>eth3</member>
              <member>eth4</member>
              <member>eth5</member>
            </layer3>
          </network>
        </entry>
      </zone>
    )"};

  const auto& test = tp.getFirstNode(xml.c_str());
  const auto& out {tp.parseConfigZone(test)};

  BOOST_TEST_REQUIRE(3 == out.size());
  for (const auto& [_, zone] : out) {
    BOOST_TEST(zone.isValid());
  }

  auto dbgStr = out.at("zone1").toDebugString();
  nmdp::testInString(dbgStr, "id: zone1,");
  nmdp::testInString(dbgStr, "ifaces: [eth0, eth 1/1]");
  nmdp::testInString(dbgStr, "includedIds: []");

  dbgStr = out.at("zone2").toDebugString();
  nmdp::testInString(dbgStr, "id: zone2,");
  nmdp::testInString(dbgStr, "ifaces: [eth2]");
  nmdp::testInString(dbgStr, "includedIds: []");

  dbgStr = out.at("zone3").toDebugString();
  nmdp::testInString(dbgStr, "id: zone3,");
  nmdp::testInString(dbgStr, "ifaces: [eth3, eth4, eth5]");
  nmdp::testInString(dbgStr, "includedIds: []");
}

BOOST_AUTO_TEST_CASE(testParseConfigVirtualRouter)
{
  TestParser tp;

  const std::string xml {R"(
      <virtual-router>
        <entry name="vrf1">
          <interface>
            <member>eth0</member>
            <member>eth1</member>
          </interface>
          <routing-table>
            <ip>
              <static-route>
                <entry name="route1">
                  <destination>1.2.3.0/23</destination>
                  <nexthop>
                    <ip-address>1.2.3.1</ip-address>
                  </nexthop>
                  <interface>eth0</interface>
                  <metric>10</metric>
                </entry>
              </static-route>
            </ip>
          </routing-table>
        </entry>
        <entry name="vrf2">
          <interface>
            <member>eth2</member>
          </interface>
          <routing-table>
            <ip>
              <static-route>
                <entry name="route1">
                  <destination>1.2.3.0/23</destination>
                  <nexthop>
                    <ip-address>1.2.3.1</ip-address>
                  </nexthop>
                  <interface>eth0</interface>
                  <metric>10</metric>
                </entry>
              </static-route>
            </ip>
            <ipv6>
              <static-route>
                <entry name="route2">
                  <destination>1::0/123</destination>
                  <nexthop>
                    <ipv6-address>1::1</ipv6-address>
                  </nexthop>
                  <interface>eth1</interface>
                  <metric>20</metric>
                </entry>
              </static-route>
            </ipv6>
          </routing-table>
        </entry>
      </virtual-router>
    )"};

  const auto& test = tp.getFirstNode(xml.c_str());
  const auto& out {tp.parseConfigVirtualRouter(test)};

  BOOST_TEST_REQUIRE(2 == out.size());
  for (const auto& [_, vrf] : out) {
    BOOST_TEST(vrf.isValid());
  }

  auto dbgStr = out.at("vrf1").toDebugString();
  nmdp::testInString(dbgStr, "vrfId: vrf1, ifaces: [eth0, eth1]");
  nmdo::Route r1;
  r1.setVrfId("vrf1");
  r1.setProtocol("static");
  r1.setDescription("route1");
  r1.setDstIpNet(nmdo::IpAddress("1.2.3.0/23"));
  r1.setNextHopIpAddr(nmdo::IpAddress("1.2.3.1"));
  r1.setOutIfaceName("eth0");
  r1.setMetric(10);
  nmdp::testInString(dbgStr, r1.toDebugString());

  dbgStr = out.at("vrf2").toDebugString();
  nmdp::testInString(dbgStr, "vrfId: vrf2, ifaces: [eth2]");
  r1.setVrfId("vrf2");
  nmdp::testInString(dbgStr, r1.toDebugString());
  nmdo::Route r2;
  r2.setVrfId("vrf2");
  r2.setProtocol("static");
  r2.setDescription("route2");
  r2.setDstIpNet(nmdo::IpAddress("1::0/123"));
  r2.setNextHopIpAddr(nmdo::IpAddress("1::1"));
  r2.setOutIfaceName("eth1");
  r2.setMetric(20);
  nmdp::testInString(dbgStr, r2.toDebugString());
}

BOOST_AUTO_TEST_CASE(testParseConfigInterfaceEntry)
{
  TestParser tp;

  const std::string xml {R"(
      <interface>
        <entry name="eth0">
          <ip>    <entry name="1.2.3.4"/> </ip>
          <ipv6>  <entry name="1::2"/>    </ipv6>
        </entry>
        <entry name="eth1">
          <ip> <entry name="net1"/> </ip>
        </entry>
        <loopback name="lo0">
          <ip> <entry name="127.0.0.10"/> </ip>
        </loopback>
        <address>
          <entry name="net1">
            <ip-netmask>1.2.3.5/24</ip-netmask>
          </entry>
        </address>
      </interface>
    )"};

  const auto& test = tp.getFirstNode(xml.c_str());

  auto ifaceNode = test.select_node("entry[@name='eth0']").node();
  auto iface = tp.parseConfigInterfaceEntry(ifaceNode);
  BOOST_TEST(iface.isValid());
  auto dbgStr = iface.toDebugString();
  nmdp::testInString(dbgStr, "name: eth0,");
  nmdp::testInString(dbgStr, "ipAddrs: [[ipAddress: 1.2.3.4/32,");
  nmdp::testInString(dbgStr, ", [ipAddress: 1::2/128,");

  ifaceNode = test.select_node("entry[@name='eth1']").node();
  iface = tp.parseConfigInterfaceEntry(ifaceNode);
  BOOST_TEST(iface.isValid());
  dbgStr = iface.toDebugString();
  nmdp::testInString(dbgStr, "name: eth1,");
  nmdp::testInString(dbgStr, "ipAddrs: [[ipAddress: 1.2.3.5/24,");

  ifaceNode = test.select_node("loopback[@name='lo0']").node();
  iface = tp.parseConfigInterfaceEntry(ifaceNode);
  BOOST_TEST(iface.isValid());
  dbgStr = iface.toDebugString();
  nmdp::testInString(dbgStr, "name: loopback,");
  nmdp::testInString(dbgStr, "ipAddrs: [[ipAddress: 127.0.0.10/32,");
}

BOOST_AUTO_TEST_CASE(testParseConfigInterface)
{
  TestParser tp;

  const std::string xml {R"(
      <interfaces>
        <loopback name="lo0"/>
        <ethernet>
          <entry name="eth0"/>
          <entry name="eth1">
            <layer3>
              <units>
                <entry name="eth1.1"/>
              </units>
            </layer3>
          </entry>
        </ethernet>
        <tunnel>
          <units>
            <entry name="tun0"/>
          </units>
        </tunnel>
      </interfaces>
    )"};

  const auto& test = tp.getFirstNode(xml.c_str());
  const auto& out {tp.parseConfigInterface(test)};

  BOOST_TEST_REQUIRE(5 == out.size());
  for (const auto& [_, iface] : out) {
    BOOST_TEST(iface.isValid());
  }

  auto dbgStr = out.at("loopback").toDebugString();
  nmdp::testInString(dbgStr, "name: loopback,");
  nmdp::testInString(dbgStr, "mediaType: loopback,");

  dbgStr = out.at("eth0").toDebugString();
  nmdp::testInString(dbgStr, "name: eth0,");
  nmdp::testInString(dbgStr, "mediaType: ethernet,");

  dbgStr = out.at("eth1").toDebugString();
  nmdp::testInString(dbgStr, "name: eth1,");
  nmdp::testInString(dbgStr, "mediaType: ethernet,");

  dbgStr = out.at("eth1.1").toDebugString();
  nmdp::testInString(dbgStr, "name: eth1.1,");
  nmdp::testInString(dbgStr, "mediaType: ethernet,");
  nmdp::testInString(dbgStr, "mode: l3,");

  dbgStr = out.at("tun0").toDebugString();
  nmdp::testInString(dbgStr, "name: tun0,");
  nmdp::testInString(dbgStr, "mediaType: tunnel,");
}

BOOST_AUTO_TEST_CASE(testParseConfigDeviceconfig)
{
  TestParser tp;
  LogicalSystem logicalSystem;

  const std::string xml {R"(
      <deviceconfig>
        <system>
          <dns-setting>
            <servers>
              <primary>1.2.3.4</primary>
              <secondary>5.6.7.8</secondary>
            </servers>
          </dns-setting>
          <domain>some.fqdn</domain>
        </system>
        <system>
          <ntp-servers>
            <primary>
              <ntp-server-address>1.2.3.4</ntp-server-address>
            </primary>
            <secondary>
              <ntp-server-address>1.2.3.5</ntp-server-address>
            </secondary>
          </ntp-servers>
        </system>
      </deviceconfig>
    )"};

  const auto& test = tp.getFirstNode(xml.c_str());
  tp.parseConfigDeviceconfig(test, logicalSystem);

  BOOST_TEST_REQUIRE(4 == logicalSystem.services.size());
  for (const auto& service : logicalSystem.services) {
    BOOST_TEST(service.isValid());
  }

  auto dbgStr = logicalSystem.services[0].toDebugString();
  nmdp::testInString(dbgStr, "serviceName: dns,");
  nmdp::testInString(dbgStr, "dstAddress: [ipAddress: 1.2.3.4/32,");

  dbgStr = logicalSystem.services[1].toDebugString();
  nmdp::testInString(dbgStr, "serviceName: dns,");
  nmdp::testInString(dbgStr, "dstAddress: [ipAddress: 5.6.7.8/32,");

  BOOST_TEST_REQUIRE(2 == logicalSystem.dnsResolvers.size());
  for (const auto& resolver : logicalSystem.dnsResolvers) {
    BOOST_TEST(resolver.isValid());
  }
  dbgStr = logicalSystem.dnsResolvers[0].toDebugString();
  nmdp::testInString(dbgStr, "dstIpAddr: [ipAddress: 1.2.3.4/32,");
  dbgStr = logicalSystem.dnsResolvers[1].toDebugString();
  nmdp::testInString(dbgStr, "dstIpAddr: [ipAddress: 5.6.7.8/32,");

  BOOST_TEST_REQUIRE(1 == logicalSystem.dnsSearchDomains.size());
  BOOST_TEST(logicalSystem.dnsSearchDomains[0] == "some.fqdn");

  dbgStr = logicalSystem.services[2].toDebugString();
  nmdp::testInString(dbgStr, "serviceName: ntp,");
  nmdp::testInString(dbgStr, "dstAddress: [ipAddress: 1.2.3.4/32,");

  dbgStr = logicalSystem.services[3].toDebugString();
  nmdp::testInString(dbgStr, "serviceName: ntp,");
  nmdp::testInString(dbgStr, "dstAddress: [ipAddress: 1.2.3.5/32,");
}

BOOST_AUTO_TEST_CASE(testParseConfigVsys)
{
  TestParser tp;

  // pre-populate some data
  nmdo::InterfaceNetwork in1 {"eth0"};
  in1.setDescription("test");
  tp.data.logicalSystems[""].ifaces["eth0"] = in1;

  const std::string xml {R"(
      <vsys>
        <entry name="vsys1">
          <import>
            <network>
              <interface>
                <member>eth0</member>
                <member>eth1</member>
              </interface>
            </network>
          </import>
          <zone/>
          <address/>
          <address-group/>
          <service/>
          <service-group/>
          <rulebase/>
        </entry>
        <entry name="vsys2"/>
        <entry name="vsys3"/>
      </vsys>
      <config>
        <shared>
          <import>
            <network>
              <interface> <member>eth2</member> </interface>
            </network>
          </import>
          <zone/>
          <address/>
          <address-group/>
          <service/>
          <service-group/>
          <rulebase/>
        </shared>
      </config>
    )"};

  const auto& test = tp.getFirstNode(xml.c_str());
  tp.parseConfigVsys(test);

  const auto& out {tp.data.logicalSystems};
  BOOST_TEST_REQUIRE(4 == out.size());

  for (const auto& [name, ls] : out) {
    BOOST_TEST(ls.name == name);

    // validity tests for non-zero existence
    for (const auto& [_, iface] : ls.ifaces) {
      BOOST_TEST(iface.isValid());
    }
    for (const auto& service : ls.aclServices) {
      BOOST_TEST(service.isValid());
    }
    for (const auto& [_, ipNetSet] : ls.aclIpNetSets) {
      BOOST_TEST(ipNetSet.isValid());
    }

    // expected existence
    if ("" == name) {
      BOOST_TEST(1 == ls.ifaces.size());
      BOOST_TEST(0 == ls.aclServices.size());
    } else if ("vsys1" == name) {
      BOOST_TEST(1 == ls.ifaces.size());
      BOOST_TEST(2 == ls.aclServices.size());
    } else {
      BOOST_TEST(0 == ls.ifaces.size());
      BOOST_TEST(1 == ls.aclServices.size());
    }
    BOOST_TEST(0 == ls.vrfs.size());
    BOOST_TEST(0 == ls.services.size());
    BOOST_TEST(0 == ls.dnsResolvers.size());
    BOOST_TEST(0 == ls.dnsSearchDomains.size());
    BOOST_TEST(0 == ls.aclZones.size());
    if ("" == name) {
      BOOST_TEST(0 == ls.aclIpNetSets.size());
    } else {
      BOOST_TEST(1 == ls.aclIpNetSets.size());
      BOOST_TEST(ls.aclIpNetSets.at("any").isValid());
      auto dbgStr {ls.aclIpNetSets.at("any").toDebugString()};
      nmdp::testInString(dbgStr, "id: any,");
      nmdp::testInString(dbgStr, "ipNets: [[ipNetwork: 0.0.0.0/0,");
      nmdp::testInString(dbgStr, ", [ipNetwork: ::/0,");
    }
    BOOST_TEST(0 == ls.aclRules.size());
  }
}

BOOST_AUTO_TEST_CASE(testParseConfig)
{
  TestParser tp;

  // Full sample config
  // - most logic should be tested elsewhere, so this is flow focused
  const std::string xml {R"(
      <config>
        <devices>
          <entry>
            <deviceconfig>
              <system>
                <dns-setting>
                  <servers>
                    <primary>8.8.8.8</primary>
                  </servers>
                </dns-setting>
                <domain>example.com</domain>
                <ntp-servers>
                  <primary>
                    <ntp-server-address>192.168.1.1</ntp-server-address>
                  </primary>
                </ntp-servers>
              </system>
            </deviceconfig>
            <network>
              <interface>
                <loopback name="lo0"/>
                <ethernet>
                  <entry name="eth0">
                    <ip> <entry name="192.168.1.1"/> </ip>
                  </entry>
                </ethernet>
              </interface>
              <virtual-router>
                <entry name="vrf1"/>
                <entry name="vrf 99"/>
              </virtual-router>
            </network>
            <vsys>
              <entry name="vsys1">
                <import>
                  <network>
                    <interface> <member>eth0</member> </interface>
                  </network>
                </import>
                <zone>
                  <entry name="zone1">
                    <network>
                      <layer3> <member>eth0</member> </layer3>
                    </network>
                  </entry>
                </zone>
                <address>
                  <entry name="address1">
                    <ip-netmask>192.168.1.0/24</ip-netmask>
                  </entry>
                </address>
                <address-group>
                  <entry name="group1">
                    <static> <member>address1</member> </static>
                  </entry>
                </address-group>
                <service>
                  <entry name="service1">
                    <protocol> <tcp> <port>80</port> </tcp> </protocol>
                  </entry>
                </service>
                <service-group>
                  <entry name="groupService1">
                    <members> <member>service1</member> </members>
                  </entry>
                </service-group>
                <rulebase>
                  <default-security-rules>
                    <rules> <entry> <action>drop</action> </entry> </rules>
                  </default-security-rules>
                </rulebase>
              </entry>
            </vsys>
          </entry>
        </devices>
      </config>
    )"};

  const auto& test = tp.getFirstNode(xml.c_str());
  tp.parseConfig(test);

  auto& out1 = tp.data.logicalSystems.at("");

  BOOST_TEST_REQUIRE(2 == out1.services.size());
  auto dbgStr = out1.services[0].toDebugString();
  nmdp::testInString(dbgStr, "serviceName: dns,");
  dbgStr = out1.services[1].toDebugString();
  nmdp::testInString(dbgStr, "serviceName: ntp,");

  BOOST_TEST_REQUIRE(1 == out1.dnsResolvers.size());
  dbgStr = out1.dnsResolvers[0].toDebugString();
  nmdp::testInString(dbgStr, "dstIpAddr: [ipAddress: 8.8.8.8");

  BOOST_TEST_REQUIRE(1 == out1.dnsSearchDomains.size());
  BOOST_TEST("example.com" == out1.dnsSearchDomains[0]);

  BOOST_TEST_REQUIRE(2 == out1.ifaces.size());
  for (const auto& [name, iface] : out1.ifaces) {
    const auto& test {std::format("name: {},", name)};
    nmdp::testInString(iface.toDebugString(), test);
  }

  BOOST_TEST_REQUIRE(2 == out1.vrfs.size());
  for (const auto& [name, vrf] : out1.vrfs) {
    const auto& test {std::format("vrfId: {}, ifaces:", name)};
    nmdp::testInString(vrf.toDebugString(), test);
  }

  auto& out2 = tp.data.logicalSystems.at("vsys1");
  BOOST_TEST(out2.name == "vsys1");

  bool t1 {out2.ifaces.end() != out2.ifaces.find("eth0")};
  BOOST_TEST(t1);
  BOOST_TEST_REQUIRE(1 == out2.ifaces.size());
  dbgStr = out2.ifaces.at("eth0").toDebugString();
  nmdp::testInString(dbgStr, "name: eth0,");
  nmdp::testInString(dbgStr, "ipAddress: 192.168.1.1/32,");

  BOOST_TEST_REQUIRE(1 == out2.aclZones.size());
  dbgStr = out2.aclZones.at("zone1").toDebugString();
  nmdp::testInString(dbgStr, "id: zone1,");

  BOOST_TEST_REQUIRE(3 == out2.aclIpNetSets.size());
  for (const auto& [name, set] : out2.aclIpNetSets) {
    const auto& test {std::format("id: {},", name)};
    nmdp::testInString(set.toDebugString(), test);
  }

  BOOST_TEST_REQUIRE(3 == out2.aclServices.size());
  dbgStr = out2.aclServices[0].toDebugString();
  nmdp::testInString(dbgStr, "id: any,");
  dbgStr = out2.aclServices[1].toDebugString();
  nmdp::testInString(dbgStr, "id: service1,");
  dbgStr = out2.aclServices[2].toDebugString();
  nmdp::testInString(dbgStr, "id: groupService1,");

  BOOST_TEST_REQUIRE(1 == out2.aclRules.size());
  dbgStr = out2.aclRules[0].toDebugString();
  nmdp::testInString(dbgStr, "action: block,");
}
