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
    using Parser::start;
    using Parser::hotfix;
    using Parser::hotfixes;
    using Parser::networkCards;
    using Parser::networkCard;
    using Parser::data;
};

BOOST_AUTO_TEST_CASE(testNetworkCard)
{
  TestParser tp;
  {
    const auto& parserRule {tp.networkCard};
    std::vector<std::string> testsOk {
      R"STR([01]: Realtek PCIe GbE Family Controller
      Connection Name: Ethernet
      DHCP Enabled:    Yes
      IP address(es)
      [01]: 192.168.0.2
      [02]: 2001:0db8:85a3:0000:0000:8a2e:0370:7334
      )STR",
      R"STR([05]: Intel(R) Wi-Fi 6 AX200 160MHz
      Connection Name: Wi-Fi
      DHCP Enabled:    Yes
      DHCP Server:     192.168.1.1
      IP address(es)
      [01]: 192.168.1.51
      [02]: fe80::564:ab07:4c1a:6834
      [03]: 2601:8c0:d00:f5b0::1cfe
      )STR"
    };

    std::vector<std::string> testsFail {
      R"STR(Connection Name: Ethernet
      DHCP Enabled:    Yes
      IP address(es)
      [01]: 192.168.0.2
      [02]: 2001:0db8:85a3:0000:0000:8a2e:0370:7334
      )STR", //Mising first string
      R"STR([01]: Realtek PCIe GbE Family Controller
      Connection Name:
      DHCP Enabled:    Yes
      IP address(es)
      [01]: 192.168.0.2
      [02]: 2001:0db8:85a3:0000:0000:8a2e:0370:7334
      )STR", //Missing connection name
      R"STR([01]: Realtek PCIe GbE Family Controller
      Connection Name: Ethernet
      DHCP Enabled:
      IP address(es)
      [01]: 192.168.0.2
      [02]: 2001:0db8:85a3:0000:0000:8a2e:0370:7334
      )STR", //Missing dhcp enabled data
      R"STR([01]: Realtek PCIe GbE Family Controller
      Connection Name: Ethernet
      DHCP Enabled:    Yes
      [01]: 192.168.0.2
      [02]: 2001:0db8:85a3:0000:0000:8a2e:0370:7334
      )STR", //Missing ipaddresses label
      R"STR([01]: Realtek PCIe GbE Family Controller
      Connection Name: Ethernet
      DHCP Enabled:    Yes
      IP address(es)
      )STR" //Missing Ipaddress data
    };
    for (const auto& test : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
                "Parse rule 'networkCard': " << test);
    }
    for (const auto& test: testsFail) {
      BOOST_TEST(!nmdp::test(test.c_str(), parserRule, blank),
                "Parse rule 'networkCard fails': " << test);
    }
    tp.clearNetworkCards();
  }
  { // Test Data
    const auto& parserRule {tp.networkCard};
    std::string out;
    std::string test {
      R"STR([01]: Realtek PCIe GbE Family Controller
      Connection Name: Ethernet
      DHCP Enabled:    Yes
      IP address(es)
      [01]: 192.168.0.2
      [02]: 2001:0db8:85a3:0000:0000:8a2e:0370:7334
      )STR",
    };
    BOOST_TEST(nmdp::testAttr(test.c_str(), parserRule, out, blank),
              "Parse rule 'networkCard': " << test);
    std::string curInterfaceName = "Realtek PCIe GbE Family Controller";
    std::cout << "Interface Name: " << tp.data.network_cards << std::endl;
    BOOST_TEST(tp.data.network_cards["realtek pcie gbe family controller"].isValid());
  }
}

BOOST_AUTO_TEST_CASE(testhotfixes)
{
  TestParser tp;
  {
    const auto& parserRule {tp.hotfixes};
    std::vector<std::string> testsOk {
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
    for (const auto& test : testsOk) {
      std::string out;
      // This test case is special because it is testing the buffer exhaustion
      BOOST_TEST(nmdp::testAttr(test.c_str(), parserRule, out, blank, false),
                "Parse rule 'hotfixes': " << test);
      //Leaving this as a quantity check due to hotfix being the actual data being parsed
      BOOST_TEST(10 == tp.data.hotfixes.size()); 
    }
    tp.clearHotfixes();
  }
  {
    //Test Buffer Exhaustion Parsing
    const auto& parserRule {tp.hotfixes};
    std::string testsOk {
      // Simulate buffer exhaustion
      R"STR(Hotfix(s):           3 Hotfix(s) Installed.
      [01]: KB000001
      [02]: KB000002
      [03]: KB000003
      [04]: K
      )STR"
    };
    std::vector<std::string> out;
    BOOST_TEST(nmdp::testAttr(testsOk.c_str(), parserRule, out, blank, false),
              "Parse rule 'hotfixes': " << testsOk);
    BOOST_TEST(3 == tp.data.hotfixes.size());
    tp.clearHotfixes();
  }
  { // Singular Hotfix
    std::string out;
    const auto& parserRule {tp.hotfix};
    std::string testsOk {
      R"STR([01]: KB000004)STR"
    };
    BOOST_TEST(nmdp::testAttr(testsOk.c_str(), parserRule, out, blank),
              "Parse rule 'hotfix': " << testsOk);
    // Checking what was put into data.hotfixes to what is supposed to be in there         
    BOOST_TEST(tp.data.hotfixes[0] == "KB000004");
    tp.clearHotfixes();
  }
}

BOOST_AUTO_TEST_CASE(testWhole)
{
  TestParser tp;
  const auto& parserRule {tp};
  std::vector<std::string> testsOk {
    R"STR(
          Host Name:           YourComputerName
          OS Name:             Microsoft Windows 10 Pro
          OS Version:          10.0.19042 N/A Build 19042
          OS Manufacturer:     Microsoft Corporation
          OS Configuration:    Standalone Workstation
          OS Build Type:       Multiprocessor Free
          Registered Owner:    Your Name
          Registered Organization: Your Organization
          Product ID:          00329-00000-00003-AA093
          Original Install Date: 01/01/2022, 12:00:00 AM
          System Boot Time:    11/22/2023, 9:00:00 AM
          System Manufacturer: Your Computer Manufacturer
          System Model:        Your Computer Model
          System Type:         x64-based PC
          Processor(s):        1 Processor(s) Installed.
                                [01]: Intel64 Family 6 Model 142 Stepping 9 GenuineIntel ~2312 GHz
          BIOS Version:        Your BIOS Version
          Windows Directory:   C:\Windows
          System Directory:    C:\Windows\system32
          Boot Device:         \Device\HarddiskVolume1
          System Locale:       en-us;English (United States)
          Input Locale:        en-us;English (United States)
          Time Zone:           (UTC-08:00) Pacific Time (US & Canada)
          Total Physical Memory: 16,384 MB
          Available Physical Memory: 8,192 MB
          Virtual Memory: Max Size: 32,768 MB
          Virtual Memory: Available: 20,480 MB
          Virtual Memory: In Use: 12,288 MB
          Page File Location(s): C:\pagefile.sys
          Domain:              WORKGROUP
          Logon Server:        \\YourLogonServer
          Hotfix(s):           10 Hotfix(s) Installed.
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
          Network Card(s):     2 NIC(s) Installed.
                                [01]: Realtek PCIe GbE Family Controller
                                  Connection Name: Ethernet
                                  DHCP Enabled:    Yes
                                  IP address(es)
                                  [01]: 192.168.0.2
                                  [02]: 2001:0db8:85a3:0000:0000:8a2e:0370:7334
                                [02]: Intel Wireless-AC 9560 160MHz
                                  Connection Name: Wi-Fi
                                  DHCP Enabled:    Yes
                                  IP address(es)
                                  [01]: 192.168.0.3
                                  [02]: 2001:0db8:85a3:0000:0000:8a2e:0370:7334
          Hyper-V Requirements:      A hypervisor has been detected. Features required for Hyper-V will not be displayed.

          )STR"
   };

  for (const auto& test : testsOk) {
    BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
              "Parse rule 'start': " << test);
    std::cout << tp.data.devInfo << std::endl;
    BOOST_TEST(tp.data.devInfo.isValid());
    std::cout << tp.data.os << std::endl;
    BOOST_TEST(tp.data.os.isValid());
    std::cout << tp.data.hotfixes << std::endl;
    BOOST_TEST(10 == tp.data.hotfixes.size());
    std::cout << tp.data.network_cards << std::endl;
    BOOST_TEST(2 == tp.data.network_cards.size());
  }
}
