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

#include <netmeld/datastore/objects/aws/NetworkAcl.hpp>


namespace netmeld::datastore::objects::aws {

  NetworkAcl::NetworkAcl()
  {}

  void
  NetworkAcl::setId(const std::string& _id)
  {
    naclId = _id;
  }
  void
  NetworkAcl::setVpcId(const std::string& _id)
  {
    vpcId = _id;
  }
  void
  NetworkAcl::addSubnetId(const std::string& _id)
  {
    if (_id.empty()) { return; } // Don't add empties
    subnetIds.insert(_id);
  }
  void
  NetworkAcl::addRule(const NetworkAclRule& _rule)
  {
    NetworkAclRule t;
    if (t == _rule) { return; } // Don't add empties
    rules.insert(_rule);
  }

  bool
  NetworkAcl::isValid() const
  {
    return !(naclId.empty());
  }

  void
  NetworkAcl::save(pqxx::transaction_base& t,
                   const nmco::Uuid& toolRunId, const std::string&)
  {
    if (!isValid()) {
      LOG_DEBUG << "AWS NetworkAcl object is not saving: " << toDebugString()
                << std::endl;
      return;
    }

    t.exec_prepared("insert_raw_aws_network_acl"
        , toolRunId
        , naclId
      );

    for (auto rule : rules) {
      rule.save(t, toolRunId, naclId);
    }

    if (!vpcId.empty()) {
      t.exec_prepared("insert_raw_aws_vpc"
          , toolRunId
          , vpcId
        );

      t.exec_prepared("insert_raw_aws_vpc_network_acl"
          , toolRunId
          , vpcId
          , naclId
        );
    }

    for (const auto& subnetId : subnetIds) {
      t.exec_prepared("insert_raw_aws_subnet"
          , toolRunId
          , subnetId
        );

      t.exec_prepared("insert_raw_aws_network_acl_subnet"
          , toolRunId
          , naclId
          , subnetId
        );
    }
  }

  std::string
  NetworkAcl::toDebugString() const
  {
    std::ostringstream oss;

    oss << '['
        << "naclId: " << naclId
        << ", vpcId: " << vpcId
        << ", subnetIds: " << subnetIds
        << ", rules: " << rules
        << ']'
        ;

    return oss.str();
  }

  std::partial_ordering
  NetworkAcl::operator<=>(const NetworkAcl& rhs) const
  {
    return std::tie( naclId
                   , vpcId
                   , subnetIds
                   , rules
                   )
       <=> std::tie( rhs.naclId
                   , rhs.vpcId
                   , rhs.subnetIds
                   , rhs.rules
                   )
      ;
  }

  bool
  NetworkAcl::operator==(const NetworkAcl& rhs) const
  {
    return 0 == operator<=>(rhs);
  }
}
