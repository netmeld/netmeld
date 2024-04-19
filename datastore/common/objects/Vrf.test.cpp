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

#include <netmeld/datastore/objects/Vrf.hpp>

namespace nmdo = netmeld::datastore::objects;


class TestVrf : public nmdo::Vrf {
  public:
    TestVrf() : Vrf() {};
    explicit TestVrf(const std::string& _vrfId) : Vrf(_vrfId) {};

  public:
    std::string getId()
    { return vrfId; }
    std::vector<std::string> getIfaces()
    { return ifaces; }
};


BOOST_AUTO_TEST_CASE(testConstructors)
{
  {
    TestVrf vrf;
    BOOST_CHECK_EQUAL("", vrf.getId());
    BOOST_CHECK_EQUAL(0, vrf.getIfaces().size());
  }

  {
    std::string id {"Some Vrf"};
    TestVrf vrf {id};
    BOOST_CHECK_EQUAL(id, vrf.getId());
    BOOST_CHECK_EQUAL(0, vrf.getIfaces().size());
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  TestVrf vrf;
  std::string id {"Some Vrf"};

  // Setters
  vrf.setId(id);
  vrf.addIface("eth0");
  vrf.addIface("eth1");

  BOOST_CHECK_EQUAL(id, vrf.getId());
  BOOST_CHECK_EQUAL(2, vrf.getIfaces().size());
  BOOST_CHECK_EQUAL("eth0", vrf.getIfaces().at(0));
  BOOST_CHECK_EQUAL("eth1", vrf.getIfaces().at(1));
}
