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
    using Parser::systemInformationStart;
    using Parser::systemFanModuleStart;
    using Parser::systemPowerSupplyStart;
    using Parser::systemPortStart;
    using Parser::systemTransceiverStart;
    using Parser::systemStorageStart;

    static constexpr std::string sysInfoOk1() {
      return
        "System information\n"
          "HW Version  \n"
          "----------- \n"
          "12.34       \n"
        "\n"
          "Serial Number\n"
          "--------------\n"
          "SNUM-123456\n"
        "\n"
          "Model                    \n"
          "------------------------ \n"
          "MODEL-1234               \n"
        "\n"
          "Description                \n"
          "-------------------------- \n"
          "This is MODEL-1234\n"
        "\n"
          "Epoch \n"
          "----- \n"
          "01.00 \n"
        "\n"
          "Mfg Date\n"
          "---------- \n"
          "2024-04-01 \n"
        "\n"
      ;
    }

    static constexpr std::string sysInfoOk2() {
      return
        "System information\n"
          "Model                    Description                        \n"
          "------------------------ -----------------------------------\n"
          "MODEL-1234               This is MODEL-1234\n"
        "\n"
          "HW Version  Serial Number  Mfg Date   Epoch \n"
          "----------- -------------- ---------- ----- \n"
          "12.34       SNUM-123456    2024-04-01 01.00 \n"
        "\n"
      ;
    }

    static constexpr std::string sysPowerOk1() {
      return
        "System has 2 power supply slots\n"
          "Slot \n"
          "----  \n"
          "1    \n"
          "2   \n"
        "\n"
          "Model           \n"
          "----------------\n"
          "POW-SUP-MOD       \n"
          "POW-SUP-MOD      \n"
        "\n"
          "Serial Number    \n"
          "---------------- \n"
          "SNUM-SUP-12      \n"
          "SNUM-SUP-34     \n"
        "\n"
      ;
    }

    static constexpr std::string sysPowerOk2() {
      return
        "System has 2 power supply slots\n"
          "Slot Model            Serial Number    \n"
          "---- ---------------- ---------------- \n"
          "1    POW-SUP-MOD      SNUM-SUP-12      \n"
          "2    POW-SUP-MOD      SNUM-SUP-34      \n"
        "\n"
      ;
    }

    static constexpr std::string sysFanOk1() {
      return
        "System has 4 fan modules\n"
          "Model            Serial Number    \n"
          "---------------- ---------------- \n"
          "FAN-MOD-123      N/A              \n"
          "FAN-MOD-123      N/A              \n"
          "FAN-MOD-123      N/A              \n"
          "FAN-MOD-123      N/A              \n"
        "\n"
          "Module  Number of Fans  \n"
          "------- ---------------\n"
          "1       1              \n"
          "2       1               \n"
          "3       1               \n"
          "4       1                \n"
        "\n"
      ;
    }

    static constexpr std::string sysFanOk2() {
      return
        "System has 4 fan modules\n"
          "Module  Number of Fans  Model            Serial Number    \n"
          "------- --------------- ---------------- ---------------- \n"
          "1       1               FAN-MOD-123      N/A              \n"
          "2       1               FAN-MOD-123      N/A              \n"
          "3       1               FAN-MOD-123      N/A              \n"
          "4       1               FAN-MOD-123      N/A              \n"
        "\n"
      ;
    }

    static constexpr std::string sysPortOk1() {
      return
        "System has 10 ports\n"
          "Type               \n"
          "------------------ \n"
          "Management         \n"
          "Switched           \n"
        "\n"
          "Count\n"
          "----\n"
          "1   \n"
          "9   \n"
        "\n"
      ;
    }

    static constexpr std::string sysPortOk2() {
      return
        "System has 10 ports\n"
          "Type               Count\n"
          "------------------ ----\n"
          "Management         1   \n"
          "Switched           9   \n"
        "\n"
      ;
    }

    static constexpr std::string sysTransceiverOk1()  {
      return
        "System has 5 switched transceiver slots\n"
          "Port Manufacturer     Model            Serial Number    Rev \n"
          "---- ---------------- ---------------- ---------------- ----\n"
          "1    Not Present                                            \n"
          "2    Manufacturer-Co  TRA-MOD-1        SNUM-TRA-123     1   \n"
          "3    Manufacturer-Co  TRA-MOD-3        SNUM-TRA-234     2   \n"
          "4    Manufacturer-Co  TRA-MOD-2        SNUM-TRA-345     3   \n"
          "5    Manufacturer-Co  TRA-MOD-5        SNUM-TRA-456     4   \n"
        "\n"
      ;
    }

    static constexpr std::string sysStorageOk1() {
      return
        "System has 3 storage devices\n"
          "Size (GB) \n"
          "--------- \n"
          "30        \n"
          "60        \n"
          "120       \n"
        "\n"
          "Mount      Type Rev      \n"
          "---------- ---- -------- \n"
          "/mnt/flash eMMC 0.0      \n"
          "/mnt/flash eMMC 0.5      \n"
          "/mnt/drive SSD  1.1      \n"
        "\n"
          "Model               Serial Number       \n"
          "------------------  -------------------- \n"
          "StorCo STO-MOD-A    FLA-1234              \n"
          "StorCo STO-MOD-A2   FLA-5678               \n"
          "SSD Co STO-MOD-B    SSD-1234567890       \n"
        "\n"
      ;
    }

    static constexpr std::string sysStorageOk2() {
      return
        "System has 2 storage devices\n"
          "Mount      Type Model              Serial Number        Rev      \n"
          "---------- ---- -----------------  -------------------- -------- \n"
          "/mnt/flash eMMC StorCo STO-MOD-A   FLA-1234             0.0      \n"
          "/mnt/drive SSD  SSD Co STO-MOD-B   SSD-1234567890       1.1      \n"
        "\n"
          "Size (GB) \n"
          "--------- \n"
          "30        \n"
          "400       \n"
        "\n"
      ;
    }

    static constexpr std::string wholeOk1() {
      return
        "System information\n"
          "Model                    \n"
          "------------------------ \n"
          "MODEL-1234        \n"
        "\n"
          "Description                                                \n"
          "---------------------------------------------------------- \n"
          "This is MODEL-1234\n"
        "\n"
          "HW Version  Serial Number  Mfg Date   Epoch \n"
          "----------- -------------- ---------- ----- \n"
          "12.34       SNUM-123456    2024-04-01 01.00 \n"
        "\n"
        "System has 2 power supply slots\n"
          "Slot Model            Serial Number    \n"
          "---- ---------------- ---------------- \n"
          "1    POW-SUP-MOD      SNUM-SUP-12      \n"
          "2    POW-SUP-MOD      SNUM-SUP-34      \n"
        "\n"
        "System has 4 fan modules\n"
          "Module  Number of Fans  Model            Serial Number    \n"
          "------- --------------- ---------------- ---------------- \n"
          "1       1               FAN-MOD-123      N/A              \n"
          "2       1               FAN-MOD-123      N/A              \n"
          "3       1               FAN-MOD-123      N/A              \n"
          "4       1               FAN-MOD-123      N/A              \n"
        "\n"
        "System has 10 ports\n"
          "Type               Count\n"
          "------------------ ----\n"
          "Management         1   \n"
          "Switched           9   \n"
        "\n"
        "System has 5 switched transceiver slots\n"
          "Port Manufacturer     Model            Serial Number    Rev \n"
          "---- ---------------- ---------------- ---------------- ----\n"
          "1    Not Present                                            \n"
          "2    Manufacturer-Co  TRA-MOD-1        SNUM-TRA-123     1   \n"
          "3    Manufacturer-Co  TRA-MOD-3        SNUM-TRA-234     2   \n"
          "4    Manufacturer-Co  TRA-MOD-2        SNUM-TRA-345     3   \n"
          "5    Manufacturer-Co  TRA-MOD-5        SNUM-TRA-456     4   \n"
        "\n"
        "System has 2 storage devices\n"
          "Mount      Type Model              Serial Number        Rev      \n"
          "---------- ---- -----------------  -------------------- -------- \n"
          "/mnt/flash eMMC StorCo STO-MOD-A   FLA-1234             0.0      \n"
          "/mnt/drive SSD  SSD Co STO-MOD-B   SSD-1234567890       1.1      \n"
        "\n"
          "Size (GB) \n"
          "--------- \n"
          "30        \n"
          "400       \n"
        "\n"
      ;
    }

};

BOOST_AUTO_TEST_CASE(testParts)
{
  // using Parser::systemInformationStart;
  // using Parser::systemFanModuleStart;
  // using Parser::systemPowerSupplyStart;
  // using Parser::systemPortStart;
  // using Parser::systemTransceiverStart;
  // using Parser::systemStorageStart;
  TestParser tp;

  {
    const auto& parserRule {tp.systemInformationStart};
    std::vector<std::string> testsOk {
      tp.sysInfoOk1(),
      tp.sysInfoOk2()
    };
    for (const auto& test : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
                 "Parse rule 'systemInformationStart':" << test);
    }
  }

  {
    const auto& parserRule {tp.systemPowerSupplyStart};
    std::vector<std::string> testsOk {
      tp.sysPowerOk1(),
      tp.sysPowerOk2()
    };
    for (const auto& test : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
                 "Parse rule 'systemPowerSupplyStart':" << test);
    }
  }

  {
    const auto& parserRule {tp.systemFanModuleStart};
    std::vector<std::string> testsOk {
      tp.sysFanOk1(),
      tp.sysFanOk2()
    };
    for (const auto& test : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
                 "Parse rule 'systemFanModuleStart':" << test);
    }
  }

  {
    const auto& parserRule {tp.systemPortStart};
    std::vector<std::string> testsOk {
      tp.sysPortOk1(),
      tp.sysPortOk2()
    };
    for (const auto& test : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
                 "Parse rule 'systemPortStart':" << test);
    }
  }

  {
    const auto& parserRule {tp.systemTransceiverStart};
    std::vector<std::string> testsOk {
      tp.sysTransceiverOk1()
    };
    for (const auto& test : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
                 "Parse rule 'systemTransceiverStart':" << test);
    }
  }

  {
    const auto& parserRule {tp.systemStorageStart};
    std::vector<std::string> testsOk {
      tp.sysStorageOk1(),
      tp.sysStorageOk2()
    };
    for (const auto& test : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
                 "Parse rule 'systemStorageStart':" << test);
    }
  }
}


BOOST_AUTO_TEST_CASE(testWhole)
{
  TestParser tp;
  const auto& parserRule {tp};

  std::vector<std::string> testsOk {
    TestParser::wholeOk1()
  };
  for (const auto& test : testsOk) {
    BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
              "Parse rule 'start': " << test);
  }

}
