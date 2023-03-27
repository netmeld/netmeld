// =============================================================================
// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
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

#include <netmeld/datastore/objects/Route.hpp>
#include <netmeld/core/utils/StringUtilities.hpp>
#include <boost/format.hpp>

namespace nmcu = netmeld::core::utils;


namespace netmeld::datastore::objects {
  Route::Route()
  {}

  void
  Route::setVrfId(const std::string& _vrfId)
  {
    vrfId = _vrfId;  // preserve case.
  }

  void
  Route::setTableId(const std::string& _tableId)
  {
    tableId = _tableId;  // preserve case.
  }

  void
  Route::setDstIpNet(const IpAddress& _dstIpNet)
  {
    dstIpNet = _dstIpNet;
  }

  void
  Route::setNextVrfId(const std::string& _nextVrfId)
  {
    nextVrfId = _nextVrfId;  // preserve case.
  }

  void
  Route::setNextTableId(const std::string& _nextTableId)
  {
    nextTableId = _nextTableId;  // preserve case.
  }

  void
  Route::setNextHopIpAddr(const IpAddress& _nextHopIpAddr)
  {
    nextHopIpAddr = _nextHopIpAddr;

    // if needed, init dstIpNet as default route
    if (!nextHopIpAddr.hasUnsetPrefix() && dstIpNet.hasUnsetPrefix()) {
      if (nextHopIpAddr.isV4()) {
        dstIpNet = IpNetwork::getIpv4Default();
      } else {
        dstIpNet = IpNetwork::getIpv6Default();
      }
      dstIpNet.setReason(defaultRouteReason);
    }
  }

  void
  Route::setIfaceName(const std::string& _ifaceName)
  {
    ifaceName = nmcu::toLower(_ifaceName);

    // if needed, init nextHopIpAddr to the correct IP version
    if (  !dstIpNet.hasUnsetPrefix()
       && nextHopIpAddr.hasUnsetPrefix()
       && !ifaceName.empty()
       ) {
      if (dstIpNet.isV4()) {
        nextHopIpAddr = IpAddress::getIpv4Default();
      } else {
        nextHopIpAddr = IpAddress::getIpv6Default();
      }
      nextHopIpAddr.setReason(defaultRouteReason);
    }
  }

  void
  Route::setProtocol(const std::string& _protocol)
  {
    protocol = nmcu::toLower(_protocol);
  }

  void
  Route::setAdminDistance(size_t _adminDistance)
  {
    adminDistance = _adminDistance;
  }

  void
  Route::setMetric(size_t _metric)
  {
    metric = _metric;
  }

  void
  Route::setActive(bool _isActive)
  {
    isActive = _isActive;
  }

  void
  Route::setNullRoute(bool _isNullRoute)
  {
    isNullRoute = _isNullRoute;
  }

  void
  Route::setDescription(const std::string& _description)
  {
    description = _description;
  }

  std::string
  Route::getNextHopIpAddrString() const
  {
    std::string nextHopIpAddrString {""};

    if (!isNullRoute) {
      nextHopIpAddrString = nextHopIpAddr.toString();
    }

    return nextHopIpAddrString;
  }

  bool
  Route::isValid() const
  {
    // table lookups require both vrf-id and table-id to be set
    bool isTableLookupRoute {
        !(nextVrfId.empty() || nextTableId.empty())
      };
    // next-hops require both ifaceName and nextHopIpAddr to be set
    bool isNextHopRoute {
        !(ifaceName.empty() || nextHopIpAddr.hasUnsetPrefix())
      };

    return (  // dstIpNet cannot be IpAddress default value; Route default OK
              (!dstIpNet.hasUnsetPrefix())
              // XOR; next-hop or table-lookup must be defined, not both
           && (  (isNextHopRoute != isTableLookupRoute)
                 // XOR; prior or null-route must be defined, not both
              != isNullRoute
              )
           )
      ;
  }

  bool
  Route::isV4() const
  {
    return dstIpNet.isV4();
  }

  bool
  Route::isV6() const
  {
    return dstIpNet.isV6();
  }

  void
  Route::save(pqxx::transaction_base& t,
              const nmco::Uuid& toolRunId, const std::string& deviceId)
  {
    if (!isValid() && !deviceId.empty()) {
      LOG_DEBUG << "Route object is not saving: " << toDebugString()
                << std::endl;
      return;
    }

    dstIpNet.save(t, toolRunId, deviceId);
    nextHopIpAddr.save(t, toolRunId, deviceId);

    t.exec_prepared("insert_raw_device_ip_route"
      , toolRunId
      , deviceId // insert converts to lower
      , vrfId
      , tableId
      , isActive
      , dstIpNet.toString()
      , nextVrfId // insert converts '' to null
      , nextTableId // insert converts '' to null
      , getNextHopIpAddrString() // insert converts '' to null
      , ifaceName // insert converts '' to null
      , protocol // insert converts to lower and '' to null
      , adminDistance
      , metric
      , description // insert converts '' to null
      );
  }

  void
  Route::saveAsMetadata(pqxx::transaction_base& t, const nmco::Uuid& toolRunId)
  {
    if (!isValid()) {
      LOG_DEBUG << "Route object is not saving as metadata: " << toDebugString()
                << std::endl;
      return;
    }

    t.exec_prepared("insert_tool_run_ip_route"
      , toolRunId
      , ifaceName
      , dstIpNet.toString()
      , getNextHopIpAddrString()
      );
  }

  std::string
  Route::toDebugString() const
  {
    std::ostringstream oss;

    oss << "["
        << "vrfId: " << vrfId
        << ", tableId: " << tableId
        << ", isActive: " << isActive
        << ", dstIpNet: " << dstIpNet
        << ", nextVrfId: " << nextVrfId
        << ", nextTableId: " << nextTableId
        << ", nextHopIpAddr: " << nextHopIpAddr
        << ", ifaceName: " << ifaceName
        << ", protocol: " << protocol
        << ", adminDistance: " << adminDistance
        << ", metric: " << metric
        << ", isNullRoute: " << isNullRoute
        << ", description: " << description
        << "]"
        ;

    return oss.str();
  }

  std::partial_ordering
  Route::operator<=>(const Route& rhs) const
  {
    return std::tie( vrfId
                   , tableId
                   , dstIpNet
                   , nextVrfId
                   , nextTableId
                   , nextHopIpAddr
                   , ifaceName
                   , protocol
                   , description
                   , adminDistance
                   , metric
                   , isActive
                   , isNullRoute
                   )
       <=> std::tie( rhs.vrfId
                   , rhs.tableId
                   , rhs.dstIpNet
                   , rhs.nextVrfId
                   , rhs.nextTableId
                   , rhs.nextHopIpAddr
                   , rhs.ifaceName
                   , rhs.protocol
                   , rhs.description
                   , rhs.adminDistance
                   , rhs.metric
                   , rhs.isActive
                   , rhs.isNullRoute
                   )
      ;
  }

  bool
  Route::operator==(const Route& rhs) const
  {
    return 0 == operator<=>(rhs);
  }
}
