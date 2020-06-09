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

#include <netmeld/datastore/objects/InterfaceNetwork.hpp>
#include <netmeld/core/utils/StringUtilities.hpp>

namespace nmdo = netmeld::datastore::objects;
namespace nmcu = netmeld::core::utils;


class TestInterfaceNetwork : public nmdo::InterfaceNetwork {
  public:
    TestInterfaceNetwork() : InterfaceNetwork() {};

  public:
    std::string getDescription() const
    { return description; }

    std::string getMediaType() const
    { return mediaType; }

    bool getIsDiscoveryProtocolEnabled() const
    { return isDiscoveryProtocolEnabled; }

    nmdo::MacAddress getMacAddr() const
    { return macAddr; }

    std::string getMode()
    { return mode; }

    bool getIsPortSecurityEnabled() const
    { return isPortSecurityEnabled; }

    unsigned short getPortSecurityMaxMacAddrs() const
    { return portSecurityMaxMacAddrs; }

    std::string getPortSecurityViolationAction() const
    { return portSecurityViolationAction; }

    bool getIsPortSecurityStickyMac() const
    { return isPortSecurityStickyMac; }

    std::set<nmdo::MacAddress> getLearnedMacAddrs() const
    { return learnedMacAddrs; }

    bool getIsBpduGuardEnabled() const
    { return isBpduGuardEnabled; }

    bool getIsBpduFilterEnabled() const
    { return isBpduFilterEnabled; }

    bool getIsPortfastEnabled() const
    { return isPortfastEnabled; }

    bool getIsPartial() const
    { return isPartial; }

    std::set<nmdo::Vlan> getVlans() const
    { return vlans; }
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  nmdo::MacAddress macAddr;
  {
    TestInterfaceNetwork interface;

    BOOST_CHECK(interface.getName().empty());
    BOOST_CHECK(interface.getDescription().empty());
    BOOST_CHECK_EQUAL("ethernet", interface.getMediaType());
    BOOST_CHECK(interface.getState());
    BOOST_CHECK(interface.getIsDiscoveryProtocolEnabled());
    BOOST_CHECK_EQUAL(macAddr, interface.getMacAddr());
    BOOST_CHECK_EQUAL("l3", interface.getMode());
    BOOST_CHECK(!interface.getIsPortSecurityEnabled());
    BOOST_CHECK_EQUAL(1, interface.getPortSecurityMaxMacAddrs());
    BOOST_CHECK_EQUAL("shutdown", interface.getPortSecurityViolationAction());
    BOOST_CHECK(!interface.getIsPortSecurityStickyMac());
    BOOST_CHECK(interface.getLearnedMacAddrs().empty());
    BOOST_CHECK(!interface.getIsBpduGuardEnabled());
    BOOST_CHECK(!interface.getIsBpduFilterEnabled());
    BOOST_CHECK(!interface.getIsPortfastEnabled());
    BOOST_CHECK(!interface.getIsPartial());
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    TestInterfaceNetwork interface;
    nmdo::IpAddress ipAddr {"1.2.3.4/24"};

    interface.addIpAddress(ipAddr);
    auto ipAddrs = interface.getIpAddresses();
    BOOST_CHECK_EQUAL(1, ipAddrs.size());
    BOOST_CHECK_EQUAL(ipAddr, ipAddrs[0]);
  }

  {
    TestInterfaceNetwork interface;
    nmdo::MacAddress macAddr {"00:11:22:33:44:55"};

    interface.addPortSecurityStickyMac(macAddr);
    interface.addPortSecurityStickyMac(macAddr);
    auto macAddrs = interface.getLearnedMacAddrs();
    BOOST_CHECK_EQUAL(1, macAddrs.size());
    BOOST_CHECK_EQUAL(1, macAddrs.count(macAddr));
  }

  {
    TestInterfaceNetwork interface;
    uint16_t vlanId {0};
    interface.addVlan(vlanId);
    interface.addVlan(vlanId);
    vlanId = UINT16_MAX;
    interface.addVlan(vlanId);

    auto vlans {interface.getVlans()};
    BOOST_CHECK_EQUAL(2, vlans.size());
    BOOST_CHECK_EQUAL(1, vlans.count(nmdo::Vlan(vlanId)));
    BOOST_CHECK_EQUAL(1, vlans.count(nmdo::Vlan(vlanId)));
  }

  {
    TestInterfaceNetwork interface;

    BOOST_CHECK(interface.getIsDiscoveryProtocolEnabled());
    interface.setDiscoveryProtocol(false);
    BOOST_CHECK(!interface.getIsDiscoveryProtocolEnabled());
    interface.setDiscoveryProtocol(true);
    BOOST_CHECK(interface.getIsDiscoveryProtocolEnabled());
  }

  {
    TestInterfaceNetwork interface;
    std::string text {"Some Description"};

    interface.setDescription(text);
    BOOST_CHECK_EQUAL(text, interface.getDescription());
  }

  {
    TestInterfaceNetwork interface;
    nmdo::MacAddress macAddr {"00:11:22:33:44:55"};

    interface.setMacAddress(macAddr);
    BOOST_CHECK_EQUAL(macAddr, interface.getMacAddr());
  }

  {
    TestInterfaceNetwork interface;
    std::string text {"MeDiAtYpE"};

    BOOST_CHECK_EQUAL("ethernet", interface.getMediaType());
    interface.setMediaType(text);
    BOOST_CHECK_EQUAL(nmcu::toLower(text), interface.getMediaType());
  }

  {
    TestInterfaceNetwork interface;
    std::string text {"NaMe"};

    interface.setName(text);
    BOOST_CHECK_EQUAL(nmcu::toLower(text), interface.getName());
  }

  {
    TestInterfaceNetwork interface;

    BOOST_CHECK(interface.getState());
    interface.setState(true);
    BOOST_CHECK(interface.getState());
    interface.setState(false);
    BOOST_CHECK(!interface.getState());
  }

  {
    TestInterfaceNetwork interface;
    std::string text {"MoDe"};

    interface.setSwitchportMode(text);
    BOOST_CHECK_EQUAL(nmcu::toLower(text), interface.getMode());
  }

  {
    TestInterfaceNetwork interface;

    BOOST_CHECK(!interface.getIsPortSecurityEnabled());
    interface.setPortSecurity(true);
    BOOST_CHECK(interface.getIsPortSecurityEnabled());
    interface.setPortSecurity(false);
    BOOST_CHECK(!interface.getIsPortSecurityEnabled());
  }

  {
    TestInterfaceNetwork interface;

    BOOST_CHECK_EQUAL(1, interface.getPortSecurityMaxMacAddrs());
    interface.setPortSecurityMaxMacAddrs(0);
    BOOST_CHECK_EQUAL(0, interface.getPortSecurityMaxMacAddrs());
    interface.setPortSecurityMaxMacAddrs(USHRT_MAX);
    BOOST_CHECK_EQUAL(USHRT_MAX, interface.getPortSecurityMaxMacAddrs());
  }

  {
    TestInterfaceNetwork interface;
    std::string text {"AcTiOn"};

    BOOST_CHECK_EQUAL("shutdown", interface.getPortSecurityViolationAction());
    interface.setPortSecurityViolationAction(text);
    BOOST_CHECK_EQUAL(nmcu::toLower(text), interface.getPortSecurityViolationAction());
  }

  {
    TestInterfaceNetwork interface;

    BOOST_CHECK(!interface.getIsPortSecurityStickyMac());
    interface.setPortSecurityStickyMac(true);
    BOOST_CHECK(interface.getIsPortSecurityStickyMac());
    interface.setPortSecurityStickyMac(false);
    BOOST_CHECK(!interface.getIsPortSecurityStickyMac());
  }

  {
    TestInterfaceNetwork interface;

    BOOST_CHECK(!interface.getIsBpduGuardEnabled());
    interface.setBpduGuard(true);
    BOOST_CHECK(interface.getIsBpduGuardEnabled());
    interface.setBpduGuard(false);
    BOOST_CHECK(!interface.getIsBpduGuardEnabled());
  }

  {
    TestInterfaceNetwork interface;

    BOOST_CHECK(!interface.getIsBpduFilterEnabled());
    interface.setBpduFilter(true);
    BOOST_CHECK(interface.getIsBpduFilterEnabled());
    interface.setBpduFilter(false);
    BOOST_CHECK(!interface.getIsBpduFilterEnabled());
  }

  {
    TestInterfaceNetwork interface;

    BOOST_CHECK(!interface.getIsPortfastEnabled());
    interface.setPortfast(true);
    BOOST_CHECK(interface.getIsPortfastEnabled());
    interface.setPortfast(false);
    BOOST_CHECK(!interface.getIsPortfastEnabled());
  }

  {
    TestInterfaceNetwork interface;

    BOOST_CHECK(!interface.getIsPartial());
    interface.setPartial(true);
    BOOST_CHECK(interface.getIsPartial());
    interface.setPartial(false);
    BOOST_CHECK(!interface.getIsPartial());
  }
}

BOOST_AUTO_TEST_CASE(testValidity)
{
  {
    TestInterfaceNetwork interface;

    BOOST_CHECK(!interface.isValid());
    interface.setName("name");
    BOOST_CHECK(interface.isValid());
    interface.setName("");
    BOOST_CHECK(!interface.isValid());
  }
}
