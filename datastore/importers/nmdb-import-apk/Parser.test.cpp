// =============================================================================
// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
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

BOOST_AUTO_TEST_CASE(testPackageLine)
{
  TestParser tp;

  {
    const auto& parserRule {tp.packageLine};

    // OK
    std::vector<std::string> testsOk {
      "mesa-gl-22.2.5-r1 x86_64 {mesa} (MIT SGI-B-2.0 BSL-1.0) [installed]\nMesa libGL runtime libraries\n",
      "mesa-gl-22.2.5-r1 x86_64 {mesa} (MIT SGI-B-2.0 BSL-1.0) [installed]\nMesa libGL runtime libraries\n",
      "mesa-gl-22.2.5-r1 x86_64 {mesa} (MIT SGI-B-2.0 BSL-1.0) [installed]\nMesa libGL runtime libraries\n",
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
    R"STR(mesa-gl-22.2.5-r1 x86_64 {mesa} (MIT SGI-B-2.0 BSL-1.0) [installed]
  Mesa libGL runtime libraries

xfce4-appfinder-4.16.1-r1 x86_64 {xfce4-appfinder} (GPL-2.0-or-later) [installed]
  Xfce application finder

libxmu-1.1.4-r0 x86_64 {libxmu} (MIT) [installed]
  X11 miscellaneous micro-utility library

pkgconf-1.9.3-r0 x86_64 {pkgconf} (ISC) [installed]
  development framework configuration tools
    )STR"
  };

  for (const auto& test : testsOk) {
    BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
              "Parse rule 'start': " << test);
  }
}
