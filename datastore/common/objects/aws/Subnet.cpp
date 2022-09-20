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

#include <netmeld/datastore/objects/aws/Subnet.hpp>


namespace netmeld::datastore::objects::aws {

  Subnet::Subnet()
  {}

  void
  Subnet::setId(const std::string& _id)
  {
    subnetId = _id;
  }
  void
  Subnet::setAvailabilityZone(const std::string& _az)
  {
    availabilityZone = _az;
  }
  void
  Subnet::setSubnetArn(const std::string& _sa)
  {
    subnetArn = _sa;
  }
  void
  Subnet::setVpcId(const std::string& _id)
  {
    vpcId = _id;
  }
  void
  Subnet::addCidrBlock(const std::string& _cidrBlock)
  {
    if (_cidrBlock.empty()) { return; }
    cidrBlocks.emplace(_cidrBlock);
  }
  void
  Subnet::addCidrBlock(const CidrBlock& _cidrBlock)
  {
    CidrBlock t;
    if (t == _cidrBlock) { return; }
    cidrBlocks.insert(_cidrBlock);
  }

  bool
  Subnet::isValid() const
  {
    return !(subnetId.empty())
        ;
  }

  void
  Subnet::save(pqxx::transaction_base& t,
                    const nmco::Uuid& toolRunId, const std::string&)
  {
    if (!isValid()) {
      LOG_DEBUG << "AWS Subnet object is not saving: " << toDebugString()
                << std::endl;
      return;
    }

    t.exec_prepared("insert_raw_aws_subnet"
        , toolRunId
        , subnetId
      );
    
    bool hasDetails {
        !(  availabilityZone.empty()
         || subnetArn.empty()
        )
      };

    if (hasDetails) {
      t.exec_prepared("insert_raw_aws_subnet_detail"
          , toolRunId
          , subnetId
          , availabilityZone
          , subnetArn
        );
    }

    for (auto cb : cidrBlocks) {
      cb.save(t, toolRunId, subnetId);

      t.exec_prepared("insert_raw_aws_subnet_cidr_block"
          , toolRunId
          , subnetId
          , cb.toString()
        );
    }

    if (!vpcId.empty()) {
      t.exec_prepared("insert_raw_aws_vpc"
          , toolRunId
          , vpcId
        );

      t.exec_prepared("insert_raw_aws_vpc_subnet"
          , toolRunId
          , vpcId
          , subnetId
        );
    }
  }

  std::string
  Subnet::toDebugString() const
  {
    std::ostringstream oss;

    oss << '['
        << "subnetId: " << subnetId
        << ", vpcId: " << vpcId
        << ", availabilityZone: " << availabilityZone
        << ", subnetArn: " << subnetArn
        << ", cidrBlocks: " << cidrBlocks
        << ']'
        ;

    return oss.str();
  }

  std::partial_ordering
  Subnet::operator<=>(const Subnet& rhs) const
  {
    if (auto cmp = subnetId <=> rhs.subnetId; 0 != cmp) {
      return cmp;
    }
    if (auto cmp = vpcId <=> rhs.vpcId; 0 != cmp) {
      return cmp;
    }
    if (auto cmp = availabilityZone <=> rhs.availabilityZone; 0 != cmp) {
      return cmp;
    }
    if (auto cmp = subnetArn <=> rhs.subnetArn; 0 != cmp) {
      return cmp;
    }

    return cidrBlocks <=> rhs.cidrBlocks;
  }

  bool
  Subnet::operator==(const Subnet& rhs) const
  {
    return 0 == operator<=>(rhs);
  }
}
