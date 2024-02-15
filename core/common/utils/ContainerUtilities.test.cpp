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

#include <netmeld/core/utils/ContainerUtilities.hpp>
#include <vector>
#include <set>

namespace nmcu = netmeld::core::utils;

BOOST_AUTO_TEST_CASE(testToString)
{
  {
    std::set<std::string> test {"a", "b", "c"};

    BOOST_TEST("a b c" == nmcu::toString(test));
    BOOST_TEST("a,b,c" == nmcu::toString(test, ','));
    BOOST_TEST("a, b, c" == nmcu::toString(test, ", "));
  }
  {
    std::vector<std::string> test {"a", "b", "c"};

    BOOST_TEST("a b c" == nmcu::toString(test));
    BOOST_TEST("a,b,c" == nmcu::toString(test, ','));
    BOOST_TEST("a, b, c" == nmcu::toString(test, ", "));
  }
}

BOOST_AUTO_TEST_CASE(testUniquePushBack)
{
  std::vector<std::string> control {"a", "bb", "c"};
  {
    std::vector<std::string> test;
    std::vector<std::string> sink;

    for (const auto& item : {"a", "a", "bb", "c", "bb"}) {
      sink.push_back(item);
      nmcu::pushBackIfUnique(&test, item);
    }
    for (const std::string& item : sink) {
      nmcu::pushBackIfUnique(&test, item);
    }

    BOOST_TEST(control == test);
  }
}
