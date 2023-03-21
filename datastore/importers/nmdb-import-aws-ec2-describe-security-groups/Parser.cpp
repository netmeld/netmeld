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

#include "Parser.hpp"

// =============================================================================
// Parser logic
// =============================================================================
Parser::Parser()
{}

void
Parser::fromJson(const json& _data)
{
  try {
    for (const auto& securityGroup : _data.at("SecurityGroups")) {
      processSecurityGroup(securityGroup);
    }
  } catch (json::out_of_range& ex) {
    LOG_ERROR << "Parse error " << ex.what() << std::endl;
  }
}


void
Parser::processSecurityGroup(const json& _securityGroup)
{
  nmdoa::SecurityGroup asg;
  asg.setId(_securityGroup.value("GroupId", ""));
  asg.setName(_securityGroup.value("GroupName", ""));
  asg.setDescription(_securityGroup.value("Description", ""));
  asg.setVpcId(_securityGroup.value("VpcId", ""));

  processPermissions(_securityGroup, asg);

  d.securityGroups.emplace_back(asg);
}

void
Parser::processPermissions(const json& _securityGroup,
                           nmdoa::SecurityGroup& _asg)
{
  {
    const auto direction {"IpPermissions"};
    for (const auto& permission : _securityGroup.at(direction)) {
      auto asgr = getRule(permission);
      _asg.addRule(asgr);
    }
  }

  {
    const auto direction {"IpPermissionsEgress"};
    for (const auto& permission : _securityGroup.at(direction)) {
      auto asgr = getRule(permission);
      asgr.setEgress();
      _asg.addRule(asgr);
    }
  }
}


nmdoa::SecurityGroupRule
Parser::getRule(const json& _permission)
{
  nmdoa::SecurityGroupRule asgr;
  asgr.setProtocol(_permission.value("IpProtocol", ""));
  asgr.setFromPort(_permission.value("FromPort", ANY));
  asgr.setToPort(_permission.value("ToPort", ANY));

  {
    std::map<std::string, std::string> keys {
          {"IpRanges", "CidrIp"}
        , {"Ipv6Ranges", "CidrIpv6"}
      };
    for (const auto& [listKey, ipKey] : keys) {
      for (const auto& ip : _permission.at(listKey)) {
        nmdoa::CidrBlock cb {ip.value(ipKey, "")};
        cb.setDescription(ip.value("Description", ""));
        asgr.addCidrBlock(cb);
      }
    }
  }
  {
    std::map<std::string, std::string> keys {
          {"PrefixListIds", "PrefixListId"}
        , {"UserIdGroupPairs", "GroupId"}
      };
    for (const auto& [listKey, idKey] : keys) {
      for (const auto& dataBlock : _permission.at(listKey)) {
        asgr.addNonCidr(dataBlock.value(idKey, ""));

        std::ostringstream oss;
        oss << dataBlock;
        asgr.addDetails(oss.str());
      }
    }
  }

  return asgr;
}


// =============================================================================
// Parser helper methods
// =============================================================================
Result
Parser::getData()
{
  Result r;
  r.emplace_back(d);
  return r;
}
