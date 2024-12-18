// =============================================================================
// Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC
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
    using Package::Package;
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  {
    TestPackage package;

    BOOST_TEST(package.getState().empty());
    BOOST_TEST(package.getName().empty());
    BOOST_TEST(package.getVersion().empty());
    BOOST_TEST(package.getArchitecture().empty());
    BOOST_TEST(package.getDescription().empty());
    BOOST_TEST(!package.isValid());
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    {
      TestPackage package;

      package.setState("abc");
      BOOST_TEST("abc" == package.getState());
    }
    {
      TestPackage package;

      package.setState("ABC");
      BOOST_TEST("ABC" == package.getState());
    }
    {
      TestPackage package;

      package.setName("packageName");
      BOOST_TEST("packageName" == package.getName());
    }
    {
      TestPackage package;

      package.setVersion("1.1.0");
      BOOST_TEST("1.1.0" == package.getVersion());
    }
    {
      TestPackage package;

      package.setVersion("1");
      BOOST_TEST("1" == package.getVersion());
    }
    {
      TestPackage package;

      package.setArchitecture("all");
      BOOST_TEST("all" == package.getArchitecture());
    }
    {
      TestPackage package;

      package.setArchitecture("amd64");
      BOOST_TEST("amd64" == package.getArchitecture());
    }
    {
      TestPackage package;

      package.setDescription("some package.");
      BOOST_TEST("some package." == package.getDescription());
    }
    {
      TestPackage package;

      package.setDescription("somepackage.");
      BOOST_TEST("somepackage." == package.getDescription());
    }
}
}
BOOST_AUTO_TEST_CASE(testValidity)
{
  {
    TestPackage package;

    BOOST_TEST(!package.isValid());
    package.setState("abc");
    BOOST_TEST(!package.isValid());
    package.setName("name");
    BOOST_TEST(!package.isValid());
    package.setVersion("1.1");
    BOOST_TEST(!package.isValid());
    package.setArchitecture("amd64");
    BOOST_TEST(!package.isValid());
    package.setDescription("some description");
    BOOST_TEST(package.isValid());
  }
}
