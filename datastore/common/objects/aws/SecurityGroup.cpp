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

#include <netmeld/datastore/objects/aws/SecurityGroup.hpp>


namespace netmeld::datastore::objects::aws {


  // -------------------------------------------------------------------------


  SecurityGroupRule::SecurityGroupRule()
  {}

  void
  SecurityGroupRule::setProtocol(const std::string& _proto)
  {
    protocol = _proto;
  }
  void
  SecurityGroupRule::setFromPort(std::int32_t _port)
  {
    fromPort = _port;
  }
  void
  SecurityGroupRule::setToPort(std::int32_t _port)
  {
    toPort = _port;
  }
  void
  SecurityGroupRule::setEgress()
  {
    egress = true;
  }
  void
  SecurityGroupRule::addIpRange(const nmdo::IpAddress& _ipAddr)
  {
    ipAddrs.insert(_ipAddr);
  }

  bool
  SecurityGroupRule::isValid() const
  {
    // TODO
    return !(protocol.empty())
        && !(fromPort == INT32_MIN || toPort == INT32_MIN)
        && (ipAddrs.size() > 0)
        ;
  }

  void
  SecurityGroupRule::save(pqxx::transaction_base& t,
                    const nmco::Uuid& toolRunId, const std::string& deviceId)
  {
    if (!isValid()) {
      LOG_DEBUG << "AWS SecurityGroupRule object is not saving: " << toDebugString()
                << std::endl;
      return;
    }

    for (auto ip : ipAddrs) {
      ip.save(t, toolRunId, deviceId);
      t.exec_prepared("insert_raw_aws_security_group_rule"
          , toolRunId
          , deviceId
          , egress
          , protocol
          , fromPort
          , toPort
          , ip.toString()
        );
    }

  }

  std::string
  SecurityGroupRule::toDebugString() const
  {
    std::ostringstream oss;

    oss << '['
        << "protocol: " << protocol
        << ", fromPort: " << fromPort
        << ", toPort: " << toPort
        << ", egress: " << egress
        << ", ipAddrs: " << ipAddrs
        << ']'
        ;

    return oss.str();
  }

  std::partial_ordering
  SecurityGroupRule::operator<=>(const SecurityGroupRule& rhs) const
  {
    if (auto cmp = protocol <=> rhs.protocol; 0 != cmp) {
      return cmp;
    }
    if (auto cmp = fromPort <=> rhs.fromPort; 0 != cmp) {
      return cmp;
    }
    if (auto cmp = toPort <=> rhs.toPort; 0 != cmp) {
      return cmp;
    }
    if (auto cmp = egress <=> rhs.egress; 0 != cmp) {
      return cmp;
    }

    return ipAddrs <=> rhs.ipAddrs;
  }

  bool
  SecurityGroupRule::operator==(const SecurityGroupRule& rhs) const
  {
    return 0 == operator<=>(rhs);
  }


  // -------------------------------------------------------------------------


  SecurityGroup::SecurityGroup()
  {}

  void
  SecurityGroup::setId(const std::string& _id)
  {
    sgId = _id;
  }
  void
  SecurityGroup::setName(const std::string& _name)
  {
    name = _name;
  }
  void
  SecurityGroup::setDescription(const std::string& _desc)
  {
    description = _desc;
  }
  void
  SecurityGroup::setVpcId(const std::string& _id)
  {
    vpcId = _id;
  }
  void
  SecurityGroup::addRule(const SecurityGroupRule& _rule)
  {
    rules.insert(_rule);
  }

  bool
  SecurityGroup::isValid() const
  {
    // TODO
    return !(   sgId.empty()
             || name.empty()
             || description.empty()
             || vpcId.empty()
            )
        && (rules.size() > 0)
        ;
  }

  void
  SecurityGroup::save(pqxx::transaction_base& t,
                    const nmco::Uuid& toolRunId, const std::string& deviceId)
  {
    if (!isValid()) {
      LOG_DEBUG << "AWS SecurityGroup object is not saving: " << toDebugString()
                << std::endl;
      return;
    }

    t.exec_prepared("insert_raw_aws_security_group"
        , toolRunId
        , sgId
      );

    t.exec_prepared("insert_raw_aws_security_group_detail"
        , toolRunId
        , sgId
        , name
        , description
      );

    for (auto rule : rules) {
      rule.save(t, toolRunId, sgId);
    }

    t.exec_prepared("insert_raw_aws_vpc"
        , toolRunId
        , vpcId
      );

    t.exec_prepared("insert_raw_aws_vpc_security_group"
        , toolRunId
        , vpcId
        , sgId
      );
  }

  std::string
  SecurityGroup::toDebugString() const
  {
    std::ostringstream oss;

    oss << '['
        << "sgId: " << sgId
        << ", name: " << name
        << ", description: " << description
        << ", vpcId: " << vpcId
        << ", rules: " << rules
        << ']'
        ;

    return oss.str();
  }

  std::partial_ordering
  SecurityGroup::operator<=>(const SecurityGroup& rhs) const
  {
    if (auto cmp = sgId <=> rhs.sgId; 0 != cmp) {
      return cmp;
    }
    if (auto cmp = name <=> rhs.name; 0 != cmp) {
      return cmp;
    }
    if (auto cmp = description <=> rhs.description; 0 != cmp) {
      return cmp;
    }
    if (auto cmp = vpcId <=> rhs.vpcId; 0 != cmp) {
      return cmp;
    }

    return rules <=> rhs.rules;
  }

  bool
  SecurityGroup::operator==(const SecurityGroup& rhs) const
  {
    return 0 == operator<=>(rhs);
  }
}
