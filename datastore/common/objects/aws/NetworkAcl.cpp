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


  // -------------------------------------------------------------------------


  NetworkAclRule::NetworkAclRule()
  {}

  void
  NetworkAclRule::setNumber(const std::int32_t _num)
  {
    number = _num;
  }
  void
  NetworkAclRule::setAction(const std::string& _action)
  {
    action = _action;
  }
  void
  NetworkAclRule::setProtocol(const std::string& _proto)
  {
    protocol = _proto;
  }
  void
  NetworkAclRule::setFromPort(const std::int32_t _num)
  {
    portRange = true;
    fromOrType = _num;
  }
  void
  NetworkAclRule::setToPort(const std::int32_t _num)
  {
    portRange = true;
    toOrCode = _num;
  }
  void
  NetworkAclRule::setIcmpType(const std::int32_t _num)
  {
    typeCode = true;
    fromOrType = _num;
  }
  void
  NetworkAclRule::setIcmpCode(const std::int32_t _num)
  {
    typeCode = true;
    toOrCode = _num;
  }
  void
  NetworkAclRule::setEgress()
  {
    egress = true;
  }
  void
  NetworkAclRule::addIpRange(const std::string& _range)
  {
    nmdo::IpAddress ipAddr {_range};
    ipAddrs.insert(ipAddr);
  }

  bool
  NetworkAclRule::isValid() const
  {
    // TODO
    return !(number == INT32_MIN || action.empty() || protocol.empty())
        && (ipAddrs.size() > 0)
        ;
  }

  void
  NetworkAclRule::save(pqxx::transaction_base& t,
                    const nmco::Uuid& toolRunId, const std::string& deviceId)
  {
    if (!isValid()) {
      LOG_DEBUG << "AWS NetworkAclRule object is not saving: "
                << toDebugString()
                << std::endl;
      return;
    }

    for (auto ip : ipAddrs) {
      ip.save(t, toolRunId, deviceId);
      t.exec_prepared("insert_raw_aws_network_acl_rule"
          , toolRunId
          , deviceId
          , egress
          , number
          , action
          , protocol
          , ip.toString()
        );
    }

    if (portRange && typeCode) {
      LOG_WARN << "AWS NetworkAclRule saving as both a port and ICMP type"
               << std::endl;
    }

    if (portRange) {
      t.exec_prepared("insert_raw_aws_network_acl_rule_port"
          , toolRunId
          , deviceId
          , egress
          , number
          , fromOrType
          , toOrCode
        );
    }

    if (typeCode) {
      t.exec_prepared("insert_raw_aws_network_acl_rule_type_code"
          , toolRunId
          , deviceId
          , egress
          , number
          , fromOrType
          , toOrCode
        );
    }
  }

  std::string
  NetworkAclRule::toDebugString() const
  {
    std::ostringstream oss;

    oss << '['
        << "number: " << number
        << ", action: " << action
        << ", protocol: " << protocol
        << ", portRange: " << portRange
        << ", typeCode: " << typeCode
        << ", fromOrType: " << fromOrType
        << ", toOrCode: " << toOrCode
        << ", egress: " << egress
        << ", ipAddrs: " << ipAddrs
        << ']'
        ;

    return oss.str();
  }

  std::partial_ordering
  NetworkAclRule::operator<=>(const NetworkAclRule& rhs) const
  {
    if (auto cmp = number <=> rhs.number; 0 != cmp) {
      return cmp;
    }
    if (auto cmp = action <=> rhs.action; 0 != cmp) {
      return cmp;
    }
    if (auto cmp = protocol <=> rhs.protocol; 0 != cmp) {
      return cmp;
    }
    if (auto cmp = portRange <=> rhs.portRange; 0 != cmp) {
      return cmp;
    }
    if (auto cmp = typeCode <=> rhs.typeCode; 0 != cmp) {
      return cmp;
    }
    if (auto cmp = fromOrType <=> rhs.fromOrType; 0 != cmp) {
      return cmp;
    }
    if (auto cmp = toOrCode <=> rhs.toOrCode; 0 != cmp) {
      return cmp;
    }
    if (auto cmp = egress <=> rhs.egress; 0 != cmp) {
      return cmp;
    }

    return ipAddrs <=> rhs.ipAddrs;
  }

  bool
  NetworkAclRule::operator==(const NetworkAclRule& rhs) const
  {
    return 0 == operator<=>(rhs);
  }


  // -------------------------------------------------------------------------


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
    subnetIds.insert(_id);
  }
  void
  NetworkAcl::addRule(const NetworkAclRule& _rule)
  {
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
    if (auto cmp = naclId <=> rhs.naclId; 0 != cmp) {
      return cmp;
    }
    if (auto cmp = vpcId <=> rhs.vpcId; 0 != cmp) {
      return cmp;
    }
    if (auto cmp = subnetIds <=> rhs.subnetIds; 0 != cmp) {
      return cmp;
    }

    return rules <=> rhs.rules;
  }

  bool
  NetworkAcl::operator==(const NetworkAcl& rhs) const
  {
    return 0 == operator<=>(rhs);
  }
}
