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

#include <netmeld/datastore/objects/AbstractDatastoreObject.hpp>
#include <netmeld/datastore/objects/DeviceInformation.hpp>
#include <netmeld/datastore/objects/Interface.hpp>
#include <netmeld/datastore/objects/IpAddress.hpp>
#include <netmeld/datastore/objects/MacAddress.hpp>
#include <netmeld/datastore/objects/aws/Instance.hpp>


namespace netmeld::datastore::objects::aws {


  // -------------------------------------------------------------------------


  NetworkInterfaceAttachment::NetworkInterfaceAttachment()
  {}

  void
  NetworkInterfaceAttachment::setId(const std::string& _id)
  {
    attachmentId = _id;
  }
  void
  NetworkInterfaceAttachment::setStatus(const std::string& _status)
  {
    status = _status;
  }
  void
  NetworkInterfaceAttachment::enableDeleteOnTermination()
  {
    deleteOnTermination = true;
  }
  void
  NetworkInterfaceAttachment::disableDeleteOnTermination()
  {
    deleteOnTermination = false;
  }

  bool
  NetworkInterfaceAttachment::isValid() const
  {
    // TODO
    return !(attachmentId.empty() || status.empty());
  }

  void
  NetworkInterfaceAttachment::save(pqxx::transaction_base& t,
                                   const nmco::Uuid& toolRunId,
                                   const std::string& deviceId)
  {
    if (!isValid()) {
      LOG_DEBUG << "AWS NetworkInterfaceAttachment object is not saving: "
                << toDebugString()
                << std::endl;
      return;
    }

    t.exec_prepared("insert_raw_aws_network_interface_attachment"
        , toolRunId
        , deviceId
        , attachmentId
        , status
        , deleteOnTermination
      );
  }

  std::string
  NetworkInterfaceAttachment::toDebugString() const
  {
    std::ostringstream oss;

    oss << '['
        << "attachmentId: " << attachmentId
        << ", status: " << status
        << ", deleteOnTermination: " << deleteOnTermination
        << ']'
        ;

    return oss.str();
  }

  std::partial_ordering
  NetworkInterfaceAttachment::operator<=>(const NetworkInterfaceAttachment& rhs) const
  {
    if (auto cmp = attachmentId <=> rhs.attachmentId; 0 != cmp) {
      return cmp;
    }
    if (auto cmp = status <=> rhs.status; 0 != cmp) {
      return cmp;
    }
    //if (auto cmp = deleteOnTermination <=> rhs.deleteOnTermination; 0 != cmp) {
    //  return cmp;
    //}

    return deleteOnTermination <=> rhs.deleteOnTermination;
  }

  bool
  NetworkInterfaceAttachment::operator==(const NetworkInterfaceAttachment& rhs) const
  {
    return 0 == operator<=>(rhs);
  }


  // -------------------------------------------------------------------------


  NetworkInterface::NetworkInterface()
  {}

  void
  NetworkInterface::setId(const std::string& _id)
  {
    nmdo::Interface::setName(_id);
    interfaceId = _id; //this->name;
  }
  void
  NetworkInterface::setDescription(const std::string& _desc)
  {
    nmdo::Interface::setDescription(_desc);
    description = _desc; //this->description;
  }
  void
  NetworkInterface::setType(const std::string& _type)
  {
    nmdo::Interface::setMediaType(_type);
    type = _type; //this->mediaType;
  }
  void
  NetworkInterface::setStatus(const std::string& _status)
  {
    status = _status;
    if ("in-use" == _status) {
      nmdo::Interface::setUp();
    } else {
      nmdo::Interface::setDown();
    }
  }
  void
  NetworkInterface::enableSourceDestinationCheck()
  {
    sourceDestinationCheck = true;
  }
  void
  NetworkInterface::disableSourceDestinationCheck()
  {
    sourceDestinationCheck = false;
  }
  void
  NetworkInterface::setAttachment(const NetworkInterfaceAttachment& _ania)
  {
    attachment = _ania;
  }
  void
  NetworkInterface::setMacAddress(const std::string& _mac)
  {
    nmdo::MacAddress macAddr {_mac};
    nmdo::Interface::setMacAddress(macAddr);
  }
  void
  NetworkInterface::addIpAddress(const nmdo::IpAddress& _ip)
  {
    nmdo::Interface::addIpAddress(_ip);
  }
  void
  NetworkInterface::setSubnetId(const std::string& _id)
  {
    subnetId = _id;
  }
  void
  NetworkInterface::setVpcId(const std::string& _id)
  {
    vpcId = _id;
  }
  void
  NetworkInterface::addSecurityGroup(const std::string& _sg)
  {
    securityGroups.insert(_sg);
  }

  bool
  NetworkInterface::isValid() const
  {
    // TODO
    return !(   interfaceId.empty()
             || type.empty()
             || status.empty()
             || subnetId.empty()
             || vpcId.empty()
            );
  }

  void
  NetworkInterface::save(pqxx::transaction_base& t,
                         const nmco::Uuid& toolRunId,
                         const std::string& deviceId)
  {
    if (!isValid()) {
      LOG_DEBUG << "AWS NetworkInterface object is not saving: "
                << toDebugString()
                << std::endl;
      return;
    }

    t.exec_prepared("insert_raw_aws_network_interface"
        , toolRunId
        , interfaceId
      );

    t.exec_prepared("insert_raw_aws_network_interface_detail"
        , toolRunId
        , interfaceId
        , type
        , sourceDestinationCheck
        , status
      );

    attachment.save(t, toolRunId, interfaceId);

    getMacAddress().save(t, toolRunId, deviceId);
    t.exec_prepared("insert_raw_aws_network_interface_mac"
        , toolRunId
        , interfaceId
        , getMacAddress().toString()
      );

    for (const auto& ip : getIpAddresses()) {
      for (const auto& alias : ip.getAliases()) {
        t.exec_prepared("insert_raw_aws_network_interface_ip"
            , toolRunId
            , interfaceId
            , ip.toString()
            , alias
          );
      }
    }

    t.exec_prepared("insert_raw_aws_subnet"
        , toolRunId
        , subnetId
      );

    t.exec_prepared("insert_raw_aws_vpc"
        , toolRunId
        , vpcId
      );

    t.exec_prepared("insert_raw_aws_network_interface_vpc_subnet"
        , toolRunId
        , interfaceId
        , vpcId
        , subnetId
      );

    for (const auto& sg : securityGroups) {
      t.exec_prepared("insert_raw_aws_security_group"
          , toolRunId
          , sg
        );

      t.exec_prepared("insert_raw_aws_network_interface_security_group"
          , toolRunId
          , interfaceId
          , sg
        );
    }

    if (!deviceId.empty()) {
      t.exec_prepared("insert_raw_aws_instance_network_interface"
          , toolRunId
          , deviceId
          , interfaceId
        );
    }
  }

  std::string
  NetworkInterface::toDebugString() const
  {
    std::ostringstream oss;

    oss << '['
        << "interfaceId: " << interfaceId
        << ", type: " << type
        << ", description: " << description
        << ", sourceDestinationCheck: " << sourceDestinationCheck
        << ", status: " << status
        << ", subnetId: " << subnetId
        << ", vpcId: " << vpcId
        << ", attachment: " << attachment.toDebugString()
        << ", securityGroups: " << securityGroups.size()
        << ", nmdo::Interface: " << nmdo::Interface::toDebugString()
        << ']'
        ;

    return oss.str();
  }

  std::partial_ordering
  NetworkInterface::operator<=>(const NetworkInterface& rhs) const
  {
    if (auto cmp = interfaceId <=> rhs.interfaceId; 0 != cmp) {
      return cmp;
    }
    if (auto cmp = type <=> rhs.type; 0 != cmp) {
      return cmp;
    }
    if (auto cmp = description <=> rhs.description; 0 != cmp) {
      return cmp;
    }
    if (auto cmp = sourceDestinationCheck <=> rhs.sourceDestinationCheck; 0 != cmp) {
      return cmp;
    }
    if (auto cmp = status <=> rhs.status; 0 != cmp) {
      return cmp;
    }
    if (auto cmp = subnetId <=> rhs.subnetId; 0 != cmp) {
      return cmp;
    }
    if (auto cmp = vpcId <=> rhs.vpcId; 0 != cmp) {
      return cmp;
    }
    if (auto cmp = attachment <=> rhs.attachment; 0 != cmp) {
      return cmp;
    }

    return securityGroups <=> rhs.securityGroups;
  }

  bool
  NetworkInterface::operator==(const NetworkInterface& rhs) const
  {
    return 0 == operator<=>(rhs);
  }


  // -------------------------------------------------------------------------


  Instance::Instance()
  {
    nmdo::DeviceInformation::setVendor("AWS");
    nmdo::DeviceInformation::setDeviceType("Instance");
  }

  void
  Instance::setId(const std::string& _id)
  {
    nmdo::DeviceInformation::setDeviceId(_id);
    instanceId = _id; //this->deviceId;
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
    interfaces.insert(_ani);
  }

  bool
  Instance::isValid() const
  {
    return !(instanceId.empty()
             || type.empty()
             || imageId.empty()
             || stateName.empty()
            );
  }

  void
  Instance::save(pqxx::transaction_base& t,
                 const nmco::Uuid& toolRunId, const std::string& deviceId)
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
        << "instanceId: " << instanceId
        << ", type: " << type
        << ", imageId: " << imageId
        << ", availabilityZone: " << availabilityZone
        << ", stateCode: " << stateCode
        << ", stateName: " << stateName
        << ", interfaces: " << interfaces
        << ']'
        ;

    return oss.str();
  }

  std::partial_ordering
  Instance::operator<=>(const Instance& rhs) const
  {
    if (auto cmp = instanceId <=> rhs.instanceId; 0 != cmp) {
      return cmp;
    }
    if (auto cmp = type <=> rhs.type; 0 != cmp) {
      return cmp;
    }
    if (auto cmp = imageId <=> rhs.imageId; 0 != cmp) {
      return cmp;
    }
    if (auto cmp = availabilityZone <=> rhs.availabilityZone; 0 != cmp) {
      return cmp;
    }
    if (auto cmp = stateCode <=> rhs.stateCode; 0 != cmp) {
      return cmp;
    }
    if (auto cmp = stateName <=> rhs.stateName; 0 != cmp) {
      return cmp;
    }

    return interfaces <=> rhs.interfaces;
  }

  bool
  Instance::operator==(const Instance& rhs) const
  {
    return 0 == operator<=>(rhs);
  }
}
