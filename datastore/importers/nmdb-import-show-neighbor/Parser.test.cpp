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
      using Parser::arpJuniperConfig;

      using Parser::arpHeaderArista;
      using Parser::arpHeaderCiscoIos;
      using Parser::arpHeaderCiscoNxos;
      using Parser::arpHeaderCiscoWlc;
      using Parser::arpHeaderJuniper;

      using Parser::arpEntryArista;
      using Parser::arpEntryCiscoIos;
      using Parser::arpEntryCiscoNxos;
      using Parser::arpEntryCiscoWlc;
      using Parser::arpEntryJuniper;
};

BOOST_AUTO_TEST_CASE(testAristaParts)
{
  TestParser tp;
  {
    const auto& parserRule {tp.arpHeaderArista};

    std::vector<std::string> testsOk {
        "Address\tAge (sec)\tHardware Addr\tInterface\n"
      , "  Address\tAge (sec)\tHardware Addr\tInterface   \n"
      , "Address Age (sec) Hardware Addr Interface\n"
      , "VRF: vrf-name\nAddress\tAge (sec)\tHardware Addr\tInterface\n"
      /* TODO 20240219
        Disabled as parser logic does not currently support IPv6 for Arista
      , "IPv6 Address\tAge\tHardware Addr\tState\tInterface\n"
      , "VRF: vrf-name\nIPv6 Address\tAge\tHardware Addr\tState\tInterface\n"
      */
      };
    for (const auto& test : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank)
                , "Parse rule 'arpHeaderArista': " << test
                );
    }
  }
  {
    const auto& parserRule {tp.arpEntryArista};

    nmdo::MacAddress tma {"1234.1234.1234"};
    tma.addIpAddress(nmdo::IpAddress("1.2.3.4"));
    nmdo::InterfaceNetwork tin;
    tin.setName("Ethernet1");
    tin.addReachableMac(tma);

    std::vector<std::string> testsOk {
        "1.2.3.4\t0:00:00\t1234.1234.1234\tEthernet1\n"
      , "  1.2.3.4 1:23:45 1234.1234.1234 Ethernet1   \n"
      , "1.2.3.4 0:00:00 1234.1234.1234 Ethernet1, Other\n"
      };
    for (const auto& test : testsOk) {
      nmdo::InterfaceNetwork out;
      BOOST_TEST(nmdp::testAttr(test.c_str(), parserRule, out, blank)
                , "Parse rule 'arpEntryArista': " << test
                );
      BOOST_TEST(tin == out);
    }
  }
  /* TODO 20240219
    Disabled as parser logic does not currently support IPv6 for Arista
  {
    const auto& parserRule {tp.arpEntryArista};

    nmdo::MacAddress tma {"1234.1234.1234"};
    tma.addIpAddress(nmdo::IpAddress("1::2"));
    nmdo::InterfaceNetwork tin;
    tin.setName("Ethernet1");
    tin.addReachableMac(tma);

    std::vector<std::string> testsOk {
        "1::2\t0:00:00\t1234.1234.1234\tREACH\tEthernet1\n"
      , "  1::2 0:00:00 1234.1234.1234 REACH Ethernet1   \n"
      , "1::2\t0:00:00\t1234.1234.1234\tREACH\tEthernet1, Other\n"
      };
    for (const auto& test : testsOk) {
      nmdo::InterfaceNetwork out;
      BOOST_TEST(nmdp::testAttr(test.c_str(), parserRule, out, blank)
                , "Parse rule 'arpEntryArista': " << test
                );
      BOOST_TEST(tin == out);
    }
  }
  */
}

BOOST_AUTO_TEST_CASE(testCiscoIosParts)
{
  TestParser tp;
  {
    const auto& parserRule {tp.arpHeaderCiscoIos};

    std::vector<std::string> testsOk {
        "Protocol\tAddress\tAge (min)\tHardware Addr\tType\tInterface\n"
      , "  Protocol\tAddress\tAge (min)\tHardware Addr\tType\tInterface   \n"
      , "Protocol Address Age (min) Hardware Addr Type Interface\n"
      };
    for (const auto& test : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank)
                , "Parse rule 'arpHeaderCiscoIos': " << test
                );
    }
  }
  {
    const auto& parserRule {tp.arpEntryCiscoIos};

    {
      nmdo::MacAddress tma {"1234.1234.1234"};
      tma.addIpAddress(nmdo::IpAddress("1.2.3.4"));
      nmdo::InterfaceNetwork tin;
      tin.setName("Ethernet1");
      tin.addReachableMac(tma);

      std::vector<std::string> testsOk {
          "Internet\t1.2.3.4\t0\t1234.1234.1234\tARPA\tEthernet1\n"
        , "  Internet\t1.2.3.4\t0\t1234.1234.1234\tARPA\tEthernet1   \n"
        , "Internet 1.2.3.4 0 1234.1234.1234 ARPA Ethernet1"
        , "Internet 1.2.3.4 123 1234.1234.1234 ARPA Ethernet1"
        , "Internet\t1.2.3.4\t-\t1234.1234.1234\tARPA\tEthernet1\n"
        , "Internet\t1.2.3.4\t0\t1234.1234.1234\tSNAP\tEthernet1\n"
        , "Internet\t1.2.3.4\t0\t1234.1234.1234\tSAP\tEthernet1\n"
        };
      for (const auto& test : testsOk) {
        nmdo::InterfaceNetwork out;
        BOOST_TEST(nmdp::testAttr(test.c_str(), parserRule, out, blank)
                  , "Parse rule 'arpEntryCiscoIos': " << test
                  );
        BOOST_TEST(tin == out);
      }
    }
    {
      nmdo::MacAddress tma;
      tma.addIpAddress(nmdo::IpAddress("1.2.3.4"));
      nmdo::InterfaceNetwork tin;
      tin.addReachableMac(tma);

      std::vector<std::string> testsOk {
          "Internet\t1.2.3.4\t0\tIncomplete\tARPA\n"
        };
      for (const auto& test : testsOk) {
        nmdo::InterfaceNetwork out;
        BOOST_TEST(nmdp::testAttr(test.c_str(), parserRule, out, blank)
                  , "Parse rule 'arpEntryCiscoIos': " << test
                  );
        BOOST_TEST(tin == out);
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(testCiscoNxosParts)
{
  TestParser tp;
  {
    const auto& parserRule {tp.arpHeaderCiscoNxos};

    std::vector<std::string> testsOk {
        "IP ARP Table for context default\nTotal number of entries: 1\n"
        "Address\tAge\tMAC Address\tInterface\tFlags\n"
      , "IP ARP Table for context other\nTotal number of entries: 123\n"
        "  Address Age MAC Address Interface Flags   \n"
      , "Flags: * - text\n\t\t+ - text\n  # - more text\n\n"
        "IP ARP Table for context default\nTotal number of entries: 1\n"
        "Address\tAge\tMAC Address\tInterface\tFlags\n"
      };
    for (const auto& test : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank)
                , "Parse rule 'arpHeaderCiscoNxos': " << test
                );
    }
  }
  {
    const auto& parserRule {tp.arpEntryCiscoNxos};

    {
      nmdo::MacAddress tma {"1234.1234.1234"};
      tma.addIpAddress(nmdo::IpAddress("1.2.3.4"));
      nmdo::InterfaceNetwork tin;
      tin.setName("Ethernet1");
      tin.addReachableMac(tma);

      std::vector<std::string> testsOk {
          "1.2.3.4\t00:00:00\t1234.1234.1234\tEthernet1\t\n"
        , "1.2.3.4\t00:00:00\t1234.1234.1234\tEthernet1\n"
        , "  1.2.3.4\t00:00:00\t1234.1234.1234\tEthernet1\t   \n"
        , "1.2.3.4\t01:23:45\t1234.1234.1234\tEthernet1\t\n"
        , "1.2.3.4 00:00:00 1234.1234.1234 Ethernet1\n"
        , "1.2.3.4\t00:00:00\t1234.1234.1234\tEthernet1\t+\n"
        , "  1.2.3.4\t00:00:00\t1234.1234.1234\tEthernet1\t+   \n"
        , "1.2.3.4 00:00:00 1234.1234.1234 Ethernet1 +\n"
        };
      for (const auto& test : testsOk) {
        nmdo::InterfaceNetwork out;
        BOOST_TEST(nmdp::testAttr(test.c_str(), parserRule, out, blank)
                  , "Parse rule 'arpEntryCiscoNxos': " << test
                  );
        BOOST_TEST(tin == out);
      }
    }
    {
      nmdo::MacAddress tma;
      tma.addIpAddress(nmdo::IpAddress("1.2.3.4"));
      nmdo::InterfaceNetwork tin;
      tin.setName("Ethernet1");
      tin.addReachableMac(tma);

      std::vector<std::string> testsOk {
          "1.2.3.4\t00:00:00\tINCOMPLETE\tEthernet1\t\n"
        , "1.2.3.4\t00:00:00\tINCOMPLETE\tEthernet1\n"
        };
      for (const auto& test : testsOk) {
        nmdo::InterfaceNetwork out;
        BOOST_TEST(nmdp::testAttr(test.c_str(), parserRule, out, blank)
                  , "Parse rule 'arpEntryCiscoNxos': " << test
                  );
        BOOST_TEST(tin == out);
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(testCiscoWlcParts)
{
  TestParser tp;
  {
    const auto& parserRule {tp.arpHeaderCiscoWlc};

    std::vector<std::string> testsOk {
        "Number of arp entries................................ 1\n"
        "    MAC Address        IP Address     Port   VLAN   Type\n"
        "------------------- ---------------- ------ ------ ------\n"
      };
    for (const auto& test : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank)
                , "Parse rule 'arpHeaderCiscoNxos': " << test
                );
    }
  }
  {
    const auto& parserRule {tp.arpEntryCiscoWlc};

    {
      nmdo::MacAddress tma {"00:11:22:33:44:55"};
      tma.addIpAddress(nmdo::IpAddress("1.2.3.4"));
      nmdo::InterfaceNetwork tin;
      tin.setName("2");
      tin.addReachableMac(tma);

      std::vector<std::string> testsOk {
          "00:11:22:33:44:55\t1.2.3.4\t1\t2\tHost\n"
        , "  00:11:22:33:44:55    1.2.3.4   1  2   Host    \n"
        , "00:11:22:33:44:55\t1.2.3.4\t1\t2\tClient\n"
        };
      for (const auto& test : testsOk) {
        nmdo::InterfaceNetwork out;
        BOOST_TEST(nmdp::testAttr(test.c_str(), parserRule, out, blank)
                  , "Parse rule 'arpEntryCiscoNxos': " << test
                  );
        BOOST_TEST(tin == out);
      }
    }
  }
}


BOOST_AUTO_TEST_CASE(testArpJuniperParts)
{
  TestParser tp;
  {
    // No Name
    const auto& parserRule {tp.arpEntryJuniper};

    nmdo::MacAddress fma {"00:11:22:33:44:55"};
    fma.addIpAddress(nmdo::IpAddress("1.2.3.4"));
    nmdo::InterfaceNetwork fin;
    fin.setName("fxp0.0");
    fin.addReachableMac(fma);

    std::vector<std::string> testsOk {
      // MAC Address, IP Address, Interface, Flags
      "00:11:22:33:44:55 1.2.3.4      fxp0.0    none\n",
      "00:11:22:33:44:55 1.2.3.4   fxp0.0    permanent published\n",
      "00:11:22:33:44:55 1.2.3.4   fxp0.0 [ge-0/0/0]    none\n"
    };
    for (const auto& test : testsOk) {
        nmdo::InterfaceNetwork out;
      BOOST_TEST(nmdp::testAttr(test.c_str(), parserRule, out, blank),
                 "Parse rule 'arpEntryJuniper': " << test);
      BOOST_TEST(fin == out);
    }

  }
  {
      // Parse Header with Name
      const auto& parserRule {tp.arpHeaderJuniper};

      std::vector<std::string> testsOk {
          "MAC Address       Address         Name                     Interface"
      };
      for (const auto& test : testsOk) {
          BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank)
                  , "Parse rule 'arpHeaderJuniper': " << test
                  );
      }
  }
  {
    // Name included
    const auto& parserRule {tp.arpEntryJuniper};

    nmdo::MacAddress fma {"00:11:22:33:44:55"};
    fma.addIpAddress(nmdo::IpAddress("1.2.3.4"));
    nmdo::InterfaceNetwork fin;
    fin.setName("fxp0.0");
    fin.addReachableMac(fma);

    std::vector<std::string> testsOk {
      // MAC Address, IP Address, Name, Interface
      "00:11:22:33:44:55 1.2.3.4   firewall.my.net          fxp0.0\n"
    };
    for (const auto& test : testsOk) {
        nmdo::InterfaceNetwork out;
      BOOST_TEST(nmdp::testAttr(test.c_str(), parserRule, out, blank),
                 "Parse rule 'arpEntryJuniper': " << test);
      BOOST_TEST(fin == out);
    }
  }
}

BOOST_AUTO_TEST_CASE(testWhole)
{
  TestParser tp;

  const auto& parserRule {tp};
  std::vector<std::string> testsOk {
  // Arista
  R"(Address Age (sec) Hardware Addr   Interface
1.2.3.4 1:23:45   1234.1234.1234  Ethernet1
1.2.3.4 0:00:00   1234.1234.1234  Ethernet1, Other
)"
, R"(VRF: vrf-name
Address Age (sec) Hardware Addr   Interface
1.2.3.4 1:23:45   1234.1234.1234  Ethernet1
1.2.3.4 0:00:00   1234.1234.1234  Ethernet1, Other
)"
  /* TODO 20240219
    Disabled as parser logic does not currently support IPv6 for Arista
, R"(IPv6 Address     Age Hardware Addr    State Interface
1::2         0:00:01 1234.1234.1234   REACH Ethernet1
1::2         0:00:01 1234.1234.1234   REACH Ethernet1, Other
)"
  */
  // Cisco IOS
, R"(Protocol  Address Age (min) Hardware Addr   Type  Interface
Internet  1.2.3.4 0         1234.1234.1234  ARPA  Ethernet1
Internet  1.2.3.4 -         1234.1234.1234  SNAP  Loopback1
Internet  1.2.3.4 123       Incomplete      ARPA
)"
  // Cisco NXOS
, R"(Flags:  * - some text
        + - text text
        # - more text

IP ARP Table for context default
Total number of entries: 1
Address Age       MAC Address     Interface Flags
1.2.3.4 00:00:00  1234.1234.1234  Ethernet1
1.2.3.4 01:02:03  1234.1234.1234  Ethernet2 +
1.2.3.4    -      1234.1234.1234  Ethernet3 *
1.2.3.4 00:00:00  INCOMPLETE      Ethernet4
)"
  // Cisco WLC/WISM
/*  Not enabled in parsing logic, so commenting out
, R"(Number of arp entries................................ 1
    MAC Address        IP Address     Port   VLAN   Type
------------------- ---------------- ------ ------ ------
00:11:22:33:44:55   1.2.3.4          1      2      Host
00:11:22:33:44:55   1.2.3.4          2      3      Client
)"
*/
  // Juniper
, R"(MAC Address       Address         Name                     Interface
00:e0:81:22:fd:74 192.168.64.10   firewall.my.net          fxp0.0
00:04:5a:65:78:e1 192.168.65.13   lab.my.net               fxp0.0)"
, R"(MAC Address       Address         Interface           Flags
00:11:22:33:44:55 1.2.3.4        Ethernet1           none
00:11:22:33:44:55 1.2.3.4        Ethernet1 [local.1] none
Total entries: 2)"
, R"(MAC Address       Address         Interface           Flags
Total entries: 0

{master:0})"
  };
  for (const auto& test : testsOk) {
    BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
               "Full parse: " << test);
  }
}
