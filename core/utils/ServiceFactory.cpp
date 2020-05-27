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

#include <netmeld/core/utils/ServiceFactory.hpp>

namespace nmcu = netmeld::core::utils;

namespace netmeld::core::utils {

  nmco::Service
  ServiceFactory::makeDhcp()
  {
    nmco::Service service;
    service.setServiceName("dhcps"); // match nmap output
    service.setProtocol("udp");
    service.addDstPort("67"); // port server uses
    service.addSrcPort("68"); // port client uses
    return service;
  }

  nmco::Service
  ServiceFactory::makeDns()
  {
    nmco::Service service;
    service.setServiceName("dns");
    service.setProtocol("udp");
    service.addDstPort("53");
    return service;
  }

  nmco::Service
  ServiceFactory::makeNtp()
  {
    nmco::Service service;
    service.setServiceName("ntp");
    service.setProtocol("udp");
    service.addDstPort("123"); // same port used by client and server
    service.addSrcPort("123");
    return service;
  }

  nmco::Service
  ServiceFactory::makeRadius()
  {
    nmco::Service service;
    service.setServiceName("radius");
    service.setProtocol("udp");
    service.addDstPort("1812"); // authentication and authorization
    service.addDstPort("1813"); // accounting
    return service;
  }

  nmco::Service
  ServiceFactory::makeSnmp()
  {
    nmco::Service service;
    service.setServiceName("snmp");
    service.setProtocol("udp");
    service.addDstPort("162"); // port manager receives on
    return service;
  }

  nmco::Service
  ServiceFactory::makeSyslog()
  {
    nmco::Service service;
    service.setServiceName("syslog");
    service.setProtocol("udp");
    service.addDstPort("514");
    return service;
  }

  //nmco::Service
  //ServiceFactory::make()
  //{
  //  nmco::Service service;
  //  service.setServiceName("");
  //  return service;
  //}
}
