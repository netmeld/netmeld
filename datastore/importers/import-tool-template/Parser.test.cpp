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

/* Notes:
   - This unit is part of the complilation process to help ensure consistency
     between templates and the actual data.
   - Various data is included and most is commented solely for educational
     purposes.
     - In non-template, remove data as makes sense.

   Guidelines:
   - Unit tests must be created for any rule that changes state of the
     data within the parser.
     - For example, if the parser function adds to, removes from, or
       otherwise manipulates a variable's contents; then that is a state
       change.
   - Unit tests must test the ability to parse input correctly.  Test what
     it must correctly parse and, if known, what it must not parse.
     - Do parse examples must be as exhaustive as possible.
     - Not parse examples can be representative and not exhaustive.
     - For example, if the parse should exactly match three alphanumeric
       characters only; then test to ensure it does that but also test to
       ensure it doesn't match if they are separated by a non-alphanumeric
       character (e.g., space, tab, newline).
   - Unit tests must test if expected state change(s) occur correctly.
     - For example, if the logic returns a parsed, matched string, then
       test the returned is as expected.
     - For example, if the logic alters the contents of an object, then
       test the content change of the object.
*/
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
    using Parser::start;
    using Parser::data;
};

BOOST_AUTO_TEST_CASE(testRuleData)
{
  TestParser tp;
  const auto& parserRule {tp.data};

  std::vector<std::string> testsOk {
      "some text"
    , "more"
    };

  for (const auto& test : testsOk) {
    nmdo::Data out;
    // NOTE: pnx::ref is needed to handle ref binding/semantic action logic
    BOOST_TEST( nmdp::test(test.c_str(), parserRule(pnx::ref(out)), blank)
              , "Rule 'ifaceName': " << test
              );
    //BOOST_TEST(ctrl == out);
  }
}

BOOST_AUTO_TEST_CASE(testGrammar)
{
  TestParser tp;
  const auto& parserRule {tp.start};

  {
    std::string test {
      "Full grammar test"
      };

    Result out;
    BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
              , "Grammar: " << test
              );
    BOOST_TEST_REQUIRE(1 == out.size());
  }
}
