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

#include <netmeld/datastore/parsers/ParserTestHelper.hpp>

#include "Parser.hpp"

namespace nmdp = netmeld::datastore::parsers;

using qi::ascii::blank;

class TestParser : public Parser
{
  public:
    using Parser::data;

    using Parser::start;
    using Parser::packageLine;
    using Parser::packageName;
};

BOOST_AUTO_TEST_CASE(testPackageLine)
{
  TestParser tp;
  const auto& parserRule {tp.packageLine};

  std::string test, dbgStr;
  nmdo::Package out;

  test = "PackageName-1version Architecture throwaway data again\nDescription\n";
  BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
            , "Parse rule 'packageLine': " << test
            );
  BOOST_TEST(out.isValid());
  dbgStr = out.toDebugString();
  nmdp::testInString(dbgStr, "state: ,");
  nmdp::testInString(dbgStr, "name: PackageName,");
  nmdp::testInString(dbgStr, "version: 1version,");
  nmdp::testInString(dbgStr, "architecture: Architecture");
  nmdp::testInString(dbgStr, "description: Description");


  test = "mesa-gl-22.2.5-r1 x86_64 {mesa} (MIT SGI-B-2.0 BSL-1.0) [installed]\nMesa libGL runtime libraries\n";
  BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
            , "Parse rule 'packageLine': " << test
            );
  BOOST_TEST(out.isValid());
  dbgStr = out.toDebugString();
  nmdp::testInString(dbgStr, "state: ,");
  nmdp::testInString(dbgStr, "name: mesa-gl,");
  nmdp::testInString(dbgStr, "version: 22.2.5-r1,");
  nmdp::testInString(dbgStr, "architecture: x86_64");
  nmdp::testInString(dbgStr, "description: Mesa libGL runtime libraries");
}

BOOST_AUTO_TEST_CASE(testPackageName)
{
  TestParser tp;
  const auto& parserRule {tp.packageName};

  // OK
  std::vector<std::string> testsOk {
    "name",
    "package-name",
    "package name"
  };

  for (const auto& test : testsOk) {
    std::string out;
    BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out)
              , "Parse rule 'packageName': " << test
              );
    BOOST_TEST(test == out);
  }

  // BAD The actual seperating of the version and package happens at packageLine
  std::vector<std::string> testsFail {
    "",
    "-1",
    "name-of-package-12",
    "packagename-1.0.0+build.123",
    "packagename-1.0-rc.1",
    "packagename-1.0.0"
  };

  for (const auto& test : testsFail) {
    BOOST_TEST( !nmdp::test(test.c_str(), parserRule)
              , "Parse rule 'packageName': " << test
              );
  }
}

BOOST_AUTO_TEST_CASE(testWhole)
{
  TestParser tp;
  const auto &parserRule {tp.start};

  std::vector<std::string> testsOk {
    R"STR(junk line1

      mesa-gl-22.2.5-r1 x86_64 {mesa} (MIT SGI-B-2.0 BSL-1.0) [installed]
           Mesa libGL runtime libraries

          junk line2

          xfce4-appfinder-4.16.1-r1 x86_64 {xfce4-appfinder} (GPL-2.0-or-later) [installed]
            Xfce application finder

          libxmu-1.1.4-r0 x86_64 {libxmu} (MIT) [installed]
            X11 miscellaneous micro-utility library

          pkgconf-1.9.3-r0 x86_64 {pkgconf} (ISC) [installed]
            development framework configuration tools
          junk line3
    )STR"
  };

  Result out;
  for (const auto& test : testsOk) {
    BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
              , "Parse rule 'start': " << test
              );
  }
  BOOST_TEST_REQUIRE(1 == out.size());
  BOOST_TEST_REQUIRE(4 == out[0].packages.size());
}
