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
    using Parser::r;
    using Parser::fromJsonV2;
    using Parser::fromJsonV3;
};

BOOST_AUTO_TEST_CASE(testFromJsonV2)
{
  // NOTE: These are primarily for testing "file" logic, not data logic
  TestParser tp;
  std::string test;
  Result out;

  const auto getResult = [&](const std::string& jsonData) {
      std::istringstream is {jsonData};
      tp.fromJsonV2(is);
      return tp.getData();
    };

  // empty, but should not error
  test = "{}";
  out = getResult(test);
  BOOST_TEST_REQUIRE(0 == out.size());

  test = "{}\n{}\n{}";
  out = getResult(test);
  BOOST_TEST_REQUIRE(0 == out.size());

  // Parsable, but no v2 data
  test = R"({"key1": "value", "key2": 123})";
  out = getResult(test);
  BOOST_TEST_REQUIRE(0 == out.size());

  test = R"({"key1": "v1", "key2": 123}
            {"key3": "v3"})";
  out = getResult(test);
  BOOST_TEST_REQUIRE(0 == out.size());

  // Parsable, some v2 data
  test = R"({"Account Number": "123abc"})";
  out = getResult(test);
  BOOST_TEST_REQUIRE(1 == out.size());
  BOOST_TEST(1 == out[0].v2Data.size());
  BOOST_TEST(0 == out[0].v3Data.size());
  tp.r =  Result();

  test = R"({"Account Number": "123abc"}
            {"Account Number": "123abc"})";
  out = getResult(test);
  BOOST_TEST_REQUIRE(1 == out.size());
  BOOST_TEST(2 == out[0].v2Data.size());
  BOOST_TEST(0 == out[0].v3Data.size());
  tp.r =  Result();
}

BOOST_AUTO_TEST_CASE(testFromJsonV3)
{
  // NOTE: These are primarily for testing "file" logic, not data logic
  TestParser tp;
  std::string test;
  Result out;

  const auto getResult = [&](const std::string& jsonData) {
      std::istringstream is {jsonData};
      tp.fromJsonV3(is);
      return tp.getData();
    };

  // empty, but should not error
  test = "[{}]";
  out = getResult(test);
  BOOST_TEST_REQUIRE(0 == out.size());

  test = "[{},{},{}]";
  out = getResult(test);
  BOOST_TEST_REQUIRE(0 == out.size());

  // Parsable, but no v3 data
  test = R"([{"key1": "value", "key2": 123}])";
  out = getResult(test);
  BOOST_TEST_REQUIRE(0 == out.size());

  test = R"([{"key1": "value", "key2": 123},
             {"key3": "v3"}
            ])";
  out = getResult(test);
  BOOST_TEST_REQUIRE(0 == out.size());

  // Parsable, some v3 data
  test = R"([{"AccountId": "123abc"}])";
  out = getResult(test);
  BOOST_TEST_REQUIRE(1 == out.size());
  BOOST_TEST(0 == out[0].v2Data.size());
  BOOST_TEST(1 == out[0].v3Data.size());
  tp.r =  Result();

  test = R"([{"AccountId": "123abc"},
             {"AccountId": "123abc"}
            ])";
  out = getResult(test);
  BOOST_TEST_REQUIRE(1 == out.size());
  BOOST_TEST(0 == out[0].v2Data.size());
  BOOST_TEST(2 == out[0].v3Data.size());
  tp.r =  Result();
}
