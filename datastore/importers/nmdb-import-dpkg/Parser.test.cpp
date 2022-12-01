// =============================================================================
// Copyright 2022 National Technology & Engineering Solutions of Sandia, LLC
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
    using Parser::packageLine;
    using Parser::packageState;
};

BOOST_AUTO_TEST_CASE(testPackageLine)
{
  TestParser tp;

  {
    const auto& parserRule {tp.packageState};

    // OK
    std::vector<char> desiredActions  = {'u','i','h','r','p'};
    std::vector<char> packageStatuses = {'n','c','H','U','F','W','t','i'};
    std::vector<char> errorFlags      = {' ','R'};

    std::ostringstream oss;
    std::string test;
    for (const auto i : desiredActions) {
      for (const auto j : packageStatuses) {
        for (const auto k : errorFlags) {
          oss << i << j << k << ' ';
          test = oss.str();
          BOOST_TEST(nmdp::test(test.c_str(), parserRule),
                     "Parse rule 'packageState': " << test);
          oss.str(std::string());
        }
      }
    }

    // BAD
    std::vector<std::string> testsFail {
      "un ",    // less than four valid chars
      " un  ",  // start with space
      "an  ",   // invalid char, start
      "ua  ",   // invalid char, middle
      "una ",   // invalid char, end
    };

    for (const auto& test : testsFail) {
      BOOST_TEST(!nmdp::test(test.c_str(), parserRule),
                 "Parse rule 'packageState': " << test);
    }
  }

  {
    const auto& parserRule {tp.packageLine};

    // OK
    std::vector<std::string> testsOk {
      "ii  pkg        1.0         all     some description\n",
      "ii  pkg-other  1.1.99.0-5  amd64   more des-cript -- ion\n",
      "ii  pkg-o12    1.0+v1      x86_64  description\n",
    };

    for (const auto& test : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
                 "Parse rule 'packageLine': " << test);
    }
  }
}

BOOST_AUTO_TEST_CASE(testWhole)
{
  TestParser tp;
  const auto &parserRule {tp};

  std::vector<std::string> testsOk {
    // dpkg -l | head
    R"STR(Desired=Unknown/Install/Remove/Purge/Hold
| Status=Not/Inst/Conf-files/Unpacked/halF-conf/Half-inst/trig-aWait/Trig-pend
|/ Err?=(none)/Reinst-required (Status,Err: uppercase=bad)
||/ Name                                   Version                          Architecture Description
+++-======================================-================================-============-==================================================================================
ii  adduser                                3.118                            all          add and remove users and groups
ii  adwaita-icon-theme                     3.38.0-1                         all          default icon theme of GNOME
ii  alsa-topology-conf                     1.2.4-1                          all          ALSA topology configuration files
ii  alsa-ucm-conf                          1.2.4-2                          all          ALSA Use Case Manager configuration files
ii  ansible                                2.10.7+merged+base+2.10.8+dfsg-1 all          Configuration management, deployment, and task execution system
    )STR"
  };

  for (const auto& test : testsOk) {
    BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
              "Parse rule 'start': " << test);
  }
}
