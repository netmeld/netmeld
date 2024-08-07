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
    using Parser::nocdp;
    using Parser::versionPIXASA;
    using Parser::spanningTreeInitial;
    using Parser::deviceAAA;
    using Parser::globalServices;
    using Parser::domainData;
    using Parser::vrfInstance;
    using Parser::channelGroup;
    using Parser::encapsulation;
    using Parser::switchport;
    using Parser::switchportPortSecurity;
    using Parser::switchportVlan;
    using Parser::interface;
    using Parser::routerId;
    using Parser::route;
    using Parser::vlan;
    using Parser::accessPolicyRelated;
    using Parser::policyMap;
    using Parser::classMap;

};

BOOST_AUTO_TEST_CASE(testParts)
{
  TestParser tp;

  {
    const auto& parserRule {tp.nocdp};

    const std::vector<std::string> nocdpTestDataOk
      {
        "no cdp run\n",
        
        "no cdp enable\n"
      }
      ;

    for (const auto& test : nocdpTestDataOk) {
      BOOST_TEST(nmdp::test(test, parserRule, blank),
                 "Parse rule 'nocdp':" << test);
    }
  }

  {
    const auto& parserRule {tp.versionPIXASA};



    const std::vector<std::string> versionPIXASATestDataOk
      {
        "PIX Version\n",
      
        "ASA Version 1\n",
      
        "ASA Version 1 a\n"
      }
      ;

    for (const auto& test : versionPIXASATestDataOk) {
      BOOST_TEST(nmdp::test(test, parserRule, blank),
                 "Parse rule 'versionPIXASA':" << test);
    }
  }

  {
    const auto& parserRule {tp.deviceAAA};

    const std::vector<std::string> deviceAAATestDataOk
      {
        
        "aaa tkn1\n",

        "aaa tkn1 tkn2\n"
        
      }
      ;

    for (const auto& test : deviceAAATestDataOk) {
      BOOST_TEST(nmdp::test(test, parserRule, blank),
                 "Parse rule 'deviceAAA':" << test);
    }
  }

  {
    const auto& parserRule {tp.globalServices};
    const std::vector<std::string> globalServicesTestDataOk =
      {
        "sntp server 1.2.3.4\n",

        "ntp server 1.2.3.4 tkn1\n",
        
        "sntp server vrp tkn1 1.2.3.4 tkn1 tkn2\n",
        
        "ntp server vrp tkn1 1.2.3.4 tkn1 tkn2\n",
        
        "snmp-server host 1.2.3.4\n",
        
        "snmp-server host 1.2.3.4 tkn1\n",
        
        "snmp-server host 1.2.3.4 tkn1 tkn2\n",
        
        "radius-server host 1.2.3.4\n",
        
        "radius-server host 1.2.3.4 tkn1\n",
        
        "radius-server host 1.2.3.4 tkn1 tkn2\n",
        
        "ip name-server 1.2.3.4\n",
        
        "ip name-server vrf tkn2 1.2.3.4 4.3.2.1\n",
        
        "logging server 1.2.3.4\n",
        
        "logging server 1.2.3.4 tkn1\n",
        
        "logging server 1.2.3.4 tkn1 tkn2\n"
      }
      ;
    for (const auto& test : globalServicesTestDataOk) {
      BOOST_TEST(nmdp::test(test, parserRule, blank),
                 "Parse rule 'globalServices':" << test);
    }
  }

  {
    const auto& parserRule {tp.spanningTreeInitial};

    const std::vector<std::string> spanningTreeInitialTestDataOk =
      {
        "spanning-tree mode mode1\n",
          
        "spanning-tree mst configuration\n"
        "    tkn1\n",
          
        "spanning-tree mst configuration\n"
        "    tkn1 tkn2\n",
          
        "spanning-tree portfast bpduguard\n",
          
        "spanning-tree portfast bpduguard tkn1\n",
          
        "spanning-tree portfast bpduguard tkn1 tkn2\n",
          
        "spanning-tree portfast bpdufilter\n",
          
        "spanning-tree portfast bpdufilter tkn1\n",
          
        "spanning-tree portfast bpdufilter tkn1 tkn2\n"
      }
      ;
      for (const auto& test : spanningTreeInitialTestDataOk) {
        BOOST_TEST(nmdp::test(test, parserRule, blank),
                  "Parse rule 'spanningTreeInitial':" << test);
      }
  }

  {
    const auto& parserRule {tp.domainData};

    const std::vector<std::string> testData =
      {
        "switchname sname1.dm\n",
        
        "hostname sname1.dm ignored\n",
        
        "ip domain-name sname1.dm\n",
        
        "ip dns domain-name sname1.dm\n",
        
        "ip dns domain name vrf vrf1 sname1.dm\n",
        
        "domain-name sname1.dm\n"
      }
      ;

    for (const auto& test : testData) {
      BOOST_TEST(nmdp::test(test, parserRule, blank),
                 "Parse rule 'domainData':" << test);
    }
  }

  {
    
    const auto& parserRule {tp.vrfInstance};

    const std::vector<std::string> testData =
      {
        "vrf definition tkn1\n"
        "    description this is a description\n",

        "vrf definition tkn1\n"
        "    ignorable this is something that will be ignored\n"
        "    description this is a description\n"
        "    anotherignorable this is something that will be ignored\n"
      }
      ;

    for (const auto& test : testData) {
      BOOST_TEST(nmdp::test(test, parserRule, blank),
                 "Parse rule 'vrfInstance':" << test);
    }
  }

  {
    
    const std::vector<std::string> channelGroupTestDataOk = 
      {
        "channel-group 1 mode tkn1",

        "channel-group 1 mode tkn1 type tkn2"
      }
      ;

    const std::vector<std::string> encapsulationTestDataOk =
      {
        "encapsulation dot1q 0",

        "encapsulation dot1Q 1"
      }
      ;

    const std::vector<std::string> switchportPortSecurityTestDataOk =
      {
        "port-security max 10 ",

        "port-security max 10 vlan 1",

        "port-security maximum 10",

        "port-security maximum 10 vlan 1",

        "port-security mac-address max 10",

        "port-security mac-address max 10 vlan 1",

        "port-security mac-address maximum 10",

        "port-security mac-address maximum 10 vlan 1",

        "port-security mac-address max 10 vlan 1",

        "port-security mac-address maximum 10",

        "port-security mac-address maximum 10 vlan 1",

        "port-security mac-address ",

        "port-security mac-address A0:A1:A2:A3:A4:A5",

        "port-security mac-address sticky",

        "port-security mac-address sticky A0:A1:A2:A3:A4:A5",

        "port-security sticky mac-address A0:A1:A2:A3:A4:A5",

        "port-security sticky mac-address A0:A1:A2:A3:A4:A5 vlan 1",

        "port-security violation tkn1",

        "port-security shutdown-time 60",

        "port-security limit rate tkn1",

        "port-security limit rate tkn1 tkn2"

      }
      ;

    const std::vector<std::string> switchportVlanTestDataOk =
    
      {
        "access native vlan add tkn1 1",

        "access native vlan 1-100"
      }
      ;

    const std::vector<std::string> switchportTestDataOk =
      {
        "switchport mode tkn1",

        "switchport nonegotiate"
      }
      ;

    const std::vector<std::string> interfaceTestDataOk =
      {
        "interface iface\n"
        "  description 123/ABC 01234-56\n",

        "interface iface2\n"
        "  description 123/ABC 01234-56\n"
        "  no ip address\n"
        "  shutdown\n"
        "  cdp enable\n"
        "  vrf member tkn1\n"
        "  standby 10 ip 1.2.3.4\n"
        "  standby 10 ip 4.3.2.1 secondary\n",

        "interface iface3 tkn1\n"
        "  description 123/ABC 01234-56\n"
        "  ip address 1.2.0.0/16\n"
        "  ip address 1.2.3.4\n"
        "  vrrp ip 4.3.2.1\n"
        "  ip helper-address 1.2.3.4\n",

        "interface iface4\n"
        "  description 123/ABC 01234-56\n"
        "  ipv6-address sname1.dm 1111:1111:1111:1111:1111:1111:1111:1111\n"
        "  ipv6-address sname2.dm\n"
        "  ipv6-address 1111:1111:1111:1111:1111:1111:1111:1111\n"
        "  ip helper-address vrf tkn1 1.2.3.4\n"
        "  ip helper-address vrf tkn1 1.2.3.4 tkn1\n"
        "  ip helper-address global 1.2.3.4 tkn1 tkn2\n"
        "  ip dhcp relay address 1.2.3.4\n"
        "  ip access-group tkn1 tkn2\n"
        "  service-policy tkn1 tkn2\n"
        "  nameif tkn1\n"
        "  vrf forwarding tkn1\n"
        "  vmware vm mac A0:A1:A2:A3:A4:A5\n"

      }
      ;

    const auto& parserRule {tp.interface};
    for (const auto& test : interfaceTestDataOk) {
      BOOST_TEST(nmdp::test(test, tp.interface, blank),
                 "Parse rule 'interface':" << test);
    }

    for (const auto& test : channelGroupTestDataOk) {
      BOOST_TEST(nmdp::test(test, tp.channelGroup, blank),
                 "Parse rule 'channelGroup':" << test);
    }

    for (const auto& test : encapsulationTestDataOk) {
      BOOST_TEST(nmdp::test(test, tp.encapsulation, blank),
                 "Parse rule 'encapsulation':" << test);
    }

    for (const auto& test : switchportTestDataOk) {
      BOOST_TEST(nmdp::test(test, tp.switchport, blank),
                 "Parse rule 'switchport':" << test);
    }

    for (const auto& test : switchportPortSecurityTestDataOk) {
      BOOST_TEST(nmdp::test(test, tp.switchportPortSecurity, blank),
                 "Parse rule 'switchportPortSecurity':" << test);
    }

    for (const auto& test : switchportVlanTestDataOk) {
      BOOST_TEST(nmdp::test(test, tp.switchportVlan, blank),
                 "Parse rule 'switchportVlan':" << test);
    }
  }

  {
    const auto& parserRule {tp.routerId};

    const std::vector<std::string> routerIdTestDataOk
      {

        "ip router-id 1.2.3.4\n",

        "ipv6 router-id 1111:1111:1111:1111:1111:1111:1111:1111\n",

        "router-id 1.2.3.4\n",

        "router-id 1111:1111:1111:1111:1111:1111:1111:1111\n",

        
      }
      ;

    for (const auto& test : routerIdTestDataOk) {
      BOOST_TEST(nmdp::test(test, parserRule, blank),
                 "Parse rule 'routerId':" << test);
    }
  }

  {
    const auto& parserRule {tp.route};

    const std::vector<std::string> routeTestDataOk
      {

        "ip route vrf tkn1 1.1.0.0 255.255.0.0 1.1.2.2\n",

        "ip route 1.1.0.0 255.255.0.0 tkn1 10\n",

        "ip route 1.1.0.0 255.255.0.0 tkn1 permanent\n",

        "ip route 1.1.0.0 255.255.0.0 vrf tkn1 track bfd 20\n",

        "ip route 1.1.0.0 255.255.0.0 track bfd 20 tag tkn2\n",

        "ip route 1.1.0.0 iface2 255.255.0.0 tag tkn2 name tkn3\n",

        "ip route 1.1.0.0 iface2 9 track bfd 20 tag tkn2 name tkn3\n",

        
      }
      ;

    for (const auto& test : routeTestDataOk) {
      BOOST_TEST(nmdp::test(test, parserRule, blank),
                 "Parse rule 'route':" << test);
    }
  }

  {
    const auto& parserRule {tp.vlan};

    const std::vector<std::string> vlanTestDataOk
      {

        "vlan 1\n",

        "vlan 1,2\n"
        "  name tkn1\n"
        "  ignored tkn2 tkn3 tkn4\n",

        "vlan 1,2-3,4\n"
        "  name tkn1\n"
        "  ignored tkn2 tkn3 tkn4\n"

        
      }
      ;

    for (const auto& test : vlanTestDataOk) {
      BOOST_TEST(nmdp::test(test, parserRule, blank),
                 "Parse rule 'vlan':" << test);
    }
  }

  {
    const auto& accessPolicyRelated {tp.accessPolicyRelated};
    const auto& policyMap {tp.policyMap};
    const auto& classMap {tp.classMap};

    const std::vector<std::string> policyMapTestDataOk =
      {
        "PM\n"
        "  class cls1\n"
      }
      ;

    const std::vector<std::string> classMapTestDataOk =
      {
        "om CM\n"
        "  match access-group name agn1\n"
      }
      ;

    const std::vector<std::string> accessPolicyRelatedTestDataOk =
      {
        "access-group tkn1 tkn2 interface tkn3"
      }
      ;


    for (const auto& test : accessPolicyRelatedTestDataOk) {
      BOOST_TEST(nmdp::test(test, accessPolicyRelated, blank),
                 "Parse rule 'accessPolicyRelated':" << test);
    }

    for (const auto& test : policyMapTestDataOk) {
      BOOST_TEST(nmdp::test(test, policyMap, blank),
                 "Parse rule 'policyMap':" << test);
    }

    for (const auto& test : classMapTestDataOk) {
      BOOST_TEST(nmdp::test(test, classMap, blank),
                 "Parse rule 'classMap':" << test);
    }
  }

}


// BOOST_AUTO_TEST_CASE(testWhole)
// {
//   // TestParser tp;
//   // const auto& parserRule {tp};

//   // std::vector<std::string> testsOk {
//   //   TestParser::wholeOk1(),
//   //   TestParser::wholeOk2()
//   // };
//   // for (const auto& test : testsOk) {
//   //   BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
//   //             "Parse rule 'start': " << test);
//   // }

// }
