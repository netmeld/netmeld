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
    TestIpNetwork ipNet {"1.2.3.4", ""};

    const size_t NUM_OCTETS {4};
    const size_t OCTET_SIZE {8};
    std::bitset<OCTET_SIZE> maskBits[NUM_OCTETS];
    std::ostringstream oss;
    for (size_t z {0}; z < NUM_OCTETS; z++) {
      for (size_t i {1}; i <= OCTET_SIZE; i++) {
        oss.str("");
        for (size_t pos {0}; pos < NUM_OCTETS; pos++) {
          oss << maskBits[pos].to_ulong();
          if ((pos+1) < NUM_OCTETS) {
            oss << '.';
          }
        }
        nmco::IpNetwork mask {oss.str()};
        ipNet.setNetmask(mask);
        BOOST_TEST(((z*OCTET_SIZE)+(i-1)) == ipNet.getCidr());

        maskBits[z].set(OCTET_SIZE-i);
      }
    }
  }
  {
    TestIpNetwork ipNet {"1234:5678:abcd:ef01:2345:6789:0abc:def0", ""};

    const size_t NUM_OCTETS {8};
    const size_t OCTET_SIZE {16};
    std::bitset<OCTET_SIZE> maskBits[NUM_OCTETS];
    std::ostringstream oss;
    for (size_t z {0}; z < NUM_OCTETS; z++) {
      for (size_t i {1}; i <= OCTET_SIZE; i++) {
        oss.str("");
        for (size_t pos {0}; pos < NUM_OCTETS; pos++) {
          oss << std::hex << maskBits[pos].to_ulong();
          if ((pos+1) < NUM_OCTETS) {
            oss << ':';
          }
        }
        nmco::IpNetwork mask {oss.str()};
        ipNet.setNetmask(mask);
        BOOST_TEST(((z*OCTET_SIZE)+(i-1)) == ipNet.getCidr());

        maskBits[z].set(OCTET_SIZE-i);
      }
    }
  }

  {
    std::vector<std::pair<unsigned int, std::string>> masks {
      {32, "0.0.0.0"},
      {31, "0.0.0.1"},
      {30, "0.0.0.3"},
      {29, "0.0.0.7"},
      {28, "0.0.0.15"},
      {27, "0.0.0.31"},
      {26, "0.0.0.63"},
      {25, "0.0.0.127"},
      {24, "0.0.0.255"},
      {23, "0.0.1.255"},
      {22, "0.0.3.255"},
      {21, "0.0.7.255"},
      {20, "0.0.15.255"},
      {19, "0.0.31.255"},
      {18, "0.0.63.255"},
      {17, "0.0.127.255"},
      {16, "0.0.255.255"},
      {15, "0.1.255.255"},
      {14, "0.3.255.255"},
      {13, "0.7.255.255"},
      {12, "0.15.255.255"},
      {11, "0.31.255.255"},
      {10, "0.63.255.255"},
      {9,  "0.127.255.255"},
      {8,  "0.255.255.255"},
      {7,  "1.255.255.255"},
      {6,  "3.255.255.255"},
      {5,  "7.255.255.255"},
      {4,  "15.255.255.255"},
      {3,  "31.255.255.255"},
      {2,  "63.255.255.255"},
      {1,  "127.255.255.255"},
      {0,  "255.255.255.255"},
    };

    for (const auto& [cidr, mask] : masks) {
      TestIpNetwork ipMask;
      ipMask.setAddress(mask);

      TestIpNetwork ipNet;
      bool is_contiguous = ipNet.setWildcardNetmask(ipMask);

      BOOST_TEST(is_contiguous);
      BOOST_TEST(ipNet.getCidr() == cidr);
    }
  }
  {
    std::vector<std::pair<unsigned int, std::string>> masks {
      {32, "0.0.0.27"},
      {32, "0.1.0.1"},
      {32, "1.1.1.1"},
    };

    for (const auto& [cidr, mask] : masks) {
      TestIpNetwork ipMask;
      ipMask.setAddress(mask);

      TestIpNetwork ipNet;
      bool is_contiguous = ipNet.setWildcardNetmask(ipMask);

      BOOST_TEST(!is_contiguous);
      BOOST_TEST(ipNet.getCidr() == cidr);
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
