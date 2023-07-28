// =============================================================================
// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
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

#include <netmeld/datastore/objects/Route.hpp>

namespace nmdo = netmeld::datastore::objects;


class TestRoute : public nmdo::Route {
  public:
    TestRoute() : Route() {};

  public:
    std::string getVrfId() const
    { return vrfId; }

    std::string getTableId() const
    { return tableId; }

    nmdo::IpNetwork getDstIpNet() const
    { return dstIpNet; }

    std::string getNextVrfId() const
    { return nextVrfId; }

    std::string getNextTableId() const
    { return nextTableId; }

    nmdo::IpAddress getNextHopIpAddr() const
    { return nextHopIpAddr; }

    std::string getIfaceName() const
    { return ifaceName; }

    std::string getProtocol() const
    { return protocol; }

    size_t getAdminDistance() const
    { return adminDistance; }

    size_t getMetric() const
    { return metric; }
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  nmdo::IpAddress ipAddr;
  {
    TestRoute route;

    BOOST_CHECK_EQUAL(ipAddr, route.getDstIpNet());
    BOOST_CHECK_EQUAL(ipAddr, route.getNextHopIpAddr());
    BOOST_CHECK(route.getIfaceName().empty());
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    TestRoute route;

    route.setVrfId("someVrfId");
    BOOST_CHECK_EQUAL("someVrfId", route.getVrfId());
  }

  {
    TestRoute route;

    route.setNextVrfId("someVrfId");
    BOOST_CHECK_EQUAL("someVrfId", route.getNextVrfId());
  }

  {
    TestRoute route;

    route.setTableId("someTableId");
    BOOST_CHECK_EQUAL("someTableId", route.getTableId());
  }

  {
    TestRoute route;

    route.setNextTableId("someTableId");
    BOOST_CHECK_EQUAL("someTableId", route.getNextTableId());
  }

  {
    TestRoute route;
    nmdo::IpAddress ipAddr {"1.2.3.4/24"};

    route.setDstIpNet(ipAddr);
    BOOST_CHECK_EQUAL(ipAddr, route.getDstIpNet());
  }

  {
    TestRoute route;
    nmdo::IpAddress ipAddr {"1.2.3.4/24"};

    route.setNextHopIpAddr(ipAddr);
    BOOST_CHECK_EQUAL(ipAddr, route.getNextHopIpAddr());
  }

  {
    TestRoute route;

    route.setIfaceName("ifaceName");
    BOOST_CHECK_EQUAL("ifacename", route.getIfaceName());
  }

  {
    TestRoute route;

    route.setProtocol("OSPF");
    BOOST_CHECK_EQUAL("ospf", route.getProtocol());
  }

  {
    TestRoute route;

    route.setAdminDistance(42);
    BOOST_CHECK_EQUAL(42, route.getAdminDistance());
  }

  {
    TestRoute route;

    route.setMetric(1000);
    BOOST_CHECK_EQUAL(1000, route.getMetric());
  }

  {
    TestRoute route;
    nmdo::IpAddress tv1;
    nmdo::IpAddress tv2 {"1.2.3.4/24"};

    BOOST_CHECK("" == route.getNextHopIpAddrString());
    route.setNextHopIpAddr(tv2);
    BOOST_CHECK(tv2.toString() == route.getNextHopIpAddrString());
    route.setNullRoute(true);
    BOOST_CHECK("" == route.getNextHopIpAddrString());
    route.setNullRoute(false);
    BOOST_CHECK(tv2.toString() == route.getNextHopIpAddrString());
    route.setNextHopIpAddr(tv1);
    BOOST_CHECK("" == route.getNextHopIpAddrString());
  }
}

BOOST_AUTO_TEST_CASE(testValidity)
{
  {
    nmdo::IpAddress ipAddr  {"1.2.3.4/24"};
    std::string     iface   {"eth101"};
    {
      TestRoute route;

      BOOST_CHECK(!route.isValid());
      route.setDstIpNet(ipAddr);
      BOOST_CHECK(!route.isValid());
      route.setNullRoute(true);
      BOOST_CHECK(route.isValid());
    }
    {
      TestRoute route;

      BOOST_CHECK(!route.isValid());
      route.setDstIpNet(ipAddr);
      BOOST_CHECK(!route.isValid());
      route.setNextHopIpAddr(ipAddr);
      BOOST_CHECK(route.isValid());
    }
    {
      TestRoute route;

      BOOST_CHECK(!route.isValid());
      route.setDstIpNet(ipAddr);
      BOOST_CHECK(!route.isValid());
      route.setNextVrfId("test");
      BOOST_CHECK(!route.isValid());
      route.setNextTableId("test");
      BOOST_CHECK(route.isValid());
    }
  }

  {
    TestRoute route;
    nmdo::IpAddress ipAddr;

    BOOST_CHECK(ipAddr.hasUnsetPrefix());
    route.setNextHopIpAddr(ipAddr);
    BOOST_CHECK(!route.isValid());
    route.setDstIpNet(ipAddr);
    BOOST_CHECK(!route.isValid());
  }
}
