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

#include <netmeld/datastore/objects/DeviceInformation.hpp>
#include <netmeld/core/utils/StringUtilities.hpp>

namespace nmcu = netmeld::core::utils;


namespace netmeld::datastore::objects {

  DeviceInformation::DeviceInformation()
  {}

  DeviceInformation::DeviceInformation(const std::string& _deviceVendor)
  {
    setVendor(_deviceVendor);
  }

  void
  DeviceInformation::setDeviceId(const std::string& _id)
  {
    deviceId = nmcu::toLower(_id);
  }

  void
  DeviceInformation::setDeviceColor(const std::string& _deviceColor)
  {
    deviceColor = _deviceColor;
  }

  void
  DeviceInformation::setDeviceType(const std::string& _deviceType)
  {
    deviceType = nmcu::toLower(_deviceType);
  }

  void
  DeviceInformation::setDescription(const std::string& _desc)
  {
    description = nmcu::toLower(_desc);
  }

  void
  DeviceInformation::setHardwareRevision(const std::string& _hRev)
  {
    hardwareRevision = nmcu::toUpper(_hRev);
  }

  void
  DeviceInformation::setModel(const std::string& _model)
  {
    model = nmcu::toUpper(_model);
  }

  void
  DeviceInformation::setSerialNumber(const std::string& _sn)
  {
    serialNumber = nmcu::toUpper(_sn);
  }

  void
  DeviceInformation::setVendor(const std::string& _vendor)
  {
    vendor = nmcu::toLower(_vendor);
  }

  std::string
  DeviceInformation::getDeviceId() const
  {
    return deviceId;
  }

  std::string
  DeviceInformation::getDeviceType() const
  {
    return deviceType;
  }

  bool
  DeviceInformation::isValid() const
  {
    return !deviceId.empty()
        ;
  }

  void
  DeviceInformation::save(pqxx::transaction_base& t,
                          const nmco::Uuid& toolRunId, const std::string&)
  {
    if (!isValid()) {
      LOG_DEBUG << "DeviceInformation object is not saving: "
                << toDebugString() << std::endl;
      return;
    }

    t.exec_prepared("insert_raw_device",
      toolRunId,
      deviceId);

    if (   !deviceType.empty()
        || !vendor.empty()
        || !model.empty()
        || !hardwareRevision.empty()
        || !serialNumber.empty()
        || !description.empty()
       )
    {
      t.exec_prepared("insert_raw_device_hardware_information",
        toolRunId,
        deviceId,
        deviceType, // query converts empty to NULL
        vendor, // query converts empty to NULL
        model, // query converts empty to NULL
        hardwareRevision, // query converts empty to NULL
        serialNumber, // query converts empty to NULL
        description); // query converts empty to NULL
    }

    if (!deviceColor.empty()) {
      t.exec_prepared("insert_device_color",
        deviceId,
        deviceColor);
    }
  }

  std::string
  DeviceInformation::toDebugString() const
  {
    std::ostringstream oss;

    oss << "["
        << "id: " << deviceId << ", "
        << "color: " << deviceColor << ", "
        << "type: " << deviceType << ", "
        << "vendor: " << vendor << ", "
        << "model: " << model << ", "
        << "rev: " << hardwareRevision << ", "
        << "sn: " << serialNumber << ", "
        << "desc: " << description
        << "]";

    return oss.str();
  }

  std::partial_ordering
  DeviceInformation::operator<=>(const DeviceInformation& rhs) const
  {
    return std::tie( deviceId
                   , deviceType
                   , deviceColor
                   , vendor
                   , model
                   , hardwareRevision
                   , serialNumber
                   , description
                   )
       <=> std::tie( rhs.deviceId
                   , rhs.deviceType
                   , rhs.deviceColor
                   , rhs.vendor
                   , rhs.model
                   , rhs.hardwareRevision
                   , rhs.serialNumber
                   , rhs.description
                   )
      ;
  }

  bool
  DeviceInformation::operator==(const DeviceInformation& rhs) const
  {
    return 0 == operator<=>(rhs);
  }
}
