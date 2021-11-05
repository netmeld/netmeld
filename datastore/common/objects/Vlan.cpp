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

#include <netmeld/datastore/objects/Vlan.hpp>
#include <netmeld/core/utils/StringUtilities.hpp>

namespace nmcu = netmeld::core::utils;


namespace netmeld::datastore::objects {

  // ===========================================================================
  // Constructors
  // ===========================================================================
  Vlan::Vlan()
  {}

  Vlan::Vlan(const uint16_t _vlanId, const std::string& _description) :
    vlanId(_vlanId)
  {
    setDescription(_description);
  }

  // ===========================================================================
  // Methods
  // ===========================================================================
  uint16_t
  Vlan::getVlanId() const
  {
    return vlanId;
  }

  void
  Vlan::setDescription(const std::string& _description)
  {
    description = _description;
  }

  void
  Vlan::setId(const uint16_t _vlanId)
  {
    vlanId = _vlanId;
  }

  void
  Vlan::setIpNet(const IpNetwork& _ipNet)
  {
    ipNet = _ipNet;
  }

  bool
  Vlan::isValid() const
  {
    // valid VLAN range is 0-4095
    return vlanId < 4096;
  }

  void
  Vlan::save(pqxx::transaction_base& t,
             const nmco::Uuid& toolRunId, const std::string& deviceId)
  {
    if (!isValid()) {
      LOG_DEBUG << "Vlan object is not saving: " << toDebugString()
                << std::endl;
      return;
    }

    if (deviceId.empty()) {
      t.exec_prepared("insert_raw_vlan",
          toolRunId,
          vlanId,
          description);

      // Associate VLAN to network
      ipNet.save(t, toolRunId, deviceId);
      if (ipNet.isValid()) {
        t.exec_prepared("insert_raw_vlan_ip_net",
            toolRunId,
            vlanId,
            ipNet.toString());
      }
    } else {
      t.exec_prepared("insert_raw_device_vlan",
          toolRunId,
          deviceId,
          vlanId,
          description);

      // Associate VLAN to network
      ipNet.save(t, toolRunId, deviceId);
      if (ipNet.isValid()) {
        t.exec_prepared("insert_raw_device_vlan_ip_net",
            toolRunId,
            deviceId,
            vlanId,
            ipNet.toString());
      }
    }
  }

  std::string
  Vlan::toDebugString() const
  {
    std::ostringstream oss;
    oss << "[" // opening bracket
        << vlanId << ", "
        << ipNet << ", "
        << description
        << "]"; // closing bracket

    return oss.str();
  }

  std::partial_ordering
  Vlan::operator<=>(const Vlan& rhs) const
  {
    if (auto cmp = vlanId <=> rhs.vlanId; 0 != cmp) {
      return cmp;
    }
    if (auto cmp = description <=> rhs.description; 0 != cmp) {
      return cmp;
    }
    return ipNet <=> rhs.ipNet;
  }

  bool
  Vlan::operator==(const Vlan& rhs) const
  {
    return 0 == operator<=>(rhs);
  }
}
