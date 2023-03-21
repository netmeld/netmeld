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
      using Parser::compartmentHeader;
      using Parser::hostData;
      using Parser::adapter;
      using Parser::ifaceTypeName;
      using Parser::dots;
      using Parser::servers;
      using Parser::ipLine;
      using Parser::getIp;
      using Parser::ifaceType;
};

BOOST_AUTO_TEST_CASE(testParts)
{
  TestParser tp;

  { // compartmentHeader
    const auto &parserRule {tp.compartmentHeader};
    std::vector<std::string> testsOk {
      //more than one equal sign in the first and last lines
      R"STR(==============================================================================
      Network Information for Compartment 1 (ACTIVE)
      ==============================================================================
      )STR",

      //one equal sign in the first and last lines
      R"STR(=
      NetworkInformationforCompartment1 (ACTIVE)
      =
      )STR"
    };
    std::vector<std::string> testsFail {
      //more than one equal sign between the first and last lines
      R"STR(==============================================================================
      Network Information for Compartment 1 (ACTIVE)
      Extra Line
      ==============================================================================
      )STR",

      //the first line of equal signs is missing
      R"STR(
      Network Information for Compartment 1 (ACTIVE)
      ==============================================================================
      )STR",

      //the last line of equal signs is missing
      R"STR(==============================================================================
      Network Information for Compartment 1 (ACTIVE)
      )STR",

      //there are non equal sign characters in the last line
      R"STR(==============================================================================
      Network Information for Compartment 1 (ACTIVE)
      ==============================================ERR================================
      )STR",

      //there are non equal sign characters in the first line
      R"STR(====ERR==========================================================================
      Network Information for Compartment 1 (ACTIVE)
      ==============================================================================
      )STR"
    };
    for (const auto& test: testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
                "Parse rule 'compartmentHeader': " << test);
    }
    for (const auto& test: testsFail) {
      BOOST_TEST(!nmdp::test(test.c_str(), parserRule, blank),
                "Parse rule 'compartmentHeader': " << test);
    }
  }

  { // hostData
    const auto &parserRule {tp.hostData};
    std::vector<std::string> testsOk {
      //Host Name, also tests empty Primary Dns Suffix
      R"STR(Host Name . . . . . . . . . . . . : TEST-DATA
      Primary Dns Suffix. . . . . . . . . . . :

      )STR",

      //Host Name, also tests non-empty Primary Dns Suffix
      R"STR(Host Name . . . . . . . . . . . . : TEST-DATA
      Primary Dns Suffix. . . . . . . . . . . : net.meld.net

      )STR"
    };
    for (const auto& test: testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
                "Parse rule 'hostData': " << test);
    }
  }

  { // adapter
    const auto &parserRule {tp.adapter};
    std::vector<std::string> testsOk {
      //adapter with no connection
      R"STR(Ethernet adapter Ethernet:

      Media State . . . . . . . . . . . : Media disconnected
      Connection-specific DNS Suffix  . : net.meld.net
      )STR",

      //adapter, ipv4, no default gateway
      R"STR(Ethernet adapter vEthernet (Default Switch):

      Connection-specific DNS Suffix  . :
      Link-local IPv6 Address . . . . . : fe80::aaaa:abcd:abcd:8080%18
      IPv4 Address. . . . . . . . . . . : 10.11.12.13
      Subnet Mask . . . . . . . . . . . : 255.255.255.240
      Default Gateway . . . . . . . . . :
      )STR",

      //adapter, ipv4, has default gateway with 1 address
      R"STR(Wireless LAN adapter Wi-Fi:

      Connection-specific DNS Suffix  . :
      Link-local IPv6 Address . . . . . : fe80::1234:1234:1234:1234%4
      IPv4 Address. . . . . . . . . . . : 192.168.1.168
      Subnet Mask . . . . . . . . . . . : 255.255.255.0
      Default Gateway . . . . . . . . . : 192.168.1.1
      )STR",

      //adapter, ipv4, has default gateway with 2+ address
      R"STR(Wireless LAN adapter Wi-Fi:

      Connection-specific DNS Suffix  . :
      Link-local IPv6 Address . . . . . : fe81::1234:1234:1234:1234%4
      IPv4 Address. . . . . . . . . . . : 192.168.2.3
      Subnet Mask . . . . . . . . . . . : 255.255.255.0
      Default Gateway . . . . . . . . . : 192.168.2.1
                                          192.168.2.2
      )STR"
    };

    for (const auto& test: testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
                "Parse rule 'adapter': " << test);
    }
  }

  { // ifaceTypeName
    const auto &parserRule {tp.ifaceTypeName};
    std::vector<std::string> testsOk {
      //1 word in adapter type
      "Ethernet adapter Ethernet:",
      //2 words in adapter type
      "Wireless LAN adapter Wi-Fi 2:",
      //3+ words in adapter type
      "Local Area Network adapter Network:"
    };
    std::vector<std::string> testsFail {
      //missing word adapter
      "Ethernet adept Ethernet:",
      //missing word adapter and adapter name
      "Wireless LAN adaptation:",

      //missing colon after adapter name
      "Local Area Network adapter Network"
    };
    for (const auto& test: testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
                "Parse rule 'ifaceTypeName': " << test);
    }
    for (const auto& test: testsFail) {
      BOOST_TEST(!nmdp::test(test.c_str(), parserRule, blank),
                "Parse rule 'ifaceTypeName': " << test);
    }
  }

  { // dots
    const auto &parserRule {tp.dots};
    std::vector<std::string> testsOk {
      ":",
      ". :",
      ". . . . . :"
    };
    for (const auto& test: testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
                "Parse rule 'dots': " << test);
    }
  }

  { // servers
    const auto &parserRule {tp.servers};
    std::vector<std::string> testsOk {

      R"STR(DHCP Server. . . . . . . . . . . . : 192.168.1.2
      )STR",

      R"STR(DNS Servers. . . . . . . . . . . . : 1.1.1.1
      )STR",

      R"STR(DNS Servers. . . . . . . . . . . . : 1.1.1.1
                                                 2.2.2.2
      )STR",

      R"STR(Test WINS Server. . . . . . . . . . . . : 1.1.1.1
      )STR"
    };
    for (const auto& test: testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
                "Parse rule 'servers': " << test);
    }
  }

  { // ipLine
    const auto &parserRule {tp.ipLine};
    std::vector<std::string> testsOk {
      R"STR(IP Address . . . . . . . . . . . . : 10.20.30.40)STR",
      R"STR(IPv4 Address . . . . . . . . . . . : 10.21.32.43)STR",
      R"STR(IPv6 Address . . . . . . . . . . . : fe80:abcd:abcd:abcd::abcd)STR",

      //tests subnet masks for IP addresses
      R"STR(IP Address . . . . . . . . . . . . : 10.20.30.40
      Subnet Mask. . . . . . . . . . . . . . . : 255.255.0.0)STR",
      R"STR(IPv4 Address . . . . . . . . . . . : 10.21.32.43
      Subnet Mask. . . . . . . . . . . . . . . : 255.255.0.0)STR",
      R"STR(IPv6 Address . . . . . . . . . . . : fe80:abcd:abcd:abcd::abcd
      Subnet Mask. . . . . . . . . . . . . . . : 2000:3000:4000:5000::)STR"
    };
    for (const auto& test: testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
                "Parse rule 'ipLine': " << test);
    }
  }

  { // getIp
    const auto &parserRule {tp.getIp};
    std::vector<std::string> testsOk {
      "192.168.168.168",
      "192.168.168.168(test)",
      "fe80:abcd:abcd:abcd::abcd",
      "fe80:abcd:abcd:abcd::abcd(test)",
    };
    std::vector<std::string> testsFail {
      "192.168.168.168(test) shouldfail"
      "fe80:abcd:abcd:abcd::abcd(test) shouldfail",
    };
    for (const auto& test: testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
                "Parse rule 'getIp': " << test);
    }
    for (const auto& test: testsFail) {
      BOOST_TEST(!nmdp::test(test.c_str(), parserRule, blank),
                "Parse rule 'getIp': " << test);
    }
  }

  { // ifaceType
    const auto &parserRule {tp.ifaceType};
    std::vector<std::string> testsOk {

      "Ethernet",
      "Wireless LAN",
      "Local Area Network"
    };
    for (const auto& test: testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
                "Parse rule 'ifaceType': " << test);
    }
  }

}

BOOST_AUTO_TEST_CASE(testWhole)
{
  TestParser tp;

  const auto& parserRule {tp};
  std::vector<std::string> testsOk {
    //ipconfig
    R"STR(Windows IP Configuration


    Unknown adapter Local Area Connection:

      Media State . . . . . . . . . . . : Media disconnected
      Connection-specific DNS Suffix  . :

    Ethernet adapter Ethernet:

      Media State . . . . . . . . . . . : Media disconnected
      Connection-specific DNS Suffix  . : AA-BCDEF.COM

    Ethernet adapter vEthernet (WSL):

      Connection-specific DNS Suffix  . :
      Link-local IPv6 Address . . . . . : fe80::fe80:fe80:fe:fe80%5
      IPv4 Address. . . . . . . . . . . : 172.172.172.1
      Subnet Mask . . . . . . . . . . . : 255.255.240.0
      Default Gateway . . . . . . . . . :

    Ethernet adapter VirtualBox Host-Only Network:

      Connection-specific DNS Suffix  . :
      Link-local IPv6 Address . . . . . : fe80::f5a4:152a:9cf3:2b32%4
      IPv4 Address. . . . . . . . . . . : 192.168.56.2
      Subnet Mask . . . . . . . . . . . : 255.255.255.0
      Default Gateway . . . . . . . . . : 192.168.56.1
                                          192.168.55.1

    Wireless LAN adapter Wi-Fi:

      Connection-specific DNS Suffix  . : net.meld.net
      IPv6 Address. . . . . . . . . . . : 2000:ab:11:2222::334
      IPv4 Address. . . . . . . . . . . : 10.0.0.2
      Subnet Mask . . . . . . . . . . . : 255.255.0.0
      Default Gateway . . . . . . . . . : 10.0.0.1

)STR",

    //ipconfig /all
    R"STR(
    Windows IP Configuration


      Host Name . . . . . . . . . . . . : PLACEHOLDER-HOST-NAME
      Primary Dns Suffix  . . . . . . . : net.meld.net
      Node Type . . . . . . . . . . . . : Hybrid
      IP Routing Enabled. . . . . . . . : No
      WINS Proxy Enabled. . . . . . . . : No
      DNS Suffix Search List. . . . . . : hsd1.nm.comcast.net

    Unknown adapter Local Area Connection:

      Media State . . . . . . . . . . . : Media disconnected
      Connection-specific DNS Suffix  . :
      Description . . . . . . . . . . . : Virtual Network Manager
      Physical Address. . . . . . . . . : 00-11-22-33-44-55
      DHCP Enabled. . . . . . . . . . . : No
      Autoconfiguration Enabled . . . . : Yes

    Ethernet adapter vEthernet (WSL):

      Connection-specific DNS Suffix  . :
      Description . . . . . . . . . . . : Virtual Ethernet Adapter
      Physical Address. . . . . . . . . : 00-AA-11-BB-22-CC
      DHCP Enabled. . . . . . . . . . . : No
      Autoconfiguration Enabled . . . . : Yes
      Link-local IPv6 Address . . . . . : fe80::1:2:3:4(Preferred)
      IPv4 Address. . . . . . . . . . . : 172.173.174.1(Preferred)
      Subnet Mask . . . . . . . . . . . : 255.255.240.0
      Default Gateway . . . . . . . . . :
      DHCPv6 IAID . . . . . . . . . . . : 123456789
      DHCPv6 Client DUID. . . . . . . . : 00-01-00-01-01-00-01-01-AB-BA-AB-BA-AB-BA
      DNS Servers . . . . . . . . . . . : fad7:1:4:ffff::1%1
                                          fbe8:2:5:ffff::2%1
                                          fcf8:3:6:ffff::3%1
      NetBIOS over Tcpip. . . . . . . . : Enabled

    Wireless LAN adapter Local Area Connection* 1:

      Media State . . . . . . . . . . . : Media disconnected
      Connection-specific DNS Suffix  . :
      Description . . . . . . . . . . . : Microsoft Wi-Fi Direct Virtual Adapter
      Physical Address. . . . . . . . . : 33-44-55-66-77-77
      DHCP Enabled. . . . . . . . . . . : Yes
      Autoconfiguration Enabled . . . . : Yes

    Wireless LAN adapter Wi-Fi:

      Connection-specific DNS Suffix  . : net.meld.net
      Description . . . . . . . . . . . : Placeholder Wi-Fi Card
      Physical Address. . . . . . . . . : FC-CF-AA-AA-CF-FC
      DHCP Enabled. . . . . . . . . . . : Yes
      Autoconfiguration Enabled . . . . : Yes
      IPv6 Address. . . . . . . . . . . : 2001:0dd:533:2002::2003(Preferred)
      Lease Obtained. . . . . . . . . . : Saturday, January 02, 2021 12:03:40 AM
      Lease Expires . . . . . . . . . . : Sunday, January 10, 2021 18:17:46 AM
      Link-local IPv6 Address . . . . . : fe80::dddd:a:0:0%13(Preferred)
      IPv4 Address. . . . . . . . . . . : 10.0.0.2(Preferred)
      Subnet Mask . . . . . . . . . . . : 255.255.255.0
      Lease Obtained. . . . . . . . . . : Saturday, April 17, 2021 2:03:38 AM
      Lease Expires . . . . . . . . . . : Saturday, April 24, 2021 12:53:56 PM
      Default Gateway . . . . . . . . . : 10.0.0.1
      DHCP Server . . . . . . . . . . . : 10.0.0.1
      DHCPv6 IAID . . . . . . . . . . . : 987654321
      DHCPv6 Client DUID. . . . . . . . : 00-01-00-01-27-72-27-72-AC-CA-AC-CA-35-53
      DNS Servers . . . . . . . . . . . : 2001:2002:fee::1
                                          2003:2004:fee::1
                                          1.1.1.1
                                          2.2.2.2
      NetBIOS over Tcpip. . . . . . . . : Enabled
    )STR",

    //ipconfig /all /allcompartments
    R"STR(
    Windows IP Configuration


    ==============================================================================
    Network Information for Compartment 1 (ACTIVE)
    ==============================================================================
      Host Name . . . . . . . . . . . . : PLACEHOLDER-HOST-NAME
      Primary Dns Suffix  . . . . . . . : net.meld.net
      Node Type . . . . . . . . . . . . : Hybrid
      IP Routing Enabled. . . . . . . . : No
      WINS Proxy Enabled. . . . . . . . : No
      DNS Suffix Search List. . . . . . : hsd1.nm.comcast.net

    Unknown adapter Local Area Connection:

      Media State . . . . . . . . . . . : Media disconnected
      Connection-specific DNS Suffix  . :
      Description . . . . . . . . . . . : Virtual Network Manager
      Physical Address. . . . . . . . . : 00-11-22-33-44-55
      DHCP Enabled. . . . . . . . . . . : No
      Autoconfiguration Enabled . . . . : Yes

    Ethernet adapter vEthernet (WSL):

      Connection-specific DNS Suffix  . :
      Description . . . . . . . . . . . : Virtual Ethernet Adapter
      Physical Address. . . . . . . . . : 00-AA-11-BB-22-CC
      DHCP Enabled. . . . . . . . . . . : No
      Autoconfiguration Enabled . . . . : Yes
      Link-local IPv6 Address . . . . . : fe80::1:2:3:4(Preferred)
      IPv4 Address. . . . . . . . . . . : 172.173.174.1(Preferred)
      Subnet Mask . . . . . . . . . . . : 255.255.240.0
      Default Gateway . . . . . . . . . :
      DHCPv6 IAID . . . . . . . . . . . : 123456789
      DHCPv6 Client DUID. . . . . . . . : 00-01-00-01-01-00-01-01-AB-BA-AB-BA-AB-BA
      DNS Servers . . . . . . . . . . . : fad7:1:4:ffff::1%1
                                          fbe8:2:5:ffff::2%1
                                          fcf8:3:6:ffff::3%1
      NetBIOS over Tcpip. . . . . . . . : Enabled

    Wireless LAN adapter Local Area Connection* 1:

      Media State . . . . . . . . . . . : Media disconnected
      Connection-specific DNS Suffix  . :
      Description . . . . . . . . . . . : Microsoft Wi-Fi Direct Virtual Adapter
      Physical Address. . . . . . . . . : 33-44-55-66-77-77
      DHCP Enabled. . . . . . . . . . . : Yes
      Autoconfiguration Enabled . . . . : Yes

    Wireless LAN adapter Wi-Fi:

      Connection-specific DNS Suffix  . : net.meld.net
      Description . . . . . . . . . . . : Placeholder Wi-Fi Card
      Physical Address. . . . . . . . . : FC-CF-AA-AA-CF-FC
      DHCP Enabled. . . . . . . . . . . : Yes
      Autoconfiguration Enabled . . . . : Yes
      IPv6 Address. . . . . . . . . . . : 2001:0dd:533:2002::2003(Preferred)
      Lease Obtained. . . . . . . . . . : Saturday, January 02, 2021 12:03:40 AM
      Lease Expires . . . . . . . . . . : Sunday, January 10, 2021 18:17:46 AM
      Link-local IPv6 Address . . . . . : fe80::dddd:a:0:0%13(Preferred)
      IPv4 Address. . . . . . . . . . . : 10.0.0.2(Preferred)
      Subnet Mask . . . . . . . . . . . : 255.255.255.0
      Lease Obtained. . . . . . . . . . : Saturday, April 17, 2021 2:03:38 AM
      Lease Expires . . . . . . . . . . : Saturday, April 24, 2021 12:53:56 PM
      Default Gateway . . . . . . . . . : 10.0.0.1
      DHCP Server . . . . . . . . . . . : 10.0.0.1
      DHCPv6 IAID . . . . . . . . . . . : 987654321
      DHCPv6 Client DUID. . . . . . . . : 00-01-00-01-27-72-27-72-AC-CA-AC-CA-35-53
      DNS Servers . . . . . . . . . . . : 2001:2002:fee::1
                                          2003:2004:fee::1
                                          1.1.1.1
                                          2.2.2.2
      NetBIOS over Tcpip. . . . . . . . : Enabled
    )STR"

  };
  for (const auto& test : testsOk) {
    BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
               "Parse rule 'start': " << test);
  }
}
