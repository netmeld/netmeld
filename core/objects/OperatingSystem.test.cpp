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

#include <float.h>

#include <netmeld/core/objects/OperatingSystem.hpp>

namespace nmco = netmeld::core::objects;


class TestOperatingSystem : public nmco::OperatingSystem {
  public:
    TestOperatingSystem() : OperatingSystem() {};
    explicit TestOperatingSystem(nmco::IpAddress& _ip) : OperatingSystem(_ip) {};

  public:
    nmco::IpAddress getIpAddr() const
    { return ipAddr; }

    std::string getVendorName() const
    { return vendorName; }

    std::string getProductName() const
    { return productName; }

    std::string getProductVersion() const
    { return productVersion; }

    std::string getCpe() const
    { return cpe; }

    double getAccuracy() const
    { return accuracy; }
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  {
    TestOperatingSystem operatingSystem;
    nmco::IpAddress ipAddr;

    BOOST_CHECK_EQUAL(ipAddr, operatingSystem.getIpAddr());
    BOOST_CHECK(operatingSystem.getVendorName().empty());
    BOOST_CHECK(operatingSystem.getProductName().empty());
    BOOST_CHECK(operatingSystem.getProductVersion().empty());
    BOOST_CHECK(operatingSystem.getCpe().empty());
    BOOST_CHECK_EQUAL(0.0, operatingSystem.getAccuracy());
  }

  {
    nmco::IpAddress ipAddr {"1.2.3.4/24"};
    TestOperatingSystem operatingSystem {ipAddr};

    BOOST_CHECK_EQUAL(ipAddr, operatingSystem.getIpAddr());
    BOOST_CHECK(operatingSystem.getVendorName().empty());
    BOOST_CHECK(operatingSystem.getProductName().empty());
    BOOST_CHECK(operatingSystem.getProductVersion().empty());
    BOOST_CHECK(operatingSystem.getCpe().empty());
    BOOST_CHECK_EQUAL(0.0, operatingSystem.getAccuracy());
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    TestOperatingSystem operatingSystem;

    operatingSystem.setVendorName("Vendor Name");
    BOOST_CHECK_EQUAL("vendor name", operatingSystem.getVendorName());
  }
  {
    TestOperatingSystem operatingSystem;

    operatingSystem.setProductName("Product Name");
    BOOST_CHECK_EQUAL("product name", operatingSystem.getProductName());
  }
  {
    TestOperatingSystem operatingSystem;

    operatingSystem.setProductVersion("Product Version");
    BOOST_CHECK_EQUAL("product version", operatingSystem.getProductVersion());
  }
  {
    TestOperatingSystem operatingSystem;

    operatingSystem.setCpe("Cpe");
    BOOST_CHECK_EQUAL("cpe", operatingSystem.getCpe());
  }
  {
    TestOperatingSystem operatingSystem;

    operatingSystem.setAccuracy(DBL_MIN);
    BOOST_CHECK_EQUAL(DBL_MIN, operatingSystem.getAccuracy());
    operatingSystem.setAccuracy(DBL_MAX);
    BOOST_CHECK_EQUAL(DBL_MAX, operatingSystem.getAccuracy());
  }
}

BOOST_AUTO_TEST_CASE(testValidity)
{
  {
    TestOperatingSystem operatingSystem;
    BOOST_CHECK(!operatingSystem.isValid());
  }

  {
    nmco::IpAddress ipAddr {"1.2.3.4/24"};
    TestOperatingSystem operatingSystem {ipAddr};
    BOOST_CHECK(!operatingSystem.isValid());
    operatingSystem.setVendorName("vendor name");
    BOOST_CHECK(operatingSystem.isValid());
  }

  {
    nmco::IpAddress ipAddr {"1.2.3.4/24"};
    TestOperatingSystem operatingSystem {ipAddr};
    BOOST_CHECK(!operatingSystem.isValid());
    operatingSystem.setProductName("product name");
    BOOST_CHECK(operatingSystem.isValid());
  }

  {
    nmco::IpAddress ipAddr {"1.2.3.4/24"};
    TestOperatingSystem operatingSystem {ipAddr};
    BOOST_CHECK(!operatingSystem.isValid());
    operatingSystem.setProductVersion("product version");
    BOOST_CHECK(operatingSystem.isValid());
  }

  {
    nmco::IpAddress ipAddr {"1.2.3.4/24"};
    TestOperatingSystem operatingSystem {ipAddr};
    BOOST_CHECK(!operatingSystem.isValid());
    operatingSystem.setCpe("cpe");
    BOOST_CHECK(operatingSystem.isValid());
  }
}
