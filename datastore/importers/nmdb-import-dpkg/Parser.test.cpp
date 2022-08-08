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
    using Parser::packageLine;
};

BOOST_AUTO_TEST_CASE(testRules)
{
  TestParser tp;

  {
    const auto& parserRule {tp.packageLine};
    // OK 
    std::vector<std::tuple<std::string, std::string>> testsOk {
      // {test, expected}
      {"ii  acl                                              2.3.1-1                              amd64        access control list - utilities", 
        "ii  acl                                              2.3.1-1                              amd64        access control list - utilities"}, // case
      {"ii  adwaita-icon-theme                               42.0-2                               all          default icon theme of GNOME",
        "ii  adwaita-icon-theme                               42.0-2                               all          default icon theme of GNOME"}, //case
      {"pn  alsa-topology-conf                               1.2.5.1-2                            all          ALSA topology configuration files",
        "pn  alsa-topology-conf                               1.2.5.1-2                            all          ALSA topology configuration files"}, // case
      {"ufR  alsa-topology-conf                               1.2.5.1-2                            all          ALSA topology configuration files",
        "ufR  alsa-topology-conf                               1.2.5.1-2                            all          ALSA topology configuration files"}, //case
    };
    for (const auto& [test, expected] : testsOk) {
      std::string out;
      BOOST_TEST(nmdp::testAttr(test.c_str(), parserRule, out),
                 "Parse rule 'packageLine': " << test);
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
                 "Parse rule 'packageLine': " << test);
    }
  }
}
