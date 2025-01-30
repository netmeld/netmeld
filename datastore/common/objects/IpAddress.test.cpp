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

#include <netmeld/datastore/objects/IpAddress.hpp>

namespace nmdo = netmeld::datastore::objects;


class TestIpAddress : public nmdo::IpAddress {
  public:
    TestIpAddress() : IpAddress() {};
    TestIpAddress(const std::string& _ip, const std::string& _desc) :
        IpAddress(_ip, _desc) {};

  public:
    using IpAddress::address;
    using IpAddress::prefix;
    using IpAddress::isResponding;
    using IpAddress::reason;
    using IpAddress::extraWeight;
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  {
    TestIpAddress ipAddr;

    BOOST_TEST(bai::address() == ipAddr.address);
    BOOST_TEST(UINT8_MAX == ipAddr.prefix);
    BOOST_TEST(false == ipAddr.isResponding);
    BOOST_TEST(ipAddr.reason.empty());
    BOOST_TEST(0.0 == ipAddr.extraWeight);
    BOOST_TEST(ipAddr.getAliases().empty());
  }

  {
    TestIpAddress ipAddr {"10.0.0.1/24", "Some Description"};

    BOOST_TEST(bai::make_address("10.0.0.1") == ipAddr.address);
    BOOST_TEST(24 == ipAddr.prefix);
    BOOST_TEST(false == ipAddr.isResponding);
    BOOST_TEST("Some Description" == ipAddr.reason);
    BOOST_TEST(0.0 == ipAddr.extraWeight);
    BOOST_TEST(ipAddr.getAliases().empty());
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    TestIpAddress ipAddr;

    BOOST_TEST(!ipAddr.isResponding);
    ipAddr.setResponding(true);
    BOOST_TEST(ipAddr.isResponding);
    ipAddr.setResponding(false);
    BOOST_TEST(!ipAddr.isResponding);
  }

  {
    TestIpAddress ipAddr;

    ipAddr.addAlias("Alias1", "Reason1");
    BOOST_TEST(ipAddr.getAliases().count("alias1"));
    BOOST_TEST("reason1" == ipAddr.reason);

    ipAddr.addAlias("Alias2", "Reason2");
    BOOST_TEST(ipAddr.getAliases().count("alias1"));
    BOOST_TEST(ipAddr.getAliases().count("alias2"));
    BOOST_TEST("reason2" == ipAddr.reason);
  }
  {
    nmdo::IpAddress ipAddr;

    std::string test {"1.2.3.4"};
    ipAddr.setAddress(test);

    auto expected {test + "/32"};
    BOOST_TEST(expected == ipAddr.toString());
  }
  {
    std::vector<std::tuple<std::string, std::string>> tests {
        {"1:2:3:4:5:6:7:8", "1:2:3:4:5:6:7:8/128"}
      , {"1:2:3:4:5:ffff:1.2.3.4", "1:2:3:4:5:ffff:102:304/128"}
      , {"1:2:3::ffff:1.2.3.4", "1:2:3::ffff:102:304/128"}
      , {"1::ffff:1.2.3.4", "1::ffff:102:304/128"}
      , {"::ffff:1.2.3.4", "::ffff:1.2.3.4/128"}
      };

    for (const auto& [test, expected] : tests) {
      nmdo::IpAddress ipAddr {test};
      BOOST_TEST(expected == ipAddr.toString());
    }
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
    ipAddr.setPrefix(0);
    BOOST_TEST(ipAddr.isValid());
    ipAddr.setPrefix(31);
    BOOST_TEST(ipAddr.isValid());
    ipAddr.setPrefix(32);
    BOOST_TEST(ipAddr.isValid());
    ipAddr.setPrefix(33);
    BOOST_TEST(!ipAddr.isValid());
  }

  {
    TestIpAddress ipAddr;

    BOOST_TEST(!ipAddr.isValid());
    ipAddr.setAddress("1234::0123");
    BOOST_TEST(ipAddr.isValid());
    ipAddr.setPrefix(0);
    BOOST_TEST(ipAddr.isValid());
    ipAddr.setPrefix(127);
    BOOST_TEST(ipAddr.isValid());
    ipAddr.setPrefix(128);
    BOOST_TEST(ipAddr.isValid());
    ipAddr.setPrefix(129);
    BOOST_TEST(!ipAddr.isValid());
  }

  {
    TestIpAddress ipAddr;

    BOOST_TEST(!ipAddr.isValid());
    ipAddr.setAddress("1:2:3:4:5:6:1.2.3.4");
    BOOST_TEST(ipAddr.isValid());
    ipAddr.setPrefix(0);
    BOOST_TEST(ipAddr.isValid());
    ipAddr.setPrefix(127);
    BOOST_TEST(ipAddr.isValid());
    ipAddr.setPrefix(128);
    BOOST_TEST(ipAddr.isValid());
    ipAddr.setPrefix(129);
    BOOST_TEST(!ipAddr.isValid());
  }
}

BOOST_AUTO_TEST_CASE(testOperatorEquality)
{
  nmdo::IpAddress ipAddr1;
  nmdo::IpAddress ipAddr2;

  ipAddr1.setAddress("192.168.1.20");
  ipAddr1.setExtraWeight(1.0);
  ipAddr2.setAddress("192.168.1.20");
  ipAddr2.setExtraWeight(1.0);
  BOOST_TEST(ipAddr1 == ipAddr2);

  // Very small (epsilon) differences in extraWeight are treated as still equal.
  ipAddr2.setExtraWeight(1.0 - (std::numeric_limits<double>::epsilon() * 128));
  BOOST_TEST(ipAddr1 == ipAddr2);
  ipAddr2.setExtraWeight(1.0 + (std::numeric_limits<double>::epsilon() * 128));
  BOOST_TEST(ipAddr1 == ipAddr2);

  // Larger (but still small) differences in extraWeight are treated as not equal.
  ipAddr2.setExtraWeight(0.999999999);
  BOOST_TEST(!(ipAddr1 == ipAddr2));
  ipAddr2.setExtraWeight(1.000000001);
  BOOST_TEST(!(ipAddr1 == ipAddr2));
}
