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

#include "StreamUtilities.hpp"

namespace nmcu = netmeld::core::utils;

BOOST_AUTO_TEST_CASE(testOstreamVector)
{
  using nmcu::operator<<;

  std::ostringstream oss;

  std::vector<size_t> testEmpty;
  oss.str("");
  oss << testEmpty;
  BOOST_TEST("[]" == oss.str());

  std::vector<size_t> testNum { 1,2,2,4,99 };
  oss.str("");
  oss << testNum;
  BOOST_TEST("[1, 2, 2, 4, 99]" == oss.str());


  std::vector<std::string> testStr { "1","2","2","4","99" };
  oss.str("");
  oss << testStr;
  BOOST_TEST("[1, 2, 2, 4, 99]" == oss.str());
}

BOOST_AUTO_TEST_CASE(testOstreamSet)
{
  using nmcu::operator<<;

  std::ostringstream oss;

  std::set<size_t> testEmpty;
  oss.str("");
  oss << testEmpty;
  BOOST_TEST("[]" == oss.str());

  std::set<size_t> testNum { 1,2,2,4,99 };
  oss.str("");
  oss << testNum;
  BOOST_TEST("[1, 2, 4, 99]" == oss.str());


  std::set<std::string> testStr { "1","2","2","4","99" };
  oss.str("");
  oss << testStr;
  BOOST_TEST("[1, 2, 4, 99]" == oss.str());
}

BOOST_AUTO_TEST_CASE(testOstreamMap)
{
  using nmcu::operator<<;

  std::ostringstream oss;

  std::map<size_t, size_t> testEmpty;
  oss.str("");
  oss << testEmpty;
  BOOST_TEST("[]" == oss.str());

  std::map<size_t, std::string> testNumStr {
      {1,"1"}
    , {2,"2"}
    , {2,"3"} // first wins
    , {4,"4"}
    , {99,"99"}
    };
  oss.str("");
  oss << testNumStr;
  BOOST_TEST("[{1, 1}, {2, 2}, {4, 4}, {99, 99}]" == oss.str());


  std::map<std::string, size_t> testStrNum {
      {"1",1}
    , {"2",2}
    , {"2",3} // first wins
    , {"4",4}
    , {"99",99}
    };
  oss.str("");
  oss << testStrNum;
  BOOST_TEST("[{1, 1}, {2, 2}, {4, 4}, {99, 99}]" == oss.str());
}

BOOST_AUTO_TEST_CASE(testOstreamTup)
{
  using nmcu::operator<<;

  std::ostringstream oss;

  oss.str("");
  oss << std::make_tuple("abc");
  BOOST_TEST("[abc]" == oss.str());

  oss.str("");
  oss << std::make_tuple("abc", 123, 456);
  BOOST_TEST("[abc, 123, 456]" == oss.str());
}

BOOST_AUTO_TEST_CASE(testOstreamAny)
{
  // TODO
}
