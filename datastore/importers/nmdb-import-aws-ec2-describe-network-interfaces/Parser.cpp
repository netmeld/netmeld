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

#include <netmeld/datastore/objects/IpAddress.hpp>
#include <netmeld/datastore/objects/aws/Attachment.hpp>

// =============================================================================
// Parser logic
// =============================================================================
Parser::Parser()
{}

// NOTE: This logic and the describe-instances parser should be the same,
//       for everything network interface related
void
Parser::fromJson(const json& _data)
{
  for (const auto& interface : _data.at("NetworkInterfaces")) {
    processInterface(interface);
  }
}

void
Parser::processInterface(const json& _json)
{
  nmdoa::NetworkInterface ani;
  const std::string iid {_json.value("NetworkInterfaceId", "")};
  ani.setId(iid);
  ani.setDescription(_json.value("Description", ""));
  ani.setStatus(_json.value("Status", ""));
  ani.setType(_json.value("InterfaceType", ""));
  ani.setMacAddress(_json.value("MacAddress", ""));
  ani.setSubnetId(_json.value("SubnetId", ""));
  ani.setVpcId(_json.value("VpcId", ""));

  if (_json.value("SourceDestCheck", true)) {
    ani.enableSourceDestinationCheck();
  } else {
    ani.disableSourceDestinationCheck();
  }

  processInterfaceAttachment(_json, ani);
  processSecurityGroups(_json, ani);
  processIps(_json, ani);

  if (ani != nmdoa::NetworkInterface()) {
    d.interfaces.emplace_back(ani);
  }
}

void
Parser::processInterfaceAttachment(const json& _json,
                                   nmdoa::NetworkInterface& _iface)
{
  if (!_json.contains("Attachment")) {
    return;
  }

  const auto& attachment {_json.at("Attachment")};
  nmdoa::Attachment ania;
  ania.setId(attachment.value("AttachmentId", ""));
  ania.setStatus(attachment.value("Status", ""));
  if (attachment.value("DeleteOnTermination", false)) {
    ania.enableDeleteOnTermination();
  } else {
    ania.disableDeleteOnTermination();
  }

  _iface.setAttachment(ania);
}

void
Parser::processSecurityGroups(const json& _json,
                              nmdoa::NetworkInterface& _iface)
{
  if (!_json.contains("Groups")) {
    return;
  }
  for (const auto& sg : _json.at("Groups")) {
    _iface.addSecurityGroup(sg.value("GroupId", ""));
  }
}

void
Parser::processIps(const json& _json, nmdoa::NetworkInterface& _iface)
{
  // Handle IPv6 addresses
  if (_json.contains("Ipv6Addresses")) {
    for (const auto& ip : _json.at("Ipv6Addresses")) {
      nmdoa::CidrBlock cb {ip.at("Ipv6Address").get<std::string>()};
      _iface.addCidrBlock(cb);
    }
  }

  // Handle IPv4 addresses
  if (_json.contains("PrivateIpAddresses")) {
    for (const auto& ip : _json.at("PrivateIpAddresses")) {
      nmdoa::CidrBlock privCb {ip.at("PrivateIpAddress").get<std::string>()};
      privCb.addAlias(ip.value("PrivateDnsName", ""));
      _iface.addCidrBlock(privCb);

      // Handle public IPs
      if (ip.contains("Association")) {
        const auto& association = ip["Association"];
        std::string dnsName {association.value("PublicDnsName", "")};
        std::set keys {"CarrierIp", "CustomerOwnedIp", "PublicIp"};
        for (const auto& key : keys) {
          if (association.contains(key)) {
            nmdoa::CidrBlock pubCb {association.value(key, "")};
            pubCb.addAlias(dnsName);
            _iface.addCidrBlock(pubCb);
          }
        }
      }
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
    r.push_back(d);
  }

  return r;
}
