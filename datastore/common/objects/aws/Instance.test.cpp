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

#include <netmeld/datastore/objects/aws/Instance.hpp>

namespace nmdoa = netmeld::datastore::objects::aws;


class TestInstance : public nmdoa::Instance {
  public:
    TestInstance() : Instance() {};

  public:
    std::string getInstanceId() const
    { return instanceId; }

    std::string getType() const
    { return type; }

    std::string getImageId() const
    { return imageId; }

    std::string getArchitecture() const
    { return architecture; }

    std::string getPlatformDetails() const
    { return platformDetails; }

    std::string getLaunchTime() const
    { return launchTime; }

    std::string getAvailabilityZone() const
    { return availabilityZone; }

    uint16_t getStateCode() const
    { return stateCode; }

    std::string getStateName() const
    { return stateName; }

    std::set<nmdoa::NetworkInterface> getInterfaces() const
    { return interfaces; }
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  {
    TestInstance tobj;

    BOOST_TEST(tobj.getInstanceId().empty());
    BOOST_TEST(tobj.getType().empty());
    BOOST_TEST(tobj.getImageId().empty());
    BOOST_TEST(tobj.getArchitecture().empty());
    BOOST_TEST(tobj.getPlatformDetails().empty());
    BOOST_TEST(tobj.getLaunchTime().empty());
    BOOST_TEST(tobj.getAvailabilityZone().empty());
    BOOST_TEST(0 == tobj.getStateCode());
    BOOST_TEST(tobj.getStateName().empty());
    BOOST_TEST(tobj.getInterfaces().empty());
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    TestInstance tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.setAvailabilityZone(tv1);
    BOOST_TEST(tv1 == tobj.getAvailabilityZone());
  }
  {
    TestInstance tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.setImageId(tv1);
    BOOST_TEST(tv1 == tobj.getImageId());
  }
  {
    TestInstance tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.setArchitecture(tv1);
    BOOST_TEST(tv1 == tobj.getArchitecture());
  }
  {
    TestInstance tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.setPlatformDetails(tv1);
    BOOST_TEST(tv1 == tobj.getPlatformDetails());
  }
  {
    TestInstance tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.setLaunchTime(tv1);
    BOOST_TEST(tv1 == tobj.getLaunchTime());
  }
  {
    TestInstance tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.setId(tv1);
    BOOST_TEST(tv1 == tobj.getInstanceId());
  }
  {
    TestInstance tobj;

    const uint16_t tv1 {123};
    tobj.setStateCode(tv1);
    BOOST_TEST(tv1 == tobj.getStateCode());
  }
  {
    TestInstance tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.setStateName(tv1);
    BOOST_TEST(tv1 == tobj.getStateName());
  }
  {
    TestInstance tobj;

    const std::string tv1 {"aBc1@3"};
    tobj.setType(tv1);
    BOOST_TEST(tv1 == tobj.getType());
  }
  {
    TestInstance tobj;

    nmdoa::NetworkInterface tv1;
    tobj.addInterface(tv1);
    auto nis = tobj.getInterfaces();
    BOOST_TEST(0 == nis.size());
    BOOST_TEST(!nis.contains(tv1));

    tv1.setId("aBc1@3");
    tobj.addInterface(tv1);
    nis = tobj.getInterfaces();
    BOOST_TEST(1 == nis.size());
    BOOST_TEST(nis.contains(tv1));
  }
}

BOOST_AUTO_TEST_CASE(testValidity)
{
  {
    TestInstance tobj;

    const std::string tv1 {"aBc1@3"};
    
    BOOST_TEST(!tobj.isValid());
    tobj.setId(tv1);
    BOOST_TEST(tobj.isValid());
  }
}
