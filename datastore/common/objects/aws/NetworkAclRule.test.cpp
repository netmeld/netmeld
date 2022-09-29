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

#include <netmeld/datastore/objects/aws/NetworkAclRule.hpp>

namespace nmdoa = netmeld::datastore::objects::aws;


class TestNetworkAclRule : public nmdoa::NetworkAclRule {
  public:
    TestNetworkAclRule() : NetworkAclRule() {};

  public:
    std::int32_t getNumber() const
    { return number; }

    std::string getAction() const
    { return action; }

    std::string getProtocol() const
    { return protocol; }

    std::int32_t getFromOrType() const
    { return fromOrType; }

    std::int32_t getToOrCode() const
    { return toOrCode; }

    std::set<nmdoa::CidrBlock> getCidrBlocks() const
    { return cidrBlocks; }

    bool getEgress() const
    { return egress; }

    bool getPortRange() const
    { return portRange; }

    bool getTypeCode() const
    { return typeCode; }
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  {
    TestNetworkAclRule tobj;

    BOOST_TEST(INT32_MIN == tobj.getNumber());
    BOOST_TEST(tobj.getAction().empty());
    BOOST_TEST(tobj.getProtocol().empty());
    BOOST_TEST(INT32_MIN == tobj.getFromOrType());
    BOOST_TEST(INT32_MIN == tobj.getToOrCode());
    BOOST_TEST(tobj.getCidrBlocks().empty());
    BOOST_TEST(!tobj.getEgress());
    BOOST_TEST(!tobj.getPortRange());
    BOOST_TEST(!tobj.getTypeCode());
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    TestNetworkAclRule tobj;

    std::int32_t tv1 {-1};
    tobj.setNumber(tv1);
    BOOST_TEST(tv1 == tobj.getNumber());
    tv1 = 1; // Doc lowest value rule number
    tobj.setNumber(tv1);
    BOOST_TEST(tv1 == tobj.getNumber());
    tv1 = 32766; // Doc highest value rule number
    tobj.setNumber(tv1);
    BOOST_TEST(tv1 == tobj.getNumber());
    tv1 = INT32_MAX;
    tobj.setNumber(tv1);
    BOOST_TEST(tv1 == tobj.getNumber());
  }
  {
    TestNetworkAclRule tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.setAction(tv1);
    BOOST_TEST(tv1 == tobj.getAction());
  }
  {
    TestNetworkAclRule tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.setProtocol(tv1);
    BOOST_TEST(tv1 == tobj.getProtocol());
  }
  {
    TestNetworkAclRule tobj;

    const std::int32_t tv1 {123};
    BOOST_TEST((!tobj.getPortRange() && !tobj.getTypeCode()));
    tobj.setFromPort(tv1);
    BOOST_TEST(tv1 == tobj.getFromOrType());
    BOOST_TEST((tobj.getPortRange() && !tobj.getTypeCode()));
  }
  {
    TestNetworkAclRule tobj;

    const std::int32_t tv1 {123};
    BOOST_TEST((!tobj.getPortRange() && !tobj.getTypeCode()));
    tobj.setToPort(tv1);
    BOOST_TEST(tv1 == tobj.getToOrCode());
    BOOST_TEST((tobj.getPortRange() && !tobj.getTypeCode()));
  }
  {
    TestNetworkAclRule tobj;

    const std::int32_t tv1 {123};
    BOOST_TEST((!tobj.getPortRange() && !tobj.getTypeCode()));
    tobj.setIcmpType(tv1);
    BOOST_TEST(tv1 == tobj.getFromOrType());
    BOOST_TEST((!tobj.getPortRange() && tobj.getTypeCode()));
  }
  {
    TestNetworkAclRule tobj;

    const std::int32_t tv1 {123};
    BOOST_TEST((!tobj.getPortRange() && !tobj.getTypeCode()));
    tobj.setIcmpCode(tv1);
    BOOST_TEST(tv1 == tobj.getToOrCode());
    BOOST_TEST((!tobj.getPortRange() && tobj.getTypeCode()));
  }
  {
    TestNetworkAclRule tobj;

    BOOST_TEST(!tobj.getEgress());
    tobj.setEgress();
    BOOST_TEST(tobj.getEgress());
  }
  {
    TestNetworkAclRule tobj;

    tobj.addCidrBlock("");
    auto trv1 = tobj.getCidrBlocks();
    BOOST_TEST(0 == trv1.size());

    const std::string tv1 {"1.2.3.4/24"};
    nmdoa::CidrBlock  tv2 {tv1};
    tobj.addCidrBlock(tv1);
    trv1 = tobj.getCidrBlocks();
    BOOST_TEST(1 == trv1.size());
    BOOST_TEST(trv1.contains(tv2));
  }
}

BOOST_AUTO_TEST_CASE(testValidity)
{
  {
    TestNetworkAclRule tobj;

    const std::int32_t tv1 {123};
    const std::string  tv2 {"aBc1@3"};

    BOOST_TEST(!tobj.isValid());
    tobj.setNumber(tv1);
    BOOST_TEST(!tobj.isValid());
    tobj.setAction(tv2);
    BOOST_TEST(!tobj.isValid());
    tobj.setProtocol(tv2);
    BOOST_TEST(!tobj.isValid());
    tobj.addCidrBlock("1.2.3.4/24");
    BOOST_TEST(tobj.isValid());
  }
}
