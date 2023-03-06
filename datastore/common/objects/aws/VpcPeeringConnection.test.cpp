// =============================================================================
// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
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

#include <netmeld/datastore/objects/aws/VpcPeeringConnection.hpp>

namespace nmdoa = netmeld::datastore::objects::aws;


class TestVpcPeeringConnection : public nmdoa::VpcPeeringConnection {
  public:
    TestVpcPeeringConnection() : VpcPeeringConnection() {};

  public:
    std::string getId() const
    { return pcxId; }
    std::string getStatusCode() const
    { return statusCode; }
    std::string getStatusMessage() const
    { return statusMessage; }
    nmdoa::Vpc getAccepter() const
    { return accepter; }
    nmdoa::Vpc getRequester() const
    { return requester; }
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  {
    TestVpcPeeringConnection tobj;

    BOOST_TEST(tobj.getId().empty());
    BOOST_TEST(tobj.getStatusCode().empty());
    BOOST_TEST(tobj.getStatusMessage().empty());
    BOOST_TEST(!tobj.getAccepter().isValid());
    BOOST_TEST(!tobj.getRequester().isValid());
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    TestVpcPeeringConnection tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.setId(tv1);
    BOOST_TEST(tv1 == tobj.getId());
  }
  {
    TestVpcPeeringConnection tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.setStatus(tv1, tv1+tv1);
    BOOST_TEST(tv1 == tobj.getStatusCode());
    BOOST_TEST((tv1+tv1) == tobj.getStatusMessage());
  }
  {
    TestVpcPeeringConnection tobj;

    const nmdoa::Vpc tv1;
    tobj.setAccepter(tv1);
    BOOST_TEST(tv1 == tobj.getAccepter());
  }
  {
    TestVpcPeeringConnection tobj;

    const nmdoa::Vpc tv1;
    tobj.setRequester(tv1);
    BOOST_TEST(tv1 == tobj.getRequester());
  }
}

BOOST_AUTO_TEST_CASE(testValidity)
{
  {
    TestVpcPeeringConnection tobj;

    const std::string tv1 {"aBc1@3"};
    const nmdoa::Vpc  tv2;
    nmdoa::Vpc  tv3;
    tv3.setId(tv1);
    tv3.setOwnerId(tv1+tv1);

    BOOST_TEST(!tv2.isValid());
    BOOST_TEST(!tobj.isValid());

    tobj.setId(tv1);
    BOOST_TEST(!tobj.isValid());
    tobj.setAccepter(tv2);
    tobj.setRequester(tv2);
    BOOST_TEST(!tobj.isValid());
    tobj.setAccepter(tv3);
    tobj.setRequester(tv3);
    BOOST_TEST(tobj.isValid());
  }
}
