// =============================================================================
// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
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
  for (const auto& json : _data.at("VpcPeeringConnections")) {
    processVpcPeeringConnection(json);
  }
}

void
Parser::processVpcPeeringConnection(const json& _json)
{
  nmdoa::VpcPeeringConnection pcx;
  pcx.setId(_json.value("VpcPeeringConnectionId", ""));

  if (_json.contains("Status")) {
    const auto& status {_json.at("Status")};
    pcx.setStatus(status.value("Code", ""), status.value("Message", ""));
  }

  processAccepter(_json, pcx);
  processRequester(_json, pcx);

  if (pcx != nmdoa::VpcPeeringConnection()) {
    d.pcxs.emplace_back(pcx);
  }
}

void
Parser::processAccepter(const json& _json,
                        nmdoa::VpcPeeringConnection& _pcx)
{
  if (!_json.contains("AccepterVpcInfo")) {
    return;
  }

  const auto& jVpc {_json.at("AccepterVpcInfo")};
  nmdoa::Vpc vpc;
  vpc.setId(jVpc.value("VpcId", ""));
  vpc.setOwnerId(jVpc.value("OwnerId", ""));

  nmdoa::CidrBlock cb;
  cb.setCidrBlock(jVpc.value("CidrBlock", ""));
  vpc.addCidrBlock(cb);
  processCidrBlockSets(jVpc, vpc);

  _pcx.setAccepter(vpc);
}

void
Parser::processRequester(const json& _json,
                         nmdoa::VpcPeeringConnection& _pcx)
{
  if (!_json.contains("RequesterVpcInfo")) {
    return;
  }

  const auto& jVpc {_json.at("RequesterVpcInfo")};
  nmdoa::Vpc vpc;
  vpc.setId(jVpc.value("VpcId", ""));
  vpc.setOwnerId(jVpc.value("OwnerId", ""));

  nmdoa::CidrBlock cb;
  cb.setCidrBlock(jVpc.value("CidrBlock", ""));
  vpc.addCidrBlock(cb);
  processCidrBlockSets(jVpc, vpc);

  _pcx.setRequester(vpc);
}

void
Parser::processCidrBlockSets(const json& _cbs, nmdoa::Vpc& _vpc)
{
  const std::vector<std::string> prefixes {
        ""
      , "Ipv6"
    };
  for (const auto& prefix : prefixes) {
    std::string tgtCidrBlockSet {prefix + "CidrBlockSet"};
    if (!_cbs.contains(tgtCidrBlockSet)) {
      continue;
    }

    for (const auto& cbs : _cbs.at(tgtCidrBlockSet)) {
      nmdoa::CidrBlock cb;
      cb.setCidrBlock(cbs.value(prefix + "CidrBlock", ""));
      _vpc.addCidrBlock(cb);
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

  if (d != Data()) {
    r.emplace_back(d);
  }

  return r;
}
