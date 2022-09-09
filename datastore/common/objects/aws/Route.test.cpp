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

#include <netmeld/datastore/objects/aws/Route.hpp>

namespace nmdoa = netmeld::datastore::objects::aws;


class TestRoute : public nmdoa::Route {
  public:
    TestRoute() : Route() {};

  public:
    std::string getTypeId() const
    { return typeId; }

    std::string getState() const
    { return state; }

    std::set<nmdo::IpNetwork> getCidrBlocks() const
    { return cidrBlocks; }
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  {
    TestRoute tobj;

    BOOST_TEST(tobj.getTypeId().empty());
    BOOST_TEST(tobj.getState().empty());
    BOOST_TEST(tobj.getCidrBlocks().empty());
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    TestRoute tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.setId(tv1);
    BOOST_TEST(tv1 == tobj.getTypeId());
  }
  {
    TestRoute tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.setState(tv1);
    BOOST_TEST(tv1 == tobj.getState());
  }
  {
    TestRoute tobj;
    
    const std::string tv1 {"1.2.3.0/24"};
    nmdo::IpNetwork tv2 {tv1};
    tv2.setReason("AWS Route CidrBlock");
    tobj.addCidrBlock(tv1);
    const auto trv1 = tobj.getCidrBlocks();
    BOOST_TEST(1 == trv1.size());
    BOOST_TEST(trv1.contains(tv2));
    // TODO fix unless use nmdoa::CidrBlock
  }
}

BOOST_AUTO_TEST_CASE(testValidity)
{
  {
    TestRoute tobj;

    const std::string tv1 {"aBc1@3"};
    
    BOOST_TEST(!tobj.isValid());
    tobj.setId(tv1);
    BOOST_TEST(tobj.isValid());
  }
}
