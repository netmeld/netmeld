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


class TestOperatingSystem : public nmdo::OperatingSystem
{
  public:
    using OperatingSystem::OperatingSystem;

    using OperatingSystem::ipAddr;
    using OperatingSystem::vendorName;
    using OperatingSystem::productName;
    using OperatingSystem::productVersion;
    using OperatingSystem::cpe;
    using OperatingSystem::accuracy;

    // explicit, so have to formally define
    TestOperatingSystem(const nmdo::IpAddress& ipa) : OperatingSystem(ipa) {};
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  {
    BOOST_TEST_REQUIRE(TestOperatingSystem() == nmdo::OperatingSystem());

    TestOperatingSystem tos;

    BOOST_TEST(nmdo::IpAddress() == tos.ipAddr);
    BOOST_TEST(tos.vendorName.empty());
    BOOST_TEST(tos.productName.empty());
    BOOST_TEST(tos.productVersion.empty());
    BOOST_TEST(tos.cpe.empty());
    BOOST_TEST(0.0 == tos.accuracy);
  }

  {
    nmdo::IpAddress ipAddr {"1.2.3.4/24"};

    BOOST_TEST_REQUIRE(  TestOperatingSystem(ipAddr)
                      == nmdo::OperatingSystem(ipAddr)
                      );

    TestOperatingSystem tos {ipAddr};

    BOOST_TEST(ipAddr == tos.ipAddr);
    BOOST_TEST(tos.vendorName.empty());
    BOOST_TEST(tos.productName.empty());
    BOOST_TEST(tos.productVersion.empty());
    BOOST_TEST(tos.cpe.empty());
    BOOST_TEST(0.0 == tos.accuracy);

    nmdo::OperatingSystem os;
    os.setIpAddr(ipAddr);
    BOOST_TEST(tos == os);
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  nmdo::IpAddress ipAddr {"1.2.3.4/24"};

  TestOperatingSystem tos;

  tos.setIpAddr(ipAddr);
  BOOST_TEST(ipAddr == tos.ipAddr);
  tos.setVendorName("Vendor Name");
  BOOST_TEST("vendor name" == tos.vendorName);
  tos.setProductName("Product Name");
  BOOST_TEST("product name" == tos.productName);
  tos.setProductVersion("Product Version");
  BOOST_TEST("product version" == tos.productVersion);
  tos.setCpe();
  BOOST_TEST("cpe:/o:vendor_name:product_name:product_version" == tos.cpe);
  tos.setCpe("AbC 123");
  BOOST_TEST("abc_123" == tos.cpe);
  tos.setAccuracy(DBL_MIN);
  BOOST_TEST(DBL_MIN == tos.accuracy);
  tos.setAccuracy(DBL_MAX);
  BOOST_TEST(DBL_MAX == tos.accuracy);
}

BOOST_AUTO_TEST_CASE(testValidity)
{
  TestOperatingSystem tos {nmdo::IpAddress("1.2.3.4/24")};
  BOOST_TEST(!tos.isValid());

  tos.setVendorName("vendor name");
  BOOST_TEST(tos.isValid());
  tos.setVendorName("");
  BOOST_TEST(!tos.isValid());

  tos.setProductName("product name");
  BOOST_TEST(tos.isValid());
  tos.setProductName("");
  BOOST_TEST(!tos.isValid());

  tos.setProductVersion("product version");
  BOOST_TEST(tos.isValid());
  tos.setProductVersion("");
  BOOST_TEST(!tos.isValid());

  tos.setCpe("cpe");
  BOOST_TEST(tos.isValid());
  tos.setCpe("");
  BOOST_TEST(!tos.isValid());
}

BOOST_AUTO_TEST_CASE(testToCpeString)
{
  TestOperatingSystem tos;
  BOOST_TEST(tos.cpe.empty());

  // not set, default to internal fields
  tos.setVendorName("Vendor");
  tos.setProductName("Product AbC Xy-Z");
  tos.setProductVersion("V 1.23.5");
  const auto& cpeStr {"cpe:/o:vendor:product_abc_xy-z:v_1.23.5"};
  BOOST_TEST(cpeStr == tos.toCpeString());

  // once set, no arbitrary change
  tos.setVendorName("alt1");
  tos.setProductName("alt2");
  tos.setProductVersion("alt3");
  BOOST_TEST(cpeStr == tos.toCpeString());
  // explicit set call to trigger update
  tos.setCpe();
  BOOST_TEST("cpe:/o:alt1:alt2:alt3" == tos.toCpeString());
}
