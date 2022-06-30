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

#include <netmeld/datastore/objects/Package.hpp>

namespace nmdo = netmeld::datastore::objects;


class TestInterface : public nmdo::Interface {
  public:
    TestInterface() : Interface() {};
    explicit TestInterface(const std::string& _name) :
        Interface(_name) {};

  public:
    std::string getMediaType() const
    { return mediaType; }

    bool getIsUp() const
    { return isUp; }

    std::string getFlags() const
    { return flags; }

    uint32_t getMtu() const
    { return mtu; }
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  nmdo::MacAddress macAddr;
  {
    TestInterface interface;

    BOOST_CHECK(interface.getName().empty());
    BOOST_CHECK_EQUAL("ethernet", interface.getMediaType());
    BOOST_CHECK(!interface.getIsUp());
    BOOST_CHECK_EQUAL(macAddr, interface.getMacAddress());
    BOOST_CHECK(interface.getFlags().empty());
    BOOST_CHECK_EQUAL(0, interface.getMtu());
  }

  {
    TestInterface interface {"Name"};

    BOOST_CHECK_EQUAL("name", interface.getName());
    BOOST_CHECK_EQUAL("ethernet", interface.getMediaType());
    BOOST_CHECK(!interface.getIsUp());
    BOOST_CHECK_EQUAL(macAddr, interface.getMacAddress());
    BOOST_CHECK(interface.getFlags().empty());
    BOOST_CHECK_EQUAL(0, interface.getMtu());
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    TestInterface interface;
    nmdo::IpAddress ipAddr {"1.2.3.4/24"};

    interface.addIpAddress(ipAddr);
    auto ipAddrs = interface.getIpAddresses();
    BOOST_CHECK_EQUAL(1, ipAddrs.size());
    BOOST_CHECK_EQUAL(ipAddr, *ipAddrs.cbegin());
  }

  {
    TestInterface interface;

    interface.setName("Name");
    BOOST_CHECK_EQUAL("name", interface.getName());
  }

  {
    TestInterface interface;

    interface.setMediaType("MediaType");
    BOOST_CHECK_EQUAL("mediatype", interface.getMediaType());
  }

  {
    TestInterface interface;
    nmdo::MacAddress macAddr {"00:11:22:33:44:55"};

    interface.setMacAddress(macAddr);
    BOOST_CHECK_EQUAL(macAddr, interface.getMacAddress());
  }

  {
    TestInterface interface;

    BOOST_CHECK(!interface.getIsUp());
    interface.setUp();
    BOOST_CHECK(interface.getIsUp());
  }

  {
    TestInterface interface;

    BOOST_CHECK(!interface.getIsUp());
    interface.setDown();
    BOOST_CHECK(!interface.getIsUp());
  }

  {
    TestInterface interface;

    interface.setFlags("some flags up here");
    BOOST_CHECK_EQUAL("some flags up here", interface.getFlags());
    BOOST_CHECK(!interface.getIsUp());

    interface.setDown();
    interface.setFlags("flag,UP");
    BOOST_CHECK(interface.getIsUp());

    interface.setDown();
    interface.setFlags("flag<UP");
    BOOST_CHECK(interface.getIsUp());
  }

  {
    TestInterface interface;

    interface.setMtu(0);
    BOOST_CHECK_EQUAL(0, interface.getMtu());
    interface.setMtu(UINT32_MAX);
    BOOST_CHECK_EQUAL(UINT32_MAX, interface.getMtu());
  }
}

BOOST_AUTO_TEST_CASE(testValidity)
{
  {
    TestInterface interface;

    BOOST_CHECK(!interface.isValid());
    interface.setName("name");
    BOOST_CHECK(interface.isValid());
    interface.setMediaType("loopback");
    BOOST_CHECK(!interface.isValid());
  }
}
