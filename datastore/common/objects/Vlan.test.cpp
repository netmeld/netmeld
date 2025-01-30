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

#include <netmeld/datastore/objects/Vlan.hpp>

namespace nmdo = netmeld::datastore::objects;


class TestVlan : public nmdo::Vlan {
  public:
    using Vlan::Vlan;

    using Vlan::description;
    using Vlan::ipNet;
    // has accessor
    //using Vlan::vlanId;
};


BOOST_AUTO_TEST_CASE(testConstructors)
{
  {
    TestVlan vlan;
    BOOST_TEST(UINT16_MAX == vlan.getVlanId());
  }

  {
    uint16_t id {10U};
    std::string desc {"Some Description"};
    TestVlan vlan {id, desc};
    BOOST_TEST(id == vlan.getVlanId());
    BOOST_TEST("Some Description" == vlan.description);
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  TestVlan vlan;
  uint16_t id {123U};
  std::string desc {"Desc"};
  nmdo::IpNetwork ipNet {"10.1.0.0/24"};

  // Setters
  vlan.setDescription(desc);
  vlan.setId(id);
  vlan.setIpNet(ipNet);

  BOOST_TEST(id == vlan.getVlanId());
  BOOST_TEST("Desc" == vlan.description);
  BOOST_TEST(ipNet == vlan.ipNet);
}

BOOST_AUTO_TEST_CASE(testValidity)
{
  TestVlan vlan;

  vlan.setId(0U); // min
  BOOST_TEST(vlan.isValid());

  vlan.setId(4095U); // max
  BOOST_TEST(vlan.isValid());

  vlan.setId(4096U);
  BOOST_TEST(!vlan.isValid());
}
