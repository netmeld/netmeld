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

#include <netmeld/datastore/objects/aws/RouteTable.hpp>

namespace nmdoa = netmeld::datastore::objects::aws;


class TestRouteTable : public nmdoa::RouteTable {
  public:
    TestRouteTable() : RouteTable() {};

  public:
    std::string getRouteTableId() const
    { return routeTableId; }

    std::string getVpcId() const
    { return vpcId; }

    std::set<std::string> getAssociations() const
    { return associations; }

    std::set<nmdoa::Route> getRoutes() const
    { return routes; }
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  {
    TestRouteTable tobj;

    BOOST_TEST(tobj.getRouteTableId().empty());
    BOOST_TEST(tobj.getVpcId().empty());
    BOOST_TEST(tobj.getAssociations().empty());
    BOOST_TEST(tobj.getRoutes().empty());
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    TestRouteTable tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.setId(tv1);
    BOOST_TEST(tv1 == tobj.getRouteTableId());
  }
  {
    TestRouteTable tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.setVpcId(tv1);
    BOOST_TEST(tv1 == tobj.getVpcId());
  }
  {
    TestRouteTable tobj;

    tobj.addAssociation("");
    auto trv1 = tobj.getAssociations();
    BOOST_TEST(0 == trv1.size());

    const std::string tv1 {"aBc1@3"};
    tobj.addAssociation(tv1);
    trv1 = tobj.getAssociations();
    BOOST_TEST(1 == trv1.size());
    BOOST_TEST(trv1.contains(tv1));
  }
  {
    TestRouteTable tobj;

    nmdoa::Route tv1;
    tobj.addRoute(tv1);
    auto trv1 = tobj.getRoutes();
    BOOST_TEST(0 == trv1.size());

    tv1.setId("aBc1@3");
    tobj.addRoute(tv1);
    trv1 = tobj.getRoutes();
    BOOST_TEST(1 == trv1.size());
    BOOST_TEST(trv1.contains(tv1));
  }
}

BOOST_AUTO_TEST_CASE(testValidity)
{
  {
    TestRouteTable tobj;

    const std::string tv1 {"aBc1@3"};

    BOOST_TEST(!tobj.isValid());
    tobj.setId(tv1);
    BOOST_TEST(tobj.isValid());
  }
}
