// =============================================================================
// Copyright 2020 National Technology & Engineering Solutions of Sandia, LLC
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

#include <netmeld/datastore/objects/PortRange.hpp>

namespace nmdo = netmeld::datastore::objects;


BOOST_AUTO_TEST_CASE(testConstructorsAndToStrings)
{
  // Constructors with numeric arguments:

  {
    nmdo::PortRange portRange(0);  // Min port number

    BOOST_CHECK_EQUAL(0, std::get<0>(portRange));
    BOOST_CHECK_EQUAL(0, std::get<1>(portRange));

    BOOST_CHECK_EQUAL("[0,0]", portRange.toString());
  }

  {
    nmdo::PortRange portRange(65535);  // Max port number

    BOOST_CHECK_EQUAL(65535, std::get<0>(portRange));
    BOOST_CHECK_EQUAL(65535, std::get<1>(portRange));

    BOOST_CHECK_EQUAL("[65535,65535]", portRange.toString());
  }

  {
    nmdo::PortRange portRange(22);

    BOOST_CHECK_EQUAL(22, std::get<0>(portRange));
    BOOST_CHECK_EQUAL(22, std::get<1>(portRange));

    BOOST_CHECK_EQUAL("[22,22]", portRange.toString());
  }

  {
    nmdo::PortRange portRange(0, 65535);  // Full port range

    BOOST_CHECK_EQUAL(    0, std::get<0>(portRange));
    BOOST_CHECK_EQUAL(65535, std::get<1>(portRange));

    BOOST_CHECK_EQUAL("[0,65535]", portRange.toString());
  }

  {
    nmdo::PortRange portRange(8000, 9999);  // Small port range

    BOOST_CHECK_EQUAL(8000, std::get<0>(portRange));
    BOOST_CHECK_EQUAL(9999, std::get<1>(portRange));

    BOOST_CHECK_EQUAL("[8000,9999]", portRange.toString());
  }

  // Constructors with string arguments (including variations in whitespace):

  {
    nmdo::PortRange portRange("[0,65535]");

    BOOST_CHECK_EQUAL(    0, std::get<0>(portRange));
    BOOST_CHECK_EQUAL(65535, std::get<1>(portRange));

    BOOST_CHECK_EQUAL("[0,65535]", portRange.toString());
  }

  {
    nmdo::PortRange portRange("[0, 65535)");

    BOOST_CHECK_EQUAL(    0, std::get<0>(portRange));
    BOOST_CHECK_EQUAL(65534, std::get<1>(portRange));

    BOOST_CHECK_EQUAL("[0,65534]", portRange.toString());
  }

  {
    nmdo::PortRange portRange("( 0,65535 ]");

    BOOST_CHECK_EQUAL(    1, std::get<0>(portRange));
    BOOST_CHECK_EQUAL(65535, std::get<1>(portRange));

    BOOST_CHECK_EQUAL("[1,65535]", portRange.toString());
  }

  {
    nmdo::PortRange portRange("( 0, 65535 )");

    BOOST_CHECK_EQUAL(    1, std::get<0>(portRange));
    BOOST_CHECK_EQUAL(65534, std::get<1>(portRange));

    BOOST_CHECK_EQUAL("[1,65534]", portRange.toString());
  }

  {
    nmdo::PortRange portRange("1024-65535");

    BOOST_CHECK_EQUAL( 1024, std::get<0>(portRange));
    BOOST_CHECK_EQUAL(65535, std::get<1>(portRange));

    BOOST_CHECK_EQUAL("[1024,65535]", portRange.toString());
  }

  {
    nmdo::PortRange portRange("1024 -- 65535");

    BOOST_CHECK_EQUAL( 1024, std::get<0>(portRange));
    BOOST_CHECK_EQUAL(65535, std::get<1>(portRange));

    BOOST_CHECK_EQUAL("[1024,65535]", portRange.toString());
  }

  {
    nmdo::PortRange portRange("443");

    BOOST_CHECK_EQUAL(443, std::get<0>(portRange));
    BOOST_CHECK_EQUAL(443, std::get<1>(portRange));

    BOOST_CHECK_EQUAL("[443,443]", portRange.toString());
  }
}
