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

#include <netmeld/datastore/objects/Vrf.hpp>
#include <netmeld/core/utils/StringUtilities.hpp>
#include <algorithm>
#include <iterator>

namespace nmcu = netmeld::core::utils;


namespace netmeld::datastore::objects {

  // ===========================================================================
  // Constructors
  // ===========================================================================
  Vrf::Vrf()
  {}

  Vrf::Vrf(const std::string& _vrfId) :
    vrfId(_vrfId)
  {}

  // ===========================================================================
  // Methods
  // ===========================================================================
  void
  Vrf::setId(const std::string& _vrfId)
  {
    vrfId = _vrfId;
  }

  std::string
  Vrf::getId() const
  {
    return vrfId;
  }

  void
  Vrf::addIface(const std::string& iface)
  {
    ifaces.emplace_back(nmcu::toLower(iface));
  }

  void
  Vrf::addRoute(const Route& route)
  {
    routes.emplace_back(route);
  }

  void
  Vrf::merge(const Vrf& other)
  {
    std::copy(
        other.ifaces.begin(),
        other.ifaces.end(),
        std::back_inserter(ifaces)
        );

    std::copy(
        other.routes.begin(),
        other.routes.end(),
        std::back_inserter(routes)
        );
  }

  void
  Vrf::save(pqxx::transaction_base& t,
            const nmco::Uuid& toolRunId, const std::string& deviceId)
  {
    t.exec_prepared("insert_raw_device_vrf",
        toolRunId,
        deviceId,
        vrfId
        );

    for (const auto& iface : ifaces) {
      t.exec_prepared("insert_raw_device_vrf_interface",
          toolRunId,
          deviceId,
          vrfId,
          iface
          );
    }

    for (auto& route : routes) {
      route.save(t, toolRunId, deviceId);
    }
  }

  std::string
  Vrf::toDebugString() const
  {
    std::ostringstream oss;
    
    oss << "[" // opening bracket
        << "vrfId: "
        << vrfId
        << ", ";

    oss << "ifaces: "
        << "[";

    for(auto beg = begin(ifaces); beg != end(ifaces); beg++) {
      oss << *beg;
      if (next(beg) != end(ifaces)) {
        oss << ", ";
      }
    }

    oss << "], ";

    oss << "routes: "
        << "[";

    for(auto beg = begin(routes); beg != end(routes); beg++) {
      oss << beg->toDebugString();
      if (next(beg) != end(routes)) {
        oss << ", ";
      }
    }

    oss << "]";

    oss << "]"; // closing bracket

    return oss.str();
  }
}
