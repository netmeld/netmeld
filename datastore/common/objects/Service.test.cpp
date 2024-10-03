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

#include <netmeld/datastore/objects/Service.hpp>

namespace nmdo = netmeld::datastore::objects;


class TestService : public nmdo::Service {
  public:
    using Service::Service;

    using Service::dstFqdn;
    using Service::dstAddress;
    using Service::srcAddress;
    using Service::isLocal;
    using Service::interfaceName;
    using Service::zone;
    using Service::serviceName;
    using Service::serviceDescription;
    using Service::serviceReason;
    using Service::protocol;
    using Service::dstPorts;
    using Service::srcPorts;

    using Service::isValidDevice;
    using Service::isValidNetwork;
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  {
    nmdo::IpAddress ipAddr;
    TestService service;

    BOOST_TEST(ipAddr == service.dstAddress);
    BOOST_TEST(ipAddr == service.srcAddress);
    BOOST_TEST(!service.isLocal);
    BOOST_TEST("-" == service.interfaceName);
    BOOST_TEST(service.zone.empty());
    BOOST_TEST(service.serviceName.empty());
    BOOST_TEST(service.serviceDescription.empty());
    BOOST_TEST(service.serviceReason.empty());
    BOOST_TEST(service.protocol.empty());
    BOOST_TEST(service.dstPorts.empty());
    BOOST_TEST(service.srcPorts.empty());
  }

  {
    nmdo::IpAddress ipAddr {"1.2.3.4/24"};
    nmdo::IpAddress dIpAddr;
    TestService service {"Service", ipAddr};

    BOOST_TEST(ipAddr == service.dstAddress);
    BOOST_TEST(dIpAddr == service.srcAddress);
    BOOST_TEST(!service.isLocal);
    BOOST_TEST("-" == service.interfaceName);
    BOOST_TEST(service.zone.empty());
    BOOST_TEST("Service" == service.serviceName);
    BOOST_TEST(service.serviceDescription.empty());
    BOOST_TEST(service.serviceReason.empty());
    BOOST_TEST(service.protocol.empty());
    BOOST_TEST(service.dstPorts.empty());
    BOOST_TEST(service.srcPorts.empty());
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    TestService service;

    service.addDstPort("1");
    BOOST_TEST(1 == service.dstPorts.size());
    BOOST_TEST(service.dstPorts.count("1"));
    service.addDstPort("0-65535");
    BOOST_TEST(2 == service.dstPorts.size());
    BOOST_TEST(service.dstPorts.count("0-65535"));
    service.addDstPort("0-65535");
    BOOST_TEST(2 == service.dstPorts.size());
  }

  {
    TestService service;

    service.addSrcPort("1");
    BOOST_TEST(1 == service.srcPorts.size());
    BOOST_TEST(service.srcPorts.count("1"));
    service.addSrcPort("0-65535");
    BOOST_TEST(2 == service.srcPorts.size());
    BOOST_TEST(service.srcPorts.count("0-65535"));
    service.addSrcPort("0-65535");
    BOOST_TEST(2 == service.srcPorts.size());
  }

  {
    TestService service;
    nmdo::IpAddress ipAddr {"1.2.3.4/24"};

    service.setDstAddress(ipAddr);
    BOOST_TEST(ipAddr == service.dstAddress);
  }

  {
    TestService service;
    nmdo::IpAddress ipAddr {"1.2.3.4/24"};

    service.setSrcAddress(ipAddr);
    BOOST_TEST(ipAddr == service.srcAddress);
  }

  {
    TestService service;

    service.setInterfaceName("Ifname");
    BOOST_TEST("ifname" == service.interfaceName);
  }

  {
    TestService service;

    service.setServiceName("Service Name");
    BOOST_TEST("Service Name" == service.serviceName);
  }

  {
    TestService service;

    service.setServiceDescription("Service Description");
    BOOST_TEST("Service Description" == service.serviceDescription);
  }

  {
    TestService service;

    service.setServiceReason("Service Reason");
    BOOST_TEST("service reason" == service.serviceReason);
  }

  {
    TestService service;

    service.setProtocol("Protocol");
    BOOST_TEST("protocol" == service.protocol);
  }
}

BOOST_AUTO_TEST_CASE(testValidity)
{
/*
  Windows netstat
    local
    protocol, dstAddress, dstPort
    none (serviceName is multi-commands to piece together)
  Linux netstat
    local
    protocol, dstAddress, dstPort
    serviceName
  Windows ipconfig
    remote
    serviceName, dstAddress, ifaceName
    none
  nmap
    remote
    protocol, dstAddress, dstPort, serviceName, serviceDesc, serviceReason
    none

  raw_network_service is for observed services from a network perspective
    they will only be remote observed
  raw_device_ip_server is for observed services from a local perspective
    they might be local hosted or remote expected

  if local || (remote && deviceId)
    insert raw_device_ip_server
  if remote && !deviceId
    insert raw_network_service
*/
  {
    nmdo::IpAddress ipAddr {"1.2.3.4/24"};
    {
      TestService service;

      BOOST_TEST(!service.isValid());
      service.setServiceName("serviceName");
      BOOST_TEST(!service.isValid());
      service.setDstAddress(ipAddr);
      BOOST_TEST(service.isValid());
      service.addDstPort("0-65535");
      BOOST_TEST(service.isValid());
      BOOST_TEST(service.isValidDevice());
      BOOST_TEST(!service.isValidNetwork());
    }

    {
      TestService service;

      BOOST_TEST(!service.isValid());
      service.setDstAddress(ipAddr);
      BOOST_TEST(!service.isValid());
      service.addDstPort("0-65535");
      BOOST_TEST(!service.isValid());
      service.setProtocol("protocol");
      BOOST_TEST(service.isValid());
      BOOST_TEST(!service.isValidDevice());
      BOOST_TEST(service.isValidNetwork());
    }
  }
}
