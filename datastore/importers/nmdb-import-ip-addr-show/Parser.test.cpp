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

namespace nmdo = netmeld::datastore::objects;
namespace nmdp = netmeld::datastore::parsers;

using qi::ascii::blank;

class TestParser : public Parser
{
  public:
    using Parser::iface;
    using Parser::inetLine;
    using Parser::ifaceName;
};

BOOST_AUTO_TEST_CASE(testRules)
{
  TestParser tp;

  {
    const auto& parserRule {tp.ifaceName};
    // OK
    std::vector<std::tuple<std::string, std::string>> testsOk {
      // {test, expected}
      {"lo", "lo"},
      {"dummy0", "dummy0"},
      {"special-_.@chars4", "special-_.@chars4"},
    };
    for (const auto& [test, expected] : testsOk) {
      std::string out;
      BOOST_TEST(nmdp::testAttr(test.c_str(), parserRule, out),
                 "Parse rule 'ifaceName': " << test);
      BOOST_TEST(expected == out);
    }
    std::vector<std::string> testsBad {
      // {test}
      {" lo"}, // rule doesn't handle spaces
      {"lo "},
      {"lo:"}, // rule doesn't fully parse, important for iface rule
    };
    for (const auto& test : testsBad) {
      std::string out;
      BOOST_TEST(!nmdp::testAttr(test.c_str(), parserRule, out),
                 "Parse rule 'ifaceName': " << test);
    }
  }

  {
    const auto& parserRule {tp.inetLine};
    // OK
    { // v4
      std::vector<std::string> testsOk {
        // {test}
        {"inet 1.1.1.1/24 brd 2.2.2.2/24 scope global dummy0\n"
         "valid_lft forever preferred_lft forever)\n"},
        {"inet 1.1.1.1/24 scope global dummy0\n"},
        {"inet 1.1.1.1/24 scope host\n"},
      };
      const auto& tip {nmdo::IpAddress("1.1.1.1/24")};
      for (const auto& test : testsOk) {
        nmdo::IpAddress out;
        BOOST_TEST(nmdp::testAttr(test.c_str(), parserRule, out, blank),
                   "Parse rule 'inetLine': " << test);
        BOOST_TEST(tip == out);
      }
    }
    { // v6
      std::vector<std::string> testsOk {
        // {test}
        {"inet6 1::1/64 brd 2::2/64 scope global dummy0\n"
         "valid_lft forever preferred_lft forever)\n"},
        {"inet6 1::1/64 scope global dummy0\n"},
        {"inet6 1::1/64 scope host\n"},
      };
      const auto& tip {nmdo::IpAddress("1::1/64")};
      for (const auto& test : testsOk) {
        nmdo::IpAddress out;
        BOOST_TEST(nmdp::testAttr(test.c_str(), parserRule, out, blank),
                   "Parse rule 'inetLine': " << test);
        BOOST_TEST(tip == out);
      }
    }
  }

  {
    const auto& parserRule {tp.iface};
    // OK
    std::vector<std::string> testsOk {
      {R"(1: eth0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP group default qlen 1000
              link/ether 00:11:22:33:44:55 brd ff:ff:ff:ff:ff:ff
              altname enp2s1
              inet 1.1.1.1/24 scope global ens33
                 valid_lft forever preferred_lft forever
              inet6 fe80::1:1111:1:1/64 scope link 
                 valid_lft forever preferred_lft forever
)"},
			// ex `ip -6 addr show` (i.e., no link line when specify v4/6)
      {R"(1: eth0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 state UP qlen 1000
							inet6 1::2/64 scope link 
								 valid_lft forever preferred_lft forever
)"},
      {R"(255: t0: flags mtu 1 tokens
              link/type 00:11:22:33:44:55 brd ff:ff:ff:ff:ff:ff opt tokens
              altname name1
              altname name2
              inet 1.1.1.1/24 scope host
              inet 2.2.2.2/24 scope host
              inet 3.3.3.3/24 scope host
)"},
      {R"(255: t0: flags mtu 1
              link/type
)"},
      {R"(255: t0: flags mtu 1
              link/type brd ff:ff:ff:ff:ff:ff
)"},
      {R"(255: t0: flags mtu 1
              link/type brd ff:ff:ff:ff:ff:ff opt tokens
)"},
      {R"(255: t0: flags mtu 1
              link/type
              altname name1
)"},
      {R"(255: t0: flags mtu 1
              link/type
              inet 1.1.1.1/24 scope host
)"},
    };
    for (const auto& test : testsOk) {
      nmdo::Interface out;
      BOOST_TEST(nmdp::testAttr(test.c_str(), parserRule, out, blank),
                 "Parse rule 'inetLine': " << test);
      BOOST_TEST(out.isValid());
    }
  }
}
