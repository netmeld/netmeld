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
  }
}

BOOST_AUTO_TEST_CASE(testhotfixes)
{
  TestParser tp;
  {
    const auto& parserRule {tp.hotfixes};
    std::vector<std::string> testsOk {
      R"STR(Hotfix(s):           10 Hotfix(s) Installed.
      [01]: KBXXXXXX
      [02]: KBXXXXXX
      [03]: KBXXXXXX
      [04]: KBXXXXXX
      [05]: KBXXXXXX
      [06]: KBXXXXXX
      [07]: KBXXXXXX
      [08]: KBXXXXXX
      [09]: KBXXXXXX
      [10]: KBXXXXXX
      )STR",
      //Simulate buffer exhaustion
      R"STR(Hotfix(s):           1 Hotfix(s) Installed.
      [01]: KBXXXXXX
      [02]: KBXXXXXX
      [03]: KBXXXXXX
      [
      )STR"
    };
    for (const auto& test : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
                "Parse rule 'hotfixes': " << test);
    }
  }
  { // Singular Hotfix
    const auto& parserRule {tp.hotfix};
    std::vector<std::string> testsOk {
      R"STR([01]: KBXXXXXX)STR"
    };

    for (const auto& test : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
                "Parse rule 'hotfix': " << test);
    }
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
          Registered Organization: 
          Product ID:          XXXXX-XXXXX-XXXXX-XXXXX
          Original Install Date: 01/01/2022, 12:00:00 AM
          System Boot Time:    11/22/2023, 9:00:00 AM
          System Manufacturer: Your Computer Manufacturer
          System Model:        Your Computer Model
          System Type:         x64-based PC
          Processor(s):        1 Processor(s) Installed.
                                [01]: Intel64 Family 6 Model 142 Stepping 9 GenuineIntel ~2.30 GHz
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
                                [01]: KBXXXXXX
                                [02]: KBXXXXXX
                                [03]: KBXXXXXX
                                [04]: KBXXXXXX
                                [05]: KBXXXXXX
                                [06]: KBXXXXXX
                                [07]: KBXXXXXX
                                [08]: KBXXXXXX
                                [09]: KBXXXXXX
                                [10]: KBXXXXXX
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
  }
}
