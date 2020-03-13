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

#include <netmeld/core/objects/IpNetwork.hpp>

namespace nmco = netmeld::core::objects;


class TestIpNetwork : public nmco::IpNetwork {
  public:
    TestIpNetwork() : IpNetwork() {};
    TestIpNetwork(const std::string& _ip, const std::string& _desc) :
        IpNetwork(_ip, _desc) {};

  public:
    IpAddr getAddress() const
    { return address; }

    uint8_t getCidr() const
    { return cidr; }

    std::string getReason() const
    { return reason; }

    uint32_t getExtraWeight() const
    { return extraWeight; }
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  {
    TestIpNetwork ipNet;

    BOOST_TEST(IpAddr() == ipNet.getAddress());
    BOOST_TEST(UINT8_MAX == ipNet.getCidr());
    BOOST_TEST(ipNet.getReason().empty());
    BOOST_TEST(0 == ipNet.getExtraWeight());
  }

  {
    TestIpNetwork ipNet {"10.0.0.1/24", "Some Description"};

    BOOST_TEST(IpAddr::from_string("10.0.0.1") == ipNet.getAddress());
    BOOST_TEST(24 == ipNet.getCidr());
    BOOST_TEST("Some Description" == ipNet.getReason());
    BOOST_TEST(0 == ipNet.getExtraWeight());
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    TestIpNetwork ipNet;

    std::string ip {"10.0.0.1"};
    ipNet.setAddress(ip);
    BOOST_TEST(IpAddr::from_string(ip) == ipNet.getAddress());
    BOOST_TEST(32 == ipNet.getCidr());
  }

  {
    TestIpNetwork ipNet;

    std::string ip {"1:2:3:4:5:6:7:8"};
    ipNet.setAddress(ip);
    BOOST_TEST(IpAddr::from_string(ip) == ipNet.getAddress());
    BOOST_TEST(128 == ipNet.getCidr());
  }

  {
    TestIpNetwork ipNet;

    ipNet.setCidr(15);
    BOOST_TEST(15 == ipNet.getCidr());

    ipNet.setCidr(150);
    BOOST_TEST(150 == ipNet.getCidr());
  }

  {
    TestIpNetwork ipNet;

    ipNet.setExtraWeight(999);
    BOOST_TEST(999 == ipNet.getExtraWeight());
  }

  {
    TestIpNetwork ipNet;

    const size_t NUM_OCTETS {4};
    const size_t OCTET_SIZE {8};
    std::bitset<OCTET_SIZE> maskBits[NUM_OCTETS];
    struct in_addr addr;
    int domain {AF_INET};
    char maskStr[INET_ADDRSTRLEN];
    for (size_t z {0}; z < NUM_OCTETS; z++) {
      for (size_t i {1}; i <= OCTET_SIZE; i++) {
        for (size_t pos {0}; pos < NUM_OCTETS; pos++) {
          addr.s_addr <<= OCTET_SIZE;
          addr.s_addr |= maskBits[pos].to_ulong();
        }
        inet_ntop(domain, &addr, maskStr, INET_ADDRSTRLEN);
        ipNet.setNetmask(nmco::IpNetwork(std::string(maskStr)));
        BOOST_TEST(((z*OCTET_SIZE)+(i-1)) == ipNet.getCidr());

        maskBits[z].set(OCTET_SIZE-i);
      }
    }
  }
  {
    TestIpNetwork ipNet;

    const size_t NUM_OCTETS {16};
    const size_t OCTET_SIZE {8};
    std::bitset<OCTET_SIZE> maskBits[NUM_OCTETS];
    struct in6_addr addr;
    int domain {AF_INET6};
    char maskStr[INET6_ADDRSTRLEN];
    for (size_t z {0}; z < NUM_OCTETS; z++) {
      for (size_t i {1}; i <= OCTET_SIZE; i++) {
        for (size_t pos {0}; pos < NUM_OCTETS; pos++) {
          addr.s6_addr[pos] = maskBits[pos].to_ulong();
        }
        inet_ntop(domain, &addr, maskStr, INET6_ADDRSTRLEN);
        ipNet.setNetmask(nmco::IpNetwork(std::string(maskStr)));
        BOOST_TEST(((z*OCTET_SIZE)+(i-1)) == ipNet.getCidr());

        maskBits[z].set(OCTET_SIZE-i);
      }
    }
  }

  {
    TestIpNetwork ipNet;

    BOOST_TEST(ipNet.getReason().empty());
    ipNet.setReason("Some Reason");
    BOOST_TEST("Some Reason" == ipNet.getReason());
  }

  {
    TestIpNetwork ipNet;

    BOOST_TEST(ipNet.isDefault());
    ipNet.setCidr(0);
    BOOST_TEST(!ipNet.isDefault());
    ipNet.setCidr(24);
    BOOST_TEST(!ipNet.isDefault());
    ipNet.setCidr(32);
    BOOST_TEST(!ipNet.isDefault());
    ipNet.setCidr(128);
    BOOST_TEST(!ipNet.isDefault());
    ipNet.setCidr(UINT8_MAX);
    BOOST_TEST(!ipNet.isDefault()); // once set, can never be unset
  }

  {
    TestIpNetwork ipNet;

    BOOST_TEST(ipNet.isV4());
    BOOST_TEST(!ipNet.isV6());

    ipNet.setAddress("1.2.3.4");
    BOOST_TEST(ipNet.isV4());
    BOOST_TEST(!ipNet.isV6());

    ipNet.setAddress("1::2");
    BOOST_TEST(!ipNet.isV4());
    BOOST_TEST(ipNet.isV6());
  }

  {
    TestIpNetwork ipNet;

    std::string dip {"0.0.0.0/" + std::to_string(UINT8_MAX)};
    BOOST_TEST(dip == ipNet.toString());

    // ensure cidr is unsigned otherwise checks for below zero are needed
    BOOST_TEST((typeid(uint8_t) == typeid(ipNet.getCidr())));

    ipNet.setAddress("1.2.3.255");
    ipNet.setCidr(24);
    BOOST_TEST(std::string("1.2.3.0/24") == ipNet.toString());
    ipNet.setAddress("1.2.3.255");
    ipNet.setCidr(27);
    BOOST_TEST(std::string("1.2.3.224/27") == ipNet.toString());

    ipNet.setAddress("fe00::aabb:ccdd");
    ipNet.setCidr(128);
    BOOST_TEST(std::string("fe00::aabb:ccdd/128") == ipNet.toString());
    ipNet.setAddress("fe00::aabb:ffff");
    ipNet.setCidr(120);
    BOOST_TEST(std::string("fe00::aabb:ff00/120") == ipNet.toString());
    ipNet.setAddress("fe00::aabb:ffff");
    ipNet.setCidr(100);
    BOOST_TEST(std::string("fe00::a000:0/100") == ipNet.toString());
  }
}

BOOST_AUTO_TEST_CASE(testValidity)
{
  {
    TestIpNetwork ipNet;

    BOOST_TEST(!ipNet.isValid());

    ipNet.setAddress("127.0.0.1");
    BOOST_TEST(!ipNet.isValid());
    ipNet.setAddress("::1");
    BOOST_TEST(!ipNet.isValid());

    ipNet.setAddress("224.0.0.0");
    BOOST_TEST(!ipNet.isValid());
    ipNet.setAddress("ff00::");
    BOOST_TEST(!ipNet.isValid());

    ipNet.setAddress("0.0.0.0");
    BOOST_TEST(!ipNet.isValid());
    ipNet.setAddress("::");
    BOOST_TEST(!ipNet.isValid());
  }

  {
    TestIpNetwork ipNet;

    BOOST_TEST(!ipNet.isValid());
    ipNet.setAddress("1.2.3.4");
    BOOST_TEST(!ipNet.isValid());
    ipNet.setCidr(0);
    BOOST_TEST(ipNet.isValid());
    ipNet.setCidr(31);
    BOOST_TEST(ipNet.isValid());
    ipNet.setCidr(32);
    BOOST_TEST(!ipNet.isValid());
    ipNet.setCidr(33);
    BOOST_TEST(!ipNet.isValid());
  }

  {
    TestIpNetwork ipNet;

    BOOST_TEST(!ipNet.isValid());
    ipNet.setAddress("0123::0123");
    BOOST_TEST(!ipNet.isValid());
    ipNet.setCidr(0);
    BOOST_TEST(ipNet.isValid());
    ipNet.setCidr(127);
    BOOST_TEST(ipNet.isValid());
    ipNet.setCidr(128);
    BOOST_TEST(!ipNet.isValid());
    ipNet.setCidr(129);
    BOOST_TEST(!ipNet.isValid());
  }
}
