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

#include <netmeld/datastore/objects/Route.hpp>

namespace nmdo = netmeld::datastore::objects;


class TestRoute : public nmdo::Route {
  public:
    TestRoute() : Route() {};

  public:
    using Route::vrfId;
    using Route::tableId;
    using Route::dstIpNet;
    using Route::nextVrfId;
    using Route::nextTableId;
    using Route::nextHopIpAddr;
    using Route::outIfaceName;
    using Route::protocol;
    using Route::adminDistance;
    using Route::metric;
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  nmdo::IpAddress ipAddr;
  {
    TestRoute route;

    BOOST_TEST(ipAddr == route.dstIpNet);
    BOOST_TEST(ipAddr == route.nextHopIpAddr);
    BOOST_TEST(route.outIfaceName.empty());
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    TestRoute route;

    route.setVrfId("someVrfId");
    BOOST_TEST("someVrfId" == route.vrfId);
  }

  {
    TestRoute route;

    route.setNextVrfId("someVrfId");
    BOOST_TEST("someVrfId" == route.nextVrfId);
  }

  {
    TestRoute route;

    route.setTableId("someTableId");
    BOOST_TEST("someTableId" == route.tableId);
  }

  {
    TestRoute route;

    route.setNextTableId("someTableId");
    BOOST_TEST("someTableId" == route.nextTableId);
  }

  {
    TestRoute route;
    nmdo::IpAddress ipAddr {"1.2.3.4/24"};

    route.setDstIpNet(ipAddr);
    BOOST_TEST(ipAddr == route.dstIpNet);
  }

  {
    TestRoute route;
    nmdo::IpAddress ipAddr {"1.2.3.4/24"};

    route.setNextHopIpAddr(ipAddr);
    BOOST_TEST(ipAddr == route.nextHopIpAddr);
  }

  {
    TestRoute route;

    route.setOutIfaceName("outIfaceName");
    BOOST_TEST("outifacename" == route.outIfaceName);
  }

  {
    TestRoute route;

    route.setProtocol("OSPF");
    BOOST_TEST("ospf" == route.protocol);
  }

  {
    TestRoute route;

    route.setAdminDistance(42);
    BOOST_TEST(42 == route.adminDistance);
  }

  {
    TestRoute route;

    route.setMetric(1000);
    BOOST_TEST(1000 == route.metric);
  }

  {
    TestRoute route;
    nmdo::IpAddress tv1;
    nmdo::IpAddress tv2 {"1.2.3.4/24"};

    BOOST_TEST("" == route.getNextHopIpAddrString());
    route.setNextHopIpAddr(tv2);
    BOOST_TEST(tv2.toString() == route.getNextHopIpAddrString());
    route.setNullRoute(true);
    BOOST_TEST("" == route.getNextHopIpAddrString());
    route.setNullRoute(false);
    BOOST_TEST(tv2.toString() == route.getNextHopIpAddrString());
    route.setNextHopIpAddr(tv1);
    BOOST_TEST("" == route.getNextHopIpAddrString());
  }
}

BOOST_AUTO_TEST_CASE(testValidity)
{
  {
    nmdo::IpAddress ipAddr  {"1.2.3.4/24"};
    std::string     iface   {"eth101"};
    {
      TestRoute route;

      BOOST_TEST(!route.isValid());
      route.setDstIpNet(ipAddr);
      BOOST_TEST(!route.isValid());
      route.setNullRoute(true);
      BOOST_TEST(route.isValid());
    }
    {
      TestRoute route;

      BOOST_TEST(!route.isValid());
      route.setDstIpNet(ipAddr);
      BOOST_TEST(!route.isValid());
      route.setNextHopIpAddr(ipAddr);
      BOOST_TEST(route.isValid());
    }
    {
      TestRoute route;

      BOOST_TEST(!route.isValid());
      route.setDstIpNet(ipAddr);
      BOOST_TEST(!route.isValid());
      route.setNextVrfId("test");
      BOOST_TEST(!route.isValid());
      route.setNextTableId("test");
      BOOST_TEST(route.isValid());
    }
    {
      TestRoute route;

      BOOST_TEST(!route.isValid());
      route.setDstIpNet(ipAddr);
      BOOST_TEST(!route.isValid());
      route.setOutIfaceName("test");
      BOOST_TEST(route.isValid());
    }
  }

  {
    TestRoute route;
    nmdo::IpAddress ipAddr;

    BOOST_TEST(ipAddr.hasUnsetPrefix());
    route.setNextHopIpAddr(ipAddr);
    BOOST_TEST(!route.isValid());
    route.setDstIpNet(ipAddr);
    BOOST_TEST(!route.isValid());
  }
}
