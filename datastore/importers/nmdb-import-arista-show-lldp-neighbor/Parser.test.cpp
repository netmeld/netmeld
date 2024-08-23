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
    using Parser::start;
};

BOOST_AUTO_TEST_CASE(testWholeNonDetail)
{
  { // IOS
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