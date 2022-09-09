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

#include <netmeld/datastore/objects/aws/NetworkInterface.hpp>

namespace nmdoa = netmeld::datastore::objects::aws;


class TestNetworkInterface : public nmdoa::NetworkInterface {
  public:
    TestNetworkInterface() : NetworkInterface() {};

  public:
    std::string getNetworkInterfaceId() const
    { return interfaceId; }

    std::string getDescription() const
    { return description; }

    std::string getType() const
    { return type; }

    bool getSourceDestinationCheck() const
    { return sourceDestinationCheck; }

    std::string getStatus() const
    { return status; }

    bool getIsUp() const
    { return isUp; }

    std::string getSubnetId() const
    { return subnetId; }

    std::string getVpcId() const
    { return vpcId; }

    nmdoa::Attachment getAttachment() const
    { return attachment; }

    std::set<std::string> getSecurityGroups() const
    { return securityGroups; }

    nmdo::MacAddress getMacAddress() const
    { return macAddr; }
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  {
    TestNetworkInterface tobj;

    BOOST_TEST(tobj.getNetworkInterfaceId().empty());
    BOOST_TEST(tobj.getType().empty());
    BOOST_TEST(tobj.getDescription().empty());
    BOOST_TEST(!tobj.getSourceDestinationCheck());
    BOOST_TEST(tobj.getStatus().empty());
    BOOST_TEST(!tobj.getIsUp());
    BOOST_TEST(tobj.getSubnetId().empty());
    BOOST_TEST(tobj.getVpcId().empty());
    BOOST_TEST(!tobj.getAttachment().isValid());
    BOOST_TEST(tobj.getSecurityGroups().empty());
    BOOST_TEST(!tobj.getMacAddress().isValid());
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    TestNetworkInterface tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.setId(tv1);
    BOOST_TEST(tv1 == tobj.getNetworkInterfaceId());
  }
  {
    TestNetworkInterface tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.setDescription(tv1);
    BOOST_TEST(tv1 == tobj.getDescription());
  }
  {
    TestNetworkInterface tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.setType(tv1);
    BOOST_TEST(tv1 == tobj.getType());
  }
  {
    TestNetworkInterface tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.setStatus(tv1);
    BOOST_TEST(tv1 == tobj.getStatus());
    BOOST_TEST(!tobj.getIsUp());
    const std::string tv2 {"in-use"};
    tobj.setStatus(tv2);
    BOOST_TEST(tv2 == tobj.getStatus());
    BOOST_TEST(tobj.getIsUp());
  }
  {
    TestNetworkInterface tobj;

    BOOST_TEST(!tobj.getSourceDestinationCheck());
    tobj.enableSourceDestinationCheck();
    BOOST_TEST(tobj.getSourceDestinationCheck());
    tobj.disableSourceDestinationCheck();
    BOOST_TEST(!tobj.getSourceDestinationCheck());
  }
  {
    TestNetworkInterface tobj;

    const nmdoa::Attachment tv1;
    tobj.setAttachment(tv1);
    BOOST_TEST(tv1 == tobj.getAttachment());
  }
  {
    TestNetworkInterface tobj;

    const std::string tv1 {"00:11:22:33:44:55"};
    tobj.setMacAddress(tv1);
    nmdo::MacAddress trv1 {tv1};
    BOOST_TEST(trv1 == tobj.getMacAddress());
  }
  {
    TestNetworkInterface tobj;

    const std::string tv1 {"1.2.3.4/24"};
    nmdo::IpAddress tv2 {tv1};

    tobj.addIpAddress(tv2);
    auto trv2 = tobj.getMacAddress().getIpAddresses();
    BOOST_TEST(1 == trv2.size());
    BOOST_TEST(trv2.contains(tv2));
  }
  {
    TestNetworkInterface tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.setSubnetId(tv1);
    BOOST_TEST(tv1 == tobj.getSubnetId());
  }
  {
    TestNetworkInterface tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.setVpcId(tv1);
    BOOST_TEST(tv1 == tobj.getVpcId());
  }
  {
    TestNetworkInterface tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.addSecurityGroup(tv1);
    const auto trv1 = tobj.getSecurityGroups();
    BOOST_TEST(1 == trv1.size());
    BOOST_TEST(trv1.contains(tv1));
  }
}

BOOST_AUTO_TEST_CASE(testValidity)
{
  {
    TestNetworkInterface tobj;

    const std::string tv1 {"aBc1@3"};
    
    BOOST_TEST(!tobj.isValid());
    tobj.setId(tv1);
    BOOST_TEST(tobj.isValid());
  }
}
