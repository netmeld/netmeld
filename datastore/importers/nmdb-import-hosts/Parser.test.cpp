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
      using Parser::line;
};

BOOST_AUTO_TEST_CASE(testParts)
{
  TestParser tp;

  { // ubuntuHost
    const auto& parserRule {tp.line};
    std::vector<std::string> testsOk {
      "127.0.0.1 view-localhost\n",
      "# BEGIN hosts added by Another Program\n",
      "143.124.3.216  program.url.com\n",
      "# END hosts added by Another Program\n",
      "127.0.0.1	localhost\n",
      "127.0.1.1	MyComputer\n",
      "\n",
      "# The following lines are desirable for IPv6 capable hosts\n",
      "::1     ip6-localhost ip6-loopback\n",
      "fe00::0 ip6-localnet\n",
      "ff00::0 ip6-mcastprefix\n",
      "ff02::1 ip6-allnodes\n",
      "ff02::2 ip6-allrouters\n"
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
       "# The following lines are desirable for IPv4 capable hosts\n",
       "127.0.0.1       localhost\n",
       "\n",
       "# 127.0.1.1 is often used for the FQDN of the machine\n",
       "127.0.1.1       thishost.mydomain.org  thishost\n",
       "192.168.1.10    foo.mydomain.org       foo\n",
       "192.168.1.13    bar.mydomain.org       bar\n",
       "146.82.138.7    master.debian.org      master\n",
       "209.237.226.90  www.opensource.org\n",
        "\n",
       "# The following lines are desirable for IPv6 capable hosts\n",
       "::1             localhost ip6-localhost ip6-loopback\n",
       "ff02::1         ip6-allnodes\n",
       "ff02::2         ip6-allrouters\n"
  };
  for (const auto& test : testsOk) {
    BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
               "Parse rule 'start': " << test);
  }
}
