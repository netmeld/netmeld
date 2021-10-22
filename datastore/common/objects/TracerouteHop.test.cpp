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
// all copies or substantial hopions of the Software.
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

#include <netmeld/datastore/objects/TracerouteHop.hpp>

namespace nmdo = netmeld::datastore::objects;


class TestTracerouteHop : public nmdo::TracerouteHop {
  public:
    TestTracerouteHop() : TracerouteHop() {};

};

BOOST_AUTO_TEST_CASE(testValidity)
{
  {
    TestTracerouteHop hop;
    BOOST_CHECK(!hop.isValid());
  }

  {
    int n = 1;
    TestTracerouteHop hop;
    hop.hopCount = n;
    BOOST_CHECK(!hop.isValid());
  }

  {
    nmdo::IpAddress origin {"10.0.0.1/24"};
    TestTracerouteHop hop;
    hop.rtrIpAddr = origin;
    BOOST_CHECK(!hop.isValid());
  }

  {
    nmdo::IpAddress destination {"100.0.0.1/24"};
    TestTracerouteHop hop;
    hop.dstIpAddr = destination;
    BOOST_CHECK(!hop.isValid());
  }

  {
    nmdo::IpAddress origin {"10.0.0.1/24"};
    nmdo::IpAddress destination {"100.0.0.1/24"};
    TestTracerouteHop hop;
    hop.rtrIpAddr = origin;
    hop.dstIpAddr = destination;
    BOOST_CHECK(!hop.isValid());
  }

  {
    int n = 1;
    nmdo::IpAddress origin {"10.0.0.1/24"};
    nmdo::IpAddress destination {"100.0.0.1/24"};
    TestTracerouteHop hop;
    hop.rtrIpAddr = origin;
    hop.dstIpAddr = destination;
    hop.hopCount = n;
    BOOST_CHECK(hop.isValid());
  }

}