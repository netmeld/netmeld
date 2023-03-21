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

#include <netmeld/datastore/objects/aws/CidrBlock.hpp>

namespace nmdoa = netmeld::datastore::objects::aws;


class TestCidrBlock : public nmdoa::CidrBlock {
  public:
    TestCidrBlock() : CidrBlock() {};
    explicit TestCidrBlock(const std::string& _cidr) : CidrBlock(_cidr) {};

  public:
    std::string getState() const
    { return state; }

    std::string getDescription() const
    { return description; }

    std::set<std::string> getAliases() const
    { return aliases; }
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  {
    TestCidrBlock tobj;

    BOOST_TEST(tobj.getCidrBlock().empty());
    BOOST_TEST(tobj.getState().empty());
  }
  {
    const std::string tv1 {"1.2.3.4/24"};
    TestCidrBlock tobj {tv1};

    BOOST_TEST(tv1 == tobj.getCidrBlock());
    BOOST_TEST(tobj.getState().empty());
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    TestCidrBlock tobj;

    const std::string tv1 {"1.2.3.4/24"};
    tobj.setCidrBlock(tv1);
    BOOST_TEST(tv1 == tobj.getCidrBlock());
  }
  {
    TestCidrBlock tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.setState(tv1);
    BOOST_TEST(tv1 == tobj.getState());
  }
  {
    TestCidrBlock tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.setDescription(tv1);
    BOOST_TEST(tv1 == tobj.getDescription());
  }
}

BOOST_AUTO_TEST_CASE(testValidity)
{
  {
    TestCidrBlock tobj;

    const std::string tv1 {"1.2.3.4/24"};

    BOOST_TEST(!tobj.isValid());
    tobj.setCidrBlock(tv1);
    BOOST_TEST(tobj.isValid());
  }
}
