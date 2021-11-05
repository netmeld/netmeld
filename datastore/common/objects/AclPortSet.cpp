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

#include <netmeld/datastore/objects/AclPortSet.hpp>
#include <netmeld/core/utils/StringUtilities.hpp>

namespace nmcu = netmeld::core::utils;


namespace netmeld::datastore::objects {

  // ===========================================================================
  // Constructors
  // ===========================================================================
  AclPortSet::AclPortSet()
  {}

  // ===========================================================================
  // Methods
  // ===========================================================================
  bool
  AclPortSet::isValid() const
  {
    return !id.empty();
  }

  void
  AclPortSet::setId(const std::string& _id)
  {
    id = _id;
  }

  void
  AclPortSet::addPortRange(const PortRange& _portRange)
  {
    portRanges.emplace_back(_portRange);
  }

  void
  AclPortSet::addIncludedId(const std::string& _includedId)
  {
    includedIds.emplace_back(_includedId);
  }

  void
  AclPortSet::save(pqxx::transaction_base& t,
      const nmco::Uuid& toolRunId, const std::string& deviceId)
  {
    if (!isValid()) {
      LOG_DEBUG << "AclPortSet object is not saving: " << toDebugString()
                << std::endl;
      return; // Always short circuit if invalid object
    }

    t.exec_prepared("insert_raw_device_acl_port_base",
        toolRunId,
        deviceId,
        id
        );

    for (const auto& portRange : portRanges) {
      t.exec_prepared("insert_raw_device_acl_port_port",
          toolRunId,
          deviceId,
          id,
          portRange
          );
    }

    for (const auto& includedId : includedIds) {
      t.exec_prepared("insert_raw_device_acl_port_include",
          toolRunId,
          deviceId,
          id,
          includedId
          );
    }
  }

  // ===========================================================================
  // Friends
  // ===========================================================================
}
