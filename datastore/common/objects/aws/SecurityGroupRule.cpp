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
    fromOrType = _port;
  }
  void
  SecurityGroupRule::setToPort(std::int32_t _port)
  {
    toOrCode = _port;
  }
  void
  SecurityGroupRule::setEgress()
  {
    egress = true;
  }
  void
  SecurityGroupRule::addCidrBlock(const std::string& _cidr)
  {
    if (_cidr.empty()) { return; } // Don't add empties
    CidrBlock cb {_cidr};
    cidrBlocks.insert(cb);
  }
  void
  SecurityGroupRule::addCidrBlock(const CidrBlock& _cidr)
  {
    CidrBlock t;
    if (t == _cidr) { return; } // Don't add empties
    cidrBlocks.insert(_cidr);
  }
  void
  SecurityGroupRule::addNonCidr(const std::string& _target)
  {
    if (_target.empty()) { return; } // Don't add empties
    nonCidrs.insert(_target);
  }
  void
  SecurityGroupRule::addDetails(const std::string& _data)
  {
    if (_data.empty()) { return; } // Don't add empties
    details.insert(_data);
  }

  bool
  SecurityGroupRule::isValid() const
  {
    return !(protocol.empty())
        && !(fromOrType == INT32_MIN || toOrCode == INT32_MIN)
        && (cidrBlocks.size() > 0 || nonCidrs.size() > 0)
        ;
  }

  void
  SecurityGroupRule::save(pqxx::transaction_base& t,
      const nmco::Uuid& toolRunId, const std::string& deviceId
    )
  {
    if (!isValid()) {
      LOG_DEBUG << "AWS SecurityGroupRule object is not saving: "
                << toDebugString()
                << std::endl;
      return;
    }

    for (auto ip : cidrBlocks) {
      ip.save(t, toolRunId, deviceId);
      if (protocol == "icmp" || protocol == "-1") {
        t.exec_prepared("insert_raw_aws_security_group_rules_type_code"
            , toolRunId
            , deviceId
            , egress
            , protocol
            , fromOrType
            , toOrCode
            , ip.toString()
          );
      }
      if (protocol != "icmp" || protocol == "-1") {
        t.exec_prepared("insert_raw_aws_security_group_rules_port"
            , toolRunId
            , deviceId
            , egress
            , protocol
            , fromOrType
            , toOrCode
            , ip.toString()
          );
      }
    }

    for (const auto& target : nonCidrs) {
      if (protocol == "icmp" || protocol == "-1") {
        t.exec_prepared("insert_raw_aws_security_group_rules_non_ip_type_code"
            , toolRunId
            , deviceId
            , egress
            , protocol
            , fromOrType
            , toOrCode
            , target
          );
      }
      if (protocol != "icmp" || protocol == "-1") {
        t.exec_prepared("insert_raw_aws_security_group_rules_non_ip_port"
            , toolRunId
            , deviceId
            , egress
            , protocol
            , fromOrType
            , toOrCode
            , target
          );
      }
    }

    for (const auto& detail : details) {
      t.exec_prepared("insert_raw_aws_security_group_rules_non_ip_detail"
          , toolRunId
          , deviceId
          , egress
          , protocol
          , fromOrType
          , toOrCode
          , detail
        );
    }
  }

  std::string
  SecurityGroupRule::toDebugString() const
  {
    std::ostringstream oss;

    oss << '['
        << "protocol: " << protocol
        << ", fromOrType: " << fromOrType
        << ", toOrCode: " << toOrCode
        << ", egress: " << egress
        << ", cidrBlocks: " << cidrBlocks
        << ", nonCidrs: " << nonCidrs
        << ", details: " << details
        << ']'
        ;

    return oss.str();
  }

  std::partial_ordering
  SecurityGroupRule::operator<=>(const SecurityGroupRule& rhs) const
  {
    return std::tie( protocol
                   , fromOrType
                   , toOrCode
                   , cidrBlocks
                   , nonCidrs
                   , details
                   , egress
                   )
       <=> std::tie( rhs.protocol
                   , rhs.fromOrType
                   , rhs.toOrCode
                   , rhs.cidrBlocks
                   , rhs.nonCidrs
                   , rhs.details
                   , rhs.egress
                   )
      ;
  }

  bool
  SecurityGroupRule::operator==(const SecurityGroupRule& rhs) const
  {
    return 0 == operator<=>(rhs);
  }
}
