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

class TestParser : public Parser {
  public:
    using Parser::nd;

    using Parser::start;

    using Parser::detailDeviceId;
    using Parser::detailIpAddress;
    using Parser::detailPlatform;
    using Parser::detailInterface;

    using Parser::noDetailCapabilityCodes;
    using Parser::noDetailHeader;
    using Parser::noDetailEntry;
    using Parser::noDetailEntryCount;
};

BOOST_AUTO_TEST_CASE(testNoDetailParts)
{
  TestParser tp;
  {
    tp.nd = NeighborData();
    const auto& parserRule {tp.noDetailCapabilityCodes};

    std::vector<std::string> testsOk {
        "Capability Codes: A - abc, B - bcd\n"
        "                  C - cde, D - def\n"
      , "Capability Codes: A - abc, B - bcd\n"
      };
    for (const auto& test : testsOk) {
      BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
                , "Parse rule 'noDetailCapabilityCodes': " << test
                );
      BOOST_TEST(NeighborData() == tp.nd);
    }
  }

  {
    tp.nd = NeighborData();
    const auto& parserRule {tp.noDetailHeader};

    std::vector<std::string> testsOk {
        "Device-ID\tLocal Intrfce\tHoldtme\tCapability\tPlatform\tPort ID\n"
      , "Device-ID  Local Intrfce  Holdtme  Capability  Platform  Port ID\n"
      , "Device ID\tLocal Intrfce\tHldtme\tCapability\tPlatform\tPort ID\n"
      , "Device ID  Local Intrfce  Hldtme  Capability  Platform  Port ID\n"
      };
    for (const auto& test : testsOk) {
      BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
                , "Parse rule 'noDetailHeader': " << test
                );
      BOOST_TEST(NeighborData() == tp.nd);
    }
  }

  {
    const auto& parserRule {tp.noDetailEntry};

    std::vector<std::tuple<std::string, std::string, std::string, std::string>>
      testsOk {
        {"host1.domain\tnic1\t123\tA\tplatform\tnic1\n"
        , "host1.domain", "platform", "nic1"
        }
      , {"host2.domain\n\tnic1\t123\tA\tplat form\tnic2\t\n"
        , "host2.domain", "plat form", "nic2"
        }
      , {"host3.domain  nic1  123  A  platform  nic3  \n"
        , "host3.domain", "platform", "nic3"
        }
      , {"host4.domain\n  nic1  123  A  plat form  nic4\n"
        , "host4.domain", "plat form", "nic4"
        }
      , {"HOST5.DOMAIN  NIC1  123  A  MODEL1   NIC5\n"
        , "HOST5.DOMAIN", "MODEL1", "NIC5"
        }
      , {"host6.domain nic1 123 A B C platform nic 6 \n"
        , "host6.domain", "platform", "nic 6"
        }
      , {"host7 nic1 123 A platform nic7"
        , "host7", "platform", "nic7"
        }
      , {"host8.domain(sn)\tnic 0/1\t123\tA B C\tplatform\tnic 0/1\n"
        , "host8.domain(sn)", "platform", "nic 0/1"
        }
      , {"host9.domain(sn)\n\tnic 0/1\t123\tA B C\tplat form\tnic 0/2\n"
        , "host9.domain(sn)", "plat form", "nic 0/2"
        }
      , {"host10.domain(sn)  nic 0/1  123  A B C  platform  nic 0/3\n"
        , "host10.domain(sn)", "platform", "nic 0/3"
        }
      , {"host11.domain(sn)\n  nic 0/1  123  A B C  plat form  nic 0/4\n"
        , "host11.domain(sn)", "plat form", "nic 0/4"
        }
      , {"HOST12.DOMAIN(SN)  NIC 0/1  123  A B C  MODEL1   NIC 0/5\n"
        , "HOST12.DOMAIN(SN)", "MODEL1", "NIC 0/5"
        }
      , {"host13.domain(sn) nic 0/1 123 A B C platform nic 0/6\n"
        , "host13.domain(sn)", "platform", "nic 0/6"
        }
      , {"host14(sn) nic 0/1 123 A B C platform nic 0/7"
        , "host14(sn)", "platform", "nic 0/7"
        }
      , {"host15.domain123\n nic123  123  A B C platform-123  nic 123/45/6\n"
        , "host15.domain123", "platform-123", "nic 123/45/6"
        }
      , {"host16.domain123\n nic 12/3  123  A B C platform-123  nic 6/45/123\n"
        , "host16.domain123", "platform-123", "nic 6/45/123"
        }
      , {"h17\n                 Gig 0/0/0         144               R    ABCD 1234 nic0"
        , "h17", "ABCD 1234", "nic0"
        }
      };
    for (const auto& [test, name, model, iface ] : testsOk) {
      tp.nd = NeighborData();
      BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
                , "Parse rule 'noDetailEntry': " << test
                );
      BOOST_TEST(name == tp.nd.curHostname);
      BOOST_TEST(iface == tp.nd.curIfaceName);
      BOOST_TEST(model == tp.nd.curModel);
      BOOST_TEST(tp.nd.curVendor.empty());
      BOOST_TEST(tp.nd.ipAddrs.empty());
    }
  }

  {
    tp.nd = NeighborData();
    const auto& parserRule {tp.noDetailEntryCount};

    std::vector<std::string> testsOk {
        "Total cdp entries displayed : 1\n"
      , "Total cdp entries displayed : 1234567890\n"
      , "Total arbitrary\n"
      };
    for (const auto& test : testsOk) {
      BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
                , "Parse rule 'noDetailEntryCount': " << test
                );
      BOOST_TEST(NeighborData() == tp.nd);
    }
  }
}

BOOST_AUTO_TEST_CASE(testDetailParts)
{
  TestParser tp;

  {
    tp.nd = NeighborData();
    const auto& parserRule {tp.detailDeviceId};
    std::vector<std::tuple<std::string, std::string>> testsOk {
        {"Device-ID: host1\n", "host1"}
      , {"Device ID: host1\n", "host1"}
      , {"SysName: host1\n", "host1"}
      , {"Device-ID: HOST1\n", "HOST1"}
      , {"Device ID: host1(sn)\n", "host1(sn)"}
      , {"Device ID: host1-host1-host1\n", "host1-host1-host1"}
      , {"Device ID: host1.domain\n", "host1.domain"}
      , {"Device ID:host1\n", "host1"}
      };
    for (const auto& [test, name] : testsOk) {
      BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
                , "Parse rule 'detailDeviceId': " << test
                );
      BOOST_TEST(name == tp.nd.curHostname);

      BOOST_TEST(tp.nd.curIfaceName.empty());
      BOOST_TEST(tp.nd.curVendor.empty());
      BOOST_TEST(tp.nd.curModel.empty());
      BOOST_TEST(tp.nd.ipAddrs.empty());
    }
  }

  {
    tp.nd = NeighborData();
    const auto& parserRule {tp.detailIpAddress};
    std::vector<std::string> testsOk {
        "IP 1.2.3.4\n"
      , "IPv4 1.2.3.4\n"
      , "IPv6 1234::1234\n"
      , "IP address: 1.2.3.4\n"
      , "IPv4 address: 1.2.3.4\n"
      , "IPv4 Address: 1.2.3.4\n"
      , "IPv6 address: 1234::1234\n"
      , "IPv6 Address: 1234::1234\n"
      , "IP 1.2.3.4 (link-local)\n"
      , "IPv4 1.2.3.4 (link-local)\n"
      , "IPv6 1234::1234 (link-local)\n"
      , "IP address: 1.2.3.4 (link-local)\n"
      , "IPv4 address: 1.2.3.4 (link-local)\n"
      , "IPv6 address: 1234::1234 (link-local)\n"
      };
    size_t count = 0;
    for (const auto& test : testsOk) {
      BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
                , "Parse rule 'detailIpAddress': " << test
                );
      count++;
      BOOST_TEST(count == tp.nd.ipAddrs.size());

      BOOST_TEST(tp.nd.curHostname.empty());
      BOOST_TEST(tp.nd.curIfaceName.empty());
      BOOST_TEST(tp.nd.curVendor.empty());
      BOOST_TEST(tp.nd.curModel.empty());
    }
  }

  {
    tp.nd = NeighborData();
    const auto& parserRule {tp.detailPlatform};
    std::vector<std::tuple<std::string, std::string, std::string>> testsOk {
        {"Platform: Cisco 1234 (PID:1234-1234)-ABC\n", "Cisco", "1234"}
      , {"Platform: cisco 1234-1234\n", "cisco", "1234-1234"}
      , {"Platform: N5K-1234, Capabilities: Abc abc Abc-abc\n", "", "N5K-1234"}
      , {"Platform: cisco 1234, Capabilities:\n", "cisco", "1234"}
      , {"Platform: AIR-1234, Capabilities:\n", "", "AIR-1234"}
      };
    for (const auto& [test, vendor, model] : testsOk) {
      tp.nd.curVendor = "";
      tp.nd.curModel  = "";
      BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
                , "Parse rule 'detailPlatform': " << test
                );
      BOOST_TEST(vendor == tp.nd.curVendor);
      BOOST_TEST(model == tp.nd.curModel);

      BOOST_TEST(tp.nd.curHostname.empty());
      BOOST_TEST(tp.nd.curIfaceName.empty());
      BOOST_TEST(tp.nd.ipAddrs.empty());
    }
  }

  {
    tp.nd = NeighborData();
    const auto& parserRule {tp.detailInterface};
    std::vector<std::tuple<std::string, std::string>> testsOk {
        {"Interface: gi1, Port ID (outgoing port): gi10\n", "gi10"}
      , {"Interface: GigEth0/0, Port ID (outgoing port): GigEth10/10\n"
        , "GigEth10/10"
        }
      };
    for (const auto& [test, iface] : testsOk) {
      BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
                , "Parse rule 'detailPlatform': " << test
                );
      BOOST_TEST(iface == tp.nd.curIfaceName);

      BOOST_TEST(tp.nd.curHostname.empty());
      BOOST_TEST(tp.nd.curVendor.empty());
      BOOST_TEST(tp.nd.curModel.empty());
      BOOST_TEST(tp.nd.ipAddrs.empty());
    }
  }
}

BOOST_AUTO_TEST_CASE(testWholeNonDetail)
{
  { // IOS
    TestParser tp;
    const auto& parserRule {tp.start};

    std::string test {
      R"STR(Capability Codes: R - Router, T - Trans Bridge, B - Source Route Bridge
                  S - Switch, H - Host, I - IGMP, r - Repeater, P - Phone,
                  D - Remote, C - CVTA, M - Two-port Mac Relay

Device ID        Local Intrfce     Holdtme    Capability  Platform  Port ID
HOST1.domain
                 Gig 1/1/2         173             R S I  ABC123    Gig 2/4/0/1
host2.domain
                 Ten 1/1/1         165              R B   123ABC    Eth 1/4/0/1
host3(abc123)
                 Ten 1/1/1         165            R S I C Abc-D1234 mgmt0

Total cdp entries displayed : 3
      )STR"
      };
    Result out;
    BOOST_TEST(nmdp::testAttr(test.c_str(), parserRule, out, blank),
              "Parse rule 'testWholeNonDetail': " << test);
    BOOST_TEST_REQUIRE(1 == out.size());
    BOOST_TEST(0 == out[0].ipAddrs.size());
    size_t conCount {3};
    BOOST_TEST(conCount == out[0].devInfos.size());
    BOOST_TEST(conCount == out[0].interfaces.size());
  }

  { // NXOS
    TestParser tp;
    const auto& parserRule {tp.start};

    std::string test {
      R"STR(Capability Codes: R - Router, T - Trans-Bridge, B - Source-Route-Bridge
                  S - Switch, H - Host, I - IGMP, r - Repeater,
                  V - VoIP-Phone, D - Remotely-Managed-Device,
                  s - Supports-STP-Dispute


Device-ID          Local Intrfce  Hldtme Capability  Platform      Port ID
HOST1.domain        Eth1/1         123    R S I s   ABC-123          Gig1/2
host2.domain(abc123)
                    Gig1/1         123    R S       ABC-123-ABC-123- Ten1/2
      )STR"
      };
    Result out;
    BOOST_TEST(nmdp::testAttr(test.c_str(), parserRule, out, blank),
              "Parse rule 'testWholeNonDetail': " << test);
    BOOST_TEST_REQUIRE(1 == out.size());
    BOOST_TEST(0 == out[0].ipAddrs.size());
    size_t conCount {2};
    BOOST_TEST(conCount == out[0].devInfos.size());
    BOOST_TEST(conCount == out[0].interfaces.size());
  }
}

BOOST_AUTO_TEST_CASE(testWholeDetail)
{
  {
    TestParser tp;
    const auto& parserRule {tp.start};
    std::string test {
      R"STR(
---------------------------------------------
Device-ID: a1234b4321
Advertisement version: 2
Platform: Cisco 1234-1234A (PID:A1234-1234A-A1234)-A
Capabilities: Router Switch IGMP
Interface: gi1, Port ID (outgoing port): gi10
Holdtime: 100
Version: 1.2.3.4
Duplex: full
Native VLAN: 1234
SysName: ABC-1234-4312
SysObjectID: 0.0
Addresses:
          IP 1.2.3.4
      )STR",
      };
    Result out;
    BOOST_TEST(nmdp::testAttr(test.c_str(), parserRule, out, blank),
              "Parse rule 'testDetailWhole': " << test);
    BOOST_TEST(1 == out.size());
  }

  {
    TestParser tp;
    const auto& parserRule {tp.start};
    std::vector<std::string> testsOk {
      R"STR(
---------------------------------------------
Device-ID: a1234b4321
Advertisement version: 2
Platform: Cisco 1234-1234A (PID:A1234-1234A-A1234)-A
Capabilities: Router Switch IGMP
Interface: gi1, Port ID (outgoing port): gi10
Holdtime: 100
Version: 1.2.3.4
Duplex: full
Native VLAN: 1234
SysName: ABC-1234-4312
SysObjectID: 0.0
Addresses:
          IP 1.2.3.4
---------------------------------------------
Device-ID: ABC-1234-4312
Advertisement version: 2
Platform: Cisco 1234-1234A (PID:A1234-1234A-A1234)-A
Capabilities: Router Switch IGMP
Interface: gi1, Port ID (outgoing port): gi10
Holdtime: 100
Version: 1.2.3.4
Duplex: full
Native VLAN: 1234
SysObjectID: 0.0
Addresses:
          IPv6 1234::1234


---------------------------------------------
Device-ID: a1234b4321
Advertisement version: 2
Platform: Cisco 1234-1234A (PID:A1234-1234A-A1234)-A
Capabilities: Router Switch IGMP
Interface: gi1, Port ID (outgoing port): gi10
Holdtime: 100
Version: 1.2.3.4
Duplex: full
Native VLAN: 1234
SysName: ABC-1234-4312.Something.Other
SysObjectID: 0.0
Addresses:
          IP 1.2.3.4
          IPv6 1234::1234
      )STR",
    };
    for (const auto& test : testsOk) {
      Result out;
      BOOST_TEST(nmdp::testAttr(test.c_str(), parserRule, out, blank),
                "Parse rule 'testDetailWhole': " << test);
      BOOST_TEST_REQUIRE(1 == out.size());
      const auto& data {out.at(0)};
      BOOST_TEST_REQUIRE(4 == data.ipAddrs.size());

      auto ipAddr {data.ipAddrs.at(0)};
      auto aliases {ipAddr.getAliases()};
      BOOST_TEST("1.2.3.4/32" == ipAddr.toString());
      BOOST_TEST(1 == aliases.size());
      BOOST_TEST("abc-1234-4312" == *aliases.cbegin());

      ipAddr = data.ipAddrs.at(1);
      aliases = ipAddr.getAliases();
      BOOST_TEST("1234::1234/128" == ipAddr.toString());
      BOOST_TEST(1 == aliases.size());
      BOOST_TEST("abc-1234-4312" == *aliases.cbegin());

      ipAddr = data.ipAddrs.at(2);
      aliases = ipAddr.getAliases();
      BOOST_TEST("1.2.3.4/32" == ipAddr.toString());
      BOOST_TEST(1 == aliases.size());
      BOOST_TEST("abc-1234-4312.something.other" == *aliases.cbegin());

      ipAddr = data.ipAddrs.at(3);
      aliases = ipAddr.getAliases();
      BOOST_TEST("1234::1234/128" == ipAddr.toString());
      BOOST_TEST(1 == aliases.size());
      BOOST_TEST("abc-1234-4312.something.other" == *aliases.cbegin());

      BOOST_TEST(3 == data.devInfos.size());
      for (const auto& devInfo : data.devInfos) {
        BOOST_TEST("abc-1234-4312" == devInfo.getDeviceId());
      }

      BOOST_TEST(3 == data.interfaces.size());
      for (const auto& [iface, name] : data.interfaces) {
        BOOST_TEST("gi10" == iface.getName());
      }
    }
  }
}
