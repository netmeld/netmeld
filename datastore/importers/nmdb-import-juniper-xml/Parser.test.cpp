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
    using Parser::parseConfigPolicies;
    using Parser::parseConfigApplicationOrTerm;
    using Parser::parseConfigApplications;
    using Parser::parseConfigAddressBook;
    using Parser::parseConfigZones;
    using Parser::parseConfigRoutingOptions;
    using Parser::parseConfigRoutingInstances;
    using Parser::parseConfigInterfaces;

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
        BOOST_TEST(false, std::format("Unchecked logical system: {}\n", name));
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(testParseConfigPolicies)
{
  TestParser tp;
  // Also tests:
  // - parseConfigPolicy

/*
<security>
  <zones>
    <security-zone>
      <address-book>
      </address-book>
    </security-zone>
  </zones>
  <policies>
    <policy>
      <from-zone-name>z1</from-zone-name>
      <to-zone-name>z2</to-zone-name>
      <policy>
        <name>pn1</name>
        <description>descript1</description>
        <match>
          <source-address>1.2.3.0/24</source-address>
          <destination-address>2.3.4.0/24</destination-address>
          <application>app1</application>
        </match>
        <then>
          <permit></permit> | <deny></deny> | <reject></reject>
          <log> <session-init/> ...  <session-close/> </log>
        </then>
      </policy>
    </policy>
    <global>
      <policy>
        <name>pn1</name>
        <description>descript1</description>
        <match>
          <source-address>1.2.3.0/24</source-address>
          <destination-address>2.3.4.0/24</destination-address>
          <application>app1</application>
          <from-zone>z1</from-zone>
          <to-zone>z1</to-zone>
        </match>
        <then>
          <permit></permit> | <deny></deny> | <reject></reject>
          <log> <session-init/> ...  <session-close/> </log>
        </then>
      </policy>
    </global>
  </policies>
<security>
*/
  { // intra-zone; none/any; deny
    const std::string xml {R"(
        <policies>
          <policy>
            <policy>
              <then> <deny /> </then>
            </policy>
          </policy>
        </policies>
      )"};

    const auto& test = tp.getNode(xml.c_str());
    const auto& out {tp.parseConfigPolicies(test)};

    BOOST_TEST(1 == out.size());
    for (size_t i {0}; i < out.size(); ++i) {
      const auto& aclRuleService {out[i]};
      BOOST_TEST(aclRuleService.isValid());

      const auto& dbgStr {aclRuleService.toDebugString()};
      nmdp::testInString(dbgStr, "priority: 2000000,");
      nmdp::testInString(dbgStr, "action: block,");
      nmdp::testInString(dbgStr, "incomingZoneId: any,");
      nmdp::testInString(dbgStr, "outgoingZoneId: any,");
      nmdp::testInString(dbgStr, "srcIpNetSetNamespace: global,");
      nmdp::testInString(dbgStr, "srcIpNetSetId: any,");
      nmdp::testInString(dbgStr, "dstIpNetSetNamespace: global,");
      nmdp::testInString(dbgStr, "dstIpNetSetId: any,");
      nmdp::testInString(dbgStr, "description: ],");
      nmdp::testInString(dbgStr, "serviceId: any]");
    }
  }

  { // intra-zone; invalid
    const std::string xml {R"(
        <policies>
          <policy>
            <policy>
            </policy>
          </policy>
        </policies>
      )"};

    const auto& test = tp.getNode(xml.c_str());
    const auto& out {tp.parseConfigPolicies(test)};

    BOOST_TEST(1 == out.size());
    for (size_t i {0}; i < out.size(); ++i) {
      const auto& aclRuleService {out[i]};
      BOOST_TEST(!aclRuleService.isValid());
    }
  }

  { // inter-zone; single; "zone" address book; reject
    const std::string xml {R"(
        <zones> <security-zone> <address-book /> </security-zone> </zones>
        <policies>
          <policy>
            <from-zone-name>z1</from-zone-name>
            <policy>
              <name>pn1</name>
              <description>descript1</description>
              <match>
                <source-address>1.2.3.0/24</source-address>
                <destination-address>2.3.4.0/24</destination-address>
                <application>app1</application>
              </match>
              <then>
                <reject />
                <log>
                  <session-init />
                  <session-close />
                </log>
              </then>
            </policy>
          </policy>
        </policies>
      )"};

    const auto& test = tp.getNode(xml.c_str(), "policies");
    const auto& out {tp.parseConfigPolicies(test)};

    BOOST_TEST(1 == out.size());
    for (size_t i {0}; i < out.size(); ++i) {
      const auto& aclRuleService {out[i]};
      BOOST_TEST(aclRuleService.isValid());

      const auto& dbgStr {aclRuleService.toDebugString()};
      nmdp::testInString(dbgStr, "priority: 3000000,");
      nmdp::testInString(dbgStr, "action: block,");
      nmdp::testInString(dbgStr, "incomingZoneId: z1,");
      nmdp::testInString(dbgStr, "outgoingZoneId: any,");
      nmdp::testInString(dbgStr, "srcIpNetSetNamespace: z1,");
      nmdp::testInString(dbgStr, "srcIpNetSetId: 1.2.3.0/24,");
      nmdp::testInString(dbgStr, "dstIpNetSetNamespace: global,");
      nmdp::testInString(dbgStr, "dstIpNetSetId: 2.3.4.0/24,");
      nmdp::testInString(dbgStr, "description: pn1],");
      nmdp::testInString(dbgStr, "serviceId: app1]");
    }
  }

  { // inter-zone; multi
    const std::string xml {R"(
        <policies>
          <policy>
            <from-zone-name>z1</from-zone-name>
            <to-zone-name>z2</to-zone-name>
            <!-- Singular; +(1*1*1) -->
            <policy>
              <name>pn1</name>
              <match>
                <source-address>1.2.3.0/24</source-address>
                <destination-address>2.3.4.0/24</destination-address>
                <application>app1</application>
              </match>
              <then> <permit /> </then>
            </policy>
            <!-- Multi; +(2*2*3) -->
            <policy>
              <name>pn1</name>
              <match>
                <source-address>1.2.3.0/24</source-address>
                <source-address>1.2.3.0/24</source-address>
                <destination-address>2.3.4.0/24</destination-address>
                <destination-address>2.3.4.0/24</destination-address>
                <application>app1</application>
                <application>app1</application>
                <application>app1</application>
              </match>
              <then> <permit /> </then>
            </policy>
          </policy>
        </policies>
      )"};

    const auto& test = tp.getNode(xml.c_str());
    const auto& out {tp.parseConfigPolicies(test)};

    BOOST_TEST(13 == out.size());
    for (size_t i {0}; i < out.size(); ++i) {
      const auto& aclRuleService {out[i]};
      BOOST_TEST(aclRuleService.isValid());

      const auto& dbgStr {aclRuleService.toDebugString()};
      if (0 == i) {
        nmdp::testInString(dbgStr, "priority: 3000000,");
      } else {
        nmdp::testInString(dbgStr, "priority: 3000001,");
      }
      nmdp::testInString(dbgStr, "action: allow,");
      nmdp::testInString(dbgStr, "incomingZoneId: z1,");
      nmdp::testInString(dbgStr, "outgoingZoneId: z2,");
      nmdp::testInString(dbgStr, "srcIpNetSetNamespace: global,");
      nmdp::testInString(dbgStr, "srcIpNetSetId: 1.2.3.0/24,");
      nmdp::testInString(dbgStr, "dstIpNetSetNamespace: global,");
      nmdp::testInString(dbgStr, "dstIpNetSetId: 2.3.4.0/24,");
      nmdp::testInString(dbgStr, "description: pn1],");
      nmdp::testInString(dbgStr, "serviceId: app1]");
    }
  }

  { // mixed
    const std::string xml {R"(
        <policies>

          <!-- Active non-global policy -->
          <policy>
            <from-zone-name>z1</from-zone-name>
            <to-zone-name>z2</to-zone-name>

            <!-- Active internal-policy; all attributes; +1 -->
            <policy>
              <name>pn1</name>
              <match>
                <source-address>1.2.3.0/24</source-address>
                <destination-address>2.3.4.0/24</destination-address>
                <application>app1</application>
              </match>
              <then> <permit /> </then>
            </policy>

            <!-- Active internal-policy; multi attributes; +12 -->
            <policy>
              <name>pn2</name>
              <match>
                <source-address>1.2.3.0/24</source-address>
                <source-address>1.2.4.0/24</source-address>
                <destination-address>2.3.4.0/24</destination-address>
                <destination-address>2.3.5.0/24</destination-address>
                <application>app1</application>
                <application>app2</application>
                <application>app3</application>
              </match>
              <then> <deny /> </then>
            </policy>

            <!-- Inactive internal-policy; +0 -->
            <policy inactive="inactive">
              <name>pn1</name>
              <match>
                <source-address>1.2.3.0/24</source-address>
                <destination-address>2.3.4.0/24</destination-address>
                <application>app1</application>
              </match>
              <then> <reject /> </then>
            </policy>
          </policy>

          <!-- Inactive non-global policy; +0 -->
          <policy inactive="inactive">
            <from-zone-name>z1</from-zone-name>
            <to-zone-name>z2</to-zone-name>

            <!-- Active internal-policy; all attributes; +0 (inactive above) -->
            <policy>
              <name>pn1</name>
              <match>
                <source-address>1.2.3.0/24</source-address>
                <destination-address>2.3.4.0/24</destination-address>
                <application>app1</application>
              </match>
              <then> <permit /> </then>
            </policy>
          </policy>

          <global>

            <!-- Active internal-policy; all attributes; +1 -->
            <policy>
              <name>pn1</name>
              <match>
                <source-address>1.2.3.0/24</source-address>
                <destination-address>2.3.4.0/24</destination-address>
                <application>app1</application>
                <from-zone>z1</from-zone>
                <to-zone>z2</to-zone>
              </match>
              <then> <permit /> </then>
            </policy>

            <!-- Active internal-policy; mulit attributes; +72 -->
            <policy>
              <name>pn2</name>
              <match>
                <source-address>1.2.3.0/24</source-address>
                <source-address>1.2.4.0/24</source-address>
                <destination-address>2.3.4.0/24</destination-address>
                <destination-address>2.3.5.0/24</destination-address>
                <application>app1</application>
                <application>app2</application>
                <application>app3</application>
                <from-zone>z1</from-zone>
                <from-zone>z2</from-zone>
                <to-zone>z1</to-zone>
                <to-zone>z2</to-zone>
                <to-zone>z3</to-zone>
              </match>
              <then> <deny /> </then>
            </policy>

            <!-- Inactive internal-policy; +0 -->
            <policy inactive="inactive">
              <name>pn1</name>
              <match>
                <source-address>1.2.3.0/24</source-address>
                <destination-address>2.3.4.0/24</destination-address>
                <application>app1</application>
                <from-zone>z1</from-zone>
                <to-zone>z1</to-zone>
              </match>
              <then> <reject /> </then>
            </policy>
          </global>
        </policies>
      )"};

    const auto& test = tp.getNode(xml.c_str());
    const auto& out {tp.parseConfigPolicies(test)};

    // 1 + 12 + 1 + 72
    BOOST_TEST(86 == out.size());
    for (size_t i {0}; i < out.size(); ++i) {
      const auto& aclRuleService {out[i]};
      BOOST_TEST(aclRuleService.isValid());

      const auto& dbgStr {aclRuleService.toDebugString()};

      if (0 == i) {
        nmdp::testInString(dbgStr, "priority: 3000000,");
      } else if (1 <= i && i < 13) {
        nmdp::testInString(dbgStr, "priority: 3000001,");
      } else if (13 == i) {
        nmdp::testInString(dbgStr, "priority: 4000000,");
      } else {
        nmdp::testInString(dbgStr, "priority: 4000001,");
      }

      if (0 == i || 13 == i) {
        nmdp::testInString(dbgStr, "action: allow,");
      } else {
        nmdp::testInString(dbgStr, "action: block,");
      }

      if ((0 <= i && i < 14) || (26 <= i && i < 38)) {
        nmdp::testInString(dbgStr, "incomingZoneId: z1,");
        nmdp::testInString(dbgStr, "outgoingZoneId: z2,");
      } else if (14 <= i && i < 26) {
        nmdp::testInString(dbgStr, "incomingZoneId: z1,");
        nmdp::testInString(dbgStr, "outgoingZoneId: z1,");
      } else if (38 <= i && i < 50) {
        nmdp::testInString(dbgStr, "incomingZoneId: z1,");
        nmdp::testInString(dbgStr, "outgoingZoneId: z3,");
      } else if (50 <= i && i < 62) {
        nmdp::testInString(dbgStr, "incomingZoneId: z2,");
        nmdp::testInString(dbgStr, "outgoingZoneId: z1,");
      } else if (62 <= i && i < 74) {
        nmdp::testInString(dbgStr, "incomingZoneId: z2,");
        nmdp::testInString(dbgStr, "outgoingZoneId: z2,");
      } else { // 74 <= i && i < 86
        nmdp::testInString(dbgStr, "incomingZoneId: z2,");
        nmdp::testInString(dbgStr, "outgoingZoneId: z3,");
      }

      nmdp::testInString(dbgStr, "srcIpNetSetNamespace: global,");
      if (  ( 0 <= i && i <  7)
         || (13 <= i && i < 20)
         || (26 <= i && i < 32)
         || (38 <= i && i < 44)
         || (50 <= i && i < 56)
         || (62 <= i && i < 68)
         || (74 <= i && i < 80)
         )
      {
        nmdp::testInString(dbgStr, "srcIpNetSetId: 1.2.3.0/24,");
      } else {
        nmdp::testInString(dbgStr, "srcIpNetSetId: 1.2.4.0/24,");
      }

      nmdp::testInString(dbgStr, "dstIpNetSetNamespace: global,");
      if (  ( 0 <= i && i <  4)
         || ( 7 <= i && i < 10)
         || (13 <= i && i < 17)
         || (20 <= i && i < 23)
         || (26 <= i && i < 29)
         || (32 <= i && i < 35)
         || (38 <= i && i < 41)
         || (44 <= i && i < 47)
         || (50 <= i && i < 53)
         || (56 <= i && i < 59)
         || (62 <= i && i < 65)
         || (68 <= i && i < 71)
         || (74 <= i && i < 77)
         || (80 <= i && i < 83)
         )
      {
        nmdp::testInString(dbgStr, "dstIpNetSetId: 2.3.4.0/24,");
      } else {
        nmdp::testInString(dbgStr, "dstIpNetSetId: 2.3.5.0/24,");
      }

      if (0 == i || 13 == i) {
        nmdp::testInString(dbgStr, "description: pn1],");
      } else {
        nmdp::testInString(dbgStr, "description: pn2],");
      }

      if (  ( 0 == i ||  1 == i ||  4 == i ||  7 == i)
         || (10 == i || 13 == i || 14 == i || 17 == i)
         || (20 <= i && ((i-20)%3) == 0)
         )
      {
        nmdp::testInString(dbgStr, "serviceId: app1]");
      } else if (  ( 2 == i ||  5 == i ||  8 == i || 11 == i)
                || (15 <= i && ((i-15)%3) == 0)
                )
      {
        nmdp::testInString(dbgStr, "serviceId: app2]");
      } else {
        nmdp::testInString(dbgStr, "serviceId: app3]");
        ;
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(testParseConfigApplicationOrTerm)
{
  TestParser tp;

  { // App; full
    const std::string xml {R"(
        <application>
          <name>abc</name>
          <protocol>def</protocol>
          <source-port>123</source-port>
          <destination-port>456</destination-port>
        </application>
      )"};

    const auto& test = tp.getNode(xml.c_str());
    const auto& out {tp.parseConfigApplicationOrTerm(test)};

    BOOST_TEST(1 == out.size());
    for (const auto& aclService : out) {
      BOOST_TEST(aclService.isValid());

      const auto& dbgStr {aclService.toDebugString()};
      nmdp::testInString(dbgStr, "id: abc,");
      nmdp::testInString(dbgStr, "protocol: def,");
      nmdp::testInString(dbgStr, "srcPortRanges: [[min: 123, max: 123]],");
      nmdp::testInString(dbgStr, "dstPortRanges: [[min: 456, max: 456]],");
      nmdp::testInString(dbgStr, "includedIds: []]");
    }
  }

  { // App; missing/inactive parts; tcp by number
    const std::string xml {R"(
        <application>
          <name>abc</name>
          <protocol>6</protocol>
          <source-port inactive="inactive">123</source-port>
        </application>
      )"};

    const auto& test = tp.getNode(xml.c_str());
    const auto& out {tp.parseConfigApplicationOrTerm(test)};

    BOOST_TEST(1 == out.size());
    for (const auto& aclService : out) {
      BOOST_TEST(aclService.isValid());

      const auto& dbgStr {aclService.toDebugString()};
      nmdp::testInString(dbgStr, "id: abc,");
      nmdp::testInString(dbgStr, "protocol: tcp,");
      nmdp::testInString(dbgStr, "srcPortRanges: [[min: 0, max: 65535]],");
      nmdp::testInString(dbgStr, "dstPortRanges: [[min: 0, max: 65535]],");
      nmdp::testInString(dbgStr, "includedIds: []]");
    }
  }

  { // Mix app/term; missing/inactive parts
    const std::string xml {R"(
        <application>
          <name>abc</name>
          <protocol inactive="inactive">123</protocol>
          <term>
            <name>def</name>
            <protocol>tcp</protocol>
            <source-port>123</source-port>
            <destination-port>456</destination-port>
          </term>
          <term> <protocol>udp</protocol> </term>
          <term> <protocol>sctp</protocol> </term>
          <term> <protocol>other</protocol> </term>
        </application>
      )"};

    const auto& test = tp.getNode(xml.c_str());
    const auto& out {tp.parseConfigApplicationOrTerm(test)};

    BOOST_TEST(4 == out.size());
    for (size_t i {0}; i < out.size(); ++i) {
      const auto& aclService {out[i]};
      BOOST_TEST(aclService.isValid());

      const auto& dbgStr {aclService.toDebugString()};
      nmdp::testInString(dbgStr, "id: abc,");
      nmdp::testInString(dbgStr, "includedIds: []]");
      if (0 == i) {
        nmdp::testInString(dbgStr, "protocol: tcp,");
        nmdp::testInString(dbgStr, "srcPortRanges: [[min: 123, max: 123]],");
        nmdp::testInString(dbgStr, "dstPortRanges: [[min: 456, max: 456]],");
      } else if (1 == i) {
        nmdp::testInString(dbgStr, "protocol: udp,");
        nmdp::testInString(dbgStr, "srcPortRanges: [[min: 0, max: 65535]],");
        nmdp::testInString(dbgStr, "dstPortRanges: [[min: 0, max: 65535]],");
      } else if (2 == i) {
        nmdp::testInString(dbgStr, "protocol: sctp,");
        nmdp::testInString(dbgStr, "srcPortRanges: [[min: 0, max: 65535]],");
        nmdp::testInString(dbgStr, "dstPortRanges: [[min: 0, max: 65535]],");
      } else {
        nmdp::testInString(dbgStr, "protocol: other,");
        nmdp::testInString(dbgStr, "srcPortRanges: [],");
        nmdp::testInString(dbgStr, "dstPortRanges: [],");
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(testParseConfigApplications)
{
  TestParser tp;

  const std::string xml {R"(
      <applications>
        <application>
          <name>app1</name>
          <protocol>tcp</protocol>
        </application>
        <application inactive="inactive">
          <name>app2</name>
          <protocol>123</protocol>
        </application>
        <application>
          <name>app3</name>
          <protocol>udp</protocol>
        </application>
        <application-set>
          <name>apps13</name>
          <application> <name>app1</name> </application>
          <application> <name>app3</name> </application>
          <application inactive="inactive"> <name>app4</name> </application>
        </application-set>
      </applications>
    )"};

  const auto& test = tp.getNode(xml.c_str());
  const auto& out {tp.parseConfigApplications(test)};

  BOOST_TEST(3 == out.size());
  for (size_t i {0}; i < out.size(); ++i) {
    const auto& aclService {out[i]};
    BOOST_TEST(aclService.isValid());

    const auto& dbgStr {aclService.toDebugString()};
    // other data checked elsewhere
    if (2 == i) {
      nmdp::testInString(dbgStr, "includedIds: [app1, app3]]");
    } else {
      nmdp::testInString(dbgStr, "includedIds: []]");
    }
  }
}

BOOST_AUTO_TEST_CASE(testParseConfigAddressBook)
{
  TestParser tp;

  {
    const std::string xml {R"(
        <address-book>
          <name>ab-Name</name>
          <address>
            <name>abc-1</name>
            <ip-prefix>1.2.3.0/24</ip-prefix>
            <dns-name> <name>abc.123</name> </dns-name>
            <ip-prefix inactive="inactive">1.2.3.0/24</ip-prefix>
            <dns-name inactive="inactive"> <name>abc123</name> </dns-name>
          </address>
          <address inactive="inactive">
            <name>abc_2</name>
            <ip-prefix>2.3.4.0/24</ip-prefix>
          </address>
          <address>
            <name>abc_2</name>
            <ip-prefix>1.2.3.0/24</ip-prefix>
            <ip-prefix>2.2.3.0/24</ip-prefix>
          </address>
          <address-set>
            <name>abc3</name>
            <address> <name>abc-1</name> </address>
            <address> <name>abc_2</name> </address>
          </address-set>
          <address-set inactive="inactive">
            <name>abc4</name>
            <address> <name>abc-1</name> </address>
          </address-set>
        </address-book>
      )"};

    const auto& test = tp.getNode(xml.c_str());
    const auto& out {tp.parseConfigAddressBook(test)};

    BOOST_TEST(1 == out.size());
    for (const auto& [abNamespace, map] : out) {
      BOOST_TEST("ab-Name" == abNamespace);
      BOOST_TEST(3 == map.size());
      for (const auto& [netName, aclIpNetSet] : map) {
        BOOST_TEST(aclIpNetSet.isValid());

        const auto& dbgStr {aclIpNetSet.toDebugString()};
        nmdp::testInString(dbgStr, std::format("[ns: {},", abNamespace));
        nmdp::testInString(dbgStr, std::format("id: {},", netName));

        if ("abc-1" == netName) {
          nmdp::testInString(dbgStr, "hostnames: [abc.123],");
        } else {
          nmdp::testInString(dbgStr, "hostnames: [],");
        }
        if ("abc-1" == netName) {
          nmdp::testInString(dbgStr, "ipNets: [[ipNetwork: 1.2.3.0/24,");
        } else if ("abc_2" == netName) {
          nmdp::testInString(dbgStr, "ipNets: [[ipNetwork: 1.2.3.0/24,");
          nmdp::testInString(dbgStr, "], [ipNetwork: 2.2.3.0/24,");
        } else {
          nmdp::testInString(dbgStr, "ipNets: [],");
        }
        if ("abc3" == netName) {
          nmdp::testInString(dbgStr, "includedIds: [[, abc-1], [, abc_2]]]");
        } else {
          nmdp::testInString(dbgStr, "includedIds: []]");
        }
      }
    }
    nmdp::testInString( tp.data.observations.toDebugString()
                      , "[notables: [FQDNs are used that must be resolved],"
                      );
  }

  { // nesting in security-zone
    const std::string xml {R"(
        <zones>
          <security-zone>
            <name>sz-ab</name>
            <address-book>
              <name>ab-Name</name>
              <address> <name>abc-1</name> </address>
              <address> <name>abc_2</name> </address>
            </address-book>
          </security-zone>
        </zones>
      )"};

    const auto& test = tp.getNode(xml.c_str());
    const auto& out {
        tp.parseConfigAddressBook(test
                                 .child("security-zone")
                                 .child("address-book")
                                 )
      };

    BOOST_TEST(1 == out.size());
    for (const auto& [abNamespace, map] : out) {
      BOOST_TEST("sz-ab" == abNamespace);
      BOOST_TEST(2 == map.size());
      for (const auto& [netName, aclIpNetSet] : map) {
        BOOST_TEST(aclIpNetSet.isValid());

        const auto& dbgStr {aclIpNetSet.toDebugString()};
        nmdp::testInString(dbgStr, std::format("[ns: {},", abNamespace));
        nmdp::testInString(dbgStr, std::format("id: {},", netName));
        // assume rest would be correct given prior checks
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(testParseConfigZones)
{
  TestParser tp;

  {
    const std::string xml {R"(
        <zone>
          <security-zone>
            <name>sz-1</name>
            <interfaces> <name>iface1</name> </interfaces>
            <interfaces inactive="inactive"> <name>iface1</name> </interfaces>
          </security-zone>
          <security-zone inactive="inactive">
            <name>sz_2</name>
            <interfaces> <name>iface2</name> </interfaces>
          </security-zone>
          <security-zone>
            <name>sz3</name>
            <interfaces> <name>iface1</name> </interfaces>
            <interfaces> <name>iface2</name> </interfaces>
          </security-zone>
        </zone>
      )"};

    const auto& test = tp.getNode(xml.c_str());
    const auto& out {tp.parseConfigZones(test)};

    BOOST_TEST(2 == out.size());
    for (const auto& [zName, aclZone] : out) {
      BOOST_TEST(aclZone.isValid());

      const auto& dbgStr {aclZone.toDebugString()};
      nmdp::testInString(dbgStr, std::format("[id: {},", zName));
      nmdp::testInString(dbgStr, "includedIds: []]");

      if ("sz-1" == zName) {
        nmdp::testInString(dbgStr, "ifaces: [iface1],");
      } else if ("sz3" == zName) {
        nmdp::testInString(dbgStr, "ifaces: [iface1, iface2],");
      } else {
        nmdp::testInString(dbgStr, "ifaces: [],");
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(testParseConfigRoutingOptions)
{
  TestParser tp;

  {
    const std::string xml {R"(
        <routing-options>
          <static>
            <route>
              <name>1.2.3.123/32</name>
              <discard />
            </route>
            <route>
              <name>1.2.3.0/24</name>
              <discard inactive="inactive" />
              <next-hop>1.2.3.1</next-hop>
              <next-hop inactive="inactive">1.2.3.2</next-hop>
            </route>
            <route>
              <name>1.2.3.0/16</name>
              <next-table>nVrf.inet.0</next-table>
            </route>
            <rib>
              <name>ro1.inet6.0</name>
              <route>
                <name>1::0/64</name>
                <next-hop>1::1</next-hop>
                <next-table inactive="inactive">nVrf.inet6.0</next-table>
              </route>
            </rib>
            <rib inactive="inactive"> <name>ro2.inet6.0</name> </rib>
          </static>
        </routing-options>
      )"};

    const auto& test = tp.getNode(xml.c_str());
    const auto& out {tp.parseConfigRoutingOptions(test)};

    BOOST_TEST(4 == out.size());
    for (size_t i {0}; i < out.size(); ++i) {
      const auto& route {out[i]};
      BOOST_TEST(route.isValid());

      const auto& dbgStr {route.toDebugString()};
      nmdp::testInString(dbgStr, "protocol: static,");
      nmdp::testInString(dbgStr, "adminDistance: 5,");


      if (0 == i) {
        nmdp::testInString(dbgStr, "dstIpNet: [ipNetwork: 1.2.3.123/32,");
      } else if (1 == i) {
        nmdp::testInString(dbgStr, "dstIpNet: [ipNetwork: 1.2.3.0/24,");
      } else if (2 == i) {
        nmdp::testInString(dbgStr, "dstIpNet: [ipNetwork: 1.2.0.0/16,");
      } else {
        nmdp::testInString(dbgStr, "dstIpNet: [ipNetwork: 1::/64,");
      }

      if (0 == i) {
        nmdp::testInString(dbgStr, "outIfaceName: discard,");
        nmdp::testInString(dbgStr, "isNullRoute: true,");
      } else {
        nmdp::testInString(dbgStr, "outIfaceName: ,");
        nmdp::testInString(dbgStr, "isNullRoute: false,");
      }

      if (1 == i) {
        nmdp::testInString(dbgStr, "nextHopIpAddr: [ipAddress: 1.2.3.1/32,");
      } else if (3 == i) {
        nmdp::testInString(dbgStr, "nextHopIpAddr: [ipAddress: 1::1/128,");
      } else {
        nmdp::testInString(dbgStr, "nextHopIpAddr: [ipAddress: 0.0.0.0/255,");
      }

      if (2 == i) {
        nmdp::testInString(dbgStr, "nextVrfId: nVrf,");
        nmdp::testInString(dbgStr, "nextTableId: inet.0,");
      } else {
        nmdp::testInString(dbgStr, "nextVrfId: ,");
        nmdp::testInString(dbgStr, "nextTableId: ,");
      }

      if (3 == i) {
        nmdp::testInString(dbgStr, "tableId: inet6.0,");
      } else {
        nmdp::testInString(dbgStr, "tableId: inet.0,");
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(testParseConfigRoutingInstances)
{
  TestParser tp;

  {
    const std::string xml {R"(
        <routing-instances>
          <instance>
            <name>ri-1</name>
            <interface> <name>iface1</name> </interface>
            <interface inactive="inactive"> <name>iface2</name> </interface>
            <routing-options>
              <static>
                <route> <name>1.2.3.0/24</name> <discard /> </route>
              </static>
            </routing-options>
          </instance>
          <instance inactive="inactive"> <name>ri_2</name> </instance>
          <instance>
            <name>ri3</name>
            <interface> <name>iface2</name> </interface>
            <routing-options inactive="inactive"> </routing-options>
          </instance>
        </routing-instances>
      )"};

    const auto& test = tp.getNode(xml.c_str());
    auto& ifaces = tp.data.logicalSystems[""].ifaces;
    ifaces["iface1"];
    const auto& out {tp.parseConfigRoutingInstances(test, ifaces)};

    BOOST_TEST(2 == out.size());
    for (const auto& [name, vrf] : out) {
      BOOST_TEST(vrf.isValid());

      const auto& dbgStr {vrf.toDebugString()};
      nmdp::testInString(dbgStr, std::format("[vrfId: {}, ifaces:", name));

      if ("ri-1" == name) {
        nmdp::testInString(dbgStr, "ifaces: [[name: iface1,");
      } else {
        nmdp::testInString(dbgStr, "ifaces: [],");
      }

      if ("ri-1" == name) {
        nmdp::testInString(dbgStr, "routes: [[vrfId: ri-1,");
      } else {
        nmdp::testInString(dbgStr, "routes: []]");
      }
    }
    nmdp::testInString( tp.data.observations.toDebugString()
                      , "[notables: [routing-instance ri3 contains"
                        " undefined interface iface2],"
                      );
  }
}

BOOST_AUTO_TEST_CASE(testParseConfigInterfaces)
{
  TestParser tp;

  {
    const std::string xml {R"(
        <interfaces>
          <!-- interface ranges -->
          <interface-range>
            <name>ifr1</name>
            <member> <name>ge-0/0/0</name> </member>
            <member> <name>ge-0/0/1</name> </member>
            <gigether-options>
              <redundant-parent> <parent>grp1</parent> </redundant-parent>
              <ieee-802.3ad> <bundle>gib1</bundle> </ieee-802.3ad>
            </gigether-options>
            <ether-options>
              <redundant-parent> <parent>erp1</parent> </redundant-parent>
              <ieee-802.3ad> <bundle>eib1</bundle> </ieee-802.3ad>
            </ether-options>
          </interface-range>

          <!-- physical interfaces -->
          <interface>
            <name>ge-0/0/0</name>
            <description>aBc 123</description>
            <disable />
            <gigether-options>
              <redundant-parent> <parent>grp2</parent> </redundant-parent>
              <ieee-802.3ad> <bundle>gib2</bundle> </ieee-802.3ad>
            </gigether-options>
            <ether-options>
              <redundant-parent> <parent>erp2</parent> </redundant-parent>
              <ieee-802.3ad> <bundle>eib2</bundle> </ieee-802.3ad>
            </ether-options>
            <fabric-options>
              <member-interfaces> <name>fm1</name> </member-interfaces>
            </fabric-options>
          </interface>
          <interface> <name>pi1</name> </interface>
          <interface> <name>pi2</name> </interface>

          <!-- logical interfaces -->
          <interface>
            <name>pi3</name>
            <unit>
              <name>123</name>
              <description>aBc 123</description>
              <vlan-id>1234</vlan-id>
              <family>
                <inet>
                  <address>
                    <name>1.2.3.123/24</name>
                    <arp>
                      <name>1.2.3.1</name>
                      <mac>00:11:22:33:44:55</mac>
                    </arp>
                  </address>
                </inet>
                <inet6>
                  <address>
                    <name>1::123/64</name>
                  </address>
                </inet6>
              </family>
            </unit>
            <unit> <name>125</name> <disable /> </unit>
            <unit>
              <name>456</name>
              <family>
                <inet>
                  <address>
                    <name>1.2.3.45/24</name>
                    <arp>
                      <mac>00:11:22:33:44:55</mac>
                    </arp>
                  </address>
                </inet>
              </family>
            </unit>
          </interface>
        </interfaces>
      )"};

    const auto& test = tp.getNode(xml.c_str());
    const auto& [ifaces, ifaceHierarchies] {tp.parseConfigInterfaces(test)};

    BOOST_TEST(17 == ifaces.size());
    for (const auto& [name, iface] : ifaces) {
      BOOST_TEST(iface.isValid());

      const auto& dbgStr {iface.toDebugString()};
      nmdp::testInString(dbgStr, std::format("[name: {},", name));

      if ("ge-0/0/0" == name || "ge-0/0/1" == name) {
        nmdp::testInString(dbgStr, "mediaType: ge,");
      } else if (  "grp1" == name || "gib1" == name
                || "erp1" == name || "eib1" == name
                || "grp2" == name || "gib2" == name
                || "erp2" == name || "eib2" == name
                || "fm1"  == name
                )
      {
        nmdp::testInString(dbgStr, "mediaType: ethernet,");
      } else {
        nmdp::testInString(dbgStr, "mediaType: pi,");
      }

      if ("ge-0/0/0" == name || "pi3.123" == name) {
        nmdp::testInString(dbgStr, "description: aBc 123,");
      } else if ("ge-0/0/1" == name) {
        nmdp::testInString(dbgStr, "description: ifr1,");
      } else {
        nmdp::testInString(dbgStr, "description: ,");
      }

      if ("ge-0/0/0" == name || "pi3.125" == name) {
        nmdp::testInString(dbgStr, "isUp: false,");
      } else {
        nmdp::testInString(dbgStr, "isUp: true,");
      }

      if ("pi3.123" == name) {
        nmdp::testInString(dbgStr, "ipAddrs: [[ipAddress: 1.2.3.123/24,");
      } else {
        nmdp::testInString(dbgStr, "ipAddrs: [],");
      }

      if ("pi3.123" == name) {
        nmdp::testInString(dbgStr, "vlans: [[vlanId: 1234,");
      } else {
        nmdp::testInString(dbgStr, "vlans: [],");
      }

      if ("pi3.123" == name) {
        nmdp::testInString( dbgStr
                          , "reachableMacs: [[macAddress: 00:11:22:33:44:55"
                            ", ipAddrs: [[ipAddress: 1.2.3.1/32,"
                          );
        nmdp::testInString(dbgStr, "isResponding: true]],");
      } else if ("pi3.456" == name) {
        nmdp::testInString( dbgStr
                          , "reachableMacs: [[macAddress: 00:11:22:33:44:55"
                            ", ipAddrs: [],"
                          );
        nmdp::testInString(dbgStr, "isResponding: true]],");
      } else {
        nmdp::testInString(dbgStr, "reachableMacs: [],");
      }
    }

    BOOST_TEST(16 == ifaceHierarchies.size());
    for (const auto& [parent, child] : ifaceHierarchies) {
      if ("ge-0/0/0" == parent) {
        BOOST_TEST((  "grp1" == child || "gib1" == child
                   || "erp1" == child || "eib1" == child
                   || "grp2" == child || "gib2" == child
                   || "erp2" == child || "eib2" == child
                   )
                  );
      } else if ("ge-0/0/1" == parent) {
        BOOST_TEST((  "grp1" == child || "gib1" == child
                   || "erp1" == child || "eib1" == child
                   )
                  );
      } else if ("fm1" == parent) {
        BOOST_TEST("ge-0/0/0" == child);
      } else if ("pi3" == parent) {
        BOOST_TEST((  "pi3.123" == child || "pi3.125" == child
                   || "pi3.456" == child
                   )
                  );
      } else {
        BOOST_TEST(false
                  , std::format( "Unexpected hierarchy: {} -> {}\n"
                               , parent, child
                               )
                  );
      }
    }
  }

  { // various "inactive" parts
    const std::string xml {R"(
        <interfaces>
          <!-- interface ranges -->
          <interface-range inactive="inactive"> <name>ifr1</name> </interface-range>
          <interface-range>
            <name>ifr2</name>
            <member inactive="inactive"> <name>difrm1</name> </member>
          </interface-range>

          <!-- physical interfaces -->
          <interface inactive="inactive"> <name>dpi1</name> </interface>

          <!-- logical interfaces -->
          <interface>
            <name>pi1</name>
            <unit inactive="inactive"> <name>124</name> </unit>
            <unit>
              <name>234</name>
              <vlan-id inactive="inactive">1234</vlan-id>
              <family inactive="inactive">
                <inet> <address> <name>9.9.9.9/24</name> </address> </inet>
              </family>
              <family>
                <inet inactive="inactive"> <address> <name>9.9.9.9/24</name> </address> </inet>
                <inet6 inactive="inactive"> <address> <name>9::9/64</name> </address> </inet6>
              </family>
              <family>
                <inet> <address inactive="inactive"> <name>9.9.9.9/24</name> </address> </inet>
                <inet6> <address inactive="inactive"> <name>9::9/64</name> </address> </inet6>
              </family>
              <family>
                <inet>
                  <address>
                    <name>1.2.3.4/24</name>
                    <arp inactive="inactive"> <name>9::9</name> <mac>9999.9999.9999</mac> </arp>
                    <arp> <name>9::9</name> <mac inactive="inactive">9999.9999.9999</mac> </arp>
                    <arp>
                      <name inactive="inactive">9.9.9.9</name>
                      <mac>00:11:22:33:44:55</mac>
                    </arp>
                  </address>
                </inet>
              </family>
            </unit>
          </interface>
        </interfaces>
      )"};

    const auto& test = tp.getNode(xml.c_str());
    const auto& [ifaces, ifaceHierarchies] {tp.parseConfigInterfaces(test)};

    BOOST_TEST(2 == ifaces.size());
    for (const auto& [name, iface] : ifaces) {
      BOOST_TEST(iface.isValid());

      const auto& dbgStr {iface.toDebugString()};
      nmdp::testInString(dbgStr, std::format("[name: {},", name));
      nmdp::testInString(dbgStr, "vlans: [],");

      if ("pi1.234" == name) {
        nmdp::testInString( dbgStr
                          , "[macAddress: Invalid MAC"
                            ", ipAddrs: [[ipAddress: 1.2.3.4/24,"
                          );
        nmdp::testInString( dbgStr
                          , "reachableMacs: [[macAddress: 00:11:22:33:44:55"
                            ", ipAddrs: [],"
                          );
      } else {
        nmdp::testInString( dbgStr
                          , "[macAddress: Invalid MAC, ipAddrs: [],"
                          );
        nmdp::testInString(dbgStr , "reachableMacs: [],");
      }
    }
    BOOST_TEST(1 == ifaceHierarchies.size());
    for (const auto& [parent, child] : ifaceHierarchies) {
      if ("pi1" == parent) {
        BOOST_TEST("pi1.234" == child);
      } else {
        BOOST_TEST(false
                  , std::format( "Unexpected hierarchy: {} -> {}\n"
                               , parent, child
                               )
                  );
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(testParseConfig)
{
  { // group and logical system focused
    TestParser tp;

    const std::string xml {R"(
        <configuration>
          <groups>
            <name>junos-defaults</name>
            <groups>
              <name>og1</name>
            </groups>
          </groups>
          <groups>
            <name>og2</name>
          </groups>
          <logical-systems>
            <name>ls1</name>
          </logical-systems>
        </configuration>
      )"};

    const auto& test = tp.getNode(xml.c_str());
    tp.parseConfig(test);

    nmdp::testInString( tp.data.observations.toDebugString()
                      , "[notables: [Groups are present."
                      );

    BOOST_TEST(2 == tp.data.logicalSystems.size());
    for (const auto& [name, ls] : tp.data.logicalSystems) {
      // Defaults
      BOOST_TEST(("" == name || "ls1" == name));
      BOOST_TEST(LogicalSystem() != ls);
      BOOST_TEST(1 == ls.ifaces.size());
      for (const auto& name : {"_self_"}) {
        BOOST_TEST_REQUIRE(ls.ifaces.contains(name));
        BOOST_TEST(ls.ifaces.at(name).isValid());
      }
      BOOST_TEST(0 == ls.ifaceHierarchies.size());
      BOOST_TEST(0 == ls.vrfs.size());
      BOOST_TEST(0 == ls.services.size());
      BOOST_TEST(0 == ls.dnsResolvers.size());
      BOOST_TEST(0 == ls.dnsSearchDomains.size());
      BOOST_TEST(2 == ls.aclZones.size());
      for (const auto& name : {"any", "junos-host"}) {
        BOOST_TEST_REQUIRE(ls.aclZones.contains(name));
        BOOST_TEST(ls.aclZones.at(name).isValid());
      }
      BOOST_TEST(1 == ls.aclIpNetSets.size());
      BOOST_TEST_REQUIRE(ls.aclIpNetSets.contains("global"));
      const auto& globalAclIpNetSets {ls.aclIpNetSets.at("global")};
      BOOST_TEST(3 == globalAclIpNetSets.size());
      for (const auto& name : {"any", "any-ipv4", "any-ipv6"}) {
        BOOST_TEST_REQUIRE(globalAclIpNetSets.contains(name));
        BOOST_TEST(globalAclIpNetSets.at(name).isValid());
      }
      BOOST_TEST(0 == ls.aclServices.size());
      BOOST_TEST(0 == ls.aclRules.size());
    }
  }

  { // config parts focused
    TestParser tp;

    const std::string xml {R"(
        <rpc-reply>
          <configuration>
            <system>
              <name-server>
                <name>1.2.3.40</name>
                <source-address> <name>1.2.3.1</name> </source-address>
              </name-server>
              <name-server>
                <name>1.2.3.41</name>
                <source-address> <name>1.2.3.1</name> </source-address>
              </name-server>
              <domain-search>ds0.fqdn</domain-search>
              <domain-search>ds1.fqdn</domain-search>
              <tacplus-server> <name>1.2.3.1</name> </tacplus-server>
              <ntp>
                <server> <name>1.2.3.1</name> </server>
                <source-address> <name>1.2.3.10</name> </source-address>
              </ntp>
            </system>
          </configuration>
        </rpc-reply>

        <!-- config may be, or not, exist in above -->
        <configuration>
          <name>conf1</name>
          <interfaces>
            <interface>
              <name>pi1</name>
              <unit> <name>iu123</name> </unit>
            </interface>
          </interfaces>
          <routing-instances>
            <instance>
              <name>ri1</name>
              <interface> <name>pi1</name> </interface>
            </instance>
          </routing-instances>
          <routing-options>
            <static>
              <route>
                <name>1.2.3.1/32</name>
                <discard />
              </route>
            </static>
          </routing-options>
          <security>
            <zones>
              <interface> <name>pi1</name> </interface>
              <security-zone>
                <name>sz1</name>
                <address-book>
                  <name>sz1.sab1</name>
                </address-book>
              </security-zone>
            </zones>
            <address-book>
              <name>sab1</name>
            </address-book>
            <policies>
              <policy> <policy> <then> <deny /> </then> </policy> </policy>
            </policies>
          </security>
          <applications>
            <application>
              <name>app1</name>
              <protocol>tcp</protocol>
            </application>
          </applications>
        </configuration>
      )"};

    const auto& test = tp.getNode(xml.c_str(), "configuration");
    tp.parseConfig(test);

    BOOST_TEST(1 == tp.data.logicalSystems.size());
    for (const auto& [name, ls] : tp.data.logicalSystems) {
      BOOST_TEST("conf1" == name);

      BOOST_TEST(2 == ls.dnsResolvers.size());
      for (size_t i {0}; i < ls.dnsResolvers.size(); ++i) {
        const auto& dr {ls.dnsResolvers[i]};
        BOOST_TEST(dr.isValid());
        nmdp::testInString( dr.toDebugString()
                          , std::format("dstIpAddr: [ipAddress: 1.2.3.4{}/32,"
                                       , i
                                       )
                          );
      }

      BOOST_TEST(2 == ls.dnsSearchDomains.size());
      for (size_t i {0}; i < ls.dnsSearchDomains.size(); ++i) {
        const auto& dsd {ls.dnsSearchDomains[i]};
        BOOST_TEST(std::format("ds{}.fqdn", i) == dsd);
      }

      BOOST_TEST(4 == ls.services.size());
      for (size_t i {0}; i < ls.services.size(); ++i) {
        const auto& srvc {ls.services[i]};
        BOOST_TEST(srvc.isValid());
        if (0 == i || 1 == i) {
          nmdp::testInString(srvc.toDebugString(), "serviceName: dns,");
        } else if (2 == i) {
          nmdp::testInString(srvc.toDebugString(), "serviceName: tacacs,");
        } else if (3 == i) {
          nmdp::testInString(srvc.toDebugString(), "serviceName: ntp,");
        } else {
          BOOST_TEST(false, std::format("Unhandled service: {}", i));
        }
      }

      // logic for these is mostly checked in prior tests
      BOOST_TEST(3 == ls.ifaces.size());
      for (const auto& name : {"_self_", "pi1", "pi1.iu123"}) {
        BOOST_TEST_REQUIRE(ls.ifaces.contains(name));
        BOOST_TEST(ls.ifaces.at(name).isValid());
      }
      BOOST_TEST(1 == ls.ifaceHierarchies.size());
      const auto& [parent, child] {ls.ifaceHierarchies[0]};
      BOOST_TEST("pi1" == parent);
      BOOST_TEST("pi1.iu123" == child);
      BOOST_TEST(2 == ls.vrfs.size());
      for (const auto& name : {"", "ri1"}) {
        BOOST_TEST_REQUIRE(ls.vrfs.contains(name));
        if ("" == name) { // empty name never valid
          BOOST_TEST(!ls.vrfs.at(name).isValid());
        } else {
          BOOST_TEST(ls.vrfs.at(name).isValid());
        }
      }

      BOOST_TEST(3 == ls.aclZones.size());
      for (const auto& name : {"any", "junos-host", "sz1"}) {
        BOOST_TEST_REQUIRE(ls.aclZones.contains(name));
        BOOST_TEST(ls.aclZones.at(name).isValid());
      }
      BOOST_TEST(3 == ls.aclIpNetSets.size());
      for (const auto& name : {"global", "sab1", "sz1"}) {
        BOOST_TEST_REQUIRE(ls.aclIpNetSets.contains(name));
        const auto& tAclIpNetSets {ls.aclIpNetSets.at(name)};
        BOOST_TEST(3 == tAclIpNetSets.size());
        for (const auto& name : {"any", "any-ipv4", "any-ipv6"}) {
          BOOST_TEST_REQUIRE(tAclIpNetSets.contains(name));
          BOOST_TEST(tAclIpNetSets.at(name).isValid());
        }
      }
      BOOST_TEST(1 == ls.aclServices.size());
      for (const auto& tAclService : ls.aclServices) {
        BOOST_TEST(tAclService.isValid());
      }
      BOOST_TEST(1 == ls.aclRules.size());
      for (const auto& tAclRule : ls.aclRules) {
        BOOST_TEST(tAclRule.isValid());
      }
    }
  }

  { // inactive focused
    TestParser tp;

    const std::string xml {R"(
        <rpc-reply>
          <configuration>
            <system>
              <name-server inactive="inactive">
                <name>1.2.3.40</name>
                <source-address> <name>1.2.3.1</name> </source-address>
              </name-server>
              <domain-search inactive="inactive">ds0.fqdn</domain-search>
              <tacplus-server inactive="inactive"> <name>1.2.3.1</name> </tacplus-server>
              <ntp inactive="inactive">
                <server inactive="inactive"> <name>1.2.3.1</name> </server>
                <source-address inactive="inactive"> <name>1.2.3.10</name> </source-address>
              </ntp>
              <ntp>
                <server inactive="inactive"> <name>1.2.3.1</name> </server>
                <source-address> <name>1.2.3.10</name> </source-address>
              </ntp>
              <ntp>
                <server> <name>1.2.3.1</name> </server>
                <source-address inactive="inactive"> <name>1.2.3.10</name> </source-address>
              </ntp>
            </system>
          </configuration>
        </rpc-reply>

        <!-- config may be, or not, exist in above -->
        <configuration>
          <name>conf1</name>
          <interfaces inactive="inactive">
            <interface>
              <name>pi1</name>
              <unit> <name>iu123</name> </unit>
            </interface>
          </interfaces>
          <routing-instances inactive="inactive">
            <instance>
              <name>ri1</name>
              <interface> <name>pi1</name> </interface>
            </instance>
          </routing-instances>
          <routing-options inactive="inactive">
            <static>
              <route>
                <name>1.2.3.1/32</name>
                <discard />
              </route>
            </static>
          </routing-options>
          <security>
            <zones inactive="inactive">
              <interface> <name>pi1</name> </interface>
              <security-zone>
                <name>sz1</name>
                <address-book>
                  <name>sz1.sab1</name>
                </address-book>
              </security-zone>
            </zones>
            <zones>
              <interface> <name>pi2</name> </interface>
              <security-zone inactive="inactive">
                <name>sz2</name>
                <address-book>
                  <name>sz2.sab1</name>
                </address-book>
              </security-zone>
            </zones>
            <zones>
              <interface> <name>pi3</name> </interface>
              <security-zone>
                <name>sz3</name>
                <address-book inactive="inactive">
                  <name>sz3.sab1</name>
                </address-book>
              </security-zone>
            </zones>
            <address-book inactive="inactive">
              <name>sab1</name>
            </address-book>
            <policies inactive="inactive">
              <policy> <policy> <then> <deny /> </then> </policy> </policy>
            </policies>
          </security>
          <applications inactive="inactive">
            <application>
              <name>app1</name>
              <protocol>tcp</protocol>
            </application>
          </applications>
          <logical-systems inactive="inactive">
            <name>ls1</name>
          </logical-systems>
        </configuration>
      )"};

    const auto& test = tp.getNode(xml.c_str(), "configuration");
    tp.parseConfig(test);

    BOOST_TEST(1 == tp.data.logicalSystems.size());
    for (const auto& [name, ls] : tp.data.logicalSystems) {
      BOOST_TEST("conf1" == name);
      BOOST_TEST(LogicalSystem() != ls);
      BOOST_TEST(1 == ls.ifaces.size());
      for (const auto& name : {"_self_"}) {
        BOOST_TEST_REQUIRE(ls.ifaces.contains(name));
        BOOST_TEST(ls.ifaces.at(name).isValid());
      }
      BOOST_TEST(0 == ls.ifaceHierarchies.size());
      BOOST_TEST(0 == ls.vrfs.size());
      BOOST_TEST(1 == ls.services.size());
      for (size_t i {0}; i < ls.services.size(); ++i) {
        const auto& srvc {ls.services[i]};
        BOOST_TEST(srvc.isValid());
        if (0 == i) {
          nmdp::testInString(srvc.toDebugString(), "serviceName: ntp,");
        } else {
          BOOST_TEST(false, std::format("Unhandled service: {}", i));
        }
      }
      BOOST_TEST(0 == ls.dnsResolvers.size());
      BOOST_TEST(0 == ls.dnsSearchDomains.size());
      BOOST_TEST(3 == ls.aclZones.size());
      for (const auto& name : {"any", "junos-host", "sz3"}) {
        BOOST_TEST_REQUIRE(ls.aclZones.contains(name));
        BOOST_TEST(ls.aclZones.at(name).isValid());
      }
      BOOST_TEST(1 == ls.aclIpNetSets.size());
      BOOST_TEST_REQUIRE(ls.aclIpNetSets.contains("global"));
      const auto& globalAclIpNetSets {ls.aclIpNetSets.at("global")};
      BOOST_TEST(3 == globalAclIpNetSets.size());
      for (const auto& name : {"any", "any-ipv4", "any-ipv6"}) {
        BOOST_TEST_REQUIRE(globalAclIpNetSets.contains(name));
        BOOST_TEST(globalAclIpNetSets.at(name).isValid());
      }
      BOOST_TEST(0 == ls.aclServices.size());
      BOOST_TEST(0 == ls.aclRules.size());
    }
  }
}
