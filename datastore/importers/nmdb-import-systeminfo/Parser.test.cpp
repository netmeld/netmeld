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
};

BOOST_AUTO_TEST_CASE(testWhole)
{
  TestParser tp;
  const auto &parserRule {tp};

  std::vector<std::string> testsOk {
    R"STR(polkit-libs                                            0.115-11.el8              x86_64              Libraries for polkit
          geolite2-country                                    20180605-1.el8               noarch              Free IP geolocation country database
          libevent                                               2.1.8-5.el8               x86_64              Abstract asynchronous event notification library
          hwdata                                                 0.314-8.8.el8             noarch              Hardware identification and configuration data
          libsolv                                               0.7.16-2.el8               x86_64              Package dependency solver
          xkeyboard-config                                        2.28-1.el8               noarch              X Keyboard Extension configuration data
          kernel-core                                           4.18.0-305.3.1.el8         x86_64              The Linux kernel
          ncurses-base                                             6.1-7.20180224.el8      noarch              Descriptions of common terminals
          initscripts                                         10.00.15-1.el8               x86_64              Basic support for legacy System V init scripts
          dnf-data                                               4.4.2-11.el8              noarch              Common data and configuration files for DNF
    )STR"
  };

  for (const auto& test : testsOk) {
    BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
              "Parse rule 'start': " << test);
  }
}
