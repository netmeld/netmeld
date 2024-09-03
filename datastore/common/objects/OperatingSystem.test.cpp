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

#include <float.h>

#include <netmeld/datastore/objects/OperatingSystem.hpp>

namespace nmdo = netmeld::datastore::objects;


class TestOperatingSystem : public nmdo::OperatingSystem {
  public:
    using OperatingSystem::OperatingSystem;

    using OperatingSystem::ipAddr;
    using OperatingSystem::vendorName;
    using OperatingSystem::productName;
    using OperatingSystem::productVersion;
    using OperatingSystem::cpe;
    using OperatingSystem::accuracy;
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  {
    TestOperatingSystem operatingSystem;
    nmdo::IpAddress ipAddr;

    BOOST_TEST(ipAddr == operatingSystem.ipAddr);
    BOOST_TEST(operatingSystem.vendorName.empty());
    BOOST_TEST(operatingSystem.productName.empty());
    BOOST_TEST(operatingSystem.productVersion.empty());
    BOOST_TEST(operatingSystem.cpe.empty());
    BOOST_TEST(0.0 == operatingSystem.accuracy);
  }

  {
    nmdo::IpAddress ipAddr {"1.2.3.4/24"};
    TestOperatingSystem operatingSystem {ipAddr};

    BOOST_TEST(ipAddr == operatingSystem.ipAddr);
    BOOST_TEST(operatingSystem.vendorName.empty());
    BOOST_TEST(operatingSystem.productName.empty());
    BOOST_TEST(operatingSystem.productVersion.empty());
    BOOST_TEST(operatingSystem.cpe.empty());
    BOOST_TEST(0.0 == operatingSystem.accuracy);
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    TestOperatingSystem operatingSystem;

    operatingSystem.setVendorName("Vendor Name");
    BOOST_TEST("vendor name" == operatingSystem.vendorName);
  }
  {
    TestOperatingSystem operatingSystem;

    operatingSystem.setProductName("Product Name");
    BOOST_TEST("product name" == operatingSystem.productName);
  }
  {
    TestOperatingSystem operatingSystem;

    operatingSystem.setProductVersion("Product Version");
    BOOST_TEST("product version" == operatingSystem.productVersion);
  }
  {
    TestOperatingSystem operatingSystem;

    operatingSystem.setCpe("Cpe");
    BOOST_TEST("cpe" == operatingSystem.cpe);
  }
  {
    TestOperatingSystem operatingSystem;

    operatingSystem.setAccuracy(DBL_MIN);
    BOOST_TEST(DBL_MIN == operatingSystem.accuracy);
    operatingSystem.setAccuracy(DBL_MAX);
    BOOST_TEST(DBL_MAX == operatingSystem.accuracy);
  }
}

BOOST_AUTO_TEST_CASE(testValidity)
{
  {
    TestOperatingSystem operatingSystem;
    BOOST_TEST(!operatingSystem.isValid());
  }

  {
    nmdo::IpAddress ipAddr {"1.2.3.4/24"};
    TestOperatingSystem operatingSystem {ipAddr};
    BOOST_TEST(!operatingSystem.isValid());
    operatingSystem.setVendorName("vendor name");
    BOOST_TEST(operatingSystem.isValid());
  }

  {
    nmdo::IpAddress ipAddr {"1.2.3.4/24"};
    TestOperatingSystem operatingSystem {ipAddr};
    BOOST_TEST(!operatingSystem.isValid());
    operatingSystem.setProductName("product name");
    BOOST_TEST(operatingSystem.isValid());
  }

  {
    nmdo::IpAddress ipAddr {"1.2.3.4/24"};
    TestOperatingSystem operatingSystem {ipAddr};
    BOOST_TEST(!operatingSystem.isValid());
    operatingSystem.setProductVersion("product version");
    BOOST_TEST(operatingSystem.isValid());
  }

  {
    nmdo::IpAddress ipAddr {"1.2.3.4/24"};
    TestOperatingSystem operatingSystem {ipAddr};
    BOOST_TEST(!operatingSystem.isValid());
    operatingSystem.setCpe("cpe");
    BOOST_TEST(operatingSystem.isValid());
  }
}
