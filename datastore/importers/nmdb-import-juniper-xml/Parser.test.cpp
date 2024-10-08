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
    using Parser::parseRoute;

    pugi::xml_node
    getNode(const char* xmlData, const std::string& nName="")
    {
      pugi::xml_parse_result result = doc.load_string(xmlData);
      BOOST_REQUIRE_MESSAGE( result
                           , std::format("Failed to load XML: {}\n", xmlData)
                           );

      if (!nName.empty()) {
        for (const auto& node : doc.children()) {
          if (nName == node.name()) {
            return node;
          }
        }
        BOOST_TEST(false, std::format("Failed to find XML node: {}\n", nName));
      }

      return doc.document_element();
    }
};

BOOST_AUTO_TEST_CASE(testEthernetSwitching)
{
  TestParser tp;

  const std::string xml {R"(
      <root>
        <l2ng-l2rtb-evpn-arp-entry>
          <l2ng-l2-mac-logical-interface>eth1</l2ng-l2-mac-logical-interface>
          <l2ng-l2-vlan-id>123</l2ng-l2-vlan-id>
          <l2ng-l2-mac-address>00:11:22:33:44:55</l2ng-l2-mac-address>
          <l2ng-l2-evpn-arp-inet-address>1.2.3.4</l2ng-l2-evpn-arp-inet-address>
        </l2ng-l2rtb-evpn-arp-entry>
        <l2ng-l2rtb-evpn-nd-entry>
          <l2ng-l2-mac-logical-interface>eth2/1</l2ng-l2-mac-logical-interface>
          <l2ng-l2-mac-address>00:11:22:33:44:55</l2ng-l2-mac-address>
          <l2ng-l2-evpn-nd-inet6-address>1::2</l2ng-l2-evpn-nd-inet6-address>
        </l2ng-l2rtb-evpn-nd-entry>
        <l2ng-l2ald-mac-entry-vlan>
          <l2ng-l2-mac-logical-interface>eth3</l2ng-l2-mac-logical-interface>
          <l2ng-l2-vlan-id>none</l2ng-l2-vlan-id>
          <l2ng-l2-mac-address>00:11:22:33:44:55</l2ng-l2-mac-address>
        </l2ng-l2ald-mac-entry-vlan>
        <l2ng-l2ald-mac-ip-entry>
          <l2ng-l2-mac-logical-interface>eth4</l2ng-l2-mac-logical-interface>
          <l2ng-l2-vlan-id>123</l2ng-l2-vlan-id>
          <l2ng-l2-mac-address>00:11:22:33:44:55</l2ng-l2-mac-address>
          <l2ng-l2-ip-address>1.2.3.4</l2ng-l2-ip-address>
        </l2ng-l2ald-mac-ip-entry>
        <l2ng-l2ald-mac-ip-entry>
          <!-- no l2ng-l2-mac-logical-interface, so don't process -->
          <l2ng-l2-vlan-id>456</l2ng-l2-vlan-id>
          <l2ng-l2-mac-address>66:77:88:99:AA:BB</l2ng-l2-mac-address>
          <l2ng-l2-ip-address>5.6.7.8</l2ng-l2-ip-address>
        </l2ng-l2ald-mac-ip-entry>
      </root>
    )"};

  const auto& test = tp.getNode(xml.c_str());
  tp.parseEthernetSwitching(test);

  const auto& out {tp.data.logicalSystems[""].ifaces};
  BOOST_TEST(4 == out.size());

  for (const auto& [name, iface] : out) {
    BOOST_TEST(iface.isValid());
    const auto& dbgStr {iface.toDebugString()};
    nmdp::testInString(dbgStr, std::format("name: {},", name));
    nmdp::testInString(dbgStr, "macAddress: 00:11:22:33:44:55,");

    if ("eth1" == name || "eth4" == name) {
      nmdp::testInString(dbgStr, "vlanId: 123,");
      nmdp::testInString(dbgStr, "ipAddress: 1.2.3.4/32,");
    } else {
      nmdp::testInString(dbgStr, "vlans: [],");
    }

    if ("eth1" == name || "eth4" == name) {
      nmdp::testInString(dbgStr, "ipAddress: 1.2.3.4/32,");
    } else if ("eth2/1" == name) {
      nmdp::testInString(dbgStr, "ipAddress: 1::2/128,");
    } else {
      nmdp::testInString(dbgStr, "44:55, ipAddrs: []");
    }
  }
}

BOOST_AUTO_TEST_CASE(testLldpNeighborInfo)
{
  TestParser tp;

  const std::string xml {R"(
      <root>
        <lldp-neighbor-information>
          <lldp-local-port-id>eth1</lldp-local-port-id>
          <lldp-local-parent-interface-name>eth0</lldp-local-parent-interface-name>
          <lldp-remote-chassis-id-subtype>Mac address</lldp-remote-chassis-id-subtype>
          <lldp-remote-chassis-id>00:11:22:33:44:55</lldp-remote-chassis-id>
          <lldp-remote-port-id-subtype>Mac address</lldp-remote-port-id-subtype>
          <lldp-remote-port-id>00:11:22:33:44:55</lldp-remote-port-id>
        </lldp-neighbor-information>
        <lldp-neighbor-information inactive="inactive">
          <lldp-local-port-id>eth2</lldp-local-port-id>
          <lldp-remote-chassis-id-subtype>Mac address</lldp-remote-chassis-id-subtype>
          <lldp-remote-chassis-id>66:77:88:99:AA:BB</lldp-remote-chassis-id>
        </lldp-neighbor-information>
        <lldp-neighbor-information>
          <lldp-local-port-id>eth3</lldp-local-port-id>
          <lldp-remote-chassis-id-subtype>Mac address</lldp-remote-chassis-id-subtype>
          <lldp-remote-chassis-id>00:11:22:33:44:55</lldp-remote-chassis-id>
          <lldp-remote-port-id-subtype>Mac address</lldp-remote-port-id-subtype>
          <lldp-remote-port-id>00:11:22:33:44:55</lldp-remote-port-id>
        </lldp-neighbor-information>

        <!-- other nodes not handled, but possibly should be:
          <lldp-remote-system-name>host.fqdn</lldp-remote-system-name>

          <lldp-remote-port-description>host_eth1/2</lldp-remote-port-description>
          <lldp-remote-port-description>host eth1/2</lldp-remote-port-description>

          <lldp-remote-port-id-subtype>Interface name</lldp-remote-port-id-subtype>
          <lldp-remote-port-id>eth0/1</lldp-remote-port-id>

          <lldp-remote-port-id-subtype>Locally assigned</lldp-remote-port-id-subtype>
          <lldp-remote-port-id>123</lldp-remote-port-id>
        -->
      </root>
    )"};

  const auto& test = tp.getNode(xml.c_str());
  tp.parseLldpNeighborInfo(test);

  const auto& out {tp.data.logicalSystems[""].ifaces};
  BOOST_TEST(3 == out.size());

  for (const auto& [name, iface] : out) {
    BOOST_TEST(iface.isValid());
    const auto& dbgStr {iface.toDebugString()};
    nmdp::testInString(dbgStr, std::format("name: {},", name));
    nmdp::testInString(dbgStr, "isPartial: true,");
    nmdp::testInString(dbgStr, "isDiscoveryProtocolEnabled: true,");

    if ("eth0" == name || "eth1" == name || "eth3" == name) {
      nmdp::testInString( dbgStr
                        , "reachableMacs: [[macAddress: 00:11:22:33:44:55,"
                        );
    } else {
      BOOST_TEST(false, std::format("Unhandled iface: {}\n", name));
    }
  }
}

BOOST_AUTO_TEST_CASE(testIpv6NeighborInfo)
{
  TestParser tp;

  const std::string xml {R"(
      <root>
        <ipv6-nd-entry>
          <ipv6-nd-interface-name>eth1</ipv6-nd-interface-name>
          <ipv6-nd-neighbor-l2-address>00:11:22:33:44:55</ipv6-nd-neighbor-l2-address>
          <ipv6-nd-neighbor-address>1::2</ipv6-nd-neighbor-address>
        </ipv6-nd-entry>
        <ipv6-nd-entry>
          <ipv6-nd-interface-name>eth2</ipv6-nd-interface-name>
          <ipv6-nd-neighbor-l2-address>66:77:88:99:AA:BB</ipv6-nd-neighbor-l2-address>
          <ipv6-nd-neighbor-address>1::3</ipv6-nd-neighbor-address>
        </ipv6-nd-entry>
        <ipv6-nd-entry>
          <ipv6-nd-interface-name>eth3</ipv6-nd-interface-name>
          <ipv6-nd-neighbor-l2-address>00:11:22:33:44:55</ipv6-nd-neighbor-l2-address>
          <ipv6-nd-neighbor-address>1::2</ipv6-nd-neighbor-address>
        </ipv6-nd-entry>

        <!-- other nodes not handled, but possibly should be:
          <ipv6-nd-state>reachable</ipv6-nd-state>
          <ipv6-nd-state>delay</ipv6-nd-state>
          <ipv6-nd-state>stale</ipv6-nd-state>

          <ipv6-nd-isrouter>yes</ipv6-nd-isrouter>
          <ipv6-nd-isrouter>no</ipv6-nd-isrouter>
        -->

      </root>
    )"};

  const auto& test = tp.getNode(xml.c_str());
  tp.parseIpv6NeighborInfo(test);

  const auto& out {tp.data.logicalSystems[""].ifaces};
  BOOST_TEST(3 == out.size());

  for (const auto& [name, iface] : out) {
    BOOST_TEST(iface.isValid());
    const auto& dbgStr {iface.toDebugString()};
    nmdp::testInString(dbgStr, std::format("name: {},", name));
    nmdp::testInString(dbgStr, "isPartial: true,");
    nmdp::testInString(dbgStr, "isDiscoveryProtocolEnabled: true,");
    nmdp::testInString(dbgStr, "isResponding: true]]");

    if ("eth1" == name || "eth3" == name) {
      nmdp::testInString( dbgStr
                        , "reachableMacs: [[macAddress: 00:11:22:33:44:55,"
                        );
      nmdp::testInString(dbgStr, "ipAddress: 1::2/128,");
    } else if ("eth2" == name) {
      nmdp::testInString( dbgStr
                        , "reachableMacs: [[macAddress: 66:77:88:99:aa:bb,"
                        );
      nmdp::testInString(dbgStr, "ipAddress: 1::3/128,");
    } else {
      BOOST_TEST(false, std::format("Unhandled iface: {}\n", name));
    }
  }
}

BOOST_AUTO_TEST_CASE(testArpTableInfo)
{
  TestParser tp;

  const std::string xml {R"(
      <root>
        <arp-table-entry>
          <interface-name>eth1</interface-name>
          <mac-address>00:11:22:33:44:55</mac-address>
          <ip-address>1.2.3.4</ip-address>
        </arp-table-entry>
        <arp-table-entry>
          <interface-name>eth2</interface-name>
          <mac-address>66:77:88:99:AA:BB</mac-address>
          <ip-address>1.2.3.5</ip-address>
        </arp-table-entry>
        <arp-table-entry>
          <interface-name>eth3 [eth99]</interface-name>
          <mac-address>00:11:22:33:44:55</mac-address>
          <ip-address>1.2.3.4</ip-address>
        </arp-table-entry>
      </root>
    )"};

  const auto& test = tp.getNode(xml.c_str());
  tp.parseArpTableInfo(test);

  const auto& out {tp.data.logicalSystems[""].ifaces};
  BOOST_TEST(3 == out.size());

  for (const auto& [name, iface] : out) {
    BOOST_TEST(iface.isValid());
    const auto& dbgStr {iface.toDebugString()};
    nmdp::testInString(dbgStr, std::format("name: {},", name));
    nmdp::testInString(dbgStr, "isPartial: true,");
    nmdp::testInString(dbgStr, "isDiscoveryProtocolEnabled: true,");
    nmdp::testInString(dbgStr, "isResponding: true]]");

    if ("eth1" == name || "eth3" == name) {
      nmdp::testInString( dbgStr
                        , "reachableMacs: [[macAddress: 00:11:22:33:44:55,"
                        );
      nmdp::testInString(dbgStr, "ipAddress: 1.2.3.4/32,");
    } else if ("eth2" == name) {
      nmdp::testInString(dbgStr, "reachableMacs: [[macAddress: 66:77:88:99:aa:bb,");
      nmdp::testInString(dbgStr, "ipAddress: 1.2.3.5/32,");
    } else {
      BOOST_TEST(false, std::format("Unhandled iface: {}\n", name));
    }
  }
}

BOOST_AUTO_TEST_CASE(testParseRoute)
{
  TestParser tp;

  const std::string xml {R"(
      <root>
        <rt-destination>1.2.3.0</rt-destination>
        <rt-prefix-length>24</rt-prefix-length>

        <rt-entry> <!-- Active route with all attributes -->
          <active-tag>*</active-tag>
          <protocol-name>abc123</protocol-name>
          <preference>123</preference>
          <metric>12</metric>
          <nh-type>Local</nh-type>
          <nh>
            <nh-table>inet.0</nh-table>
            <to>1.2.3.1</to>
            <via>eth0</via>
          </nh>
        </rt-entry>

        <rt-entry> <!-- Inactive route with all attributes -->
          <active-tag>!</active-tag>
          <protocol-name>ab12</protocol-name>
          <preference>12</preference>
          <metric>1</metric>
          <nh-type>Local</nh-type>
          <nh>
            <nh-table>ab12.inet6.0</nh-table>
            <to>1::1</to>
            <nh-local-interface>eth1</nh-local-interface>
          </nh>
        </rt-entry>

        <rt-entry> <!-- Discard next-hop type -->
          <active-tag>*</active-tag>
          <nh-type>Discard</nh-type>
        </rt-entry>

        <rt-entry> <!-- Reject next-hop type -->
          <active-tag>*</active-tag>
          <nh-type>Reject</nh-type>
        </rt-entry>

        <rt-entry> <!-- Next-hop table only; name.type.instance -->
          <active-tag>*</active-tag>
          <nh>
            <nh-table>a_Bc-123.type.123</nh-table>
          </nh>
        </rt-entry>
      </root>
    )"};

  const auto& test = tp.getNode(xml.c_str());
  const auto& routes = tp.parseRoute(test);

  BOOST_TEST(5 == routes.size());
  for (size_t i {0}; i < routes.size(); ++i) {
    const auto& route {routes[i]};
    BOOST_TEST(route.isValid());

    const auto& dbgStr {route.toDebugString()};
    nmdp::testInString(dbgStr, "dstIpNet: [ipNetwork: 1.2.3.0/24,");

    if (1 == i) {
      nmdp::testInString(dbgStr, "isActive: false,");
    } else {
      nmdp::testInString(dbgStr, "isActive: true,");
    }

    if (0 == i) {
      nmdp::testInString(dbgStr, "protocol: abc123,");
      nmdp::testInString(dbgStr, "adminDistance: 123,");
      nmdp::testInString(dbgStr, "metric: 12,");
    } else if (1 == i) {
      nmdp::testInString(dbgStr, "protocol: ab12,");
      nmdp::testInString(dbgStr, "adminDistance: 12,");
      nmdp::testInString(dbgStr, "metric: 1,");
    } else {
      nmdp::testInString(dbgStr, "protocol: ,");
      nmdp::testInString(dbgStr, "adminDistance: 0,");
      nmdp::testInString(dbgStr, "metric: 0,");
    }

    if (0 == i) {
      nmdp::testInString(dbgStr, "outIfaceName: eth0,");
      nmdp::testInString(dbgStr, "isNullRoute: false,");
    } else if (1 == i) {
      nmdp::testInString(dbgStr, "outIfaceName: eth1,");
      nmdp::testInString(dbgStr, "isNullRoute: false,");
    } else if (2 == i) {
      nmdp::testInString(dbgStr, "outIfaceName: discard,");
      nmdp::testInString(dbgStr, "isNullRoute: true,");
    } else if (3 == i) {
      nmdp::testInString(dbgStr, "outIfaceName: reject,");
      nmdp::testInString(dbgStr, "isNullRoute: true,");
    } else {
      nmdp::testInString(dbgStr, "outIfaceName: ,");
      nmdp::testInString(dbgStr, "isNullRoute: false,");
    }

    if (0 == i) {
      nmdp::testInString(dbgStr, "nextVrfId: , nextTableId: inet.0,");
    } else if (1 == i) {
      nmdp::testInString(dbgStr, "nextVrfId: ab12, nextTableId: inet6.0,");
    } else if (4 == i) {
      nmdp::testInString(dbgStr, "nextVrfId: a_Bc-123, nextTableId: type.123,");
    } else {
      nmdp::testInString(dbgStr, "nextVrfId: , nextTableId: ,");
    }

    if (0 == i) {
      nmdp::testInString(dbgStr, "nextHopIpAddr: [ipAddress: 1.2.3.1/32,");
    } else if (1 == i) {
      nmdp::testInString(dbgStr, "nextHopIpAddr: [ipAddress: 1::1/128,");
    } else {
      nmdp::testInString(dbgStr, "nextHopIpAddr: [ipAddress: 0.0.0.0/0,");
    }
  }
}

BOOST_AUTO_TEST_CASE(testParseRouteMulticast)
{
  TestParser tp;

  const std::string xml {R"(
      <root>
        <!-- Skipped -->
        <rt-destination>255.2.3.0, 1.2.3.1</rt-destination>
        <rt-prefix-length>24</rt-prefix-length>
        <rt-entry>
          <active-tag>*</active-tag>
          <protocol-name>abc123</protocol-name>
          <preference>123</preference>
          <metric>12</metric>
          <nh-type>Local</nh-type>
          <nh>
            <nh-table>inet.0</nh-table>
            <to>1.2.3.1</to>
            <via>eth0</via>
          </nh>
        </rt-entry>
      </root>
    )"};

  const auto& test = tp.getNode(xml.c_str());
  const auto& routes = tp.parseRoute(test);

  BOOST_TEST(0 == routes.size());
}

BOOST_AUTO_TEST_CASE(testParseRouteInfo)
{
  TestParser tp;
  // Also tests: parseRouteLogicalSystemName
  // Also tests: parseRouteTable

  const auto& out {tp.data.logicalSystems};

  { // Route table with logical system
    const std::string xml {R"(
        <output>logical-system: aB12</output>
        <route-info>
          <route-table>
            <table-name>abc123.inet.0</table-name>
            <rt>
              <rt-destination>1.2.3.0</rt-destination>
              <rt-prefix-length>24</rt-prefix-length>
              <rt-entry>
                <active-tag>*</active-tag>
                <nh-type>Discard</nh-type>
              </rt-entry>
            </rt>
          </route-table>
        </route-info>
      )"};

    const auto& test = tp.getNode(xml.c_str(), "route-info");
    tp.parseRouteInfo(test);
    BOOST_TEST(1 == out.size());
  }

  { // Route table with default logical system
    const std::string xml {R"(
      <output>logical-system: default</output>
      <route-info>
        <route-table>
          <table-name>ab12.inet6.0</table-name>
          <rt>
            <rt-destination>1::0</rt-destination>
            <rt-prefix-length>64</rt-prefix-length>
            <rt-entry>
              <active-tag>*</active-tag>
              <nh-type>Discard</nh-type>
            </rt-entry>
          </rt>
        </route-table>
      </route-info>
      )"};

    const auto& test = tp.getNode(xml.c_str(), "route-info");
    tp.parseRouteInfo(test);
    BOOST_TEST(2 == out.size());
  }

  { // Ignored route table (non-inet); "unknown" logical system
    const std::string xml {R"(
      <output>logical-system: abc 123</output>
      <route-info>
        <route-table>
          <table-name>abc123.evpn.0</table-name>
        </route-table>
      </route-info>
      )"};

    const auto& test = tp.getNode(xml.c_str(), "route-info");
    tp.parseRouteInfo(test);
    BOOST_TEST(3 == out.size());
  }

  { // Inactive route table
    const std::string xml {R"(
      <route-info>
        <route-table inactive="inactive">
          <table-name>abc123.inet.0</table-name>
          <rt>
              <rt-destination>1.2.3.0</rt-destination>
              <rt-prefix-length>24</rt-prefix-length>
              <rt-entry>
                <active-tag>*</active-tag>
              </rt-entry>
          </rt>
        </route-table>
      </route-info>
      )"};

    const auto& test = tp.getNode(xml.c_str());
    tp.parseRouteInfo(test);
    BOOST_TEST(3 == out.size());
  }

  { // Route table with inactive route
    const std::string xml {R"(
      <route-info>
        <route-table>
          <table-name>abc123.inet.0</table-name>
          <rt inactive="inactive">
              <rt-destination>1.2.3.0</rt-destination>
              <rt-prefix-length>24</rt-prefix-length>
              <rt-entry>
                <active-tag>*</active-tag>
              </rt-entry>
          </rt>
        </route-table>
      </route-info>
      )"};

    const auto& test = tp.getNode(xml.c_str());
    tp.parseRouteInfo(test);
    BOOST_TEST(3 == out.size());
  }

  { // post checks; aggregate of all tables
    BOOST_TEST(3 == out.size());
    for (const auto& [name, ls] : out) {
      if ("" == name) {
        BOOST_TEST(2 == ls.vrfs.size());
        for (const auto& [name, vrf] : ls.vrfs) {
          const auto& dbgStr {vrf.toDebugString()};
          nmdp::testInString( dbgStr
                            , std::format("[vrfId: {}, ifaces: [],", name)
                            );
          if ("ab12" == name) {
            nmdp::testInString(dbgStr, "isNullRoute: true,");
            nmdp::testInString(dbgStr, "routes: [[vrfId: ab12,");
          } else if ("abc123" == name) {
            nmdp::testInString(dbgStr, "routes: []]");
          } else {
            BOOST_TEST(false, std::format("Unhandled VRF: {}\n", name));
          }
        }
      } else if ("unknown" == name) {
        BOOST_TEST(0 == ls.vrfs.size());
      } else if ("ab12" == name) {
        BOOST_TEST(1 == ls.vrfs.size());
        const auto& dbgStr {ls.vrfs.at("abc123").toDebugString()};
        nmdp::testInString(dbgStr, "[vrfId: abc123, ifaces: []");
        nmdp::testInString(dbgStr, "isNullRoute: true,");
      } else {
        BOOST_TEST(false, std::format("Unchecked LS: {}\n", name));
      }
    }
  }
}
