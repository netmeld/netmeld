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

#include <netmeld/datastore/objects/aws/SecurityGroup.hpp>

namespace nmdoa = netmeld::datastore::objects::aws;


class TestSecurityGroup : public nmdoa::SecurityGroup {
  public:
    TestSecurityGroup() : SecurityGroup() {};

  public:
    std::string getSecurityGroupId() const
    { return sgId; }

    std::string getName() const
    { return name; }

    std::string getDescription() const
    { return description; }

    std::string getVpcId() const
    { return vpcId; }

    std::set<nmdoa::SecurityGroupRule> getRules() const
    { return rules; }
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  {
    TestSecurityGroup tobj;

    BOOST_TEST(tobj.getSecurityGroupId().empty());
    BOOST_TEST(tobj.getName().empty());
    BOOST_TEST(tobj.getDescription().empty());
    BOOST_TEST(tobj.getVpcId().empty());
    BOOST_TEST(tobj.getRules().empty());
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    TestSecurityGroup tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.setId(tv1);
    BOOST_TEST(tv1 == tobj.getSecurityGroupId());
  }
  {
    TestSecurityGroup tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.setName(tv1);
    BOOST_TEST(tv1 == tobj.getName());
  }
  {
    TestSecurityGroup tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.setDescription(tv1);
    BOOST_TEST(tv1 == tobj.getDescription());
  }
  {
    TestSecurityGroup tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.setVpcId(tv1);
    BOOST_TEST(tv1 == tobj.getVpcId());
  }
  {
    TestSecurityGroup tobj;

    nmdoa::SecurityGroupRule tv1;
    tobj.addRule(tv1);
    auto trv1 = tobj.getRules();
    BOOST_TEST(0 == trv1.size());

    tv1.setProtocol("a");
    tobj.addRule(tv1);
    trv1 = tobj.getRules();
    BOOST_TEST(1 == trv1.size());
    BOOST_TEST(trv1.contains(tv1));
  }
}

BOOST_AUTO_TEST_CASE(testValidity)
{
  {
    TestSecurityGroup tobj;

    const std::string v1 {"aBc1@3"};
    
    BOOST_TEST(!tobj.isValid());
    tobj.setId(v1);
    BOOST_TEST(tobj.isValid());
  }
}
