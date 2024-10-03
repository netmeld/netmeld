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

class TestParser : public Parser {
  public:
    using Parser::DEFAULT_ZONE;

    using Parser::start;
    using Parser::config;
    using Parser::interface;
    using Parser::service;
    using Parser::policy;
    using Parser::route;
    using Parser::d; // Expose the data structure for testing
};

BOOST_AUTO_TEST_CASE(testRouteRule)
{
  TestParser tp;
  const auto& parserRule {tp.start};
  
  // set route ip/prefix interface ifaceName [gateway ip]
  // set routing-options static route ip/prefix next-hop (ip|nic)
  const std::string test {R"(
      set route 1.2.3.0/24 interface eth0
      set route 1.2.4.0/24 interface eth1 gateway 1.2.4.1
      set route 1.2.5.0/24 interface eth1 gateway 1.2.4.1
      set routing-options static route 1.2.3.0/24 next-hop 1.2.3.1
      set routing-options static route 1.2.4.0/24 next-hop 1.2.4.1
      set routing-options static route 1.2.5.0/24 next-hop eth0
    )"};

  BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
            , "Rule 'route': " << test
            );

  BOOST_TEST(0 == tp.d.ifaces.size());
  BOOST_TEST(0 == tp.d.networkBooks.size());
  BOOST_TEST(0 == tp.d.serviceBooks.size());
  BOOST_TEST(0 == tp.d.ruleBooks.size());
  BOOST_TEST(!tp.d.observations.isValid());

  BOOST_TEST(6 == tp.d.routes.size());
  for (size_t i {0}; i < tp.d.routes.size(); ++i) {
    const auto& route {tp.d.routes[i]};
    BOOST_TEST(route.isValid());

    const auto& dbgStr {route.toDebugString()};
    nmdp::testInString(dbgStr, "isActive: true");

    if (0 == i) {
      nmdp::testInString(dbgStr, "dstIpNet: [ipNetwork: 1.2.3.0/24,");
      nmdp::testInString(dbgStr, "nextHopIpAddr: [ipAddress: 0.0.0.0/255");
      nmdp::testInString(dbgStr, "outIfaceName: eth0,");
    }
    if (1 == i) {
      nmdp::testInString(dbgStr, "dstIpNet: [ipNetwork: 1.2.4.0/24,");
      nmdp::testInString(dbgStr, "nextHopIpAddr: [ipAddress: 1.2.4.1/32");
      nmdp::testInString(dbgStr, "outIfaceName: eth1,");
    }
    if (2 == i) {
      nmdp::testInString(dbgStr, "dstIpNet: [ipNetwork: 1.2.5.0/24,");
      nmdp::testInString(dbgStr, "nextHopIpAddr: [ipAddress: 1.2.4.1/32");
      nmdp::testInString(dbgStr, "outIfaceName: eth1,");
    }
    if (3 == i) {
      nmdp::testInString(dbgStr, "dstIpNet: [ipNetwork: 1.2.3.0/24,");
      nmdp::testInString(dbgStr, "nextHopIpAddr: [ipAddress: 1.2.3.1/32");
      nmdp::testInString(dbgStr, "outIfaceName: ,");
    }
    if (4 == i) {
      nmdp::testInString(dbgStr, "dstIpNet: [ipNetwork: 1.2.4.0/24,");
      nmdp::testInString(dbgStr, "nextHopIpAddr: [ipAddress: 1.2.4.1/32");
      nmdp::testInString(dbgStr, "outIfaceName: ,");
    }
    if (5 == i) {
      nmdp::testInString(dbgStr, "dstIpNet: [ipNetwork: 1.2.5.0/24,");
      nmdp::testInString(dbgStr, "nextHopIpAddr: [ipAddress: 0.0.0.0/255");
      nmdp::testInString(dbgStr, "outIfaceName: eth0,");
    }
    if (5 < i) {
      BOOST_TEST(false, std::format("Unchecked route: {}\n", i));
    }
  }
}

BOOST_AUTO_TEST_CASE(testInterfaceRules)
{
  // exercises: interface, interfaces, ifactTypeName, vrouter
  TestParser tp;
  const auto& parserRule {tp.start};

  // set interface NAME (ip|ipv6|mip|tag|zone)
  //   ip (IPA/P|unnumbered interface NAME)
  //   ipv6 ip IPA/P
  //   mip IPA host IPA [netmask MASK] [(vr|vrouter) NAME]
  //   tag NUM zone NAME
  //   zone NAME
  // set interfaces NAME [unit NUM] [(family (inet|inet6) address IPA)|disable]
  const std::string test {R"(
      set interface eth1 ip 1.2.3.4/24
      set interface eth2.1 ip 1.2.1.4/24
      set interface eth3/1 ip manageable
      set interface tunnel.4 ip unnumbered interface eth2
      set interface eth05 ipv6 ip 1::2/64
      set interface eth06 mip 1.2.3.4 host 1.2.3.1
      set interface eth07 mip 1.2.3.4 host 1.2.3.1 netmask 255.255.255.0
      set interface eth08 mip 1.2.3.4 host 1.2.3.1 netmask 255.255.255.0 vr vr1
      set interface eth09 mip 1.2.3.4 host 1.2.3.1 netmask 255.255.255.0 vrouter vr1
      set interface eth10 tag 1 zone z1
      set interface eth11 zone z1

      set interfaces eth12
      set interfaces eth13 unit 1
      set interfaces eth14/1
      set interfaces eth15/1 unit 1
      set interfaces eth16 family inet address 1.2.3.4
      set interfaces eth17 family inet6 address 1::2
      set interfaces eth18 disable
    )"};

  BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
            , "Rule 'interface': " << test
            );
  
  BOOST_TEST(0 == tp.d.routes.size());
  BOOST_TEST(0 == tp.d.networkBooks.size());
  BOOST_TEST(0 == tp.d.serviceBooks.size());
  BOOST_TEST(0 == tp.d.ruleBooks.size());

  BOOST_TEST(18 == tp.d.ifaces.size());
  for (const auto& [name, iface] : tp.d.ifaces) {
    BOOST_TEST(iface.isValid());

    const auto& dbgStr {iface.toDebugString()};
    nmdp::testInString(dbgStr, "isDiscoveryProtocolEnabled: false,");

    // NOTE: "set interfaces" logic alters name; if no "unit" appends ".0"
    nmdp::testInString(dbgStr, std::format("[name: {},", name));

    // up checks
    if ("eth18.0" == name) {
      nmdp::testInString(dbgStr, "isUp: false,");
    } else {
      nmdp::testInString(dbgStr, "isUp: true,");
    }
    // media type checks
    if (name.starts_with("eth")) {
      nmdp::testInString(dbgStr, "mediaType: eth,");
    } else {
      nmdp::testInString(dbgStr, "mediaType: tunnel,");
    }
    // ip checks
    if (  "eth3/1" == name || "tunnel.4" == name || "eth06" == name
       || "eth07" == name || "eth08" == name || "eth09" == name
       || "eth10" == name || "eth11" == name || "eth12.0" == name
       || "eth13.1" == name || "eth14/1.0" == name || "eth15/1.1" == name
       || "eth18.0" == name
       )
    {
      nmdp::testInString(dbgStr, "ipAddrs: [],");
    } else if ("eth1" == name) {
      nmdp::testInString(dbgStr, "ipAddrs: [[ipAddress: 1.2.3.4/24,");
    } else if ("eth2.1" == name) {
      nmdp::testInString(dbgStr, "ipAddrs: [[ipAddress: 1.2.1.4/24,");
    } else if ("eth05" == name) {
      nmdp::testInString(dbgStr, "ipAddrs: [[ipAddress: 1::2/64,");
    } else if ("eth06" == name || "eth16.0" == name) {
      nmdp::testInString(dbgStr, "ipAddrs: [[ipAddress: 1.2.3.4/32,");
    } else if ("eth17.0" == name) {
      nmdp::testInString(dbgStr, "ipAddrs: [[ipAddress: 1::2/128,");
    } else {
      BOOST_TEST(false, std::format("Unchecked iface: {}\n", name));
    }
  }

  BOOST_TEST(tp.d.observations.isValid());
  {
    const auto& dbgStr {tp.d.observations.toDebugString()};
    nmdp::testInString( dbgStr
                      , "unsupportedFeatures: [iface alias: eth2, mip defined]]"
                      );
  }
}

BOOST_AUTO_TEST_CASE(testServiceRule)
{
  TestParser tp;
  const auto& parserRule {tp.start};

  // set service NAME (protocol|+) PROTO
  //   PROTO: ( (icmp type NUM code NUM)
  //          | ((NUM|tcp|udp) src-port LOW-HIGH dst-port LOW-HIGH)
  //          )
  const std::string test {R"(
      set service ssh protocol tcp src-port 0-65535 dst-port 22-22
      set service web protocol tcp src-port 0-65535 dst-port 80-80
      set service web + tcp src-port 0-65535 dst-port 443-443
      set service "abc def" protocol 12 src-port 123-456 dst-port 1234-5678
      set service ping protocol icmp type 8 code 0
      set service ping + icmp type 0 code 0
      set servcie oThEr protocol 123
    )"};

  BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
            , "Rule 'service': " << test
            );

  BOOST_TEST(0 == tp.d.routes.size());
  BOOST_TEST(0 == tp.d.ifaces.size());
  BOOST_TEST(0 == tp.d.networkBooks.size());
  BOOST_TEST(0 == tp.d.ruleBooks.size());
  BOOST_TEST(!tp.d.observations.isValid());

  BOOST_TEST(1 == tp.d.serviceBooks.size());
  BOOST_TEST(tp.d.serviceBooks.contains(tp.DEFAULT_ZONE));
  const auto& acServiceBooks {tp.d.serviceBooks.at(tp.DEFAULT_ZONE)};
  BOOST_TEST(4 == acServiceBooks.size());
  for (const auto& [name, acServiceBook] : acServiceBooks) {
    BOOST_TEST(acServiceBook.isValid());

    const auto& dbgStr {acServiceBook.toDebugString()};
    nmdp::testInString(dbgStr, "[id: global,");
    nmdp::testInString(dbgStr, std::format("[id: global, name: {},", name));

    if ("ssh" == name) {
      nmdp::testInString(dbgStr, "data: [tcp:0-65535:22-22]]");
    } else if ("web" == name) {
      nmdp::testInString( dbgStr
                        , "data: [tcp:0-65535:443-443, tcp:0-65535:80-80]]"
                        );
    } else if ("abc def" == name) {
      nmdp::testInString(dbgStr, "data: [12:123-456:1234-5678]]");
    } else if ("ping" == name) {
      nmdp::testInString(dbgStr, "data: [icmp:0:0, icmp:8:0]]");
    } else if ("oThEr" == name) {
      nmdp::testInString(dbgStr, "data: [123::]]");
    } else {
      BOOST_TEST(false, std::format("Unchecked service: {}\n", name));
    }
  }
}

BOOST_AUTO_TEST_CASE(testAddressRule)
{
  TestParser tp;
  const auto& parserRule {tp.start};

  // set address ZONE NAME (IPA MASK|IPAP|FQDN) [COMMENT]
  const std::string test {R"(
      set address z1 n1 1.2.3.4 255.255.255.0
      set address z2 n2/2 1.2.3.4/24
      set address z3 n3 host.dom
      set address z3 n3 1.2.3.4/32
      set address z4 n2 1::2/64
      set address z4 n1 1.2.3.4/24 "some text"
    )"};

  BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
            , "Rule 'address': " << test
            );

  BOOST_TEST(0 == tp.d.routes.size());
  BOOST_TEST(0 == tp.d.ifaces.size());
//  BOOST_TEST(0 == tp.d.networkBooks.size());
  BOOST_TEST(0 == tp.d.serviceBooks.size());
  BOOST_TEST(0 == tp.d.ruleBooks.size());
  BOOST_TEST(!tp.d.observations.isValid());

  BOOST_TEST(4 == tp.d.networkBooks.size());
  for (const auto& [zone, acNetworkBooks] : tp.d.networkBooks) {
    if ("z4" == zone) {
      BOOST_TEST(2 == acNetworkBooks.size());
    } else {
      BOOST_TEST(1 == acNetworkBooks.size());
    }
    for (const auto& [name, acNetworkBook] : acNetworkBooks) {
      BOOST_TEST(acNetworkBook.isValid());

      const auto& dbgStr {acNetworkBook.toDebugString()};
      nmdp::testInString(dbgStr, std::format("[id: {}, name: {},", zone, name));

      if ("n1" == name || "n2/2" == name) {
        nmdp::testInString(dbgStr, "data: [1.2.3.4/24]]");
      } else if ("n2" == name) {
        nmdp::testInString(dbgStr, "data: [1::2/64]]");

      } else if ("n3" == name) {
        nmdp::testInString(dbgStr, "data: [1.2.3.4/32, host.dom]]");
      } else {
        BOOST_TEST(false, std::format("Unchecked network: {}\n", name));
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(testGroupRule)
{
  TestParser tp;
  const auto& parserRule {tp.start};

  // set group (address ZONE|service) NAME [add NAME] [COMMENT]
  const std::string test {R"(
      set group service s1
      set group service s2
      set group service s3 add s1
      set group service s3 add o2

      set group address z1 single
      set group address z2 single
      set group address z2 multi add o1
      set group address z2 multi add o2 "some text"
    )"};

  BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
            , "Rule 'group': " << test
            );

  BOOST_TEST(0 == tp.d.routes.size());
  BOOST_TEST(0 == tp.d.ifaces.size());
  BOOST_TEST(0 == tp.d.ruleBooks.size());
  BOOST_TEST(!tp.d.observations.isValid());

  BOOST_TEST(1 == tp.d.serviceBooks.size());
  BOOST_TEST(tp.d.serviceBooks.contains(tp.DEFAULT_ZONE));
  const auto& acServiceBooks {tp.d.serviceBooks.at(tp.DEFAULT_ZONE)};
  BOOST_TEST(3 == acServiceBooks.size());
  for (const auto& [name, acServiceBook] : acServiceBooks) {
    BOOST_TEST(acServiceBook.isValid());

    const auto& dbgStr {acServiceBook.toDebugString()};
    nmdp::testInString(dbgStr, std::format("[id: global, name: {},", name));

    if ("s1" == name || "o2" == name) {
      nmdp::testInString(dbgStr, "data: []]");
    } else if ("s3" == name) {
      nmdp::testInString(dbgStr, "data: [o2, s1]]");
    } else {
      BOOST_TEST(false, std::format("Unchecked service: {}\n", name));
    }
  }

  // only track populated ("add") books
  BOOST_TEST(1 == tp.d.networkBooks.size());
  for (const auto& [zone, acNetworkBooks] : tp.d.networkBooks) {
    if ("z2" == zone) {
      BOOST_TEST(3 == acNetworkBooks.size());
    } else {
      BOOST_TEST(1 == acNetworkBooks.size());
    }
    for (const auto& [name, acNetworkBook] : acNetworkBooks) {
      BOOST_TEST(acNetworkBook.isValid());

      const auto& dbgStr {acNetworkBook.toDebugString()};
      nmdp::testInString(dbgStr, std::format("[id: {}, name: {},", zone, name));

      if ("o1" == name || "o2" == name) {
        nmdp::testInString(dbgStr, "data: []]");
      } else if ("multi" == name) {
        nmdp::testInString(dbgStr, "data: [o1, o2]]");
      } else {
        BOOST_TEST(false, std::format("Unchecked network: {}\n", name));
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(testPolicyRule)
{
  TestParser tp;
  const auto& parserRule {tp.start};

  // set policy id NUM ((from ZONE1 to ZONE2 SRC DST SERVICE ACTIONS)|disable)
  // --and--
  // set policy id NUM
  // set (src-address|dst-address|service|log) VALUE
  // exit
  const std::string test {R"(
      set policy id 1 from z1 to z2 src1 dst1 srvc1 act1
      set policy id 1 disable

      set policy id 2 from z1 to z2 src1 dst1 srvc1 act1

      set policy id 123 from z2 to z3 src2 dst3 srvc3 act1 act2 act3
      set policy id 123
      set other attribute values
      set log l1
      exit

      set policy id 3 from z1 to z2 src1 dst1 srvc1 act1

      set policy id 2
      set src-address src2
      set dst-address dst2
      set service srvc2
      set log l1
      exit
    )"};

  BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
            , "Rule 'group': " << test
            );

  BOOST_TEST(0 == tp.d.routes.size());
  BOOST_TEST(0 == tp.d.ifaces.size());
  BOOST_TEST(0 == tp.d.networkBooks.size());
  BOOST_TEST(0 == tp.d.serviceBooks.size());
  BOOST_TEST(!tp.d.observations.isValid());

  BOOST_TEST(1 == tp.d.ruleBooks.size());
  BOOST_TEST(tp.d.ruleBooks.contains(tp.DEFAULT_ZONE));
  const auto& acRuleBooks {tp.d.ruleBooks.at(tp.DEFAULT_ZONE)};
  BOOST_TEST(4 == acRuleBooks.size());
  for (const auto& [id, acRuleBook] : acRuleBooks) {
    BOOST_TEST(acRuleBook.isValid());

    const auto& dbgStr {acRuleBook.toDebugString()};
    nmdp::testInString(dbgStr, std::format("id: {},", id));
    nmdp::testInString(dbgStr, "srcIfaces: [],");
    nmdp::testInString(dbgStr, "dstIfaces: [],");
    nmdp::testInString(dbgStr, "description: ]");

    // enabled
    if (1 == id) {
      nmdp::testInString(dbgStr, "enabled: false,");
    } else {
      nmdp::testInString(dbgStr, "enabled: true,");
    }

    // zone ids
    if (123 == id) {
      nmdp::testInString(dbgStr, "srcId: z2,");
      nmdp::testInString(dbgStr, "dstId: z3,");
    } else {
      nmdp::testInString(dbgStr, "srcId: z1,");
      nmdp::testInString(dbgStr, "dstId: z2,");
    }

    // rest
    if (1 == id || 3 == id) {
      nmdp::testInString(dbgStr, "srcs: [src1],");
      nmdp::testInString(dbgStr, "dsts: [dst1],");
      nmdp::testInString(dbgStr, "services: [srvc1],");
      nmdp::testInString(dbgStr, "actions: [act1],");
    } else if (2 == id) {
      nmdp::testInString(dbgStr, "srcs: [src1, src2],");
      nmdp::testInString(dbgStr, "dsts: [dst1, dst2],");
      nmdp::testInString(dbgStr, "services: [srvc1, srvc2],");
      nmdp::testInString(dbgStr, "actions: [act1, l1],");
    } else if (123 == id) {
      nmdp::testInString(dbgStr, "srcs: [src2],");
      nmdp::testInString(dbgStr, "dsts: [dst3],");
      nmdp::testInString(dbgStr, "services: [srvc3],");
      nmdp::testInString(dbgStr, "actions: [act1, act2, act3, l1],");
    } else {
      BOOST_TEST(false, std::format("Unchecked rule: {}\n", id));
    }
  }
}
