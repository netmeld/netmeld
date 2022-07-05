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
  */
  for (const auto& instance : _reservation.at("Instances")) {
    std::string deviceId {instance.at("InstanceId")};

    if ("running" != instance.at("State").value("Name", "")) {
      std::ostringstream oss;
      oss << "Instance (" << deviceId
          << ") not 'running'"
          ;
      d.observations.addNotable(oss.str());
      continue;
    }

    nmdo::DeviceInformation device;
    device.setVendor("AWS");
    device.setDeviceType("Cloud Instance");
    device.setDeviceId(deviceId);
    device.setDescription(instance.value("KeyName", ""));

    device.setModel(instance.value("InstanceType", ""));
    device.setHardwareRevision(instance.value("LaunchTime", ""));
    device.setSerialNumber(instance.value("ImageId", ""));

    d.devices.emplace_back(device);

    processInterfaces(instance, deviceId);
  }
}

void
Parser::processInterfaces(const json& _instance, const std::string& _deviceId)
{
  /* TODO
    "InterfaceType"
    "SubnetId"
    "VpcId"
  */
  std::vector<nmdo::Interface> ifaces;
  for (const auto& interface : _instance.at("NetworkInterfaces")) {
    if ("attached" != interface.at("Attachment").value("Status", "")) {
      LOG_WARN << "Interface not 'attached'" << std::endl;
      continue;
    }

    nmdo::Interface iface;
    iface.setName(interface.at("NetworkInterfaceId"));
    iface.setDescription(interface.value("Description", ""));
    if ("in-use" == interface.value("Status", "")) {
      iface.setUp();
    }

    nmdo::MacAddress macAddr;
    macAddr.setMac(interface.at("MacAddress").get<std::string>());
    iface.setMacAddress(macAddr);
    processIps(interface, iface);

    ifaces.emplace_back(iface);

    // TODO
    //processSecurityGroups(interface);
  }
  d.interfaces.emplace(_deviceId, ifaces);
}

void
Parser::processSecurityGroups(const json& _interface)
{
  /* TODO
    "GroupName"
  */
  LOG_INFO << "\t\t";
  for (const auto& sg : _interface.at("Groups")) {
    LOG_INFO << sg.at("GroupId") << "; ";
  }
  LOG_INFO << "EOL" << std::endl;
}

void
Parser::processIps(const json& _interface, nmdo::Interface& _iface)
{
  // Handle IPv6 addresses
  if (_interface.at("Ipv6Addresses").size() > 0) {
    for (const auto& ip : _interface.at("Ipv6Addresses")) {
      nmdo::IpAddress ipAddr {
          ip.at("Ipv6Addresses").get<std::string>(), REASON
        };
      _iface.addIpAddress(ipAddr);
    }
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
