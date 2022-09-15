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

#include <netmeld/datastore/objects/aws/SecurityGroupRule.hpp>


namespace netmeld::datastore::objects::aws {

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
  SecurityGroupRule::addCidrBlock(const std::string& _cidr)
  {
    if (_cidr.empty()) { return; }
    CidrBlock cb {_cidr};
    cidrBlocks.insert(cb);
  }
  void
  SecurityGroupRule::addNonCidr(const std::string& _target)
  {
    if (_target.empty()) { return; }
    nonCidrs.insert(_target);
  }

  bool
  SecurityGroupRule::isValid() const
  {
    return !(protocol.empty())
        && !(fromPort == INT32_MIN || toPort == INT32_MIN)
        && (cidrBlocks.size() > 0 || nonCidrs.size() > 0)
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

    for (auto ip : cidrBlocks) {
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

    for (const auto& target : nonCidrs) {
      t.exec_prepared("insert_raw_aws_security_group_rule_non_ip"
          , toolRunId
          , deviceId
          , egress
          , protocol
          , fromPort
          , toPort
          , target
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
        << ", cidrBlocks: " << cidrBlocks
        << ", nonCidrs: " << nonCidrs
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
    if (auto cmp = cidrBlocks <=> rhs.cidrBlocks; 0 != cmp) {
      return cmp;
    }
    if (auto cmp = nonCidrs <=> rhs.nonCidrs; 0 != cmp) {
      return cmp;
    }

    return egress <=> rhs.egress;
  }

  bool
  SecurityGroupRule::operator==(const SecurityGroupRule& rhs) const
  {
    return 0 == operator<=>(rhs);
  }
}
