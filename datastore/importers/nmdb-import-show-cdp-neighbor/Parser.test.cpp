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
    using Parser::hostnameValue;
    using Parser::ipAddressValue;
};
BOOST_AUTO_TEST_CASE(testParts)
{
  TestParser tp;

  {
    const auto& parserRule {tp.hostnameValue};
    std::vector<std::string> testsOk {
      "Device-ID: hostname\n",
      "Device ID: hostname\n",
      "SysName: hostname\n",
      "Device ID: hostname(hostname)\n",
      "Device ID: hostname-hostname-hostname\n",
      "Device ID: hostname.hostname-hostname\n",
      "Device ID:hostname\n",
    };
    for (const auto& test : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
                "Parse rule 'hostnameValue': " << test);
    }
  }

  {
    const auto& parserRule {tp.ipAddressValue};
    std::vector<std::string> testsOk {
      "IP 1.2.3.4\n",
      "IPv4 1.2.3.4\n",
      "IPv6 1234::1234\n",
      "IP address: 1.2.3.4\n",
      "IPv4 address: 1.2.3.4\n",
      "IPv6 address: 1234::1234\n",
      "IP 1.2.3.4 (link-local)\n",
      "IPv4 1.2.3.4 (link-local)\n",
      "IPv6 1234::1234 (link-local)\n",
      "IP address: 1.2.3.4 (link-local)\n",
      "IPv4 address: 1.2.3.4 (link-local)\n",
      "IPv6 address: 1234::1234 (link-local)\n",
    };
    for (const auto& test : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
                "Parse rule 'ipAddressValue': " << test);
    }
  }
}

BOOST_AUTO_TEST_CASE(testWhole)
{
}
