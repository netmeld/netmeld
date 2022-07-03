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

#include <netmeld/datastore/objects/ToolObservations.hpp>

namespace nmdo = netmeld::datastore::objects;


class TestToolObservations : public nmdo::ToolObservations {
  public:
    TestToolObservations() : ToolObservations() {};

  public:
    std::set<std::string> getNotables() const
    { return notables; }
    std::set<std::string> getUnsupportedFeatures() const
    { return unsupportedFeatures; }
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  {
    TestToolObservations tto;
    BOOST_TEST(tto.getNotables().empty());
    BOOST_TEST(tto.getUnsupportedFeatures().empty());
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    TestToolObservations tto;

    std::set<std::string> values {
      "first data",
      "some data", "some data",
      "more data",
    };
    for (const auto& value : values) {
      tto.addNotable(value);
      BOOST_TEST(tto.getNotables().count(value));
    }
    BOOST_TEST(values == tto.getNotables());
  }

  {
    TestToolObservations tto;

    std::set<std::string> values {
      "first data",
      "some data", "some data",
      "more data",
    };
    for (const auto& value : values) {
      tto.addUnsupportedFeature(value);
      BOOST_TEST(tto.getUnsupportedFeatures().count(value));
    }
    BOOST_TEST(values == tto.getUnsupportedFeatures());
  }
}
