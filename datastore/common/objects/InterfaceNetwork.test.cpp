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

#include <netmeld/datastore/objects/InterfaceNetwork.hpp>
#include <netmeld/core/utils/StringUtilities.hpp>

namespace nmdo = netmeld::datastore::objects;
namespace nmcu = netmeld::core::utils;


class TestInterfaceNetwork : public nmdo::InterfaceNetwork {
  public:
    using InterfaceNetwork::InterfaceNetwork;

    using InterfaceNetwork::description;
    using InterfaceNetwork::isPartial;
    using InterfaceNetwork::mediaType;
    using InterfaceNetwork::isDiscoveryProtocolEnabled;
    using InterfaceNetwork::macAddr;
    using InterfaceNetwork::mode;
    using InterfaceNetwork::isPortSecurityEnabled;
    using InterfaceNetwork::portSecurityMaxMacAddrs;
    using InterfaceNetwork::portSecurityViolationAction;
    using InterfaceNetwork::isPortSecurityStickyMac;
    using InterfaceNetwork::learnedMacAddrs;
    using InterfaceNetwork::reachableMacAddrs;
    using InterfaceNetwork::isBpduGuardEnabled;
    using InterfaceNetwork::isBpduFilterEnabled;
    using InterfaceNetwork::isPortfastEnabled;
    // has accessor
    //using InterfaceNetwork::name;
    //using InterfaceNetwork::isUp;
    //using InterfaceNetwork::vlans;
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  nmdo::MacAddress macAddr;
  {
    TestInterfaceNetwork tin;

    BOOST_TEST(tin.getName().empty());
    BOOST_TEST(tin.description.empty());
    BOOST_TEST("ethernet" == tin.mediaType);
    BOOST_TEST(tin.getState());
    BOOST_TEST(tin.isDiscoveryProtocolEnabled);
    BOOST_TEST(macAddr == tin.macAddr);
    BOOST_TEST("l3" == tin.mode);
    BOOST_TEST(!tin.isPortSecurityEnabled);
    BOOST_TEST(1 == tin.portSecurityMaxMacAddrs);
    BOOST_TEST("shutdown" == tin.portSecurityViolationAction);
    BOOST_TEST(!tin.isPortSecurityStickyMac);
    BOOST_TEST(tin.learnedMacAddrs.empty());
    BOOST_TEST(!tin.isBpduGuardEnabled);
    BOOST_TEST(!tin.isBpduFilterEnabled);
    BOOST_TEST(!tin.isPortfastEnabled);
    BOOST_TEST(!tin.isPartial);
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    TestInterfaceNetwork tin;
    nmdo::IpAddress ipAddr {"1.2.3.4/24"};

    tin.addIpAddress(ipAddr);
    auto ipAddrs = tin.getIpAddresses();
    BOOST_TEST(1 == ipAddrs.size());
    BOOST_TEST(ipAddr == *ipAddrs.cbegin());
  }

  {
    TestInterfaceNetwork tin;
    nmdo::MacAddress macAddr {"00:11:22:33:44:55"};

    tin.addPortSecurityStickyMac(macAddr);
    tin.addPortSecurityStickyMac(macAddr);
    auto macAddrs = tin.learnedMacAddrs;
    BOOST_TEST(1 == macAddrs.size());
    BOOST_TEST(1 == macAddrs.count(macAddr));
  }

  {
    TestInterfaceNetwork tin;
    uint16_t vlanId {0};
    tin.addVlan(vlanId);
    tin.addVlan(vlanId);
    vlanId = UINT16_MAX;
    tin.addVlan(vlanId);

    auto vlans {tin.getVlans()};
    BOOST_TEST(2 == vlans.size());
    BOOST_TEST(1 == vlans.count(nmdo::Vlan(vlanId)));
    BOOST_TEST(1 == vlans.count(nmdo::Vlan(vlanId)));
  }

  {
    TestInterfaceNetwork tin;

    BOOST_TEST(tin.isDiscoveryProtocolEnabled);
    tin.setDiscoveryProtocol(false);
    BOOST_TEST(!tin.isDiscoveryProtocolEnabled);
    tin.setDiscoveryProtocol(true);
    BOOST_TEST(tin.isDiscoveryProtocolEnabled);
  }

  {
    TestInterfaceNetwork tin;
    std::string text {"Some Description"};

    tin.setDescription(text);
    BOOST_TEST(text == tin.description);
  }

  {
    TestInterfaceNetwork tin;
    nmdo::MacAddress macAddr {"00:11:22:33:44:55"};

    tin.setMacAddress(macAddr);
    BOOST_TEST(macAddr == tin.macAddr);
  }

  {
    TestInterfaceNetwork tin;
    std::string text {"MeDiAtYpE"};

    BOOST_TEST("ethernet" == tin.mediaType);
    tin.setMediaType(text);
    BOOST_TEST(nmcu::toLower(text) == tin.mediaType);
  }

  {
    TestInterfaceNetwork tin;
    std::string text {"NaMe"};

    tin.setName(text);
    BOOST_TEST(nmcu::toLower(text) == tin.getName());
  }

  {
    TestInterfaceNetwork tin;

    BOOST_TEST(tin.getState());
    tin.setState(true);
    BOOST_TEST(tin.getState());
    tin.setState(false);
    BOOST_TEST(!tin.getState());
  }

  {
    TestInterfaceNetwork tin;
    std::string text {"MoDe"};

    tin.setSwitchportMode(text);
    BOOST_TEST(nmcu::toLower(text) == tin.mode);
  }

  {
    TestInterfaceNetwork tin;

    BOOST_TEST(!tin.isPortSecurityEnabled);
    tin.setPortSecurity(true);
    BOOST_TEST(tin.isPortSecurityEnabled);
    tin.setPortSecurity(false);
    BOOST_TEST(!tin.isPortSecurityEnabled);
  }

  {
    TestInterfaceNetwork tin;

    BOOST_TEST(1 == tin.portSecurityMaxMacAddrs);
    tin.setPortSecurityMaxMacAddrs(0);
    BOOST_TEST(0 == tin.portSecurityMaxMacAddrs);
    tin.setPortSecurityMaxMacAddrs(USHRT_MAX);
    BOOST_TEST(USHRT_MAX == tin.portSecurityMaxMacAddrs);
  }

  {
    TestInterfaceNetwork tin;
    std::string text {"AcTiOn"};

    BOOST_TEST("shutdown" == tin.portSecurityViolationAction);
    tin.setPortSecurityViolationAction(text);
    BOOST_TEST(nmcu::toLower(text) == tin.portSecurityViolationAction);
  }

  {
    TestInterfaceNetwork tin;

    BOOST_TEST(!tin.isPortSecurityStickyMac);
    tin.setPortSecurityStickyMac(true);
    BOOST_TEST(tin.isPortSecurityStickyMac);
    tin.setPortSecurityStickyMac(false);
    BOOST_TEST(!tin.isPortSecurityStickyMac);
  }

  {
    TestInterfaceNetwork tin;

    BOOST_TEST(!tin.isBpduGuardEnabled);
    tin.setBpduGuard(true);
    BOOST_TEST(tin.isBpduGuardEnabled);
    tin.setBpduGuard(false);
    BOOST_TEST(!tin.isBpduGuardEnabled);
  }

  {
    TestInterfaceNetwork tin;

    BOOST_TEST(!tin.isBpduFilterEnabled);
    tin.setBpduFilter(true);
    BOOST_TEST(tin.isBpduFilterEnabled);
    tin.setBpduFilter(false);
    BOOST_TEST(!tin.isBpduFilterEnabled);
  }

  {
    TestInterfaceNetwork tin;

    BOOST_TEST(!tin.isPortfastEnabled);
    tin.setPortfast(true);
    BOOST_TEST(tin.isPortfastEnabled);
    tin.setPortfast(false);
    BOOST_TEST(!tin.isPortfastEnabled);
  }

  {
    TestInterfaceNetwork tin;

    BOOST_TEST(!tin.isPartial);
    tin.setPartial(true);
    BOOST_TEST(tin.isPartial);
    tin.setPartial(false);
    BOOST_TEST(!tin.isPartial);
  }
}

BOOST_AUTO_TEST_CASE(testValidity)
{
  {
    TestInterfaceNetwork tin;

    BOOST_TEST(!tin.isValid());
    tin.setName("name");
    BOOST_TEST(tin.isValid());
    tin.setName("");
    BOOST_TEST(!tin.isValid());
  }
}
