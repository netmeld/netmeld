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

#include <netmeld/core/objects/Route.hpp>
#include <netmeld/core/utils/StringUtilities.hpp>

namespace nmcu = netmeld::core::utils;


namespace netmeld::core::objects {
  Route::Route()
  {}

  void
  Route::setDstNet(const IpAddress& _dstNet)
  {
    dstNet = _dstNet;
  }

  void
  Route::setRtrIp(const IpAddress& _rtrIp)
  {
    rtrIp = _rtrIp;
  }

  void
  Route::setIfaceName(const std::string& _ifaceName)
  {
    ifaceName = nmcu::toLower(_ifaceName);
  }

  bool
  Route::isValid() const
  {
    return !(rtrIp.isDefault() && dstNet.isDefault())
        ;
  }

  void
  Route::updateForSave(const bool isMetadataSave)
  {
    if (rtrIp.isDefault() && !dstNet.isDefault()) {
      if (isMetadataSave && dstNet.isV6()) {
        rtrIp = IpAddress::getIpv6Default();
      } else {
        // Always use IPv4 default for datastore as only one null case
        rtrIp = IpAddress::getIpv4Default();
      }
    }
    else if (!rtrIp.isDefault() && dstNet.isDefault()) {
      if (rtrIp.isV4()) {
        dstNet = IpNetwork::getIpv4Default();
      } else {
        dstNet = IpNetwork::getIpv6Default();
      }
    }
  }

  void
  Route::save(pqxx::transaction_base& t,
              const Uuid& toolRunId, const std::string& deviceId)
  {
    if (!isValid() && !deviceId.empty()) {
      LOG_DEBUG << "Route object is not saving: " << toDebugString()
                << std::endl;
      return;
    }
    updateForSave(false);

    dstNet.save(t, toolRunId, deviceId);
    rtrIp.save(t, toolRunId, deviceId);

    t.exec_prepared("insert_raw_device_ip_route",
        toolRunId,
        deviceId, // insert converts to lower
        ifaceName, // insert converts '' to null
        dstNet.toString(),
        rtrIp.toString()); // insert converts 0.0.0.0/0 to null
  }

  void
  Route::saveAsMetadata(pqxx::transaction_base& t, const Uuid& toolRunId)
  {
    if (!isValid()) {
      LOG_DEBUG << "Route object is not saving as metadata: " << toDebugString()
                << std::endl;
      return;
    }
    updateForSave(true);

    t.exec_prepared("insert_tool_run_ip_route",
        toolRunId,
        ifaceName,
        dstNet.toString(),
        rtrIp.toString());
  }

  std::string
  Route::toDebugString() const
  {
    std::ostringstream oss;

    oss << "[";

    oss << ifaceName
        << ", " << dstNet.toString()
        << ", " << rtrIp.toString()
        ;

    oss << "]";

    return oss.str();
  }

  bool
  operator==(const Route& first, const Route& second)
  {
    return first.dstNet == second.dstNet
        && first.rtrIp == second.rtrIp
        && first.ifaceName == second.ifaceName
        ;
  }
}
