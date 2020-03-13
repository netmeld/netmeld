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

#include <netmeld/core/objects/AcRule.hpp>

namespace nmco = netmeld::core::objects;


class TestAcRule : public nmco::AcRule {
  public:
    TestAcRule() : AcRule() {};
  public:
    size_t getRuleId() const
    { return id; }

    const std::string& getRuleDescription() const
    { return description; }

    bool isEnabled() const
    { return enabled; }
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  {
    TestAcRule tar;
    BOOST_TEST(!tar.isValid());
    BOOST_TEST(0 == tar.getRuleId());
    BOOST_TEST(tar.getSrcId().empty());
    BOOST_TEST(tar.getRuleDescription().empty());
    BOOST_TEST(tar.getSrcs().empty());
    BOOST_TEST(tar.getSrcIfaces().empty());
    BOOST_TEST(tar.getDstId().empty());
    BOOST_TEST(tar.getDsts().empty());
    BOOST_TEST(tar.getDstIfaces().empty());
    BOOST_TEST(tar.getServices().empty());
    BOOST_TEST(tar.getActions().empty());
    BOOST_TEST(tar.isEnabled());
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    TestAcRule tar;
    std::vector<size_t> vs {0, SIZE_MAX};
    for (const auto& v1 : vs) {
      tar.setRuleId(v1);
      BOOST_TEST((v1 == tar.getRuleId()));
    }
  }


  // Standardized functions to test get/set
  auto fStr =
    []
    (void (TestAcRule::*x)(const std::string&),
     const std::string& (TestAcRule::*y)() const,
     const std::vector<std::string> vs)
  {
    TestAcRule tar;
    for (const auto& v1 : vs) {
      (tar.*x)(v1);
      BOOST_TEST((v1 == (tar.*y)()));
    }
  };
  auto fVec =
    []
    (void (TestAcRule::*x)(const std::string&),
     const std::vector<std::string>& (TestAcRule::*y)() const,
     const std::vector<std::string> vs)
  {
    TestAcRule tar;
    for (const auto& v1 : vs) {
      (tar.*x)(v1);
      auto c1 {(tar.*y)()};
      auto r1 {std::find(c1.begin(), c1.end(), v1)};
      BOOST_TEST((r1 != c1.end()));
    }
    auto cs {(tar.*y)()};
    BOOST_TEST(vs == cs);
  };

  std::vector<std::string> vals;
  vals = {"data", "SomeThing", "Other-ran_Dom"};
  { fStr(&TestAcRule::setRuleDescription, &TestAcRule::getRuleDescription, vals); };
  { fStr(&TestAcRule::setSrcId, &TestAcRule::getSrcId, vals); };
  { fVec(&TestAcRule::addSrc, &TestAcRule::getSrcs, vals); }
  { fVec(&TestAcRule::addSrcIface, &TestAcRule::getSrcIfaces, vals); }

  { fStr(&TestAcRule::setDstId, &TestAcRule::getDstId, vals); };
  { fVec(&TestAcRule::addDst, &TestAcRule::getDsts, vals); }
  { fVec(&TestAcRule::addDstIface, &TestAcRule::getDstIfaces, vals); }

  { fVec(&TestAcRule::addAction, &TestAcRule::getActions, vals); }

  vals = {"tcp:1:2", "tpc-udp:50:8", "icmp:10"};
  { fVec(&TestAcRule::addService, &TestAcRule::getServices, vals); }
}

BOOST_AUTO_TEST_CASE(testValidity)
{
  // Only valid if everything set
  {
    TestAcRule tar;
    const std::string val {"someData-Wee!"};
    BOOST_TEST(!tar.isValid());
    tar.setRuleId(SIZE_MAX);
    BOOST_TEST(!tar.isValid());
    tar.setSrcId(val);
    BOOST_TEST(!tar.isValid());
    tar.addSrc(val);
    BOOST_TEST(!tar.isValid());
    tar.addSrcIface(val);
    BOOST_TEST(!tar.isValid());
    tar.setDstId(val);
    BOOST_TEST(!tar.isValid());
    tar.addDst(val);
    BOOST_TEST(!tar.isValid());
    tar.addDstIface(val);
    BOOST_TEST(!tar.isValid());
    tar.addAction(val);
    BOOST_TEST(!tar.isValid());
    tar.addService(val);
    BOOST_TEST(tar.isValid());
    
    tar.setRuleDescription(val);
    BOOST_TEST(tar.isValid());
  }
}
