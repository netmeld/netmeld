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

#include <netmeld/core/utils/StringUtilities.hpp>

namespace nmcu = netmeld::core::utils;

BOOST_AUTO_TEST_CASE(testToLower)
{
  std::map<std::string, std::string> tests {
      {"A", "a"}
    , {"Abc", "abc"}
    , {"AbC", "abc"}
    , {"123", "123"}
    , {" aBc DeF ", " abc def "}
    , {"", ""}
    };

  for (const auto& [key, value] : tests) {
    BOOST_TEST(value == nmcu::toLower(key));
  }
}

BOOST_AUTO_TEST_CASE(testToUpper)
{
  std::map<std::string, std::string> tests {
      {"A", "A"}
    , {"Abc", "ABC"}
    , {"AbC", "ABC"}
    , {"123", "123"}
    , {" aBc DeF ", " ABC DEF "}
    , {"", ""}
    };

  for (const auto& [key, value] : tests) {
    BOOST_TEST(value == nmcu::toUpper(key));
  }
}

BOOST_AUTO_TEST_CASE(testTrim)
{
  std::map<std::string, std::string> tests {
      {"a", "a"}
    , {" B ", "B"}
    , {" c", "c"}
    , {"D ", "D"}
    , {"  e     ", "e"}
    , {" aBc DeF ", "aBc DeF"}
    , {"       ", ""}
    };

  for (const auto& [key, value] : tests) {
    BOOST_TEST(value == nmcu::trim(key));
  }
}

BOOST_AUTO_TEST_CASE(testGetSrvcString)
{
  BOOST_TEST("a:b:c" == nmcu::getSrvcString("a", "b", "c"));
  BOOST_TEST("1:2:3" == nmcu::getSrvcString("1", "2", "3"));
}

BOOST_AUTO_TEST_CASE(testExpandCiscoIfaceName)
{
  const std::vector<std::tuple<std::string, std::string>> tests {
    // replace
      {"et0", "ethernet0"}
    , {"fa 1", "fastethernet 1"}
    , {"fi2/3", "fivegigabitethernet2/3"}
    , {"fo 4", "fortygigabitethernet 4"}
    , {"Gi5", "gigabitethernet5"}
    , {"lO6", "loopback6"}
    , {"PO7", "port-channel7"}
    , {"se8", "serial8"}
    , {"te9", "tengigabitethernet9"}
    , {"tu10", "tunnel10"}
    , {"tw11", "twogigabitethernet11"}
    , {"twe12", "twentyfivegigabitethernet12"}
    , {"vl13", "vlan13"}
    // no-replace
    , {"ethernet14", "ethernet14"}
    , {"other", "other"}
    // non-sensical, but the logic does it
    , {"tuple", "tunnelple"}
    };

  for (const auto& [test, expected] : tests) {
    
    BOOST_TEST(expected == nmcu::expandCiscoIfaceName(test));
  }
}
