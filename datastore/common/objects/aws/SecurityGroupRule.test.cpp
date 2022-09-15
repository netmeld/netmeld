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

#include <netmeld/datastore/objects/aws/SecurityGroupRule.hpp>

namespace nmdoa = netmeld::datastore::objects::aws;


class TestSecurityGroupRule : public nmdoa::SecurityGroupRule {
  public:
    TestSecurityGroupRule() : SecurityGroupRule() {};

  public:
    std::string getProtocol() const
    { return protocol; }

    std::int32_t getFromPort() const
    { return fromPort; }

    std::int32_t getToPort() const
    { return toPort; }

    std::set<nmdoa::CidrBlock> getCidrBlocks() const
    { return cidrBlocks; }

    std::set<std::string> getNonCidr() const
    { return nonCidrs; }

    bool getEgress() const
    { return egress; }
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  {
    TestSecurityGroupRule tobj;

    BOOST_TEST(tobj.getProtocol().empty());
    BOOST_TEST(INT32_MIN == tobj.getFromPort());
    BOOST_TEST(INT32_MIN == tobj.getToPort());
    BOOST_TEST(tobj.getCidrBlocks().empty());
    BOOST_TEST(tobj.getNonCidr().empty());
    BOOST_TEST(!tobj.getEgress());
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    TestSecurityGroupRule tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.setProtocol(tv1);
    BOOST_TEST(tv1 == tobj.getProtocol());
  }
  {
    TestSecurityGroupRule tobj;

    const std::int32_t tv1 {123};
    tobj.setFromPort(tv1);
    BOOST_TEST(tv1 == tobj.getFromPort());
  }
  {
    TestSecurityGroupRule tobj;

    const std::int32_t tv1 {123};
    tobj.setToPort(tv1);
    BOOST_TEST(tv1 == tobj.getToPort());
  }
  {
    TestSecurityGroupRule tobj;

    tobj.addCidrBlock("");
    auto trv1 = tobj.getCidrBlocks();
    BOOST_TEST(0 == trv1.size());

    const std::string tv1 {"1.2.3.0/24"};
    nmdoa::CidrBlock tv2 {tv1};
    tobj.addCidrBlock(tv1);
    trv1 = tobj.getCidrBlocks();
    BOOST_TEST(1 == trv1.size());
    BOOST_TEST(trv1.contains(tv2));
  }
  {
    TestSecurityGroupRule tobj;

    tobj.addNonCidr("");
    auto trv1 = tobj.getNonCidr();
    BOOST_TEST(0 == trv1.size());

    const std::string tv1 {"aBc1@3"};
    tobj.addNonCidr(tv1);
    trv1 = tobj.getNonCidr();
    BOOST_TEST(1 == trv1.size());
    BOOST_TEST(trv1.contains(tv1));
  }
  {
    TestSecurityGroupRule tobj;

    BOOST_TEST(!tobj.getEgress());
    tobj.setEgress();
    BOOST_TEST(tobj.getEgress());
  }
}

BOOST_AUTO_TEST_CASE(testValidity)
{
  {
    TestSecurityGroupRule tobj;

    const std::string v1 {"aBc1@3"};
    std::int32_t v2 {0};
    
    BOOST_TEST(!tobj.isValid());
    tobj.setProtocol(v1);
    BOOST_TEST(!tobj.isValid());
    tobj.setFromPort(v2);
    BOOST_TEST(!tobj.isValid());
    tobj.setToPort(v2);
    BOOST_TEST(!tobj.isValid());
    tobj.addCidrBlock("1.2.3.0/24");
    BOOST_TEST(tobj.isValid());
  }
  {
    TestSecurityGroupRule tobj;

    const std::string v1 {"aBc1@3"};
    std::int32_t v2 {0};
    
    BOOST_TEST(!tobj.isValid());
    tobj.setProtocol(v1);
    BOOST_TEST(!tobj.isValid());
    tobj.setFromPort(v2);
    BOOST_TEST(!tobj.isValid());
    tobj.setToPort(v2);
    BOOST_TEST(!tobj.isValid());
    tobj.addCidrBlock(v1);
    BOOST_TEST(tobj.isValid());
  }
}
