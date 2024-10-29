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

#include <netmeld/datastore/objects/Port.hpp>

namespace nmdo = netmeld::datastore::objects;


class TestPort : public nmdo::Port {
  public:
    using Port::Port;

    using Port::state;
    using Port::reason;
    // has accessor
    //using Port::port;
    //using Port::protocol;
    //using Port::ipAddr;
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  {
    TestPort port;
    std::string ipStr {nmdo::IpAddress().toString()};

    BOOST_TEST(INT_MAX == port.getPort());
    BOOST_TEST(ipStr == port.getIpAddress().toString());
    BOOST_TEST("" == port.getProtocol());
    BOOST_TEST("" == port.state);
    BOOST_TEST("" == port.reason);
  }

  {
    nmdo::IpAddress ipAddr {"10.0.0.1/24"};
    TestPort port {ipAddr};

    BOOST_TEST(INT_MAX == port.getPort());
    BOOST_TEST(ipAddr.toString() == port.getIpAddress().toString());
    BOOST_TEST("" == port.getProtocol());
    BOOST_TEST("" == port.state);
    BOOST_TEST("" == port.reason);
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    TestPort port;
    int p;

    p = 0;
    port.setPort(p);
    BOOST_TEST(p == port.getPort());

    p = -1;
    port.setPort(p);
    BOOST_TEST(p == port.getPort());
  }

  {
    TestPort port;
    std::string p;

    p = "Proto";
    port.setProtocol(p);
    BOOST_TEST("proto" == port.getProtocol());
  }

  {
    TestPort port;
    std::string r;

    r = "Some Reason";
    port.setReason(r);
    BOOST_TEST("some reason" == port.reason);
  }

  {
    TestPort port;
    std::string s;

    s = "Open|Filtered";
    port.setState(s);
    BOOST_TEST("open|filtered" == port.state);
  }
}

BOOST_AUTO_TEST_CASE(testValidity)
{
  {
    TestPort port;
    BOOST_TEST(!port.isValid());
  }

  {
    nmdo::IpAddress ipAddr {"10.0.0.1/24"};
    TestPort port {ipAddr};
    BOOST_TEST(!port.isValid());
  }

  {
    nmdo::IpAddress ipAddr {"10.0.0.1/24"};
    TestPort port {ipAddr};
    port.setProtocol("proto");

    port.setPort(0);
    BOOST_TEST(port.isValid());

    port.setPort(65535);
    BOOST_TEST(port.isValid());

    port.setPort(-1);
    BOOST_TEST(port.isValid());


    port.setPort(-2);
    BOOST_TEST(!port.isValid());

    port.setPort(65536);
    BOOST_TEST(!port.isValid());
  }

  {
    nmdo::IpAddress ipAddr {"10.0.0.1/24"};
    TestPort port {ipAddr};
    port.setPort(0);

    port.setProtocol("proto");
    BOOST_TEST(port.isValid());

    port.setProtocol("");
    BOOST_TEST(!port.isValid());
  }

  {
    TestPort port;
    port.setPort(0);
    port.setProtocol("proto");

    BOOST_TEST(!port.isValid());
  }
}
