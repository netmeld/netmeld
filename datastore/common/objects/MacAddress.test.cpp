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

#include <netmeld/datastore/objects/MacAddress.hpp>

namespace nmdo = netmeld::datastore::objects;


class TestMacAddress : public nmdo::MacAddress {
  public:
    TestMacAddress() : MacAddress() {};
    explicit TestMacAddress(const std::string& _mac) : MacAddress(_mac) {};
    explicit TestMacAddress(std::vector<uint8_t>& _mac) : MacAddress(_mac) {};

  public:
    std::vector<uint8_t> getMacAddr() const
    { return macAddr; }

    bool getIsResponding() const
    { return isResponding; }
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  {
    TestMacAddress macAddr;

    BOOST_CHECK_EQUAL("Invalid MAC", macAddr.toString());
    BOOST_CHECK(macAddr.getIpAddrs().empty());
    BOOST_CHECK_EQUAL(false, macAddr.getIsResponding());
  }

  {
    std::string mac {"01:02:33:0a:0b:cc"};
    TestMacAddress macAddr {mac};

    BOOST_CHECK_EQUAL(mac, macAddr.toString());
    BOOST_CHECK(macAddr.getIpAddrs().empty());
    BOOST_CHECK_EQUAL(false, macAddr.getIsResponding());
  }

  {
    std::vector<uint8_t> mac {1,2,3,10,11,12};
    TestMacAddress macAddr {mac};

    for (size_t i {0}; i<mac.size(); i++) {
      BOOST_CHECK_EQUAL(mac[i], macAddr.getMacAddr()[i]);
    }
    BOOST_CHECK(macAddr.getIpAddrs().empty());
    BOOST_CHECK_EQUAL(false, macAddr.getIsResponding());
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    TestMacAddress macAddr;
    std::vector<uint8_t> mac {1,2,3,10,11,12};

    macAddr.setMac(mac);
    for (size_t i {0}; i<mac.size(); i++) {
      BOOST_CHECK_EQUAL(mac[i], macAddr.getMacAddr()[i]);
    }
  }

  {
    TestMacAddress macAddr;
    std::string mac("01:02:03:0a:0b:0c");

    macAddr.setMac(mac);
    BOOST_CHECK_EQUAL(mac, macAddr.toString());
  }

  {
    TestMacAddress macAddr;

    macAddr.setResponding(true);
    BOOST_CHECK_EQUAL(true, macAddr.getIsResponding());

    macAddr.setResponding(false);
    BOOST_CHECK_EQUAL(false, macAddr.getIsResponding());
  }

  {
    TestMacAddress macAddr;
    nmdo::IpAddress ipAddr("10.0.0.1/24");

    macAddr.addIp(ipAddr);
    for (const auto& ia : macAddr.getIpAddrs()) {
      BOOST_CHECK_EQUAL(ipAddr, ia);
    }
  }

  {
    std::vector<uint8_t> mac;
    TestMacAddress macAddr;

    BOOST_CHECK_EQUAL("Invalid MAC", macAddr.toString());

    mac = {1,2,3,10,11,12};
    macAddr.setMac(mac);
    BOOST_CHECK_EQUAL("01:02:03:0a:0b:0c", macAddr.toString());

    mac = {1,2,3,4,10,11,12,13};
    macAddr.setMac(mac);
    BOOST_CHECK_EQUAL("01:02:03:04:0a:0b:0c:0d", macAddr.toString());
  }
}

BOOST_AUTO_TEST_CASE(testValidity)
{
  {
    TestMacAddress macAddr;
    BOOST_CHECK(!macAddr.isValid());
  }

  {
    TestMacAddress macAddr;
    std::vector<uint8_t> mac;

    mac = {1,2,3,10,11,12};
    macAddr.setMac(mac);
    BOOST_CHECK(macAddr.isValid());

    mac = {1,2,3,10,11};
    macAddr.setMac(mac);
    BOOST_CHECK(!macAddr.isValid());

    mac = {1,2,3,4,10,11,12};
    macAddr.setMac(mac);
    BOOST_CHECK(!macAddr.isValid());

    mac = {1,2,3,4,10,11,12,13};
    macAddr.setMac(mac);
    BOOST_CHECK(!macAddr.isValid());

    mac = {0,0,0,0,0,0};
    macAddr.setMac(mac);
    BOOST_CHECK(macAddr.isValid());

    mac = {UINT8_MAX,UINT8_MAX,UINT8_MAX,UINT8_MAX,UINT8_MAX,UINT8_MAX};
    macAddr.setMac(mac);
    BOOST_CHECK(macAddr.isValid());
  }
}
