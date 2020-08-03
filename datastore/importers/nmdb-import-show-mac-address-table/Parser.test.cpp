// =============================================================================
// Copyright 2017 National Technology & Engineering Solutions of Sandia, LLC
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
};

BOOST_AUTO_TEST_CASE(testParts)
{
  TestParser tp;

  {
    const auto& parserRule = tp.vlanId;
    std::vector<std::tuple<std::string, unsigned short>> testsOk {
      {"0", 0},
      {"101", 101},
      {"65535", USHRT_MAX},
    };
    for (const auto& test : testsOk) {
      unsigned short out;
      BOOST_TEST(nmdp::testAttr(std::get<0>(test).c_str(), parserRule, out));
      BOOST_TEST(std::get<1>(test) == out);
    }
    std::vector<std::tuple<std::string, unsigned short>> testsBad {
      {" 0", 0},
      {"101 ", 101},
      {"65536", 6553},
    };
    for (const auto& test : testsBad) {
      unsigned short out;
      BOOST_TEST(!nmdp::testAttr(std::get<0>(test).c_str(), parserRule, out));
      BOOST_TEST(std::get<1>(test) == out);
    }
  }

  {
    const auto& parserRule = tp.portName;
    std::vector<std::tuple<std::string, std::string>> testsOk {
      {"0", "0"},
      {"gi10", "gi10"},
      {"GigaEther1/2/3", "GigaEther1/2/3"},
      {"dynamic gi10", "gi10"},
      {"dynamic ip gi10", "gi10"},
    };
    for (const auto& test : testsOk) {
      std::string out;
      BOOST_TEST(nmdp::testAttr(std::get<0>(test).c_str(), parserRule, out));
      BOOST_TEST(std::get<1>(test) == out);
    }
    std::vector<std::tuple<std::string, std::string>> testsOkNotFull {
      {"0 dynamic", "0"},
      {"gi10 dynamic ip", "gi10"},
    };
    for (const auto& test : testsOkNotFull) {
      std::string out;
      BOOST_TEST(nmdp::testAttr(std::get<0>(test).c_str(), parserRule, out, false));
      BOOST_TEST(std::get<1>(test) == out);
    }
  }

  {
    const auto& parserRule = tp.link;
    std::vector<std::string> testsOk {
      // vlan mac port type
      "1      12:34:12:34:12:34  port1  type\n",
      "65535  1234.1234.1234     0      type\n",
      // ? vlan mac port type
      "G 1  1234.1234.1234  port1  type\n",
      "R 1  1234.1234.1234  port1  type\n",
      "O 1  1234.1234.1234  port1  type\n",
      "* 1  1234.1234.1234  port1  type\n",
      "+ 1  1234.1234.1234  port1  type\n",
      // vlan mac type protocols|pv port
      "1  1234.1234.1234  type  protocol          port1\n",
      "1  1234.1234.1234  type  protocols,protos  port1\n",
      "1  1234.1234.1234  type  pv                port1\n",
      // vlan mac type age secure ntfy ports
      "1  1234.1234.1234  type  0    T  T  port1",
      "1  1234.1234.1234  type  -99  F  F  port1",
      "1  1234.1234.1234  type  300  C  C  port1",
      "1  1234.1234.1234  type  0-9  ~  ~  port1",
      // vlan mac type learn age port
      "1  1234.1234.1234  type  Yes  0    port1",
      "1  1234.1234.1234  type  No   -99  port1",
    };
    for (const auto& test : testsOk) {
      nmdo::InterfaceNetwork out;
      BOOST_TEST(nmdp::testAttr(test.c_str(), parserRule, out, blank),
                 "Failed parse for 'link': " << test << '\n');
      //LOG_INFO << "value: " << out << '\n';
    }
    std::vector<std::string> testsBad {
      "A 1 01:23:45:67:89:ab gi1 self\n",
      "All 01:23:45:67:89:ab gi1 self\n",
      "1 g1:23:45:67:89:ab gi1 self\n",
      "1 01:23:45:67:89:ab noport self\n",
    };
    for (const auto& test : testsBad) {
      nmdo::InterfaceNetwork out;
      BOOST_TEST(!nmdp::testAttr(test.c_str(), parserRule, out, blank),
                 "Failed parse for 'link': " << test << '\n');
    }
  }
}


BOOST_AUTO_TEST_CASE(testWhole)
{
  TestParser tp;

  {
    const auto& parserRule = tp;
    std::vector<std::string> testsOk {
      R"STR(
      1      00:11:22:33:44:55  port1 type
      101    00:11:22:33:44:55  type  port1
      65535  00:11:22:33:44:55  type  protocol,proto port1
      )STR",
      R"STR(
      1      00:11:22:33:44:55  port1 type
      101    00:11:22:33:44:55  type  port1
      65535  00:11:22:33:44:55  type  protocol,proto port1)STR",
      R"STR(
      Garbage Text
      ------------
      1      00:11:22:33:44:55  port1 type
      101    00:11:22:33:44:55  type  port1
      65535  00:11:22:33:44:55  type  protocol,proto port1

      ------------
      Garbage Text)STR",
    };
    for (const auto& test : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
                 "Failed parse for 'link': " << test << '\n');

    }
  }
}
