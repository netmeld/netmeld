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

#include <netmeld/datastore/objects/Service.hpp>

namespace nmdo = netmeld::datastore::objects;


class TestService : public nmdo::Service {
  public:
    TestService() : Service() {};
    TestService(const std::string& _name, nmdo::IpAddress& _ip) :
        Service(_name, _ip) {};

  public:
    std::string const getAnyPort() const
    { return ANY_PORT; }

    nmdo::IpAddress getDstAddress() const
    { return dstAddress; }

    nmdo::IpAddress getSrcAddress() const
    { return srcAddress; }

    bool getIsLocal() const
    { return isLocal; }

    std::string getInterfaceName() const
    { return interfaceName; }

    std::string getZone() const
    { return zone; }

    std::string getServiceDescription() const
    { return serviceDescription; }

    std::string getServiceReason() const
    { return serviceReason; }

    std::string getProtocol() const
    { return protocol; }

    std::set<std::string> getDstPorts() const
    { return dstPorts; }

    std::set<std::string> getSrcPorts() const
    { return srcPorts; }

    bool isValidDevice() const
    { return Service::isValidDevice(); }

    bool isValidNetwork() const
    { return Service::isValidNetwork(); }
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  {
    nmdo::IpAddress ipAddr;
    TestService service;

    BOOST_CHECK_EQUAL("0-65535", service.getAnyPort());
    BOOST_CHECK_EQUAL(ipAddr, service.getDstAddress());
    BOOST_CHECK_EQUAL(ipAddr, service.getSrcAddress());
    BOOST_CHECK(!service.getIsLocal());
    BOOST_CHECK_EQUAL("-", service.getInterfaceName());
    BOOST_CHECK(service.getZone().empty());
    BOOST_CHECK(service.getServiceName().empty());
    BOOST_CHECK(service.getServiceDescription().empty());
    BOOST_CHECK(service.getServiceReason().empty());
    BOOST_CHECK(service.getProtocol().empty());
    BOOST_CHECK(service.getDstPorts().empty());
    BOOST_CHECK(service.getSrcPorts().empty());
  }

  {
    nmdo::IpAddress ipAddr {"1.2.3.4/24"};
    nmdo::IpAddress dIpAddr;
    TestService service {"Service", ipAddr};

    BOOST_CHECK_EQUAL("0-65535", service.getAnyPort());
    BOOST_CHECK_EQUAL(ipAddr, service.getDstAddress());
    BOOST_CHECK_EQUAL(dIpAddr, service.getSrcAddress());
    BOOST_CHECK(!service.getIsLocal());
    BOOST_CHECK_EQUAL("-", service.getInterfaceName());
    BOOST_CHECK(service.getZone().empty());
    BOOST_CHECK_EQUAL("Service", service.getServiceName());
    BOOST_CHECK(service.getServiceDescription().empty());
    BOOST_CHECK(service.getServiceReason().empty());
    BOOST_CHECK(service.getProtocol().empty());
    BOOST_CHECK(service.getDstPorts().empty());
    BOOST_CHECK(service.getSrcPorts().empty());
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    TestService service;

    service.addDstPort("1");
    BOOST_CHECK_EQUAL(1, service.getDstPorts().size());
    BOOST_CHECK(service.getDstPorts().count("1"));
    service.addDstPort("0-65535");
    BOOST_CHECK_EQUAL(2, service.getDstPorts().size());
    BOOST_CHECK(service.getDstPorts().count("0-65535"));
    service.addDstPort("0-65535");
    BOOST_CHECK_EQUAL(2, service.getDstPorts().size());
  }

  {
    TestService service;

    service.addSrcPort("1");
    BOOST_CHECK_EQUAL(1, service.getSrcPorts().size());
    BOOST_CHECK(service.getSrcPorts().count("1"));
    service.addSrcPort("0-65535");
    BOOST_CHECK_EQUAL(2, service.getSrcPorts().size());
    BOOST_CHECK(service.getSrcPorts().count("0-65535"));
    service.addSrcPort("0-65535");
    BOOST_CHECK_EQUAL(2, service.getSrcPorts().size());
  }

  {
    TestService service;
    nmdo::IpAddress ipAddr {"1.2.3.4/24"};

    service.setDstAddress(ipAddr);
    BOOST_CHECK_EQUAL(ipAddr, service.getDstAddress());
  }

  {
    TestService service;
    nmdo::IpAddress ipAddr {"1.2.3.4/24"};

    service.setSrcAddress(ipAddr);
    BOOST_CHECK_EQUAL(ipAddr, service.getSrcAddress());
  }

  {
    TestService service;

    service.setInterfaceName("Ifname");
    BOOST_CHECK_EQUAL("ifname", service.getInterfaceName());
  }

  {
    TestService service;

    service.setServiceName("Service Name");
    BOOST_CHECK_EQUAL("Service Name", service.getServiceName());
  }

  {
    TestService service;

    service.setServiceDescription("Service Description");
    BOOST_CHECK_EQUAL("Service Description", service.getServiceDescription());
  }

  {
    TestService service;

    service.setServiceReason("Service Reason");
    BOOST_CHECK_EQUAL("service reason", service.getServiceReason());
  }

  {
    TestService service;

    service.setProtocol("Protocol");
    BOOST_CHECK_EQUAL("protocol", service.getProtocol());
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

      BOOST_CHECK(!service.isValid());
      service.setServiceName("serviceName");
      BOOST_CHECK(!service.isValid());
      service.setDstAddress(ipAddr);
      BOOST_CHECK(service.isValid());
      service.addDstPort("0-65535");
      BOOST_CHECK(service.isValid());
      BOOST_CHECK(service.isValidDevice());
      BOOST_CHECK(!service.isValidNetwork());
    }

    {
      TestService service;

      BOOST_CHECK(!service.isValid());
      service.setDstAddress(ipAddr);
      BOOST_CHECK(!service.isValid());
      service.addDstPort("0-65535");
      BOOST_CHECK(!service.isValid());
      service.setProtocol("protocol");
      BOOST_CHECK(service.isValid());
      BOOST_CHECK(!service.isValidDevice());
      BOOST_CHECK(service.isValidNetwork());
    }
  }
}
