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
      using Parser::nameColExists;
      using Parser::trlvColExists;

      // ARP
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

      // NDP
      using Parser::ndpHeaderArista;
      using Parser::ndpHeaderCiscoIos;
      using Parser::ndpHeaderCiscoNxos;

      using Parser::ndpEntryArista;
      using Parser::ndpEntryCiscoIos;
      using Parser::ndpEntryCiscoNxos;
};

BOOST_AUTO_TEST_CASE(testAristaParts)
{
  TestParser tp;

  // ARP
  {
    const auto& parserRule {tp.arpHeaderArista};

    std::vector<std::string> testsOk {
        "Address\tAge (sec)\tHardware Addr\tInterface\n"
      , "  Address\tAge (sec)\tHardware Addr\tInterface   \n"
      , "Address Age (sec) Hardware Addr Interface\n"
      , "VRF: vrf-name\nAddress\tAge (sec)\tHardware Addr\tInterface\n"
      };
    for (const auto& test : testsOk) {
      BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
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
      BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
                , "Parse rule 'arpEntryArista': " << test
                );
      BOOST_TEST(tin == out);
    }
  }
  // NDP
  {
    const auto& parserRule {tp.ndpHeaderArista};

    std::vector<std::string> testsOk {
        "IPv6 Address\tAge\tHardware Addr\tState\tInterface\n"
      , "VRF: vrf-name\nIPv6 Address\tAge\tHardware Addr\tState\tInterface\n"
      };
    for (const auto& test : testsOk) {
      BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
                , "Parse rule 'ndpHeaderArista': " << test
                );
    }
  }
  {
    const auto& parserRule {tp.ndpEntryArista};

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
      BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
                , "Parse rule 'ndpEntryArista': " << test
                );
      BOOST_TEST(tin == out);
    }
  }
}

BOOST_AUTO_TEST_CASE(testCiscoIosParts)
{
  TestParser tp;

  // ARP
  {
    const auto& parserRule {tp.arpHeaderCiscoIos};

    std::vector<std::string> testsOk {
        "Protocol\tAddress\tAge (min)\tHardware Addr\tType\tInterface\n"
      , "  Protocol\tAddress\tAge (min)\tHardware Addr\tType\tInterface   \n"
      , "Protocol Address Age (min) Hardware Addr Type Interface\n"
      };
    for (const auto& test : testsOk) {
      BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
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
        BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
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
        BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
                  , "Parse rule 'arpEntryCiscoIos': " << test
                  );
        BOOST_TEST(tin == out);
      }
    }
  }
  // NDP
  {
    const auto& parserRule {tp.ndpHeaderCiscoIos};

    std::vector<std::tuple<bool, std::string>> testsOk {
        {false, "IPv6 Address\tAge\tLink-layer Addr\tState\tInterface\n"}
      , {false, "  IPv6 Address\tAge\tLink-layer Addr\tState\tInterface   \n"}
      , {false, "IPv6 Address Age Link-layer Addr State Interface\n"}
      , {true , "IPv6 Address\tTRLV\tAge\tLink-layer Addr\tState\tInterface\n"}
      , {true , "IPv6 Address TRLV Age Link-layer Addr State Interface\n"}
      };
    for (const auto& [details, test] : testsOk) {
      BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
                , "Parse rule 'ndpHeaderCiscoIos': " << test
                );
      BOOST_TEST(details == tp.trlvColExists);
    }
  }
  {
    const auto& parserRule {tp.ndpEntryCiscoIos};

    {
      nmdo::MacAddress tma {"1234.1234.1234"};
      tma.addIpAddress(nmdo::IpAddress("1::2"));
      nmdo::InterfaceNetwork tin;
      tin.setName("Ethernet1");
      tin.addReachableMac(tma);

      std::vector<std::tuple<bool, std::string>> testsOk {
          {false, "1::2\t0\t1234.1234.1234\tREACH\tEthernet1\n"}
        , {false, "  1::2\t0\t1234.1234.1234\tREACH\tEthernet1   \n"}
        , {false, "1::2 0 1234.1234.1234 REACH Ethernet1"}
        , {false, "1::2 123 1234.1234.1234 REACH Ethernet1"}
        , {false, "1::2\t-\t1234.1234.1234\tREACH\tEthernet1\n"}
        , {false, "1::2\t0\t1234.1234.1234\tSTALE\tEthernet1\n"}
        , {false, "1::2\t0\t1234.1234.1234\tDELAY\tEthernet1\n"}
        , {false, "1::2\t0\t1234.1234.1234\tPROBE\tEthernet1\n"}
        , {false, "1::2\t0\t1234.1234.1234\t????\tEthernet1\n"}
        , {true , "1::2\t0\t0\t1234.1234.1234\tREACH\tEthernet1\n"}
        , {true , "1::2 123 - 1234.1234.1234 REACH Ethernet1\n"}
        };
      for (const auto& [details, test] : testsOk) {
        tp.trlvColExists = details;
        nmdo::InterfaceNetwork out;
        BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
                  , "Parse rule 'ndpEntryCiscoIos': " << test
                  );
        BOOST_TEST(tin == out);
      }
    }
    {
      nmdo::MacAddress tma;
      tma.addIpAddress(nmdo::IpAddress("1::2"));
      nmdo::InterfaceNetwork tin;
      tin.setName("Ethernet1");
      tin.addReachableMac(tma);

      std::vector<std::tuple<bool, std::string>> testsOk {
          {false, "1::2\t0\t-\tINCMP\tEthernet1\n"}
        , {false, "1::2\t-\t-\tINCMP\tEthernet1\n"}
        , {true , "1::2\t0\t0\t-\tINCMP\tEthernet1\n"}
        , {true , "1::2\t123\t-\t-\tINCMP\tEthernet1\n"}
        };
      tp.trlvColExists = false;
      for (const auto& [details, test] : testsOk) {
        tp.trlvColExists = details;
        nmdo::InterfaceNetwork out;
        BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
                  , "Parse rule 'ndpEntryCiscoIos': " << test
                  );
        BOOST_TEST(tin == out);
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(testCiscoNxosParts)
{
  TestParser tp;

  // ARP
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
      BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
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
        BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
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
        BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
                  , "Parse rule 'arpEntryCiscoNxos': " << test
                  );
        BOOST_TEST(tin == out);
      }
    }
  }
  // NDP
  {
    const auto& parserRule {tp.ndpHeaderCiscoNxos};

    std::vector<std::string> testsOk {
        "IPv6 Adjacency Table for VRF default\nTotal number of entries: 1\n"
        "Address\tAge\tMAC Address\tPref\tSource\tInterface\n"
      , "IPv6 Adjacency Table for VRF other\nTotal number of entries: 123\n"
        " Address\tAge\tMAC Address\tPref\tSource\tInterface  \n"
      , "IPv6 Adjacency Table for VRF default\nTotal number of entries: 1\n"
        "Address\tAge\tMAC Address\tPref\tSource\tInterface\tFlags\n"
      , "IPv6 Adjacency Table for VRF default\nTotal number of entries: 1\n"
        "Address\tAge\tMAC Address\tPref\tSource\tInterface\tMobility\tFlags\n"
      , "Flags: # - text\n\t\tG - text\n  R - more text\n\n"
        "IPv6 Adjacency Table for VRF default\nTotal number of entries: 1\n"
        "Address\tAge\tMAC Address\tPref\tSource\tInterface\n"
      };
    for (const auto& test : testsOk) {
      BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
                , "Parse rule 'ndpHeaderCiscoNxos': " << test
                );
    }
  }
  {
    const auto& parserRule {tp.ndpEntryCiscoNxos};

    {
      nmdo::MacAddress tma {"1234.1234.1234"};
      tma.addIpAddress(nmdo::IpAddress("1::2"));
      nmdo::InterfaceNetwork tin;
      tin.setName("Ethernet1");
      tin.addReachableMac(tma);

      std::vector<std::string> testsOk {
          "1::2\n\t00:00:00\t1234.1234.1234\t12\ticmpv6\tEthernet1\n"
        , "1::2\n\t00:00:00\t1234.1234.1234\t12\tother1\tEthernet1\t\n"
        , "  1::2\n\t00:00:00\t1234.1234.1234\t99\ticmpv6\tEthernet1   \n"
        , "1::2\n\t01:23:45\t1234.1234.1234\t12\ticmpv6\tEthernet1\n"
        , "1::2\n\t123w30d\t1234.1234.1234\t12\ticmpv6\tEthernet1\n"
        , "1::2\n 00:00:00 1234.1234.1234 12 icmpv6 Ethernet1\n"
        , "1::2\n\t00:00:00\t1234.1234.1234\t12\ticmpv6\tEthernet1\t\n"
        , "1::2\n\t00:00:00\t1234.1234.1234\t12\ticmpv6\tEthernet1\ta\tb\tc\n"
        , "1::2\n\t00:00:00\t1234.1234.1234\t12\ticmpv6\tEthernet1\tabc\n"
        };
      for (const auto& test : testsOk) {
        nmdo::InterfaceNetwork out;
        BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
                  , "Parse rule 'ndpEntryCiscoNxos': " << test
                  );
        BOOST_TEST(tin == out);
      }
    }
    {
      nmdo::MacAddress tma;
      tma.addIpAddress(nmdo::IpAddress("1::2"));
      nmdo::InterfaceNetwork tin;
      tin.setName("Ethernet1");
      tin.addReachableMac(tma);

      std::vector<std::string> testsOk {
          "1::2\n\t00:00:00\tINCOMPLETE\t12\ticmpv6\tEthernet1\n"
        , "1::2\n\t00:00:00\tINCOMPLETE\t12\ticmpv6\tEthernet1\t\n"
        };
      for (const auto& test : testsOk) {
        nmdo::InterfaceNetwork out;
        BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
                  , "Parse rule 'ndpEntryCiscoNxos': " << test
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
      BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
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
        BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
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
      // Parse Header
      const auto& parserRule  {tp.arpHeaderJuniper};
      const auto& nameFound   {tp.nameColExists};

      std::string headerNoName {
          "MAC Address       Address         Interface        Flags"
        };
      BOOST_TEST( nmdp::test(headerNoName.c_str(), parserRule, blank)
                , "Parse rule 'arpHeaderJuniper': " << headerNoName
                );
      BOOST_TEST(nameFound == false);

      std::string headerName {
          "MAC Address       Address         Name                     Interface"
        };
      BOOST_TEST( nmdp::test(headerName.c_str(), parserRule, blank)
                , "Parse rule 'arpHeaderJuniper': " << headerName
                );
      BOOST_TEST(nameFound == true);
  }
  {
    // Name included
    const auto& parserRule {tp.arpEntryJuniper};

    nmdo::MacAddress fma {"00:11:22:33:44:55"};
    fma.addIpAddress(nmdo::IpAddress("1.2.3.4"));
    nmdo::InterfaceNetwork fin;
    fin.setName("fxp0.0");
    fin.addReachableMac(fma);

    std::vector<std::string> testsName {
        // MAC Address, IP Address, Name, Interface
        "00:11:22:33:44:55 1.2.3.4   firewall.my.net          fxp0.0\n"
      };
    tp.nameColExists = true;
    for (const auto& test : testsName) {
      nmdo::InterfaceNetwork out;
      BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
                , "Parse rule 'arpEntryJuniper': " << test
                );
      BOOST_TEST(fin == out);
    }

    std::vector<std::string> testsNoName {
        // MAC Address, IP Address, Interface, Flags
        "00:11:22:33:44:55 1.2.3.4   fxp0.0    none\n"
      , "00:11:22:33:44:55 1.2.3.4   fxp0.0    permanent published\n"
      , "00:11:22:33:44:55 1.2.3.4   fxp0.0 [ge-0/0/0]    none\n"
      };
    tp.nameColExists = false;
    for (const auto& test : testsNoName) {
      nmdo::InterfaceNetwork out;
      BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
                , "Parse rule 'arpEntryJuniper': " << test
                );
      BOOST_TEST(fin == out);
    }
  }
}

BOOST_AUTO_TEST_CASE(testWhole)
{
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
    , R"(IPv6 Address     Age Hardware Addr    State Interface
         1::2         0:00:01 1234.1234.1234   REACH Ethernet1
         1::2         0:00:01 1234.1234.1234   REACH Ethernet1, Other
    )"
    // Cisco IOS
    , R"(Protocol  Address Age (min) Hardware Addr   Type  Interface
         Internet  1.2.3.4 0         1234.1234.1234  ARPA  Ethernet1
         Internet  1.2.3.4 -         1234.1234.1234  SNAP  Loopback1
         Internet  1.2.3.4 123       Incomplete      ARPA
    )"
    , R"(IPv6 Address Age Link-layer Addr State Interface
         1::2           0 1234.1234.1234  REACH Ethernet1
         1::2           0 1234.1234.1234  STALE Ethernet2
    )"
    , R"(IPv6 Address TRLV Age Link-layer Addr State Interface
         1::2           0    0 1234.1234.1234  REACH Ethernet1
         1::2           0    0 1234.1234.1234  STALE Ethernet2
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
    , R"(Flags: # - Adjacencies Throttled for Glean
                G - Adjacencies of vPC peer with G/W bit
                R - Adjacencies learnt remotely

         IPv6 Adjacency Table for VRF default
         Total number of entries: 2
         Address         Age       MAC Address     Pref Source     Interface
         1::2
                         00:04:52  1234.1234.1234  50   icmpv6     Ethernet1
         1::2
                             1w2d  1234.1234.1234  10   icmpv6     Ethernet2
    )"
    , R"(Flags: # - Adjacencies Throttled for Glean
                G - Adjacencies of vPC peer with G/W bit
                R - Adjacencies learnt remotely

         IPv6 Adjacency Table for VRF default
         Total number of entries: 2
         Address  Age       MAC Address     Pref Source  Interface  Mobility  Flags
         1::2
                  00:04:52  1234.1234.1234  50   icmpv6  Ethernet1
         1::2
                      1w2d  1234.1234.1234  10   icmpv6  Ethernet2  abc       abc
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
         00:04:5a:65:78:e1 192.168.65.13   lab.my.net               fxp0.0
      )"
    , R"(MAC Address       Address         Interface           Flags
         00:11:22:33:44:55 1.2.3.4        Ethernet1           none
         00:11:22:33:44:55 1.2.3.4        Ethernet1 [local.1] none
         Total entries: 2
      )"
    , R"(MAC Address       Address         Interface           Flags
         Total entries: 0

         {master:0}
      )"
    };
  for (const auto& test : testsOk) {
    TestParser tp; // putting here emulates new tool run per test
    const auto& parserRule {tp};
    BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
              , "Full parse: " << test
              );
  }
}
