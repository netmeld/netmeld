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
    for (const auto& vpc : _data.at("Vpcs")) {
      processVpcs(vpc);
    }
  } catch (json::out_of_range& ex) {
    LOG_ERROR << "Parse error " << ex.what() << std::endl;
  }
}


// TODO https://awscli.amazonaws.com/v2/documentation/api/latest/reference/ec2/describe-vpcs.html

void
Parser::processVpcs(const json& _vpc)
{
  /* TODO
    "CidrBlockAssociationSet"
  */
  if ("available" != _vpc.at("State")) {
    std::ostringstream oss;
    oss << "VPC (" << _vpc
        << ") not 'available'"
        ;
    d.observations.addNotable(oss.str());
    return;
  }

  nmdo::IpNetwork ipNet {
        _vpc.at("CidrBlock").get<std::string>(), REASON
    };
  std::string description {REASON};
  for (const auto& tag : _vpc.at("Tags")) {
    if ("Name" == tag.at("Key")) {
      description = tag.at("Value");
    }
  }
  ipNet.setReason(description);

  d.ipNets.emplace_back(ipNet);
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
