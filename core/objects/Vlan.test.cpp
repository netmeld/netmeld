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

#include <netmeld/core/objects/Vlan.hpp>

namespace nmco = netmeld::core::objects;


class TestVlan : public nmco::Vlan {
  public:
    TestVlan() : Vlan() {};
    TestVlan(uint16_t _id, const std::string& _desc) : Vlan(_id, _desc) {};

  public:
    std::string getDescription()
    { return description; }

    nmco::IpNetwork getIpNet()
    { return ipNet; }
};


BOOST_AUTO_TEST_CASE(testConstructors)
{
  {
    TestVlan vlan;
    BOOST_CHECK_EQUAL(UINT16_MAX, vlan.getVlanId());
  }

  {
    uint16_t id {10U};
    std::string desc {"Some Description"};
    TestVlan vlan {id, desc};
    BOOST_CHECK_EQUAL(id, vlan.getVlanId());
    BOOST_CHECK_EQUAL("some description", vlan.getDescription());
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  TestVlan vlan;
  uint16_t id {123U};
  std::string desc {"Desc"};
  nmco::IpNetwork ipNet {"10.1.0.0/24"};

  // Setters
  vlan.setDescription(desc);
  vlan.setId(id);
  vlan.setIpNet(ipNet);

  BOOST_CHECK_EQUAL(id, vlan.getVlanId());
  BOOST_CHECK_EQUAL("desc", vlan.getDescription());
  BOOST_CHECK_EQUAL(ipNet, vlan.getIpNet());
}

BOOST_AUTO_TEST_CASE(testValidity)
{
  TestVlan vlan;

  vlan.setId(0U); // min
  BOOST_CHECK(vlan.isValid());

  vlan.setId(4095U); // max
  BOOST_CHECK(vlan.isValid());

  vlan.setId(4096U);
  BOOST_CHECK(!vlan.isValid());
}
