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

#include <netmeld/datastore/objects/Interface.hpp>

namespace nmdo = netmeld::datastore::objects;


class TestInterface : public nmdo::Interface {
  public:
    using Interface::Interface;

    using Interface::mediaType;
    using Interface::isUp;
    using Interface::flags;
    using Interface::mtu;
    // has accessor
    //using Interface::name;
    //using Interface::macAddr;
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  nmdo::MacAddress macAddr;
  {
    TestInterface interface;

    BOOST_TEST(interface.getName().empty());
    BOOST_TEST("ethernet" == interface.mediaType);
    BOOST_TEST(!interface.isUp);
    BOOST_TEST(macAddr == interface.getMacAddress());
    BOOST_TEST(interface.flags.empty());
    BOOST_TEST(0 == interface.mtu);
  }

  {
    TestInterface interface {"Name"};

    BOOST_TEST("name" == interface.getName());
    BOOST_TEST("ethernet" == interface.mediaType);
    BOOST_TEST(!interface.isUp);
    BOOST_TEST(macAddr == interface.getMacAddress());
    BOOST_TEST(interface.flags.empty());
    BOOST_TEST(0 == interface.mtu);
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    TestInterface interface;
    nmdo::IpAddress ipAddr {"1.2.3.4/24"};

    interface.addIpAddress(ipAddr);
    auto ipAddrs = interface.getIpAddresses();
    BOOST_TEST(1 == ipAddrs.size());
    BOOST_TEST(ipAddr == *ipAddrs.cbegin());
  }

  {
    TestInterface interface;

    interface.setName("Name");
    BOOST_TEST("name" == interface.getName());
  }

  {
    TestInterface interface;

    interface.setMediaType("MediaType");
    BOOST_TEST("mediatype" == interface.mediaType);
  }

  {
    TestInterface interface;
    nmdo::MacAddress macAddr {"00:11:22:33:44:55"};

    interface.setMacAddress(macAddr);
    BOOST_TEST(macAddr == interface.getMacAddress());
  }

  {
    TestInterface interface;

    BOOST_TEST(!interface.isUp);
    interface.setUp();
    BOOST_TEST(interface.isUp);
  }

  {
    TestInterface interface;

    BOOST_TEST(!interface.isUp);
    interface.setDown();
    BOOST_TEST(!interface.isUp);
  }

  {
    TestInterface interface;

    interface.setFlags("some flags up here");
    BOOST_TEST("some flags up here" == interface.flags);
    BOOST_TEST(!interface.isUp);

    interface.setDown();
    interface.setFlags("flag,UP");
    BOOST_TEST(interface.isUp);

    interface.setDown();
    interface.setFlags("flag<UP");
    BOOST_TEST(interface.isUp);
  }

  {
    TestInterface interface;

    interface.setMtu(0);
    BOOST_TEST(0 == interface.mtu);
    interface.setMtu(UINT32_MAX);
    BOOST_TEST(UINT32_MAX == interface.mtu);
  }
}

BOOST_AUTO_TEST_CASE(testValidity)
{
  {
    TestInterface interface;

    BOOST_TEST(!interface.isValid());
    interface.setName("name");
    BOOST_TEST(interface.isValid());
    interface.setMediaType("loopback");
    BOOST_TEST(!interface.isValid());
  }
}
