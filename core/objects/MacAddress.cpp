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

#include <iomanip>

#include <netmeld/core/objects/MacAddress.hpp>
#include <netmeld/core/parsers/ParserHelper.hpp>
#include <netmeld/core/parsers/ParserMacAddress.hpp>

namespace nmcp = netmeld::core::parsers;


namespace netmeld::core::objects {
  MacAddress::MacAddress()
  {}

  MacAddress::MacAddress(const std::vector<uint8_t>& _macAddr) :
    macAddr(_macAddr)
  {}

  MacAddress::MacAddress(const std::string& _macAddr)
  {
    setMac(_macAddr);
  }

  void
  MacAddress::setMac(const std::string& _macAddr)
  {
    nmco::MacAddress m1
      {nmcp::fromString<nmcp::ParserMacAddress, MacAddress>(_macAddr)};
    setMac(m1);
  }

  void
  MacAddress::setMac(const std::vector<uint8_t>& _macAddr)
  {
    macAddr = _macAddr;
  }

  void
  MacAddress::setMac(const MacAddress& _macAddr)
  {
    setMac(_macAddr.macAddr);
  }

  void
  MacAddress::addIp(IpAddress& ipAddr)
  {
    ipAddrs.insert(ipAddr);
  }

  void
  MacAddress::setResponding(bool _isUp)
  {
    isResponding = _isUp;
  }

  bool
  MacAddress::isValid() const
  {
    return 6 == macAddr.size();// || 8 == macAddr.size();
  }

  std::vector<IpAddress>
  MacAddress::getIpAddrs() const
  {
    return std::vector<IpAddress>(ipAddrs.begin(), ipAddrs.end());
  }

  void
  MacAddress::save(pqxx::transaction_base& t,
                   const Uuid& toolRunId, const std::string& deviceId)
  {
    if (isValid()) {
      t.exec_prepared("insert_raw_mac_addr",
        toolRunId,
        toString(),
        isResponding);
    } else {
      LOG_DEBUG << "MacAddress object is not saving: " << toDebugString()
                << std::endl;
    }

    for (auto& ipAddr : getIpAddrs()) {
      ipAddr.setResponding(isResponding);

      ipAddr.save(t, toolRunId, deviceId);

      if (!ipAddr.isValid()) { continue; }

      if (isValid()) {
        t.exec_prepared("insert_raw_mac_addr_ip_addr",
          toolRunId,
          toString(),
          ipAddr.toString());
      } else {
        LOG_DEBUG << "MAC/IP association is not performed." << std::endl;
      }
    }
  }

  std::string
  MacAddress::toString() const
  {
    if (!(6 == macAddr.size() || 8 == macAddr.size())) {
      return "Invalid MAC";
    }

    std::ostringstream oss;
    auto iter {macAddr.begin()};
    oss << std::hex << std::setfill('0') << std::setw(2) 
        << static_cast<unsigned int>(*iter);
    ++iter;
    for (; iter != macAddr.end(); ++iter) {
      oss << ':' << std::hex << std::setfill('0') << std::setw(2) 
          << static_cast<unsigned int>(*iter);
    }

    return oss.str();
  }

  std::string
  MacAddress::toDebugString() const
  {
    std::ostringstream oss;

    oss << "[";
    oss << toString() << ", ";
    for (const auto& ip : ipAddrs) { oss << ip.toDebugString() << ", "; }
    oss << isResponding;
    oss << "]";

    return oss.str();
  }

  bool
  operator<(const MacAddress& first, const MacAddress& second)
  {
    return first.macAddr < second.macAddr;
  }

  bool
  operator==(const MacAddress& first, const MacAddress& second)
  {
    return first.macAddr == second.macAddr
        && first.ipAddrs == second.ipAddrs
        && first.isResponding == second.isResponding
        ;
  }
}
