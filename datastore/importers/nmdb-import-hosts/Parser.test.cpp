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

class TestParser : public Parser {
    public:
      using Parser::line;
};

BOOST_AUTO_TEST_CASE(testParts)
{
  TestParser tp;

  {
    const auto& parserRule {tp.line};
    std::vector<std::string> testsOk {
      // ip hostname
      "127.0.0.1 my-localhost\n",
      // ip hostname alias1 alias2 alias3
      "127.0.0.1 view-localhost localhost ip4-localhost\n",
      // ip hostname comment
      "143.124.3.216 program.url.com # host added by Another Program\n",
      // ip hostname alias1 comment
      "130.140.150.160 myvpn.routing.com myvpn # Added by MyVPN\n",
      // comment
      "# The following lines are desirable for IPv6 capable hosts\n",
      // Blank line
      "\n",
      // ipv6 hostname
      "::1     ip6-localhost ip6-loopback\n",
      "ff00::0 ip6-mcastprefix\n"
    };
    for (const auto& test : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
                 "Parse rule 'hosts': " << test);
    }
  }
}

BOOST_AUTO_TEST_CASE(testWhole)
{
  TestParser tp;

  const auto& parserRule {tp};
  std::vector<std::string> testsOk {
    // Taken from the man page for /etc/hosts (man7.org)
    R"(# The following lines are desirable for IPv4 capable hosts
127.0.0.1       localhost

# 127.0.1.1 is often used for the FQDN of the machine
127.0.1.1       thishost.mydomain.org  thishost
192.168.1.10    foo.mydomain.org       foo
192.168.1.13    bar.mydomain.org       bar
146.82.138.7    master.debian.org      master
209.237.226.90  www.opensource.org

# The following lines are desirable for IPv6 capable hosts
::1             localhost ip6-localhost ip6-loopback
ff02::1         ip6-allnodes
ff02::2         ip6-allrouters
)"
  };
  for (const auto& test : testsOk) {
    BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
               "Parse rule 'start': " << test);
  }
}
