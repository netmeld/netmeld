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

#include <netmeld/core/objects/Route.hpp>

namespace nmco = netmeld::core::objects;


class TestRoute : public nmco::Route {
  public:
    TestRoute() : Route() {};

  public:
    void ufs(bool isMetadata)
    { updateForSave(isMetadata); }

    nmco::IpNetwork getDstNet() const
    { return dstNet; }

    nmco::IpAddress getRtrIp() const
    { return rtrIp; }

    std::string getIfaceName() const
    { return ifaceName; }
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  nmco::IpAddress ipAddr;
  {
    TestRoute route;

    BOOST_CHECK_EQUAL(ipAddr, route.getDstNet());
    BOOST_CHECK_EQUAL(ipAddr, route.getRtrIp());
    BOOST_CHECK(route.getIfaceName().empty());
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    TestRoute route;
    nmco::IpAddress ipAddr {"1.2.3.4/24"};

    route.setDstNet(ipAddr);
    BOOST_CHECK_EQUAL(ipAddr, route.getDstNet());
  }

  {
    TestRoute route;
    nmco::IpAddress ipAddr {"1.2.3.4/24"};

    route.setRtrIp(ipAddr);
    BOOST_CHECK_EQUAL(ipAddr, route.getRtrIp());
  }

  {
    TestRoute route;

    route.setIfaceName("ifaceName");
    BOOST_CHECK_EQUAL("ifacename", route.getIfaceName());
  }

  {
    TestRoute r0;
    nmco::IpAddress ipAddr   {"1.2.3.4/24"};
    nmco::IpAddress dIpAddr = nmco::IpAddress::getIpv4Default();
    {
      TestRoute r1, r2;

      BOOST_CHECK_EQUAL(r1, r2);
      r2.ufs(false);
      BOOST_CHECK_EQUAL(r1, r2);
      r1.setRtrIp(ipAddr);
      r2.setRtrIp(ipAddr);
      BOOST_CHECK_EQUAL(r1, r2);
      r2.ufs(false);
      BOOST_CHECK_NE(r1, r2);
      BOOST_CHECK_EQUAL(dIpAddr, r2.getDstNet());
    }

    nmco::IpAddress rIpAddr = nmco::IpAddress::getIpv4Default();
    {
      TestRoute r1, r2;

      BOOST_CHECK_EQUAL(r1, r2);
      r2.ufs(false);
      BOOST_CHECK_EQUAL(r1, r2);
      r1.setDstNet(ipAddr);
      r2.setDstNet(ipAddr);
      BOOST_CHECK_EQUAL(r1, r2);
      r2.ufs(false);
      BOOST_CHECK_NE(r1, r2);
      BOOST_CHECK_EQUAL(rIpAddr, r2.getRtrIp());
    }
  }

  {
    TestRoute r0;
    nmco::IpAddress ipAddr   {"1234::1234/64"};
    nmco::IpAddress dIpAddr = nmco::IpAddress::getIpv6Default();
    {
      TestRoute r1, r2;

      BOOST_CHECK_EQUAL(r1, r2);
      r2.ufs(false);
      BOOST_CHECK_EQUAL(r1, r2);
      r1.setRtrIp(ipAddr);
      r2.setRtrIp(ipAddr);
      BOOST_CHECK_EQUAL(r1, r2);
      r2.ufs(false);
      BOOST_CHECK_NE(r1, r2);
      BOOST_CHECK_EQUAL(dIpAddr, r2.getDstNet());
    }

    nmco::IpAddress rIpAddr = nmco::IpAddress::getIpv4Default();
    {
      TestRoute r1, r2;

      BOOST_CHECK_EQUAL(r1, r2);
      r2.ufs(false);
      BOOST_CHECK_EQUAL(r1, r2);
      r1.setDstNet(ipAddr);
      r2.setDstNet(ipAddr);
      BOOST_CHECK_EQUAL(r1, r2);
      r2.ufs(false);
      BOOST_CHECK_NE(r1, r2);
      BOOST_CHECK_EQUAL(rIpAddr, r2.getRtrIp());
    }
    {
      TestRoute r1, r2;

      BOOST_CHECK_EQUAL(r1, r2);
      r2.ufs(true);
      BOOST_CHECK_EQUAL(r1, r2);
      r1.setDstNet(ipAddr);
      r2.setDstNet(ipAddr);
      BOOST_CHECK_EQUAL(r1, r2);
      r2.ufs(true);
      BOOST_CHECK_NE(r1, r2);
      BOOST_CHECK_EQUAL(dIpAddr, r2.getRtrIp());
    }
  }
}

BOOST_AUTO_TEST_CASE(testValidity)
{
  {
    nmco::IpAddress ipAddr {"1.2.3.4/24"};
    {
      TestRoute route;

      BOOST_CHECK(!route.isValid());
      route.setRtrIp(ipAddr);
      BOOST_CHECK(route.isValid());
    }

    {
      TestRoute route;

      BOOST_CHECK(!route.isValid());
      route.setDstNet(ipAddr);
      BOOST_CHECK(route.isValid());
    }
  }

  {
    TestRoute route;
    nmco::IpAddress ipAddr;

    BOOST_CHECK(ipAddr.isDefault());
    route.setRtrIp(ipAddr);
    BOOST_CHECK(!route.isValid());
    route.setDstNet(ipAddr);
    BOOST_CHECK(!route.isValid());
  }
}
