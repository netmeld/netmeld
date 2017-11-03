// =============================================================================
// Copyright 2017 National Technology & Engineering Solutions of Sandia, LLC
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

#include <netmeld/common/cve.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/adapted.hpp>


using std::get;
using std::string;

namespace qi = boost::spirit::qi;


BOOST_AUTO_TEST_CASE(test_cve_ctor_num)
{
  if (true) {
    CVE cve(1999, 1);

    BOOST_CHECK_EQUAL(1999, cve.year());
    BOOST_CHECK_EQUAL(1, cve.number());
  }

  if (true) {
    CVE cve(2010, 1000);

    BOOST_CHECK_EQUAL(2010, cve.year());
    BOOST_CHECK_EQUAL(1000, cve.number());
  }

  if (true) {
    CVE cve(2015, 10000);

    BOOST_CHECK_EQUAL(2015, cve.year());
    BOOST_CHECK_EQUAL(10000, cve.number());
  }
}


BOOST_AUTO_TEST_CASE(test_cve_ctor_str)
{
  // Normal syntax: 4-digit year, 4-digit (zero padded) number.
  if (true) {
    CVE cve("CVE-1999-0001");

    BOOST_CHECK_EQUAL(1999, cve.year());
    BOOST_CHECK_EQUAL(1, cve.number());
  }
  if (true) {
    CVE cve("CVE-1999-9999");

    BOOST_CHECK_EQUAL(1999, cve.year());
    BOOST_CHECK_EQUAL(9999, cve.number());
  }

  // New syntax: 4-digit year, 5-digit number (when needed).
  if (true) {
    CVE cve("CVE-2015-10000");

    BOOST_CHECK_EQUAL(2015, cve.year());
    BOOST_CHECK_EQUAL(10000, cve.number());
  }
  if (true) {
    CVE cve("CVE-2015-99999");

    BOOST_CHECK_EQUAL(2015, cve.year());
    BOOST_CHECK_EQUAL(99999, cve.number());
  }

  // New syntax: 4-digit year, 6-digit number (when needed).
  if (true) {
    CVE cve("CVE-2015-100000");

    BOOST_CHECK_EQUAL(2015, cve.year());
    BOOST_CHECK_EQUAL(100000, cve.number());
  }
  if (true) {
    CVE cve("CVE-2015-999999");

    BOOST_CHECK_EQUAL(2015, cve.year());
    BOOST_CHECK_EQUAL(999999, cve.number());
  }
}


BOOST_AUTO_TEST_CASE(test_cve_str)
{
  // CVE numbers must be zero padded to 4-digits.
  if (true) {
    CVE cve(2015, 1);

    BOOST_CHECK_EQUAL("CVE-2015-0001", cve.str());
  }

  if (true) {
    CVE cve(2015, 9999);

    BOOST_CHECK_EQUAL("CVE-2015-9999", cve.str());
  }

  // CVE numbers can also be longer than 4-digits when needed.
  if (true) {
    CVE cve(2015, 999999);

    BOOST_CHECK_EQUAL("CVE-2015-999999", cve.str());
  }
}
