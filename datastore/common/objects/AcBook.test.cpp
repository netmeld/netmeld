// =============================================================================
// Copyright 2017 National Technology & Engineering Solutions of Sandia, LLC
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

#include <netmeld/datastore/objects/AcBook.hpp>

namespace nmdo = netmeld::datastore::objects;

typedef std::string T;

class TestAcBook : public nmdo::AcBook<T> {
  public:
    TestAcBook() : AcBook() {};
  public:
    const std::string& getId() const
    { return id; }
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  {
    TestAcBook tab;
    BOOST_TEST(!tab.isValid());
    BOOST_TEST(tab.getId().empty());
    BOOST_TEST(tab.getName().empty());
    BOOST_TEST(tab.getData().empty());
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  // Standardized functions to test get/set
  auto fStr =
    []
    (void (TestAcBook::*x)(const std::string&),
     const std::string& (TestAcBook::*y)() const,
     const std::vector<T>& vs)
  {
    TestAcBook tab;
    for (const auto& v1 : vs) {
      (tab.*x)(v1);
      BOOST_TEST((v1 == (tab.*y)()));
    }
  };
  auto fVec =
    []
    (void (TestAcBook::*x)(const T&),
     std::set<T> (TestAcBook::*y)() const,
     const std::vector<T> vs)
  {
    TestAcBook tab;
    for (const auto& v1 : vs) {
      (tab.*x)(v1);
      auto c1 {(tab.*y)()};
      auto r1 {std::find(c1.begin(), c1.end(), v1)};
      BOOST_TEST((r1 != c1.end()));
    }
    std::set<T> vs1 {vs.begin(), vs.end()};
    auto cs {(tab.*y)()};
    BOOST_TEST(vs1 == cs);
  };

  std::vector<T> vals;
  vals = {"data", "SomeThing", "Other-ran_Dom"};
  { fStr(&TestAcBook::setId, &TestAcBook::getId, vals); };
  { fStr(&TestAcBook::setName, &TestAcBook::getName, vals); }

  { fVec(&TestAcBook::addData, &TestAcBook::getData, vals); }
}

BOOST_AUTO_TEST_CASE(testValidity)
{
  {
    TestAcBook tab;
    const T val {"someData-Wee!"};
    BOOST_TEST(!tab.isValid());
    tab.setId(val);
    BOOST_TEST(!tab.isValid());
    tab.setName(val);
    BOOST_TEST(tab.isValid());
    tab.addData(val);
    BOOST_TEST(tab.isValid());
    tab.addData(val);
    BOOST_TEST(tab.isValid());
  }
}
