// =============================================================================
// Copyright 2017 National Technology & Engineering Solutions of Sandia, LLC
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
    using Parser::hostnameValue;
    using Parser::ipAddressValue;
    using Parser::platformValue;
    using Parser::interfaceValue;
};
BOOST_AUTO_TEST_CASE(testParts)
{
  TestParser tp;

  {
    const auto& parserRule {tp.hostnameValue};
    std::vector<std::string> testsOk {
      "Device-ID: hostname\n",
      "Device ID: hostname\n",
      "SysName: hostname\n",
      "Device-ID: HOSTNAME\n",
      "Device ID: hostname(hostname)\n",
      "Device ID: hostname-hostname-hostname\n",
      "Device ID: hostname.hostname-hostname\n",
      "Device ID:hostname\n",
    };
    for (const auto& test : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
                "Parse rule 'hostnameValue': " << test);
    }
  }

  {
    const auto& parserRule {tp.ipAddressValue};
    std::vector<std::string> testsOk {
      "IP 1.2.3.4\n",
      "IPv4 1.2.3.4\n",
      "IPv6 1234::1234\n",
      "IP address: 1.2.3.4\n",
      "IPv4 address: 1.2.3.4\n",
      "IPv4 Address: 1.2.3.4\n",
      "IPv6 address: 1234::1234\n",
      "IPv6 Address: 1234::1234\n",
      "IP 1.2.3.4 (link-local)\n",
      "IPv4 1.2.3.4 (link-local)\n",
      "IPv6 1234::1234 (link-local)\n",
      "IP address: 1.2.3.4 (link-local)\n",
      "IPv4 address: 1.2.3.4 (link-local)\n",
      "IPv6 address: 1234::1234 (link-local)\n",
    };
    for (const auto& test : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
                "Parse rule 'ipAddressValue': " << test);
    }
  }

  {
    const auto& parserRule {tp.platformValue};
    std::vector<std::string> testsOk {
      "Platform: Cisco 1234 (PID:1234-1234)-ABC\n",
      "Platform: cisco 1234-1234\n",
      "Platform: N5K-1234, Capabilities: Abc abc Abc-abc\n",
      "Platform: cisco 1234, Capabilities:\n",
      "Platform: AIR-1234, Capabilities:\n",
    };
    for (const auto& test : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
                "Parse rule 'platformValue': " << test);
    }
  }

  {
    const auto& parserRule {tp.interfaceValue};
    std::vector<std::string> testsOk {
      "Interface: gi1, Port ID (outgoing port): gi10\n",
      "Interface: GigEth0/0, Port ID (outgoing port): GigEth10/10\n",
    };
    for (const auto& test : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
                "Parse rule 'platformValue': " << test);
    }
  }
}

BOOST_AUTO_TEST_CASE(testWhole)
{
  {
    const auto& parserRule {TestParser()};
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
      )STR",
    };
    for (const auto& test : testsOk) {
      Result out;
      BOOST_TEST(nmdp::testAttr(test.c_str(), parserRule, out, blank),
                "Parse rule 'testWhole': " << test);
      BOOST_TEST(1 == out.size());
    }
  }

  {
    const auto& parserRule {TestParser()};
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
                "Parse rule 'testWhole': " << test);
//      BOOST_TEST(3 == out.size());
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
