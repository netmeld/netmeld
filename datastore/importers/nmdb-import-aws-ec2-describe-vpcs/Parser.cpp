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

#include <netmeld/datastore/objects/aws/CidrBlock.hpp>

// =============================================================================
// Parser logic
// =============================================================================
Parser::Parser()
{}

void
Parser::fromJson(const json& _data)
{
  for (const auto& vpc : _data.at("Vpcs")) {
    processVpcs(vpc);
  }
}

void
Parser::processVpcs(const json& _vpc)
{
  nmdoa::Vpc avpc;
  avpc.setId(_vpc.value("VpcId", ""));
  avpc.setOwnerId(_vpc.value("OwnerId", ""));
  avpc.setState(_vpc.value("State", ""));

  processCidrBlockAssociationSet(_vpc, avpc);

  d.vpcs.emplace_back(avpc);
}

void
Parser::processCidrBlockAssociationSet(const json& _cbas, nmdoa::Vpc& _avpc)
{
  const std::vector<std::string> prefixes {
        ""
      , "Ipv6"
    };
  for (const auto& prefix : prefixes) {
    std::string tgtCidrBlockSet {prefix + "CidrBlockAssociationSet"};
    if (!_cbas.contains(tgtCidrBlockSet)) {
      continue;
    }

    for (const auto& cbas : _cbas.at(tgtCidrBlockSet)) {
      nmdoa::CidrBlock avcb;
      avcb.setCidrBlock(cbas.value(prefix + "CidrBlock", ""));
      avcb.setState(cbas.at(prefix + "CidrBlockState").value("State", ""));

      _avpc.addCidrBlock(avcb);
    }
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
