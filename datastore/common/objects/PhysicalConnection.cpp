// =============================================================================
// Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC
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

#include <netmeld/datastore/objects/PhysicalConnection.hpp>
#include <netmeld/datastore/objects/InterfaceNetwork.hpp>
#include <netmeld/core/utils/StringUtilities.hpp>

namespace nmcu = netmeld::core::utils;


namespace netmeld::datastore::objects {

  PhysicalConnection::PhysicalConnection()
  {}

  void
  PhysicalConnection::setDstDeviceId(const std::string& _id)
  {
    dstDeviceId = nmcu::toLower(_id);
  }

  void
  PhysicalConnection::setSrcDeviceId(const std::string& _id)
  {
    srcDeviceId = nmcu::toLower(_id);
  }

  void
  PhysicalConnection::setDstIfaceName(const std::string& _name)
  {
    dstIfaceName = nmcu::toLower(_name);
  }

  void
  PhysicalConnection::setSrcIfaceName(const std::string& _name)
  {
    srcIfaceName = nmcu::toLower(_name);
  }


  bool
  PhysicalConnection::isValid() const
  {
    return !(  srcDeviceId.empty()
            || srcIfaceName.empty()
            || dstDeviceId.empty()
            || dstIfaceName.empty()
            )
        ;
  }

  void
  PhysicalConnection::save( pqxx::transaction_base& t
                          , const nmco::Uuid& toolRunId
                          , const std::string& deviceId
                          )
  {
    if (srcDeviceId.empty() && !deviceId.empty()) {
      srcDeviceId = deviceId;
    }
    if (!isValid()) {
      LOG_DEBUG << "PhysicalConnection object is not saving: "
                << toDebugString() << std::endl;
      return;
    }

    // Ensure both devices are in DB
    for (const auto& devId : {srcDeviceId, dstDeviceId}) {
      t.exec_prepared( "insert_raw_device"
                     , toolRunId
                     , devId
                     );
    }

    // Ensure src network interface information in DB
    InterfaceNetwork srcIn {srcIfaceName};
    srcIn.setPartial(true); // full details unknown
    srcIn.save(t, toolRunId, deviceId);

    // Add port-to-port connectivity
    t.exec_prepared( "insert_raw_device_phys_connection"
                   , toolRunId
                   , srcDeviceId
                   , srcIfaceName
                   , dstDeviceId
                   , dstIfaceName
                   );
  }

  std::string
  PhysicalConnection::toDebugString() const
  {
    std::ostringstream oss;

    oss << "["
        << "srcDeviceId: " << srcDeviceId
        << ", srcIfaceName: " << srcIfaceName
        << ", dstDeviceId: " << dstDeviceId
        << ", dstIfaceName: " << dstIfaceName
        << "]";

    return oss.str();
  }

  std::strong_ordering
  PhysicalConnection::operator<=>(const PhysicalConnection& rhs) const
  {
    return std::tie( srcDeviceId
                   , srcIfaceName
                   , dstDeviceId
                   , dstIfaceName
                   )
       <=> std::tie( rhs.srcDeviceId
                   , rhs.srcIfaceName
                   , rhs.dstDeviceId
                   , rhs.dstIfaceName
                   )
      ;
  }

  bool
  PhysicalConnection::operator==(const PhysicalConnection& rhs) const
  {
    return 0 == operator<=>(rhs);
  }
}
