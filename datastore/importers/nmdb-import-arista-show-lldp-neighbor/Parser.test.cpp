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

    using Parser::port;

    using Parser::noDetailTableInfo;
    using Parser::noDetailHeader;
    using Parser::noDetailEntry;

    using Parser::detailHeader;
    using Parser::detailNeighborLine;
    using Parser::detailDiscovered;
    using Parser::detailChassisId;
    using Parser::detailPortId;
    using Parser::detailPortDescription;
    using Parser::detailSystemName;
    using Parser::detailSystemDescription;
    using Parser::detailCapabilities;
    using Parser::detailVlan;
};

BOOST_AUTO_TEST_CASE(testUtil)
{
  TestParser tp;
  {
    const auto& parserRule {tp.port};

    std::vector<std::string> testsOk {
        "Ethernet1"
      , "Ethernet 2/1"
      , "Ethernet3/1"
      , "Ethernet 4"
      , "net12-2"
      };
    for (const auto& test : testsOk) {
      std::string out;
      BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
                , "Parse rule 'port': " << test
                );
      BOOST_TEST(test == out);
    }
  }
}

BOOST_AUTO_TEST_CASE(testNoDetailParts)
{
  TestParser tp;

  {
    tp.nd = NeighborData();
    const auto& parserRule {tp.noDetailTableInfo};

    std::vector<std::string> testsOk {
        "Last table change time   : 0:00:16 ago\n"
        "Number of table inserts  : 11\n"
        "Number of table deletes  : 5\n"
        "Number of table drops    : 0\n"
        "Number of table age-outs : 0\n"
      , "Last table change time   : 23:59:59 ago\n"
        "Number of table inserts  : 11\n"
        "Number of table deletes  : 500\n"
        "Number of table drops    : 100\n"
        "Number of table age-outs : 100\n"
        "\n"
      };
    for (const auto& test : testsOk) {
      BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
                , "Parser rule 'noDetailTableInfo': " << test
                );
      BOOST_TEST(NeighborData() == tp.nd);
    }
  }

  {
    tp.nd = NeighborData();
    const auto& parserRule {tp.noDetailHeader};

    std::vector<std::string> testsOk {
        "Port  Neighbor Device ID  Neighbor Port ID  TTL\n"
        "-----------------------------------------------\n"
      , "Port\tNeighbor Device ID\tNeighbor Port ID\tTTL\n"
        "-----------------------------------------------\n"
      };
    for (const auto& test : testsOk) {
      BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
                , "Parser rule 'noDetailHeader': " << test
                );
      BOOST_TEST(NeighborData() == tp.nd);
    }
  }

  {
    const auto& parserRule {tp.noDetailEntry};

    std::vector<std::tuple<std::string, std::string, std::string, std::string>> testsOk {
        {"Et1  HOST1.domain  Ethernet1  120\n"
        , "Et1", "HOST1.domain", "Ethernet1"
        }
      , {"Et2\tHOST2.domain\tEthernet2\t120\n"
        , "Et2", "HOST2.domain", "Ethernet2"
        }
      , {"Et3  host3.domain  Ethernet3  120\n"
        , "Et3", "host3.domain", "Ethernet3"
        }
      , {"Et4\thost4.domain\tEthernet4\t120\n"
        , "Et4", "host4.domain", "Ethernet4"
        }
      , {"Et5/1  HOST5.domain  Ethernet5  120\n"
        , "Et5/1", "HOST5.domain", "Ethernet5"
        }
      , {"Et6/1\tHOST6.domain\tEthernet6\t120\n"
        , "Et6/1", "HOST6.domain", "Ethernet6"
        }
      , {"Et7  HOST7.domain  Ethernet 7/1  120\n"
        , "Et7", "HOST7.domain", "Ethernet 7/1"
        }
      , {"Et8\tHOST8.domain\tEthernet 8/1\t120\n"
        , "Et8", "HOST8.domain", "Ethernet 8/1"
        }
      , {"Et9  1234.5678.90ab  Ethernet9  120\n"
        , "Et9", "1234.5678.90ab", "Ethernet9"
        }
      , {"Et10\t1234.5678.90ab\tEthernet10\t120\n"
        , "Et10", "1234.5678.90ab", "Ethernet10"
        }
      , {"Et11  HOST11.domain  net11-2  120\n"
        , "Et11", "HOST11.domain", "net11-2"
        }
      , {"Et12\tHOST12.domain\tnet12-2\t120\n"
        , "Et12", "HOST12.domain", "net12-2"
        }
      };
    for (const auto& [test, srcIfaceName, hostName, ifaceName] : testsOk) {
      tp.nd = NeighborData();
      BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
                , "Parse rule 'noDetailEntry': " << test
                );
      BOOST_TEST(srcIfaceName == tp.nd.srcIfaceName);
      BOOST_TEST(hostName == tp.nd.curHostname);
      BOOST_TEST(ifaceName == tp.nd.curIfaceName);
    }
  }
}

BOOST_AUTO_TEST_CASE(testDetailParts)
{
  TestParser tp;

  {
    tp.nd = NeighborData();
    const auto& parserRule {tp.detailHeader};

    std::vector<std::tuple<std::string, std::string>> testsOk {
        {"Interface Ethernet 4/1 detected 1 LLDP neighbors:\n\n"
        , "Ethernet 4/1"
        }
      , {"Interface Ethernet4/1 detected 1 LLDP neighbors:\n\n"
        , "Ethernet4/1"
        }
      , {"Interface Ethernet 4 detected 1 LLDP neighbors:\n\n"
        , "Ethernet 4"
        }
      , {"Interface Ethernet4 detected 1 LLDP neighbors:\n\n"
        , "Ethernet4"
        }
      };
    for (const auto& [test, srcIfaceName] : testsOk) {
      BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
                , "Parse rule 'detailHeader': " << test
                );
      BOOST_TEST(srcIfaceName == tp.nd.srcIfaceName);
    }
  }

  {
    tp.nd = NeighborData();
    const auto& parserRule {tp.detailNeighborLine};

    std::vector<std::string> testsOk {
        "\tNeighbor 1234.5678.90ab/\"123\", age 1 seconds\n"
      , "\tNeighbor 1234.5678.90ab/\"Ethernet6\", age 4 seconds\n"
      , "\tNeighbor 1234.5678.90ab/\"Ethernet6/25\", age 7 seconds\n"
      , "\tNeighbor 1234.5678.90ab/\"Ethernet 6/25\", age 10 seconds\n"
      , "\tNeighbor 1234.5678.90ab/Ethernet6, age 13 seconds\n"
      , "\tNeighbor 1234.5678.90ab/Ethernet6/25, age 16 seconds\n"
      , "\tNeighbor 1234.5678.90ab/Ethernet 6/25, age 19 seconds\n"
      };
    for (const auto& test : testsOk) {
      BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
                , "Parse rule 'detailNeighborLine': " << test
                );
      BOOST_TEST(NeighborData() == tp.nd);
    }
  }

  {
    tp.nd = NeighborData();
    const auto& parserRule {tp.detailDiscovered};

    std::vector<std::string> testsOk {
        "\tDiscovered 5 days, 3:58:58 ago; Last changed 5 days, 0:00:01 ago\n"
      , "\tDiscovered 365 days, 00:07:18 ago; Last changed 38 days, 18:07:16 ago\n"
      };
    for (const auto& test : testsOk) {
      BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
                , "Parse rule 'detailDiscovered': " << test
                );
      BOOST_TEST(NeighborData() == tp.nd);
    }
  }

  {
    const auto& parserRule {tp.detailChassisId};

    std::vector<std::tuple<std::string,std::string>> testsOk {
        {"- Chassis ID type: MAC address (4)\nChassis ID     : 1234.5678.90ab\n"
        , "1234.5678.90ab"
        }
      , {"- Chassis ID type: MAC address (4)\nChassis ID     : 001c.730f.11a8qqq\n"
        , "001c.730f.11a8qqq"
        }
      };
    for (const auto& [test, chassisId] : testsOk) {
      tp.nd = NeighborData();
      BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
                , "Parse rule 'detailChassisId': " << test
                );
      BOOST_TEST(chassisId == tp.nd.curMacAddr);
    }
  }

  {
    const auto& parserRule {tp.detailPortId};

    std::vector<std::tuple<std::string,std::string>> testsOk {
        {"- Port ID type: Interface name(1)\nPort ID     : \"Ethernet1\"\n"
        , "Ethernet1"
        }
      , {"- Port ID type: Interface name (5)\nPort ID     : \"Ethernet6/25\"\n"
        , "Ethernet6/25"
        }
      , {"- Port ID type: Interface name(1)\nPort ID     : \"Ethernet 1\"\n"
        , "Ethernet 1"
        }
      , {"- Port ID type: Interface name (5)\nPort ID     : \"Ethernet 6/25\"\n"
        , "Ethernet 6/25"
        }
      };
    for (const auto& [test, portId] : testsOk) {
      tp.nd = NeighborData();
      BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
                , "Parse rule 'detailPortId': " << test
                );
      BOOST_TEST(portId == tp.nd.curIfaceName);
    }
  }

  {
    const auto& parserRule {tp.detailPortDescription};

    std::vector<std::tuple<std::string,std::string>> testsOk {
        {"  - Port Description: \"HOST_Et1\"\n"
        , "HOST_Et1"
        }
      , {"- Port Description: \"\"\n"
        , ""
        }
      };
    for (const auto& [test, portDescription] : testsOk) {
      tp.nd = NeighborData();
      BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
                , "Parse rule 'detailPortDescription': " << test
                );
      BOOST_TEST(portDescription == tp.nd.curPortDescription);
    }
  }

  {
    const auto& parserRule {tp.detailSystemName};

    std::vector<std::tuple<std::string,std::string>> testsOk {
        {"- System Name: \"HOST.domain\"\n"
        , "HOST.domain"
        }
      , {"- System Name: \"host1.domain\"\n"
        , "host1.domain"
        }
      , {"- System Name: \"web.site.com\"\n"
        , "web.site.com"
        }
      };
    for (const auto& [test, systemName] : testsOk) {
      tp.nd = NeighborData();
      BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
                , "Parse rule 'detailSystemName': " << test
                );
      BOOST_TEST(systemName == tp.nd.curHostname);
    }
  }

  {
    const auto& parserRule {tp.detailSystemDescription};

    std::vector<std::tuple<std::string,std::string>> testsOk {
        {"- System Description: \"Arista Networks EOS version 4.26.5M running on an Arista Networks DCS-7280SR-48C6\"\n"
        , "Arista Networks EOS version 4.26.5M running on an Arista Networks DCS-7280SR-48C6"
        }
      , {"- System Description: \"\"\n"
        , ""
        }
      };
    for (const auto& [test, systemDescription] : testsOk) {
      tp.nd = NeighborData();
      BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
                , "Parse rule 'detailSystemDescription': " << test
                );
      BOOST_TEST(systemDescription == tp.nd.curSysDescription);
    }
  }

  {
    const auto& parserRule {tp.detailCapabilities};

    std::vector<std::tuple<std::string,std::string>> testsOk {
        {"- System Capabilities : Bridge, Router\nEnabled Capabilities: Bridge, Router\n"
        , "Bridge, Router"
        }
      , {"- System Capabilities : Bridge, Router\nEnabled Capabilities: Bridge\n"
    //  , "Bridge"
        , "Bridge, Router"
        }
      };
    for (const auto& [test, capabilities] : testsOk) {
      tp.nd = NeighborData();
      BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
                , "Parse rule 'detailCapabilities': " << test
                );
      BOOST_TEST(capabilities == tp.nd.curDeviceType);
    }
  }

  {
    tp.nd = NeighborData();
    const auto& parserRule {tp.detailVlan};

    std::vector<std::tuple<std::string, unsigned short>> testsOk {
        {"VLAN ID: 1234, VLAN Name: \"vlan-1234\"\n"
        , 1234
        }
      , {"VLAN ID: 5678, VLAN Name: \"vlan-5678\"\n"
        , 5678
        }
      , {"- IEEE802.1 Port VLAN ID: 1234\n"
        , 1234
        }
      , {"- IEEE802.1 Port VLAN ID: 5678\n"
        , 5678
        }
      };
    for (const auto& [test, vlanId] : testsOk) {
      BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
                , "Parse rule 'detailVlan': " << test
                );
      BOOST_TEST(vlanId == tp.nd.curVlans.back());
    }
    BOOST_TEST(testsOk.size() == tp.nd.curVlans.size());
  }
}

BOOST_AUTO_TEST_CASE(testWholeNonDetail)
{
  {
    TestParser tp;
    const auto& parserRule {tp.start};

    std::string test {
      R"STR(Last table change time   : 0:00:16 ago
Number of table inserts  : 11
Number of table deletes  : 5
Number of table drops    : 0
Number of table age-outs : 0
 
Port            Neighbor Device ID                     Neighbor Port ID    TTL
------------ -------------------------------------- ---------------------- ---
Et1             HOST1.domain                           Ethernet1           120
Et2             host2.domain                           Ethernet2           120
Et3/1           host3.domain                           123                 120
Et4/1           host4.domain                           321                 120
Et5/1           1234.5678.90ab                         Ethernet 1/1        120
Ma1             host5.domain                           net1-2              120
      )STR"
      };
    Result out;
    BOOST_TEST(nmdp::testAttr(test.c_str(), parserRule, out, blank),
              "Parse rule 'testWholeNonDetail': " << test);
    BOOST_TEST_REQUIRE(1 == out.size());
  }
}

BOOST_AUTO_TEST_CASE(testWholeDetail)
{
  {
    TestParser tp;
    const auto& parserRule {tp.start};
    std::string test {
      R"STR(Interface Ethernet1 detected 0 LLDP neighbors:
 
Interface Ethernet2 detected 1 LLDP neighbors:
 
  Neighbor 1234.5678.90ab/"Ethernet1", age 7 seconds
  Discovered 38 days, 18:07:18 ago; Last changed 38 days, 18:07:16 ago
  - Chassis ID type: MAC address (4)
    Chassis ID     : 1234.5678.90ab
  - Port ID type: Interface name(1)
    Port ID     : "Ethernet1"
  - Time To Live: 120 seconds
  - Port Description: "HOST_Et1"
  - System Name: "HOST.domain"
  - System Description: "Arista Networks EOS version 4.26.5M running on an Arista Networks DCS-7280SR-48C6"
  - System Capabilities : Bridge, Router
    Enabled Capabilities: Bridge, Router
  - Management Address Subtype: IPv4
    Management Address        : 1.2.3.4
    Interface Number Subtype  : ifIndex (2)
    Interface Number          : 1234567
    OID String                :
  - IEEE802.1 Port VLAN ID: 1
  - IEEE802.1/IEEE802.3 Link Aggregation
    Link Aggregation Status: Capable, Enabled (0x03)
    Port ID                : 12345678
  - IEEE802.3 Maximum Frame Size: 12345 bytes
 
Interface Ethernet3/1 detected 1 LLDP neighbors:
 
  Neighbor 4321.5678.90ab/"123", age 27 seconds
  Discovered 45 days, 17:19:54 ago; Last changed 45 days, 17:19:54 ago
  - Chassis ID type: MAC address (4)
    Chassis ID     : 4321.5678.90ab
  - Port ID type: Locally assigned(7)
    Port ID     : "123"
  - Time To Live: 120 seconds
  - Port Description: "HOST_Et123"
  - System Name: "host1.domain"
  - System Description: "Juniper Networks, Inc. srx4600 internet router, kernel JUNOS 21.2R3.8, Build date: 2022-03-10 07:06:51 UTC Copyright (c) 1996-2022 Juniper Networks, Inc."
  - System Capabilities : Bridge, Router
    Enabled Capabilities: Bridge, Router
  - IEEE802.1 VLAN Name
    VLAN ID: 1234, VLAN Name: "vlan-1234"
    VLAN ID: 5678, VLAN Name: "vlan-5678"
  - IEEE802.1/IEEE802.3 Link Aggregation
    Link Aggregation Status: Capable, Enabled (0x03)
    Port ID                : 123
  - IEEE802.3 MAC/PHY Configuration/Status
    Auto-negotiation       : Not Supported
                             10BASE-T (half-duplex)
                             10BASE-T (full-duplex)
                             100BASE-TX (half-duplex)
                             100BASE-TX (full-duplex)
                             1000BASE-X (half-duplex)
                             1000BASE-X (full-duplex)
                             1000BASE-T (full-duplex)
    Operational MAU Type   : Unknown
  - IEEE802.3 Maximum Frame Size: 1518 bytes
  - LLDP-MED Capabilities:
    'Capabilities' capable                 : True
    'Network Policy' capable               : True
    'Location Identification' capable      : True
    'Extended Power via MDI - PSE' capable : True
    'Extended Power via MDI - PD' capable  : False
    'Inventory' capable                    : False
    Device type                            : Network connectivity
      )STR",
      };
    Result out;
    BOOST_TEST(nmdp::testAttr(test.c_str(), parserRule, out, blank),
              "Parse rule 'testDetailWhole': " << test);
    BOOST_TEST(1 == out.size());
    size_t conCount {2};
    BOOST_TEST(conCount == out[0].devInfos.size());
    BOOST_TEST(conCount == out[0].interfaces.size());
  }
}