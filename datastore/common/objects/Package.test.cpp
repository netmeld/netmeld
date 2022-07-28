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

#include <netmeld/datastore/objects/Package.hpp>

namespace nmdo = netmeld::datastore::objects;


class TestPackage : public nmdo::Package {
  public:
    TestPackage() : Package() {};
    explicit TestPackage(const std::string& _status) :
        Package(_status) {};

  public:
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  {
    TestPackage package {"ii"};

    BOOST_CHECK_EQUAL("ii", package.getStatus());
    BOOST_CHECK(package.getName().empty());
    BOOST_CHECK(package.getVersion().empty());
    BOOST_CHECK(package.getArchitecture().empty());
    BOOST_CHECK(package.getDescription().empty());

  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    {
    TestPackage package;

    package.setStatus("ii");
    BOOST_CHECK_EQUAL("ii", package.getStatus());
  }
  {
    TestPackage package;

    package.setName("packageName");
    BOOST_CHECK_EQUAL("packageName", package.getName());
  }
    {
    TestPackage package;

    package.setVersion("1.1.0");
    BOOST_CHECK_EQUAL("1.1.0", package.getVersion());
  }
    {
    TestPackage package;

    package.setArchitecture("all");
    BOOST_CHECK_EQUAL("all", package.getArchitecture());
  }
    {
    TestPackage package;

    package.setDescription("some package.");
    BOOST_CHECK_EQUAL("some package.", package.getDescription());
  }
}
}
BOOST_AUTO_TEST_CASE(testValidity)
{
  {
    TestPackage package;

    BOOST_CHECK(!package.isValid());
    package.setName("name");
    BOOST_CHECK(!package.isValid());
    package.setVersion("1.1");
    BOOST_CHECK(package.isValid());
  }
}
