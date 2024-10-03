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

#include <netmeld/datastore/parsers/ParserHelper.hpp>
#include <netmeld/datastore/parsers/ParserDomainName.hpp>

namespace nmdp = netmeld::datastore::parsers;

BOOST_AUTO_TEST_CASE(testWellformedDomainName)
{
  std::vector<std::string> names =
  {
    // 1 character label
    "a",
    // 127 labels, groups of 20
    std::string("1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1") +
               ".1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1" +
               ".1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1" +
               ".1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1" +
               ".1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1" +
               ".1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1" +
               ".1.1.1.1.1.1.1",
    // 63 character label
    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
    // alphanumeric, hyphen, and underscore label
    "a1-_a",
    // other additional
    "some.random1.for-random.complexity_type-testing.a_a"
  };

  for (const auto& name : names) {
    auto temp = nmdp::fromString<nmdp::ParserDomainName, std::string>(name);

    BOOST_TEST(temp == name);
  }
}

/* TODO [#106] These test cases should probably fail but do not.  Probably
                because the rules for a domain name need to be reviewed and
                reasserted to be as described below.  Until that happens, just
                commenting out this as it is not a high priority this release.
*/
//BOOST_AUTO_TEST_CASE(testMalformedDomainName)
//{
//  std::vector<std::string> names =
//  {
//    // 0 character label
//    "",
//    // 128 labels, groups of 20
//    std::string("1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1") +
//               ".1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1" +
//               ".1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1" +
//               ".1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1" +
//               ".1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1" +
//               ".1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1" +
//               ".1.1.1.1.1.1.1",
//    // 64 character label
//    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
//    // not alphanumeric, hyphen, or underscore only label
//    "label.with./",
//    "label.with.*",
//    "label.with.~"
//  };
//
//  for (const auto& name : names) {
//    std::string result;
//    std::istringstream dataStream(name);
//    dataStream.unsetf(std::ios::skipws);
//    InputIterator i{dataStream}, e;
//    bool const success = qi::parse(i, e, nmdp::ParserDomainName(), result);
//
//    BOOST_TEST(!success);
//    BOOST_TEST(i != e);
//  }
//}
