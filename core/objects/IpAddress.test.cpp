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

#include <typeinfo>

#include <boost/test/unit_test.hpp>

#include <netmeld/core/objects/IpAddress.hpp>

namespace nmco = netmeld::core::objects;


class TestIpAddress : public nmco::IpAddress {
  public:
    TestIpAddress() : IpAddress() {};
    TestIpAddress(const std::string& _ip, const std::string& _desc) :
        IpAddress(_ip, _desc) {};

  public:
    IpAddr getAddress() const
    { return address; }

    uint8_t getCidr() const
    { return cidr; }

    bool getIsResponding() const
    { return isResponding; }

    std::string getReason() const
    { return reason; }

    uint32_t getExtraWeight() const
    { return extraWeight; }

    std::set<std::string> getAliases() const
    { return aliases; }
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  {
    TestIpAddress ipAddr;

    BOOST_TEST(IpAddr() == ipAddr.getAddress());
    BOOST_TEST(UINT8_MAX == ipAddr.getCidr());
    BOOST_TEST(false == ipAddr.getIsResponding());
    BOOST_TEST(ipAddr.getReason().empty());
    BOOST_TEST(0 == ipAddr.getExtraWeight());
    BOOST_TEST(ipAddr.getAliases().empty());
  }

  {
    TestIpAddress ipAddr {"10.0.0.1/24", "Some Description"};

    BOOST_TEST(IpAddr::from_string("10.0.0.1") == ipAddr.getAddress());
    BOOST_TEST(24 == ipAddr.getCidr());
    BOOST_TEST(false == ipAddr.getIsResponding());
    BOOST_TEST("Some Description" == ipAddr.getReason());
    BOOST_TEST(0 == ipAddr.getExtraWeight());
    BOOST_TEST(ipAddr.getAliases().empty());
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    TestIpAddress ipAddr;

    BOOST_TEST(!ipAddr.getIsResponding());
    ipAddr.setResponding(true);
    BOOST_TEST(ipAddr.getIsResponding());
    ipAddr.setResponding(false);
    BOOST_TEST(!ipAddr.getIsResponding());
  }

  {
    TestIpAddress ipAddr;

    ipAddr.addAlias("Alias1", "Reason1");
    BOOST_TEST(ipAddr.getAliases().count("alias1"));
    BOOST_TEST("reason1" == ipAddr.getReason());

    ipAddr.addAlias("Alias2", "Reason2");
    BOOST_TEST(ipAddr.getAliases().count("alias1"));
    BOOST_TEST(ipAddr.getAliases().count("alias2"));
    BOOST_TEST("reason2" == ipAddr.getReason());
  }
}

BOOST_AUTO_TEST_CASE(testValidity)
{
  {
    TestIpAddress ipAddr;

    BOOST_TEST(!ipAddr.isValid());

    ipAddr.setAddress("127.0.0.1");
    BOOST_TEST(!ipAddr.isValid());
    ipAddr.setAddress("::1");
    BOOST_TEST(!ipAddr.isValid());

    ipAddr.setAddress("224.0.0.0");
    BOOST_TEST(!ipAddr.isValid());
    ipAddr.setAddress("ff00::");
    BOOST_TEST(!ipAddr.isValid());

    ipAddr.setAddress("0.0.0.0");
    BOOST_TEST(!ipAddr.isValid());
    ipAddr.setAddress("::");
    BOOST_TEST(!ipAddr.isValid());
  }

  {
    TestIpAddress ipAddr;

    BOOST_TEST(!ipAddr.isValid());
    ipAddr.setAddress("1.2.3.4");
    BOOST_TEST(ipAddr.isValid());
    ipAddr.setCidr(0);
    BOOST_TEST(ipAddr.isValid());
    ipAddr.setCidr(31);
    BOOST_TEST(ipAddr.isValid());
    ipAddr.setCidr(32);
    BOOST_TEST(ipAddr.isValid());
    ipAddr.setCidr(33);
    BOOST_TEST(!ipAddr.isValid());
  }

  {
    TestIpAddress ipAddr;

    BOOST_TEST(!ipAddr.isValid());
    ipAddr.setAddress("1234::0123");
    BOOST_TEST(ipAddr.isValid());
    ipAddr.setCidr(0);
    BOOST_TEST(ipAddr.isValid());
    ipAddr.setCidr(127);
    BOOST_TEST(ipAddr.isValid());
    ipAddr.setCidr(128);
    BOOST_TEST(ipAddr.isValid());
    ipAddr.setCidr(129);
    BOOST_TEST(!ipAddr.isValid());
  }
}
