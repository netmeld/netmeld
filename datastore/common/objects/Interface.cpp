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

#include <netmeld/datastore/objects/Interface.hpp>
#include <netmeld/core/utils/StringUtilities.hpp>

namespace nmcu = netmeld::core::utils;


namespace netmeld::datastore::objects {

  Interface::Interface()
  {}

  Interface::Interface(const std::string& _name) :
    name(nmcu::toLower(_name))
  {}

  void
  Interface::setName(const std::string& _name)
  {
    name = nmcu::toLower(_name);
  }

  void
  Interface::setMediaType(const std::string& _mediaType)
  {
    mediaType = nmcu::toLower(_mediaType);
  }

  void
  Interface::setMacAddress(const MacAddress& _macAddr)
  {
    macAddr.setMac(_macAddr);
  }

  void
  Interface::setUp()
  {
    isUp = true;
  }

  void
  Interface::setDown()
  {
    isUp = false;
  }

  void
  Interface::addIpAddress(const IpAddress& ipAddr)
  {
    macAddr.addIpAddress(ipAddr);
  }

  std::string
  Interface::getName() const
  {
    return name;
  }

  MacAddress
  Interface::getMacAddress() const
  {
    return macAddr;
  }

  const std::set<IpAddress>&
  Interface::getIpAddresses() const
  {
    return macAddr.getIpAddresses();
  }

  bool
  Interface::isValid() const
  {
    return !name.empty()
        && !mediaType.empty()
        && "loopback" != mediaType
        ;
  }

  void
  Interface::save(pqxx::transaction_base& t,
                  const nmco::Uuid& toolRunId, const std::string& deviceId)
  {
    if (!isValid() && !deviceId.empty()) {
      LOG_DEBUG << "Interface object is not saving: " << toDebugString()
                << std::endl;
      return;
    }

    //LOG_DEBUG << "Inserting interface" << std::endl;
    t.exec_prepared("insert_raw_device_interface",
      toolRunId,
      deviceId,
      name,
      mediaType,
      isUp,
      nullptr);

    macAddr.setResponding(isUp);
    macAddr.save(t, toolRunId, deviceId);

    // Tie interface to MAC
    if (macAddr.isValid()) {
      t.exec_prepared("insert_raw_device_mac_addr",
        toolRunId,
        deviceId,
        name,
        macAddr.toString());
    } else {
      LOG_WARN << "Invalid MAC for: "
           << deviceId << ", " << name << ", " << macAddr
           << std::endl;
    }

    // Tie interface to IP
    for (auto& ipAddr : macAddr.getIpAddresses()) {
      if (!ipAddr.isValid()) { continue; }

      t.exec_prepared("insert_raw_device_ip_addr",
        toolRunId,
        deviceId,
        name,
        ipAddr.toString());
    }
  }

  void
  Interface::saveAsMetadata(pqxx::transaction_base& t,
                            const nmco::Uuid& toolRunId)
  {
    if (!isValid()) {
      LOG_DEBUG << "Interface object is not saving as metadata: "
                << toDebugString() << std::endl;
      return;
    }

    t.exec_prepared("insert_tool_run_interface",
      toolRunId,
      name,
      mediaType,
      isUp);

    if (macAddr.isValid()) {
      t.exec_prepared("insert_tool_run_mac_addr",
        toolRunId,
        name,
        macAddr.toString());
    }

    for (const auto& ipAddr : macAddr.getIpAddresses()) {
      if (!ipAddr.isValid()) { continue; }

      t.exec_prepared("insert_tool_run_ip_addr",
        toolRunId,
        name,
        ipAddr.toString());
    }
  }

  std::string
  Interface::toDebugString() const
  {
    std::ostringstream oss;

    oss << "["; // opening bracket

    oss << name << ", "
        << std::boolalpha << isUp << ", "
        << mediaType << ", "
        << macAddr.toDebugString() << ", "
        ;

    oss << "]"; // closing bracket

    return oss.str();
  }

  void
  Interface::setFlags(const std::string& _flags)
  {
    flags = _flags;

    if (flags.find(",UP") != std::string::npos ||
        flags.find("<UP") != std::string::npos)
    { setUp(); }
    else { setDown(); }
  }

  void
  Interface::setMtu(uint32_t _mtu)
  {
    mtu = _mtu;
  }
}
