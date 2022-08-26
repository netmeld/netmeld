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
    for (const auto& networkAcl : _data.at("NetworkAcls")) {
      processNetworkAcl(networkAcl);
    }
  } catch (json::out_of_range& ex) {
    LOG_ERROR << "Parse error " << ex.what() << std::endl;
  }
}


void
Parser::processNetworkAcl(const json& _networkAcl)
{
  nmdoa::NetworkAcl anacl;
  anacl.setId(_networkAcl.value("NetworkAclId", ""));
  anacl.setVpcId(_networkAcl.value("VpcId", ""));
  for (const auto& association : _networkAcl.at("Associations")) {
    anacl.addSubnetId(association.value("SubnetId", ""));
  }

  processEntries(_networkAcl.at("Entries"), anacl);

  d.networkAcls.emplace_back(anacl);
}

void
Parser::processEntries(const json& _entries, nmdoa::NetworkAcl& _anacl)
{
  for (const auto& entry : _entries) {
    nmdoa::NetworkAclRule anaclr;
    anaclr.setNumber(entry.value("RuleNumber", -1));
    anaclr.setAction(entry.value("RuleAction", ""));
    anaclr.setProtocol(entry.value("Protocol", ""));
    
    std::set<std::string> keys {
          "CidrBlock"
        , "Ipv6CidrBlock"
      };
    for (const auto& key : keys) {
      if (!entry.contains(key)) {
        continue;
      }
      anaclr.addIpRange(entry.value(key, ""));
    }
    if (entry.value("Egress", false)) {
      anaclr.setEgress();
    }

    if (entry.contains("PortRange")) {
      const auto& range = entry.at("PortRange");
      anaclr.setFromPort(range.value("From", -1));
      anaclr.setToPort(range.value("To", -1));
    }

    if (entry.contains("IcmpTypeCode")) {
      const auto& tc = entry.at("IcmpTypeCode");
      anaclr.setFromPort(tc.value("Type", -1));
      anaclr.setToPort(tc.value("Code", -1));
    }

/*
IcmpTypeCode -> (structure)
  Code -> (integer)
  Type -> (integer)
PortRange -> (structure)
  From -> (integer)
  To -> (integer)
*/
    _anacl.addRule(anaclr);
  }
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
