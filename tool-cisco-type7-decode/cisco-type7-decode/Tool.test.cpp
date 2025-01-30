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
#include "cisco-type7-decode.cpp"

class TestTool : public Tool
{
  public:
    using Tool::decoded;

    using Tool::fromByte;
    using Tool::getBytes;
    using Tool::decode;
};

BOOST_AUTO_TEST_CASE(testFromByte)
{
  TestTool tt;

  std::string b10Str {"0123456789"};
  BOOST_TEST(01 == tt.fromByte(std::string_view(b10Str).substr(0,2), 10));
  BOOST_TEST(12 == tt.fromByte(std::string_view(b10Str).substr(1,2), 10));
  BOOST_TEST(89 == tt.fromByte(std::string_view(b10Str).substr(8,2), 10));

  std::string b16Str {"0123456789abcdef"};
  BOOST_TEST(0x01 == tt.fromByte(std::string_view(b16Str).substr(0,2), 16));
  BOOST_TEST(0x12 == tt.fromByte(std::string_view(b16Str).substr(1,2), 16));
  BOOST_TEST(0x9a == tt.fromByte(std::string_view(b16Str).substr(9,2), 16));
  BOOST_TEST(0xef == tt.fromByte(std::string_view(b16Str).substr(14,2), 16));

  BOOST_CHECK_THROW( tt.fromByte(std::string_view("ab").substr(0,2), 10)
                   , std::runtime_error
                   );
  BOOST_CHECK_THROW( tt.fromByte(std::string_view("g0").substr(0,2), 16)
                   , std::runtime_error
                   );
}

BOOST_AUTO_TEST_CASE(testGetBytes)
{
  TestTool tt;

  auto result {tt.getBytes("01")};
  BOOST_TEST(1 == result.size());
  BOOST_TEST(1 == result.at(0));

  result  = tt.getBytes("0000FF");
  BOOST_TEST(3 == result.size());
  BOOST_TEST(0 == result.at(0));
  BOOST_TEST(0x00 == result.at(1));
  BOOST_TEST(0xff == result.at(2));


  BOOST_CHECK_THROW(tt.getBytes("0"), std::runtime_error);
  BOOST_CHECK_THROW(tt.getBytes("012"), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(testDecode)
{
  std::vector<std::pair<std::string, std::string>> testsOk {
      { R"(01)"                     , R"()"   }
    , { R"(021605481811003348)"     , R"(password)"   }
    , { R"(0236051F4F115F33481F48)" , R"(Pa$$w0rd1!)" }
    , { R"(01150F165E1C07032D)"     , R"(firewall)"   }
    };

  for (const auto& [encoded, decoded] : testsOk) {
    TestTool tt;
    tt.decode(encoded);
    BOOST_TEST(decoded == tt.decoded);
  }
}
