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

#include <netmeld/datastore/objects/aws/NetworkInterface.hpp>


namespace netmeld::datastore::objects::aws {

  NetworkInterface::NetworkInterface()
  {}

  void
  NetworkInterface::setId(const std::string& _id)
  {
    interfaceId = _id;
  }
  void
  NetworkInterface::setDescription(const std::string& _desc)
  {
    description = _desc;
  }
  void
  NetworkInterface::setType(const std::string& _type)
  {
    type = _type;
  }
  void
  NetworkInterface::setStatus(const std::string& _status)
  {
    status = _status;
    if ("in-use" == _status) {
      isUp = true;
    } else {
      isUp = false;
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
  NetworkInterface::setAttachment(const Attachment& _ania)
  {
    attachment = _ania;
  }
  void
  NetworkInterface::setMacAddress(const std::string& _mac)
  {
    macAddr = _mac;
  }
  void
  NetworkInterface::addCidrBlock(const CidrBlock& _cidr)
  {
    cidrBlocks.insert(_cidr);
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

  std::set<CidrBlock>
  NetworkInterface::getCidrBlocks() const
  {
    return cidrBlocks;
  }

  bool
  NetworkInterface::isValid() const
  {
    return !(interfaceId.empty())
         ;
  }

  void
  NetworkInterface::save(pqxx::transaction_base& t,
      const nmco::Uuid& toolRunId, const std::string& deviceId
    )
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

    bool hasDetails {
        !(type.empty() || status.empty())
      };

    if (hasDetails) {
      t.exec_prepared("insert_raw_aws_network_interface_detail"
          , toolRunId
          , interfaceId
          , type
          , sourceDestinationCheck
          , status
          , description
        );
    }

    attachment.save(t, toolRunId, interfaceId);

    if (!macAddr.empty()) {
      t.exec_prepared("insert_raw_aws_network_interface_mac"
          , toolRunId
          , interfaceId
          , macAddr
        );
    }

    for (auto cb : cidrBlocks) {
      cb.save(t, toolRunId, interfaceId);
      t.exec_prepared("insert_raw_aws_network_interface_ip"
          , toolRunId
          , interfaceId
          , cb.getCidrBlock()
        );
    }

    if (!subnetId.empty()) {
      t.exec_prepared("insert_raw_aws_subnet"
          , toolRunId
          , subnetId
        );
    }

    if (!vpcId.empty()) {
      t.exec_prepared("insert_raw_aws_vpc"
          , toolRunId
          , vpcId
        );
    }

    if (!(subnetId.empty() || vpcId.empty())) {
      t.exec_prepared("insert_raw_aws_network_interface_vpc_subnet"
          , toolRunId
          , interfaceId
          , vpcId
          , subnetId
        );
    }

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
        << ", isUp: " << isUp
        << ", subnetId: " << subnetId
        << ", vpcId: " << vpcId
        << ", attachment: " << attachment.toDebugString()
        << ", securityGroups: " << securityGroups
        << ", macAddr: " << macAddr
        << ", cidrBlocks: " << cidrBlocks
        << ']'
        ;

    return oss.str();
  }

  std::partial_ordering
  NetworkInterface::operator<=>(const NetworkInterface& rhs) const
  {
    return std::tie( interfaceId
                   , type
                   , description
                   , sourceDestinationCheck
                   , status
                   , isUp
                   , subnetId
                   , vpcId
                   , attachment
                   , securityGroups
                   , macAddr
                   , cidrBlocks
                   )
       <=> std::tie( rhs.interfaceId
                   , rhs.type
                   , rhs.description
                   , rhs.sourceDestinationCheck
                   , rhs.status
                   , rhs.isUp
                   , rhs.subnetId
                   , rhs.vpcId
                   , rhs.attachment
                   , rhs.securityGroups
                   , rhs.macAddr
                   , rhs.cidrBlocks
                   )
      ;
  }

  bool
  NetworkInterface::operator==(const NetworkInterface& rhs) const
  {
    return 0 == operator<=>(rhs);
  }
}
