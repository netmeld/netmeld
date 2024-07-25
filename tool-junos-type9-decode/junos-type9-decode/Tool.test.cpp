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

#define UNIT_TESTING
#include "junos-type9-decode.cpp"

class TestTool : public Tool
{
  public:
    using Tool::decoded;

    using Tool::getNext;
    using Tool::gapDecode;
    using Tool::decode;
};

BOOST_AUTO_TEST_CASE(testGetNext)
{
  TestTool tt;

  auto result {tt.getNext("abc123", 3)};
  BOOST_TEST("abc" == std::get<0>(result));
  BOOST_TEST("123" == std::get<1>(result));
  
  result = tt.getNext("abc123", 1);
  BOOST_TEST("a" == std::get<0>(result));
  BOOST_TEST("bc123" == std::get<1>(result));

  BOOST_CHECK_THROW(tt.getNext("" , 0), std::runtime_error);
  BOOST_CHECK_THROW(tt.getNext("a", 2), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(testGapDecode)
{
  TestTool tt;
  std::vector<uint8_t> gaps, moduli;
  
  gaps    = {};
  moduli  = {};
  BOOST_TEST(0x0 == tt.gapDecode(gaps, moduli));

  gaps    = {1};
  moduli  = {1};
  BOOST_TEST(0x1 == tt.gapDecode(gaps, moduli));

  gaps    = {1,2};
  moduli  = {1,2};
  BOOST_TEST(0x5 == tt.gapDecode(gaps, moduli));
  
  gaps    = {1,2,3,4,5};
  moduli  = {5,4,3,2,1};
  BOOST_TEST(0x23 == tt.gapDecode(gaps, moduli));
  
  gaps    = {1};
  moduli  = {127};
  BOOST_TEST(0x7f == tt.gapDecode(gaps, moduli));

  gaps    = {1};
  moduli  = {255};
  BOOST_CHECK_THROW(tt.gapDecode(gaps, moduli), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(testDecode)
{
  std::vector<std::pair<std::string, std::string>> testsOk {
      { R"(2GgZjHkPQ39.PhrvLVb.P5Tz6)"         , R"(password)"   }
    , { R"(/m8k9uBIRSKvWF3K8LX-dk.PfT369ApuO)" , R"(Pa$$w0rd1!)" }
    };

  for (const auto& [encoded, decoded] : testsOk) {
    TestTool tt;
    tt.decode(encoded);
    BOOST_TEST(decoded == tt.decoded);
  }

  std::vector<std::string> testsBad {
      R"(!)"
    , R"($9$)"
    , R"(2GgZjH)"
    };

  for (const auto& test : testsBad) {
    TestTool tt;
    BOOST_CHECK_THROW(tt.decode(test)  , std::runtime_error);
  }
}
