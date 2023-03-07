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

#include <netmeld/datastore/objects/aws/TransitGatewayAttachment.hpp>

namespace nmdoa = netmeld::datastore::objects::aws;


class TestTransitGatewayAttachment : public nmdoa::TransitGatewayAttachment {
  public:
    TestTransitGatewayAttachment() : TransitGatewayAttachment() {};

  public:
    std::string getTgwAttachmentId() const
    { return tgwAttachmentId; }
    std::string getTgwId() const
    { return tgwId; }
    std::string getState() const
    { return state; }
    std::string getTgwOwnerId() const
    { return tgwOwnerId; }
    std::string getResourceId() const
    { return resourceId; }
    std::string getResourceOwnerId() const
    { return resourceOwnerId; }
    std::string getResourceType() const
    { return resourceType; }
    std::string getAssociationState() const
    { return associationState; }
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  {
    TestTransitGatewayAttachment tobj;

    BOOST_TEST(tobj.getTgwAttachmentId().empty());
    BOOST_TEST(tobj.getTgwId().empty());
    BOOST_TEST(tobj.getState().empty());
    BOOST_TEST(tobj.getTgwOwnerId().empty());
    BOOST_TEST(tobj.getResourceId().empty());
    BOOST_TEST(tobj.getResourceOwnerId().empty());
    BOOST_TEST(tobj.getResourceType().empty());
    BOOST_TEST(tobj.getAssociationState().empty());
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    TestTransitGatewayAttachment tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.setAssociationState(tv1);
    BOOST_TEST(tv1 == tobj.getAssociationState());
  }
  {
    TestTransitGatewayAttachment tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.setResourceId(tv1);
    BOOST_TEST(tv1 == tobj.getResourceId());
  }
  {
    TestTransitGatewayAttachment tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.setResourceOwnerId(tv1);
    BOOST_TEST(tv1 == tobj.getResourceOwnerId());
  }
  {
    TestTransitGatewayAttachment tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.setResourceType(tv1);
    BOOST_TEST(tv1 == tobj.getResourceType());
  }
  {
    TestTransitGatewayAttachment tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.setState(tv1);
    BOOST_TEST(tv1 == tobj.getState());
  }
  {
    TestTransitGatewayAttachment tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.setTgwAttachmentId(tv1);
    BOOST_TEST(tv1 == tobj.getTgwAttachmentId());
  }
  {
    TestTransitGatewayAttachment tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.setTgwId(tv1);
    BOOST_TEST(tv1 == tobj.getTgwId());
  }
  {
    TestTransitGatewayAttachment tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.setTgwOwnerId(tv1);
    BOOST_TEST(tv1 == tobj.getTgwOwnerId());
  }
}

BOOST_AUTO_TEST_CASE(testValidity)
{
  {
    TestTransitGatewayAttachment tobj;

    const std::string tv1 {"aBc1@3"};

    BOOST_TEST(!tobj.isValid());
    tobj.setTgwAttachmentId(tv1);
    BOOST_TEST(!tobj.isValid());
    tobj.setTgwId(tv1);
    BOOST_TEST(!tobj.isValid());
    tobj.setState(tv1);
    BOOST_TEST(tobj.isValid());
  }
}
