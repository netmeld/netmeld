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

using qi::ascii::blank;

class TestParser : public Parser
{
  public:
    using Parser::comment;
    using Parser::protocol;
    using Parser::portRange;
    using Parser::line;
    using Parser::start;
};

BOOST_AUTO_TEST_CASE(testRuleLine)
{
  TestParser tp;
  const auto& parserRule {tp.line};
  // tests portRange as well
  // tests protocol as well
  // tests comment as well

  std::vector<std::tuple<std::string, std::string, std::string>> testsOk {
      {"T: 123\n"                 , "T"  , "[123,123]"  }
    , {"U: 123-456\n"             , "U"  , "[123,456]"  }
    , {"Y: 123 # with comment\n"  , "Y"  , "[123,123]"  }
    , {"# comment only line\n"    , ""   , "[0,0]"      }
    , {"TUY: 0-65535\n"           , "TUY", "[0,65535]"  }
    };

  for (const auto& [test, protocols, portRange] : testsOk) {
    Data out;
    BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
              , "Parser rule 'line': " << test
              );
    BOOST_TEST(protocols == out.protocols);
    BOOST_TEST(portRange == out.portRange.toString());
  }

  std::vector<std::string> testsBad {
      "T: 123-456"
    , "t: 123-456\n"
    , ": 123-456\n"
    , "T: -456\n"
    , "T: 123-\n"
    , "A: 123-456\n"
    , "T: 1a3-456\n"
    , "T: 123-4a6\n"
    , "T: 456-123\n"
    };

  for (const auto& test : testsBad) {
    Data out;
    BOOST_TEST( !nmdp::testAttr(test.c_str(), parserRule, out, blank)
              , "Parser rule 'line': " << test
              );
  }
}
