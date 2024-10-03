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

#include <netmeld/datastore/objects/MacAddress.hpp>

namespace nmdo = netmeld::datastore::objects;


class TestMacAddress : public nmdo::MacAddress {
  public:
    using MacAddress::MacAddress;

    using MacAddress::macAddr;
    using MacAddress::isResponding;
    // has accessor
    //using MacAddress::ipAddrs;
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  {
    TestMacAddress macAddr;

    BOOST_TEST("Invalid MAC" == macAddr.toString());
    BOOST_TEST(macAddr.getIpAddresses().empty());
    BOOST_TEST(false == macAddr.isResponding);
  }

  {
    std::string mac {"01:02:33:0a:0b:cc"};
    TestMacAddress macAddr {mac};

    BOOST_TEST(mac == macAddr.toString());
    BOOST_TEST(macAddr.getIpAddresses().empty());
    BOOST_TEST(false == macAddr.isResponding);
  }

  {
    std::vector<uint8_t> mac {1,2,3,10,11,12};
    TestMacAddress macAddr {mac};

    for (size_t i {0}; i<mac.size(); i++) {
      BOOST_TEST(mac[i] == macAddr.macAddr[i]);
    }
    BOOST_TEST(macAddr.getIpAddresses().empty());
    BOOST_TEST(false == macAddr.isResponding);
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    TestMacAddress macAddr;
    std::vector<uint8_t> mac {1,2,3,10,11,12};

    macAddr.setMac(mac);
    for (size_t i {0}; i<mac.size(); i++) {
      BOOST_TEST(mac[i] == macAddr.macAddr[i]);
    }
  }

  {
    TestMacAddress macAddr;
    std::string mac("01:02:03:0a:0b:0c");

    macAddr.setMac(mac);
    BOOST_TEST(mac == macAddr.toString());
  }

  {
    TestMacAddress macAddr;

    macAddr.setResponding(true);
    BOOST_TEST(true == macAddr.isResponding);

    macAddr.setResponding(false);
    BOOST_TEST(false == macAddr.isResponding);
  }

  {
    TestMacAddress macAddr;
    nmdo::IpAddress ipAddr("10.0.0.1/24");

    macAddr.addIpAddress(ipAddr);
    for (const auto& ia : macAddr.getIpAddresses()) {
      BOOST_TEST(ipAddr == ia);
    }
  }

  {
    std::vector<uint8_t> mac;
    TestMacAddress macAddr;

    BOOST_TEST("Invalid MAC" == macAddr.toString());

    mac = {1,2,3,10,11,12};
    macAddr.setMac(mac);
    BOOST_TEST("01:02:03:0a:0b:0c" == macAddr.toString());

    mac = {1,2,3,4,10,11,12,13};
    macAddr.setMac(mac);
    BOOST_TEST("01:02:03:04:0a:0b:0c:0d" == macAddr.toString());
  }
}

BOOST_AUTO_TEST_CASE(testValidity)
{
  {
    TestMacAddress macAddr;
    BOOST_TEST(!macAddr.isValid());
  }

  {
    TestMacAddress macAddr;
    std::vector<uint8_t> mac;

    mac = {1,2,3,10,11,12};
    macAddr.setMac(mac);
    BOOST_TEST(macAddr.isValid());

    mac = {1,2,3,10,11};
    macAddr.setMac(mac);
    BOOST_TEST(!macAddr.isValid());

    mac = {1,2,3,4,10,11,12};
    macAddr.setMac(mac);
    BOOST_TEST(!macAddr.isValid());

    mac = {1,2,3,4,10,11,12,13};
    macAddr.setMac(mac);
    BOOST_TEST(!macAddr.isValid());

    mac = {0,0,0,0,0,0};
    macAddr.setMac(mac);
    BOOST_TEST(macAddr.isValid());

    mac = {UINT8_MAX,UINT8_MAX,UINT8_MAX,UINT8_MAX,UINT8_MAX,UINT8_MAX};
    macAddr.setMac(mac);
    BOOST_TEST(macAddr.isValid());
  }
}
