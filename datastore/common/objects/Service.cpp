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

#include <numeric>

#include <netmeld/datastore/objects/Port.hpp>
#include <netmeld/datastore/objects/Service.hpp>
#include <netmeld/core/utils/StringUtilities.hpp>

namespace nmcu = netmeld::core::utils;


namespace netmeld::datastore::objects {

  Service::Service()
  {}

  Service::Service(const std::string& _name, const IpAddress& _addr) :
    dstAddress(_addr)
  {
    setServiceName(_name);
  }

  void
  Service::addDstPort(const std::string& port)
  {
    dstPorts.insert(port);
  }

  void
  Service::addSrcPort(const std::string& port)
  {
    srcPorts.insert(port);
  }

  std::string
  Service::getServiceName() const
  {
    return serviceName;
  }

  void
  Service::setDstAddress(const IpAddress& _addr)
  {
    dstAddress = _addr;
  }

  void
  Service::setSrcAddress(const IpAddress& _addr)
  {
    srcAddress = _addr;
  }

  void
  Service::setInterfaceName(const std::string& _name)
  {
    interfaceName = nmcu::toLower(_name);
  }

  void
  Service::setServiceName(const std::string& _name)
  {
    serviceName = _name;
  }

  void
  Service::setServiceDescription(const std::string& _desc)
  {
    serviceDescription = nmcu::toLower(_desc);
  }

  void
  Service::setServiceReason(const std::string& _reason)
  {
    serviceReason = nmcu::toLower(_reason);
  }

  void
  Service::setProtocol(const std::string& _protocol)
  {
    protocol = nmcu::toLower(_protocol);
  }

  bool
  Service::isValid() const
  {
    return isValidDevice()
        || isValidNetwork();
  }

  bool
  Service::isValidDevice() const
  {
    return !serviceName.empty()
        && dstAddress.isValid()
        ;
  }

  bool
  Service::isValidNetwork() const
  {
    return dstAddress.isValid()
        && !dstPorts.empty()
        && !protocol.empty()
        ;
  }

  void
  Service::save(pqxx::transaction_base& t,
                const nmco::Uuid& toolRunId, const std::string& deviceId)
  {
    if (!isValid()) {
      LOG_DEBUG << "Service object is not saving: " << toDebugString()
                << std::endl;
      return;
    }

    // IpAddress.isResponding state should be set before passed to this
    dstAddress.save(t, toolRunId, deviceId);
    srcAddress.save(t, toolRunId, deviceId);

    if (isValidDevice() && !deviceId.empty()) {
      LOG_DEBUG << "Saving as Device Service\n";
      saveAsDevice(t, toolRunId, deviceId);
    }
    if (isValidNetwork() && deviceId.empty()) {
      LOG_DEBUG << "Saving as Network Service\n";
      saveAsNetwork(t, toolRunId);
    }
  }

  void
  Service::saveAsDevice(pqxx::transaction_base& t,
                        const nmco::Uuid& toolRunId,
                        const std::string& deviceId)
  {
    if (dstPorts.empty()) {
      t.exec_prepared("insert_raw_device_ip_server",
          toolRunId,
          deviceId,
          interfaceName,
          serviceName,
          dstAddress.toString(),
          nullptr,
          isLocal,
          serviceDescription); // insert converts '' to null
    } else {
      for (const auto& dstPort : dstPorts) {
        t.exec_prepared("insert_raw_device_ip_server",
            toolRunId,
            deviceId,
            interfaceName,
            serviceName,
            dstAddress.toString(),
            dstPort,
            isLocal,
            serviceDescription); // insert converts '' to null
      }
    }
  }

  void
  Service::saveAsNetwork(pqxx::transaction_base& t,
                         const nmco::Uuid& toolRunId)
  {
    if (srcAddress.isDefault()) {
      // Always use IPv4 default for datastore as only one null case
      srcAddress = IpAddress::getIpv4Default();
    }

    for (const auto& dstPort : dstPorts) {
      Port port(dstAddress);
      port.setProtocol(protocol);
      port.setPort(std::stoi(dstPort));
      port.save(t, toolRunId, "");

      t.exec_prepared("insert_raw_network_service",
          toolRunId,
          dstAddress.toString(),
          protocol,
          dstPort,
          serviceName,
          serviceDescription,
          serviceReason,
          srcAddress.toString());
    }
  }

  std::string
  Service::toDebugString() const
  {
    std::ostringstream oss;
    oss << "["
        // Start of formatting
        << dstAddress << ", "
        << srcAddress << ", "
        << isLocal << ", "
        << interfaceName << ", "
        << zone << ", "
        << serviceName << ", "
        << serviceDescription << ", "
        << serviceReason << ", "
        << protocol << ", "
        << dstPorts
        << srcPorts
        // End of formatting
        << "]";

    return oss.str();
  }

  bool
  operator==(const Service& first, const Service& second)
  {
    return first.dstAddress == second.dstAddress
        && first.srcAddress == second.srcAddress
        && first.isLocal == second.isLocal
        && first.interfaceName == second.interfaceName
        && first.zone == second.zone
        && first.serviceName == second.serviceName
        && first.serviceDescription == second.serviceDescription
        && first.serviceReason == second.serviceReason
        && first.protocol == second.protocol
        && first.dstPorts == second.dstPorts
        && first.srcPorts == second.srcPorts
        ;
  }

  bool
  operator<(const Service& first, const Service& second)
  {
    if (first == second) { return false; }

    if (first.serviceName == second.serviceName) {
      if (first.protocol == second.protocol) {
        return first.dstAddress < second.dstAddress;
      } else {
        return first.protocol < second.protocol;
      }
    } else {
      return first.serviceName < second.serviceName;
    }
  }
}
