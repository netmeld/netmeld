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
    using Parser::interface;
    using Parser::routerId;
    using Parser::route;
    using Parser::vlan;
    using Parser::accessPolicyRelated;

    static constexpr std::array<const char*, 2> getNoCDPTestData()
    {
      return {
        "no cdp run\n",
        
        "no cdp enable\n"
      }
      ;
    }


    static constexpr std::array<const char*, 3> getVersionPIXASATestData()
    {
      return {
        "PIX Version\n",
      
        "ASA Version 1\n",
      
        "ASA Version 1 a\n"
      }
      ;
    }

    static constexpr std::array<const char*, 2> getDeviceAAATestData()
    {
      return {
        "aaa tkn1\n",

        "aaa tkn1 tkn2\n"
      }
      ;
    }


    static constexpr std::array<const char*, 15> getGlobalServicesTestData()
    {
      return {
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
    }

    static constexpr std::array<const char*, 9> getSpanningTreeInitialTestData()
    {
      return {
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
    }

    static constexpr std::array<const char*, 6> getDomainDataTestData()
    {
      return {
        "switchname sname1.dm\n",
        
        "hostname sname1.dm\n",
        
        "ip domain-name sname1.dm\n",
        
        "ip dns domain-name sname1.dm\n",
        
        "ip dns domain-name vrf vrf1 sname1.dm\n",
        
        "domain-name sname1.dm\n"
      }
      ;
    }

    static constexpr std::array<const char*, 2> getVRFInstanceTestData()
    {
      return {
        "vrf definition tkn1\n"
        "    description this is a description\n",

        "vrf definition tkn1\n"
        "    ignorable this is something that will be ignored\n"
        "    description this is a description\n"
        "    anotherignorable this is something that will be ignored\n"
      };
    }

    static constexpr std::array<const char*, 1> getInterfaceTestData()
    {
      return {
        "interface iface\n"
        "  description 123/ABC 01234-56\n"
        "  no ip address\n"
        "  shutdown\n"
        "  switchport port-security mac-address sticky\n"
        "  cdp enable\n",

        "interface iface tkn1\n"
        "  description 123/ABC 01234-56\n"
        "  ip address 1.2.3.4\n"
        "  vrrp ip 4.3.2.1\n"
        "  switchport port-security mac-address sticky\n"
      };
    }

    // static constexpr std::string wholeOk1() {
    //   return
    //     sysInfoOk1() + sysStorageOk1() +
    //     sysFanOk1() + sysPortOk1() +
    //     sysTransceiverOk1() + sysPowerOk1()
    //   ;
    // }

    // static constexpr std::string wholeOk2() {
    //   return
    //     sysInfoOk2() + sysStorageOk2() +
    //     sysFanOk2() + sysPortOk2() +
    //     sysTransceiverOk1() + sysPowerOk2()
    //   ;
    // }

};

BOOST_AUTO_TEST_CASE(testParts)
{
  TestParser tp;

  {
    const auto& parserRule {tp.nocdp};
    for (const auto& test : tp.getNoCDPTestData()) {
      BOOST_TEST(nmdp::test(test, parserRule, blank),
                 "Parse rule 'nocdp':" << test);
    }
  }

  {
    const auto& parserRule {tp.versionPIXASA};
    for (const auto& test : tp.getVersionPIXASATestData()) {
      BOOST_TEST(nmdp::test(test, parserRule, blank),
                 "Parse rule 'versionPIXASA':" << test);
    }
  }

  {
    const auto& parserRule {tp.deviceAAA};
    for (const auto& test : tp.getDeviceAAATestData()) {
      BOOST_TEST(nmdp::test(test, parserRule, blank),
                 "Parse rule 'deviceAAA':" << test);
    }
  }

  {
    const auto& parserRule {tp.globalServices};
    for (const auto& test : tp.getGlobalServicesTestData()) {
      BOOST_TEST(nmdp::test(test, parserRule, blank),
                 "Parse rule 'globalServices':" << test);
    }
  }

  {
    const auto& parserRule {tp.spanningTreeInitial};
    for (const auto& test : tp.getSpanningTreeInitialTestData()) {
      BOOST_TEST(nmdp::test(test, parserRule, blank),
                 "Parse rule 'spanningTreeInitial':" << test);
    }
  }

  {
    const auto& parserRule {tp.domainData};
    for (const auto& test : tp.getDomainDataTestData()) {
      BOOST_TEST(nmdp::test(test, parserRule, blank),
                 "Parse rule 'domainData':" << test);
    }
  }

  {
    const auto& parserRule {tp.vrfInstance};
    for (const auto& test : tp.getVRFInstanceTestData()) {
      BOOST_TEST(nmdp::test(test, parserRule, blank),
                 "Parse rule 'vrfInstance':" << test);
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
