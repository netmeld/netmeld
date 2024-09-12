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

BOOST_AUTO_TEST_CASE(testParseConfigService)
{
  TestParser tp;
  // Sample XML data
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

  auto test = tp.getFirstNode(xml.c_str());
  auto out {tp.parseConfigService(test)};

  BOOST_TEST_REQUIRE(5 == out.size());

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

BOOST_AUTO_TEST_CASE(testParseConfigServiceGroup)
{
  TestParser tp;
  // Sample XML data
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

  auto test = tp.getFirstNode(xml.c_str());
  auto out {tp.parseConfigServiceGroup(test)};

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

  auto test = tp.getFirstNode(xml.c_str());
  auto out {tp.parseConfigRulebase(test, tp.ls)};

  BOOST_TEST_REQUIRE(4 == out.size());

  nmdp::testInString(out[0].toDebugString(), "priority: 1000000,");
  nmdp::testInString(out[1].toDebugString(), "priority: 1000001,");
  nmdp::testInString(out[2].toDebugString(), "priority: 2000000,");
  nmdp::testInString(out[3].toDebugString(), "priority: 2000001,");

  for (size_t i {0}; i < out.size(); ++i) {
    const auto dbgStr {out[i].toDebugString()};
    nmdp::testInString(dbgStr, "action: block,");
    nmdp::testInString(dbgStr, "incomingZoneId: any,");
    nmdp::testInString(dbgStr, "outgoingZoneId: any,");
    nmdp::testInString(dbgStr, "srcIpNetSetId: any,");
    nmdp::testInString(dbgStr, "dstIpNetSetId: any,");
    nmdp::testInString(dbgStr, "description: ],");
    nmdp::testInString(dbgStr, "serviceId: any]");
  }
}

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

  auto test = tp.getFirstNode(xml.c_str());
  auto out {tp.parseConfigRules(test, 0, tp.ls)};

  BOOST_TEST_REQUIRE(23 == out.size());

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
