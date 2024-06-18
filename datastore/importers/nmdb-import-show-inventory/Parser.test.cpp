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
    using Parser::start;
    using Parser::deviceInfo;
};

BOOST_AUTO_TEST_CASE(testDeviceInfo)
{
  TestParser tp;

  {
    const auto& parserRule {tp.deviceInfo};

    const std::string test {
        R"(NAME: "abc def", DESCR: "abc def"
           PID: abc-123           , VID: v123 , SN: a1234567890)"
      };

    Result out;
    BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
              , "Parser rule 'deviceInfo': " << test
              );
    BOOST_TEST_REQUIRE(1 == out.size());
    const auto& tStr1 {out[0].toDebugString()};
    nmdp::testInString(tStr1, "vendor: cisco,");
    nmdp::testInString(tStr1, "type: abc def,");
    nmdp::testInString(tStr1, "desc: abc def]");
    nmdp::testInString(tStr1, "model: ABC-123,");
    nmdp::testInString(tStr1, "rev: V123,");
    nmdp::testInString(tStr1, "sn: A1234567890,");
  }
}

BOOST_AUTO_TEST_CASE(testWhole)
{
  TestParser tp;
  const auto& parserRule {tp.start};
  
  {
    const std::string test {R"(
            NAME: "abc def", DESCR: "abc def"
            PID: abc-123           , VID: v123 , SN: a1234567890

            NAME: "ghi 123", DESCR: "ghi"
            PID: ghi-12345678901234, VID: v1234, SN: b1234567890
            
            NAME: "abc def", DESCR: "abc def"
            PID: abc-123           , VID: v123 , SN: c1234567890
         )"
      };

    Result out;
    BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
              , "Parser rule 'start': " << test
              );
    BOOST_TEST_REQUIRE(3 == out.size());
  }
  {
    const std::vector<std::string> testsOk {
          // no newline before or after
          {R"(NAME: "abc def", DESCR: "abc def"
              PID: abc-123           , VID: v123 , SN: a1234567890)"
          }
          // garbage prior, newline separated
        , {R"(junk data
              NAME: "abc def", DESCR: "abc def"
              PID: abc-123           , VID: v123 , SN: a1234567890)"
          }
          // garbage prior, inline; newline then garbage after
        , {R"(junk dataNAME: "abc def", DESCR: "abc def"
              PID: abc-123           , VID: v123 , SN: a1234567890
              junk data
           )"
          }
          // garbage prior, inline; newline after
        , {R"(
              junk data!NAME: "abc def", DESCR: "abc def"
              PID: abc-123           , VID: v123 , SN: a1234567890
           )"
          }
      };

    for (const auto test : testsOk) {
      Result out;
      BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
                , "Parser rule 'start': " << test
                );
      BOOST_TEST_REQUIRE(1 == out.size());
    }
  }
}
