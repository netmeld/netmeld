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

// =============================================================================
// Parser logic
// =============================================================================
Parser::Parser()
{}

void
Parser::fromJson(const json& _data)
{
  for (const auto& subnet : _data.at("Subnets")) {
    processSubnets(subnet);
  }
}

void
Parser::processSubnets(const json& _subnet)
{
  const std::string id {_subnet.value("SubnetId", "")};

  if ("available" != _subnet.at("State")) {
    std::ostringstream oss;
    oss << "Subnet (" << id
        << ") not 'available'"
        ;
    d.observations.addNotable(oss.str());
  }

  nmdoa::Subnet subnet;
  subnet.setId(id);
  subnet.setVpcId(_subnet.value("VpcId", ""));
  subnet.setAvailabilityZone(_subnet.value("AvailabilityZone", ""));
  subnet.setSubnetArn(_subnet.value("SubnetArn", ""));

  subnet.addCidrBlock(_subnet.value("CidrBlock", ""));
  for (const auto& cbas : _subnet.at("Ipv6CidrBlockAssociationSet")) {
    nmdoa::CidrBlock cb;
    cb.setCidrBlock(cbas.value("Ipv6CidrBlock", ""));
    cb.setState(cbas.at("Ipv6CidrBlockState").value("State", ""));
    subnet.addCidrBlock(cb);
  }

  if (subnet != nmdoa::Subnet()) {
    d.subnets.emplace_back(subnet);
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
    r.push_back(d);
  }

  return r;
}
