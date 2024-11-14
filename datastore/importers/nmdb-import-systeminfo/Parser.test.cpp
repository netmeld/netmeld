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

class TestParser : public Parser
{
  public:
    using Parser::data;

    using Parser::hotfix;
    using Parser::hotfixes;
    using Parser::networkCard;

    // testWhole exercises/tests the following functionality
    //using Parser::systemInfo;
    //using Parser::networkCards;
    //using Parser::hostName;
    //using Parser::osName;
    //using Parser::osVersion;
    //using Parser::osManufacturer;
    //using Parser::osConfiguration;
    //using Parser::systemManufacturer;
    //using Parser::systemModel;
    //using Parser::systemType;
};

BOOST_AUTO_TEST_CASE(testNetworkCard)
{
  TestParser tp;
  const auto& parserRule {tp.networkCard};

  std::vector<std::string> testsOk {
      R"STR([01]: Interface Type 1
      Connection Name:  Ethernet
      DHCP Enabled:     Yes
      DHCP Server:      1.2.3.4
      IP address(es)
      [01]: 1.2.3.4
      [02]: 1::2
      Status:           Media disconnected
      )STR"
    , R"STR([01]: Interface Type 2
      Connection Name:  1 Wi-Fi Conn
      )STR"
    , R"STR([01]: Interface Type 3
      Connection Name:  Something Else
      DHCP Enabled:     No
      Status:           Media disconnected
      )STR"
    , R"STR([01]: Interface Type 4
      Connection Name:  Bluetooth
      )STR"
    };

  size_t i {0};
  for (const auto& test : testsOk) {
    BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
              , "Parse rule 'networkCard': " << test
              );

    const auto& ifaceMap {tp.data.networkCards};
    BOOST_TEST_REQUIRE(++i == ifaceMap.size());

    const auto& ifName {std::format("Interface Type {}", i)};
    BOOST_TEST_REQUIRE(ifaceMap.contains(ifName));

    const auto& dbgStr {ifaceMap.at(ifName).toDebugString()};
    nmdp::testInString(dbgStr, std::format("description: {},", ifName));

    // InterfaceNetwork object standardizes on lowercase name
    if (1 == i) {
      nmdp::testInString(dbgStr, "name: ethernet");
    } else if (2 == i) {
      nmdp::testInString(dbgStr, "name: 1 wi-fi conn");
    } else if (3 == i) {
      nmdp::testInString(dbgStr, "name: something else");
    } else if (4 == i) {
      nmdp::testInString(dbgStr, "name: bluetooth");
    } else {
      BOOST_TEST(false, std::format("Unhandled name for {}\n", i));
    }

    if (2 == i) {
      nmdp::testInString(dbgStr, "mediaType: wi-fi,");
    } else if (4 == i) {
      nmdp::testInString(dbgStr, "mediaType: bluetooth,");
    } else {
      nmdp::testInString(dbgStr, "mediaType: ethernet,");
    }

    if (1 == i) {
      nmdp::testInString(dbgStr, "ipAddress: 1.2.3.4/32,");
      nmdp::testInString(dbgStr, "ipAddress: 1::2/128,");
    } else {
      nmdp::testInString(dbgStr, "ipAddrs: [],");
    }

    if (1 == i || 3 == i) {
      nmdp::testInString(dbgStr, "isUp: false,");
    } else {
      nmdp::testInString(dbgStr, "isUp: true,");
    }
  }
}

BOOST_AUTO_TEST_CASE(testhotfixes)
{
  TestParser tp;

  { // typical flow
    tp.data.hotfixes.clear();

    const auto& parserRule {tp.hotfixes};
    std::string test {
        R"STR(Hotfix(s):           10 Hotfix(s) Installed.
        [01]: KB000001
        [02]: KB000002
        [03]: KB000003
        [04]: KB000003
        [05]: KB000004
        [06]: KB000005
        [07]: KB000006
        [08]: KB000007
        [09]: KB000008
        [10]: KB000009
        )STR"
      };

    BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
              , "Parse rule 'hotfixes': " << test
              );
    // just quantity, content is later
    BOOST_TEST(10 == tp.data.hotfixes.size());
  }

  { // Test Buffer Exhaustion Parsing
    tp.data.hotfixes.clear();

    const auto& parserRule {tp.hotfixes};
    std::string test {
        R"STR(Hotfix(s):           3 Hotfix(s) Installed.
        [01]: KB000001
        [02]: KB000002
        [03]: KB000003
        [04]: K
        )STR"
      };

    std::vector<std::string> out;
    BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
              , "Parse rule 'hotfixes': " << test
              );
    BOOST_TEST(3 == tp.data.hotfixes.size());
  }

  { // Singular Hotfix
    tp.data.hotfixes.clear();

    const auto& parserRule {tp.hotfix};
    std::string test { R"STR([01]: KB000004)STR" };

    std::string out;
    BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
              , "Parse rule 'hotfix': " << test
              );
    // content check
    BOOST_TEST("KB000004" == tp.data.hotfixes[0]);
  }
}

BOOST_AUTO_TEST_CASE(testWhole)
{
  //nmcu::LoggerSingleton::getInstance().setLevel(nmcu::Severity::ALL);
  TestParser tp;

  const auto& parserRule {tp};

  std::string test {R"(
         Host Name:                 YourComputerName
         OS Name:                   Microsoft Windows 10 Pro
         OS Version:                12.3.4.12345 N/A Build 12345
         OS Manufacturer:           Microsoft Corporation
         OS Configuration:          Standalone Workstation
         OS Build Type:             Multiprocessor Free
         Registered Owner:          Your Name
         Registered Organization:   Your Organization
         Product ID:                12345-12345-12345-12345
         Original Install Date:     01/01/2022, 12:00:00 AM
         System Boot Time:          11/22/2023, 9:00:00 AM
         System Manufacturer:       Your Computer Manufacturer
         System Model:              Your Computer Model
         System Type:               x64-based PC
         Processor(s):              1 Processor(s) Installed.
                                    [01]: Intel64 Family 6 ... ~2312 GHz
         BIOS Version:              BIOS Version
         Windows Directory:         C:\Windows
         System Directory:          C:\Windows\system32
         Boot Device:               \Device\HarddiskVolume1
         System Locale:             en-us;English (United States)
         Input Locale:              en-us;English (United States)
         Time Zone:                 (UTC-08:00) Pacific Time (US & Canada)
         Total Physical Memory:     16,384 MB
         Available Physical Memory: 8,192 MB
         Virtual Memory: Max Size:  32,768 MB
         Virtual Memory: Available: 20,480 MB
         Virtual Memory: In Use:    12,288 MB
         Page File Location(s):     C:\pagefile.sys
         Domain:                    WORKGROUP
         Logon Server:              \\YourLogonServer
         Hotfix(s):                 10 Hotfix(s) Installed.
                                    [01]: KB000001
                                    [02]: KB000002
                                    [03]: KB000003
                                    [04]: KB000004
                                    [05]: KB000005
                                    [06]: KB000006
                                    [07]: KB000007
                                    [08]: KB000008
                                    [09]: KB000009
                                    [10]: KB000010
         Network Card(s):           2 NIC(s) Installed.
                                    [01]: Realtek PCIe GbE Family Controller
                                          Connection Name: Ethernet
                                          DHCP Enabled:    Yes
                                          IP address(es)
                                          [01]: 1.2.3.4
                                          [02]: 1::2
                                    [02]: Intel Wireless-AC 9560 160MHz
                                          Connection Name: Wi-Fi
                                          DHCP Enabled:    Yes
                                          IP address(es)
                                          [01]: 1.2.3.4
                                          [02]: 1::2
         Hyper-V Requirements:      A hypervisor has been detected. ...
      )"
    };

  BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
            , "Parse rule 'start': " << test
            );

  BOOST_TEST(tp.data.devInfo.isValid());
  BOOST_TEST(tp.data.os.isValid());
  BOOST_TEST(10 == tp.data.hotfixes.size());
  BOOST_TEST(2 == tp.data.networkCards.size());

  const auto& devInfo {tp.data.devInfo};
  const auto& devOs   {tp.data.os};

  auto dbgStr {devInfo.toDebugString()};
  // hostName
  nmdp::testInString(dbgStr, "id: yourcomputername,");
  // systemManufacturer
  nmdp::testInString(dbgStr, "vendor: your computer manufacturer,");
  // systemModel
  nmdp::testInString(dbgStr, "model: YOUR COMPUTER MODEL,");
  // systemType
  nmdp::testInString(dbgStr, "type: x64-based pc,");
  // osConfiguration
  nmdp::testInString(dbgStr, "desc: standalone workstation]");

  dbgStr = devOs.toDebugString();
  // osName
  nmdp::testInString(dbgStr, "productName: windows 10 pro,");
  // osVersion
  nmdp::testInString(dbgStr, "productVersion: 12.3.4.12345 n/a build 12345,");
  // osManufacturer
  nmdp::testInString(dbgStr, "vendorName: microsoft,"); 
  // setCpe
  nmdp::testInString( dbgStr
                    , "cpe: cpe:/o:microsoft:windows_10_pro"
                      ":12.3.4.12345_n/a_build_12345,"
                    );
  nmdp::testInString(dbgStr, "accuracy: 1]");
}
