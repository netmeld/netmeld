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

#include <netmeld/datastore/objects/DeviceInformation.hpp>

namespace nmdo = netmeld::datastore::objects;


class TestDeviceInformation : public nmdo::DeviceInformation {
  public:
    TestDeviceInformation() : DeviceInformation() {};

  public:
    std::string getColor() const
    { return deviceColor; }

    std::string getVendor() const
    { return vendor; }

    std::string getModel() const
    { return model; }

    std::string getHardwareRevision() const
    { return hardwareRevision; }

    std::string getSerialNumber() const
    { return serialNumber; }

    std::string getDescription() const
    { return description; }
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  {
    TestDeviceInformation devInfo;

    BOOST_TEST(devInfo.getDeviceId().empty());
    BOOST_TEST(devInfo.getDeviceType().empty());
    BOOST_TEST(devInfo.getColor().empty());
    BOOST_TEST(devInfo.getVendor().empty());
    BOOST_TEST(devInfo.getModel().empty());
    BOOST_TEST(devInfo.getHardwareRevision().empty());
    BOOST_TEST(devInfo.getSerialNumber().empty());
    BOOST_TEST(devInfo.getDescription().empty());
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    TestDeviceInformation devInfo;

    devInfo.setDeviceId("Device-Id");
    BOOST_TEST("device-id", devInfo.getDeviceId());
  }
  {
    TestDeviceInformation devInfo;

    devInfo.setDeviceColor("Device Color");
    BOOST_TEST("Device Color", devInfo.getDeviceId());
  }
  {
    TestDeviceInformation devInfo;

    devInfo.setDeviceType("Device Type");
    BOOST_TEST("device type", devInfo.getDeviceType());
  }
  {
    TestDeviceInformation devInfo;

    devInfo.setDescription("Description");
    BOOST_TEST("description", devInfo.getDescription());
  }
  {
    TestDeviceInformation devInfo;

    devInfo.setHardwareRevision("rev 1");
    BOOST_TEST("REV 1", devInfo.getHardwareRevision());
  }
  {
    TestDeviceInformation devInfo;

    devInfo.setModel("model");
    BOOST_TEST("MODEL", devInfo.getModel());
  }
  {
    TestDeviceInformation devInfo;

    devInfo.setSerialNumber("sn 123");
    BOOST_TEST("SN 123", devInfo.getSerialNumber());
  }
  {
    TestDeviceInformation devInfo;

    devInfo.setVendor("Vendor");
    BOOST_TEST("vendor", devInfo.getVendor());
  }
}

BOOST_AUTO_TEST_CASE(testValidity)
{
  {
    TestDeviceInformation devInfo;

    BOOST_TEST(!devInfo.isValid());
    devInfo.setDeviceId("did");
    BOOST_TEST(devInfo.isValid());
    devInfo.setDeviceId("");
    BOOST_TEST(!devInfo.isValid());
  }
  {
    TestDeviceInformation devInfo;

    BOOST_TEST(!devInfo.isValid());
    devInfo.setDeviceId("did");
    devInfo.setDeviceType("device type");
    BOOST_TEST(devInfo.isValid());
  }
  {
    TestDeviceInformation devInfo;

    BOOST_TEST(!devInfo.isValid());
    devInfo.setDeviceId("did");
    devInfo.setDescription("description");
    BOOST_TEST(devInfo.isValid());
  }
  {
    TestDeviceInformation devInfo;

    BOOST_TEST(!devInfo.isValid());
    devInfo.setDeviceId("did");
    devInfo.setHardwareRevision("rev 1");
    BOOST_TEST(devInfo.isValid());
  }
  {
    TestDeviceInformation devInfo;

    BOOST_TEST(!devInfo.isValid());
    devInfo.setDeviceId("did");
    devInfo.setModel("model");
    BOOST_TEST(devInfo.isValid());
  }
  {
    TestDeviceInformation devInfo;

    BOOST_TEST(!devInfo.isValid());
    devInfo.setDeviceId("did");
    devInfo.setSerialNumber("sn 123");
    BOOST_TEST(devInfo.isValid());
  }
  {
    TestDeviceInformation devInfo;

    BOOST_TEST(!devInfo.isValid());
    devInfo.setDeviceId("did");
    devInfo.setVendor("Vendor");
    BOOST_TEST(devInfo.isValid());
  }
}
