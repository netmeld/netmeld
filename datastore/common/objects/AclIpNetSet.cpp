// =============================================================================
// Copyright 2020 National Technology & Engineering Solutions of Sandia, LLC
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

#include <netmeld/datastore/objects/AclIpNetSet.hpp>
#include <netmeld/core/utils/StringUtilities.hpp>

namespace nmcu = netmeld::core::utils;


namespace netmeld::datastore::objects {

  // ===========================================================================
  // Constructors
  // ===========================================================================
  AclIpNetSet::AclIpNetSet()
  {}

  // ===========================================================================
  // Methods
  // ===========================================================================
  bool
  AclIpNetSet::isValid() const
  {
    return !id.empty();
  }

  void
  AclIpNetSet::setId(const std::string& _id, const std::string& _ns)
  {
    ns = _ns;
    id = _id;
  }

  void
  AclIpNetSet::addIpNet(const IpNetwork& _ipNet)
  {
    ipNets.emplace_back(_ipNet);
  }

  void
  AclIpNetSet::addHostname(const std::string& _hostname)
  {
    hostnames.emplace_back(nmcu::toLower(_hostname));
  }

  void
  AclIpNetSet::addIncludedId(const std::string& _includedId, const std::string& _includedNs)
  {
    includedIds.emplace_back(_includedNs, _includedId);
  }

  void
  AclIpNetSet::save(pqxx::transaction_base& t,
      const nmco::Uuid& toolRunId, const std::string& deviceId)
  {
    if (!isValid()) {
      LOG_DEBUG << "AclIpNetSet object is not saving: " << toDebugString()
                << std::endl;
      return; // Always short circuit if invalid object
    }

    t.exec_prepared("insert_raw_device_acl_ip_net_base",
        toolRunId,
        deviceId,
        ns,
        id
        );

    for (const auto& ipNet : ipNets) {
      t.exec_prepared("insert_raw_device_acl_ip_net_ip_net",
          toolRunId,
          deviceId,
          ns,
          id,
          ipNet.toString()
          );
    }

    for (const auto& hostname : hostnames) {
      t.exec_prepared("insert_raw_device_dns_reference",
          toolRunId,
          deviceId,
          hostname
          );
      t.exec_prepared("insert_raw_device_acl_ip_net_hostname",
          toolRunId,
          deviceId,
          ns,
          id,
          hostname
          );
    }

    for (const auto& includedId : includedIds) {
      t.exec_prepared("insert_raw_device_acl_ip_net_include",
          toolRunId,
          deviceId,
          ns,
          id,
          std::get<0>(includedId),  // ns
          std::get<1>(includedId)   // id
          );
    }
  }

  std::string
  AclIpNetSet::toDebugString() const
  {
    std::ostringstream oss;

    oss << "["
        << "ns: "
        << ns
        << ", "
        << "id: "
        << id
        << ", ";

    oss << "ipNets: "
        << "[";
        
    if (!ipNets.empty()) {
        oss << ipNets.front().toDebugString();
    
      for (auto it = ipNets.begin() + 1; it != ipNets.end(); it++) {
        oss << ", "
            << it->toDebugString();
      }
    }
    oss << "]"
        << ", ";

    oss << "hostnames: "
        << "[";
        
    if (!hostnames.empty()) {
        oss << hostnames.front();
    
      for (auto it = hostnames.begin() + 1; it != hostnames.end(); it++) {
        oss << ", "
            << *it;
      }
    }
    oss << "]"
        << ", ";

    oss << "includedIds: "
        << "[";

    if (!includedIds.empty()) {
        oss << "[" << std::get<0>(includedIds.front()) << ", " << std::get<1>(includedIds.front()) << "]"; 
    
      for (auto it = includedIds.begin() + 1; it != includedIds.end(); it++) {
        oss << ", "
            << "["
            << std::get<0>(*it) << ", " << std::get<1>(*it)
            << "]"; 
      }
    }
    oss << "]";

    oss << "]";

    return oss.str();
  }

  // ===========================================================================
  // Friends
  // ===========================================================================
}
