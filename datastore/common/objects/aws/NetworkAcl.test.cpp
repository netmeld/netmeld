// =============================================================================
// Copyright 2022 National Technology & Engineering Solutions of Sandia, LLC
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

#include <netmeld/datastore/objects/aws/NetworkAcl.hpp>

namespace nmdoa = netmeld::datastore::objects::aws;


class TestNetworkAcl : public nmdoa::NetworkAcl {
  public:
    TestNetworkAcl() : NetworkAcl() {};

  public:
    std::string getNetworkAclId() const
    { return naclId; }

    std::string getVpcId() const
    { return vpcId; }

    std::set<std::string> getSubnetIds() const
    { return subnetIds; }

    std::set<nmdoa::NetworkAclRule> getRules() const
    { return rules; }
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  {
    TestNetworkAcl tobj;

    BOOST_TEST(tobj.getNetworkAclId().empty());
    BOOST_TEST(tobj.getVpcId().empty());
    BOOST_TEST(tobj.getSubnetIds().empty());
    BOOST_TEST(tobj.getRules().empty());
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    TestNetworkAcl tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.setId(tv1);
    BOOST_TEST(tv1 == tobj.getNetworkAclId());
  }
  {
    TestNetworkAcl tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.setVpcId(tv1);
    BOOST_TEST(tv1 == tobj.getVpcId());
  }
  {
    TestNetworkAcl tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.addSubnetId(tv1);
    const auto sis = tobj.getSubnetIds();
    BOOST_TEST(1 == sis.size());
    BOOST_TEST(sis.contains(tv1));
  }
  {
    TestNetworkAcl tobj;

    const nmdoa::NetworkAclRule tv1;
    tobj.addRule(tv1);
    const auto rs = tobj.getRules();
    BOOST_TEST(1 == rs.size());
    BOOST_TEST(rs.contains(tv1));
  }
}

BOOST_AUTO_TEST_CASE(testValidity)
{
  {
    TestNetworkAcl tobj;

    const std::string tv1  {"aBc1@3"};
    
    BOOST_TEST(!tobj.isValid());
    tobj.setId(tv1);
    BOOST_TEST(tobj.isValid());
  }
}
