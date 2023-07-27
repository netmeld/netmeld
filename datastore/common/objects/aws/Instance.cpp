// =============================================================================
// Copyright 2022 National Technology & Engineering Solutions of Sandia, LLC
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

#include <netmeld/datastore/objects/aws/Instance.hpp>

namespace netmeld::datastore::objects::aws {

  Instance::Instance()
  {}

  void
  Instance::setId(const std::string& _id)
  {
    instanceId = _id;
  }
  void
  Instance::setAvailabilityZone(const std::string& _az)
  {
    availabilityZone = _az;
  }
  void
  Instance::setImageId(const std::string& _id)
  {
    imageId = _id;
  }
  void
  Instance::setArchitecture(const std::string& _arch)
  {
    architecture = _arch;
  }
  void
  Instance::setPlatformDetails(const std::string& _pd)
  {
    platformDetails = _pd;
  }
  void
  Instance::setLaunchTime(const std::string& _lt)
  {
    launchTime = _lt;
  }
  void
  Instance::setStateCode(const uint16_t _code)
  {
    stateCode = _code;
  }
  void
  Instance::setStateName(const std::string& _name)
  {
    stateName = _name;
  }
  void
  Instance::setType(const std::string& _type)
  {
    type = _type;
  }
  void
  Instance::addInterface(const NetworkInterface& _ani)
  {
    const NetworkInterface t;
    if (t == _ani) { return; } // Don't add empties
    interfaces.insert(_ani);
  }

  bool
  Instance::isValid() const
  {
    return !instanceId.empty();
  }

  void
  Instance::save(pqxx::transaction_base& t,
                 const nmco::Uuid& toolRunId, const std::string&)
  {
    if (!isValid()) {
      LOG_DEBUG << "AWS Instance object is not saving: " << toDebugString()
                << std::endl;
      return;
    }

    t.exec_prepared("insert_raw_aws_instance"
        , toolRunId
        , instanceId
      );

    t.exec_prepared("insert_raw_aws_instance_detail"
        , toolRunId
        , instanceId
        , type
        , imageId
        , architecture
        , platformDetails
        , launchTime
        , availabilityZone
        , stateCode
        , stateName
      );

    for (auto interface : interfaces) {
      interface.save(t, toolRunId, instanceId);
    }
  }

  std::string
  Instance::toDebugString() const
  {
    std::ostringstream oss;

    oss << '['
        << "instanceId: " << instanceId << ", "
        << "type: " << type << ", "
        << "imageId: " << imageId << ", "
        << "architecture: " << architecture << ", "
        << "platformDetails: " << platformDetails << ", "
        << "launchTime: " << launchTime << ", "
        << "availabilityZone: " << availabilityZone << ", "
        << "stateCode: " << stateCode << ", "
        << "stateName: " << stateName << ", "
        << "interfaces: " << interfaces
        << ']'
        ;

    return oss.str();
  }

  std::partial_ordering
  Instance::operator<=>(const Instance& rhs) const
  {
    return std::tie( instanceId
                   , type
                   , imageId
                   , architecture
                   , platformDetails
                   , launchTime
                   , availabilityZone
                   , stateCode
                   , stateName
                   , interfaces
                   )
       <=> std::tie( rhs.instanceId
                   , rhs.type
                   , rhs.imageId
                   , rhs.architecture
                   , rhs.platformDetails
                   , rhs.launchTime
                   , rhs.availabilityZone
                   , rhs.stateCode
                   , rhs.stateName
                   , rhs.interfaces
                   )
      ;
  }

  bool
  Instance::operator==(const Instance& rhs) const
  {
    return 0 == operator<=>(rhs);
  }
}
