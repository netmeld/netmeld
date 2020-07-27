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

#include <netmeld/datastore/objects/IpNetwork.hpp>

namespace nmdo = netmeld::datastore::objects;


class TestIpNetwork : public nmdo::IpNetwork {
  public:
    TestIpNetwork() : IpNetwork() {};
    TestIpNetwork(const std::string& _ip, const std::string& _desc) :
        IpNetwork(_ip, _desc) {};

  public:
    IpAddr getAddress() const
    { return address; }

    size_t getPrefix() const
    { return prefix; }

    std::string getReason() const
    { return reason; }

    double getExtraWeight() const
    { return extraWeight; }

    template<size_t n>
      std::bitset<n> getBits() const
      { return asBitset<n>(); }
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  {
    TestIpNetwork ipNet;

    BOOST_TEST(IpAddr() == ipNet.getAddress());
    BOOST_TEST(UINT8_MAX == ipNet.getPrefix());
    BOOST_TEST(ipNet.getReason().empty());
    BOOST_TEST(0.0 == ipNet.getExtraWeight());
  }

  {
    TestIpNetwork ipNet {"10.0.0.1/24", "Some Description"};

    BOOST_TEST(IpAddr::from_string("10.0.0.1") == ipNet.getAddress());
    BOOST_TEST(24 == ipNet.getPrefix());
    BOOST_TEST("Some Description" == ipNet.getReason());
    BOOST_TEST(0.0 == ipNet.getExtraWeight());
  }
}

BOOST_AUTO_TEST_CASE(testSettersSimple)
{
  {
    const std::vector<std::tuple<std::string, size_t>> testsOk {
      {"1.2.3.4", 32},
      {"1:2:3:4:5:6:7:8", 128},
    };

    for (const auto& [ip, prefix] : testsOk) {
      TestIpNetwork ipNet {ip, ""};
      BOOST_TEST(IpAddr::from_string(ip) == ipNet.getAddress());
      BOOST_TEST(prefix == ipNet.getPrefix());

    }
  }

  {
    TestIpNetwork ipNet;

    ipNet.setPrefix(15);
    BOOST_TEST(15 == ipNet.getPrefix());
    ipNet.setPrefix(150);
    BOOST_TEST(150 == ipNet.getPrefix());
  }

  {
    TestIpNetwork ipNet;

    ipNet.setExtraWeight(999.0);
    BOOST_TEST(999.0 == ipNet.getExtraWeight());
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
    ipNet.setPrefix(0);
    BOOST_TEST(!ipNet.isDefault());
    ipNet.setPrefix(24);
    BOOST_TEST(!ipNet.isDefault());
    ipNet.setPrefix(32);
    BOOST_TEST(!ipNet.isDefault());
    ipNet.setPrefix(128);
    BOOST_TEST(!ipNet.isDefault());
    ipNet.setPrefix(UINT8_MAX);
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

    // ensure prefix is unsigned otherwise checks for below zero are needed
    BOOST_TEST((typeid(size_t) == typeid(ipNet.getPrefix())));

    ipNet.setAddress("1.2.3.255");
    ipNet.setPrefix(24);
    BOOST_TEST(std::string("1.2.3.0/24") == ipNet.toString());
    ipNet.setAddress("1.2.3.255");
    ipNet.setPrefix(27);
    BOOST_TEST(std::string("1.2.3.224/27") == ipNet.toString());

    ipNet.setAddress("fe00::aabb:ccdd");
    ipNet.setPrefix(128);
    BOOST_TEST(std::string("fe00::aabb:ccdd/128") == ipNet.toString());
    ipNet.setAddress("fe00::aabb:ffff");
    ipNet.setPrefix(120);
    BOOST_TEST(std::string("fe00::aabb:ff00/120") == ipNet.toString());
    ipNet.setAddress("fe00::aabb:ffff");
    ipNet.setPrefix(100);
    BOOST_TEST(std::string("fe00::a000:0/100") == ipNet.toString());
  }
}

BOOST_AUTO_TEST_CASE(testSettersIpAsBits)
{
  {
    const std::vector<std::tuple<size_t, std::string>> tests {
      // hex, ip
      {0x0, "0.0.0.0"},
      {0xff, "0.0.0.255"},
      {0xffff, "0.0.255.255"},
      {0xffffff, "0.255.255.255"},
      {0xffffffff, "255.255.255.255"},
      {0xff00ff00, "255.0.255.0"},
      {0x00ff00ff, "0.255.0.255"},
      {0x01010101, "1.1.1.1"},
      {0x02020202, "2.2.2.2"},
      {0x04040404, "4.4.4.4"},
      {0x08080808, "8.8.8.8"},
    };
    const size_t bits = 32;
    for (const auto& [hex, ipStr] : tests) {
      TestIpNetwork ip {ipStr, ""};
      std::bitset<bits> test(hex);
      BOOST_TEST(test == ip.getBits<bits>());
    }
  }
  {
    const std::vector<std::tuple<size_t, size_t, std::string>> tests {
      // hi, lo, ip
      {0x0, 0x0,
        "::"},
      {0x0, 0xffff,
        "::ffff"},
      {0x0, 0xffffffff,
        "::ffff:ffff"},
      {0x0, 0xffffffffffff,
        "::ffff:ffff:ffff"},
      {0x0, 0xffffffffffffffff,
        "::ffff:ffff:ffff:ffff"},
      {0xffff, 0xffffffffffffffff,
        "::ffff:ffff:ffff:ffff:ffff"},
      {0xffffffff, 0xffffffffffffffff,
        "::ffff:ffff:ffff:ffff:ffff:ffff"},
      {0xffffffffffff, 0xffffffffffffffff,
        "::ffff:ffff:ffff:ffff:ffff:ffff:ffff"},
      {0xffffffffffffffff, 0xffffffffffffffff,
        "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"},
      {0x0001000100010001, 0x0001000100010001,
        "1:1:1:1:1:1:1:1"},
      {0x0010001000100010, 0x0010001000100010,
        "10:10:10:10:10:10:10:10"},
      {0x0100010001000100, 0x0100010001000100,
        "100:100:100:100:100:100:100:100"},
      {0x1000100010001000, 0x1000100010001000,
        "1000:1000:1000:1000:1000:1000:1000:1000"},
      {0x1001100110011001, 0x1001100110011001,
        "1001:1001:1001:1001:1001:1001:1001:1001"},
    };
    const size_t bits = 128;
    for (const auto& [hi, lo, ipStr] : tests) {
      TestIpNetwork ip {ipStr, ""};
      std::bitset<bits> test(hi);
      test <<= 64;
      test |= lo;
      BOOST_TEST(test == ip.getBits<bits>());
    }
  }
}

BOOST_AUTO_TEST_CASE(testSettersMask)
{
  {
    TestIpNetwork test     {"1.2.3.4", ""};
    TestIpNetwork netmask  {"255.255.255.255", ""};
    TestIpNetwork wildmask {"0.0.0.0", ""};

    // Normal
    test.setNetmask(netmask);
    BOOST_TEST(32 == test.getPrefix());
    test.setWildcardMask(wildmask);
    BOOST_TEST(32 == test.getPrefix());

    // Guess -- always err to /0
    test.setMask(netmask);
    BOOST_TEST(0 == test.getPrefix());
    test.setMask(wildmask);
    BOOST_TEST(0 == test.getPrefix());

    // Bad
    test.setNetmask(wildmask);
    BOOST_TEST(0 == test.getPrefix());
    test.setWildcardMask(netmask);
    BOOST_TEST(0 == test.getPrefix());
  }
  { // IPv4 Wildmask and Netmask creation and test
    TestIpNetwork test       {"1.2.3.4", ""};
    const size_t octetCount  {4};
    const size_t octetSize   {8};
    const char   separator   {'.'};
    const auto&  decorator = std::dec;
    const size_t maxPrefix  {(octetCount*octetSize)};
    std::bitset<octetSize> octets[octetCount];
    for (size_t i {0}, count {0}; i < octetCount; ++i) {
      for (size_t j {0}; j < octetSize; ++j) {
        octets[i][j]=1;

        std::ostringstream ossWm, ossNm;
        for (size_t k {octetCount}; 0 != k; --k) {
          ossWm << decorator << octets[k-1].to_ullong();
          if (0 != k-1) { ossWm << separator;  }
          octets[k-1].flip();
          ossNm << decorator << octets[k-1].to_ullong();
          if (0 != k-1) { ossNm << separator;  }
          octets[k-1].flip();
        }
        TestIpNetwork wildmask {ossWm.str(), ""};
        TestIpNetwork netmask  {ossNm.str(), ""};
        size_t        prefix   {maxPrefix-(++count)};

        //LOG_INFO << prefix
        //         << " -- " << netmask
        //         << " -- " << wildmask
        //         << '\n';

        // Normal
        test.setNetmask(netmask);
        BOOST_TEST(prefix == test.getPrefix());
        test.setWildcardMask(wildmask);
        BOOST_TEST(prefix == test.getPrefix());

        // Guess -- always err to /0
        test.setMask(netmask);
        BOOST_TEST(prefix == test.getPrefix());
        test.setMask(wildmask);
        BOOST_TEST(prefix == test.getPrefix());

        // Bad
        test.setNetmask(wildmask);
        BOOST_TEST(maxPrefix == test.getPrefix());
        test.setWildcardMask(netmask);
        BOOST_TEST(maxPrefix == test.getPrefix());
      }
    }
  }
  { // IPv6 Wildmask and Netmask creation and test
    TestIpNetwork test       {"1:2:3:4:5:6:7:8", ""};
    const size_t octetCount  {8};
    const size_t octetSize   {16};
    const char   separator   {':'};
    const auto&  decorator = std::hex;
    const size_t maxPrefix  {(octetCount*octetSize)};
    std::bitset<octetSize> octets[octetCount];
    for (size_t i {0}, count {0}; i < octetCount; ++i) {
      for (size_t j {0}; j < octetSize; ++j) {
        octets[i][j]=1;

        std::ostringstream ossWm, ossNm;
        for (size_t k {octetCount}; 0 != k; --k) {
          ossWm << decorator << octets[k-1].to_ullong();
          if (0 != k-1) { ossWm << separator;  }
          octets[k-1].flip();
          ossNm << decorator << octets[k-1].to_ullong();
          if (0 != k-1) { ossNm << separator;  }
          octets[k-1].flip();
        }
        TestIpNetwork wildmask {ossWm.str(), ""};
        TestIpNetwork netmask  {ossNm.str(), ""};
        size_t        prefix   {maxPrefix-(++count)};

        //LOG_INFO << prefix
        //         << " -- " << netmask
        //         << " -- " << wildmask
        //         << '\n';

        // Normal
        test.setNetmask(netmask);
        BOOST_TEST(prefix == test.getPrefix());
        test.setWildcardMask(wildmask);
        BOOST_TEST(prefix == test.getPrefix());

        // Guess -- always err to /0
        test.setMask(netmask);
        BOOST_TEST(prefix == test.getPrefix());
        test.setMask(wildmask);
        BOOST_TEST(prefix == test.getPrefix());

        // Bad
        test.setNetmask(wildmask);
        BOOST_TEST(maxPrefix == test.getPrefix());
        test.setWildcardMask(netmask);
        BOOST_TEST(maxPrefix == test.getPrefix());
      }
    }
  }

  {
    TestIpNetwork ipNet {"1234:5678:abcd:ef01:2345:6789:0abc:def0", ""};
    std::vector<std::tuple<unsigned int, std::string>> masks {
      {128, "ffff:0000:ffff::0"},
      {128, "ffff:ffaf::0"},
      {128, "1:1:1:1:1:1:1:1"},
    };

    for (const auto& [prefix, mask] : masks) {
      ipNet.setNetmask(nmdo::IpNetwork(mask));
      BOOST_TEST(prefix == ipNet.getPrefix());
    }
  }

  {
    std::vector<std::tuple<unsigned int, std::string>> masks {
      {32, "0.0.0.27"},
      {32, "0.1.0.1"},
      {32, "1.1.1.1"},
    };

    for (const auto& [prefix, mask] : masks) {
      TestIpNetwork ipMask;
      ipMask.setAddress(mask);

      TestIpNetwork ipNet;
      bool is_contiguous = ipNet.setWildcardMask(ipMask);

      BOOST_TEST(!is_contiguous);
      BOOST_TEST(ipNet.getPrefix() == prefix);
    }
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
    ipNet.setPrefix(0);
    BOOST_TEST(ipNet.isValid());
    ipNet.setPrefix(31);
    BOOST_TEST(ipNet.isValid());
    ipNet.setPrefix(32);
    BOOST_TEST(!ipNet.isValid());
    ipNet.setPrefix(33);
    BOOST_TEST(!ipNet.isValid());
  }

  {
    TestIpNetwork ipNet;

    BOOST_TEST(!ipNet.isValid());
    ipNet.setAddress("0123::0123");
    BOOST_TEST(!ipNet.isValid());
    ipNet.setPrefix(0);
    BOOST_TEST(ipNet.isValid());
    ipNet.setPrefix(127);
    BOOST_TEST(ipNet.isValid());
    ipNet.setPrefix(128);
    BOOST_TEST(!ipNet.isValid());
    ipNet.setPrefix(129);
    BOOST_TEST(!ipNet.isValid());
  }
}
