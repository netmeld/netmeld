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

#include <netmeld/datastore/objects/IpAddress.hpp>

// =============================================================================
// Parser logic
// =============================================================================
Parser::Parser()
{}

void
Parser::fromJson(const json& _data)
{
  try {
    for (const auto& reservation : _data.at("Reservations")) {
      processInstances(reservation);
    }
  } catch (json::out_of_range& ex) {
    LOG_ERROR << "Parse error " << ex.what() << std::endl;
  }
}


// TODO https://docs.aws.amazon.com/cli/latest/reference/ec2/describe-instances.html

void
Parser::processInstances(const json& _reservation)
{
  /* TODO
    "Architecture"
    "PlatformDetails"
    "SecurityGroups"
    "VpcId"
    "LaunchTime" -- hardware revision?
  */
  for (const auto& instance : _reservation.at("Instances")) {
    std::string id {instance.at("InstanceId")};

    nmdoa::Instance ai;
    ai.setId(id);
    ai.setType(instance.value("InstanceType", ""));
    ai.setImageId(instance.value("ImageId", ""));
    ai.setAvailabilityZone(
        instance.at("Placement").value("AvailabilityZone", "")
      );
    ai.setStateCode(instance.at("State").at("Code").get<uint16_t>());
    ai.setStateName(instance.at("State").value("Name", ""));

    processInterfaces(instance, ai);

    d.instances.emplace_back(ai);
  }
}

void
Parser::processInterfaces(const json& _instance, nmdoa::Instance& _ai)
{
  for (const auto& interface : _instance.at("NetworkInterfaces")) {
    nmdoa::NetworkInterface ani;
    const std::string iid {interface.value("NetworkInterfaceId", "")};
    ani.setId(iid);
    ani.setDescription(interface.value("Description", ""));
    ani.setStatus(interface.value("Status", ""));
    ani.setType(interface.value("InterfaceType", ""));
    ani.setMacAddress(interface.value("MacAddress", ""));
    ani.setSubnetId(interface.value("SubnetId", ""));
    ani.setVpcId(interface.value("VpcId", ""));

    if (interface.value("SourceDestCheck", true)) {
      ani.enableSourceDestinationCheck();
    } else {
      ani.disableSourceDestinationCheck();
    }

    try {
      const auto& attachment {interface.at("Attachment")};
      processInterfaceAttachments(attachment, ani);
    } catch (const json::out_of_range& e) {
      // TODO -- what to do if no attachment
      LOG_WARN << "No attachment for interface: " << iid << '\n';
    }
    processSecurityGroups(interface, ani);
    processIps(interface, ani);

    _ai.addInterface(ani);
  }
}

void
Parser::processInterfaceAttachments(const json& _attachment,
                                    nmdoa::NetworkInterface& _iface)
{
  nmdoa::NetworkInterfaceAttachment ania;
  ania.setId(_attachment.value("AttachmentId", ""));
  ania.setStatus(_attachment.value("Status", ""));
  if (_attachment.at("DeleteOnTermination")) {
    ania.enableDeleteOnTermination();
  } else {
    ania.disableDeleteOnTermination();
  }

  _iface.setAttachment(ania);
}

void
Parser::processSecurityGroups(const json& _interface,
                              nmdoa::NetworkInterface& _iface)
{
  for (const auto& sg : _interface.at("Groups")) {
    _iface.addSecurityGroup(sg.value("GroupId", ""));
  }
}

void
Parser::processIps(const json& _interface, nmdoa::NetworkInterface& _iface)
{
  // Handle IPv6 addresses
  for (const auto& ip : _interface.at("Ipv6Addresses")) {
    nmdo::IpAddress ipAddr {
        ip.at("Ipv6Addresses").get<std::string>(), REASON
      };
    _iface.addIpAddress(ipAddr);
  }

  // Handle IPv4 addresses
  for (const auto& ip : _interface.at("PrivateIpAddresses")) {
    nmdo::IpAddress ipAddr {
          ip.at("PrivateIpAddress").get<std::string>(), REASON
      };
    ipAddr.addAlias(ip.value("PrivateDnsName", ""), REASON);
    _iface.addIpAddress(ipAddr);

    // Handle public IPs
    if (ip.contains("Association")) {
      const auto& association = ip["Association"];
      std::string dnsName {association.value("PublicDnsName", "")};
      std::set keys {"CarrierIp", "CustomerOwnedIp", "PublicIp"};
      for (const auto& key : keys) {
        if (association.contains(key)) {
          nmdo::IpAddress pubIpAddr {
                association.at(key).get<std::string>(), REASON
            };
          if (!dnsName.empty()) {
            pubIpAddr.addAlias(dnsName, REASON);
          }
          _iface.addIpAddress(pubIpAddr);
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
  r.emplace_back(d);
  return r;
}
