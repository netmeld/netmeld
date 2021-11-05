// =============================================================================
// Copyright 2021 National Technology & Engineering Solutions of Sandia, LLC
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

#include <netmeld/core/utils/StringUtilities.hpp>
#include <netmeld/datastore/parsers/ParserHelper.hpp>
#include <netmeld/datastore/parsers/ParserIpAddress.hpp>
#include <netmeld/datastore/utils/ServiceFactory.hpp>

#include <regex>


namespace nmcu = netmeld::core::utils;
namespace nmdo = netmeld::datastore::objects;
namespace nmdp = netmeld::datastore::parsers;
namespace nmdu = netmeld::datastore::utils;


Data
Parser::getData()
{
  return data;
}


std::tuple<nmdo::IpAddress, std::string>
Parser::parseIpAddrVrfStr(const std::string& ipAddrVrfStr) const
{
  std::string ipAddrStr;
  std::string vrfId;
  
  const std::regex re("([^%]+)(%\\d+)?(.*)");
  std::smatch m;
  if (std::regex_match(ipAddrVrfStr, m, re)) {
    if (1 < m.size()) {
      ipAddrStr = m[1];
    }
    if (2 < m.size()) {
      vrfId = m[2];
      vrfId.erase(0, 1);  // remove leading '%';
    }
    if (3 < m.size()) {
      ipAddrStr += m[3];
    }
  }

  // Convert various "default" network routes to IP CIDRs.
  if ("default-inet6" == ipAddrStr) {
    ipAddrStr = "::/0";
  }
  else if ("default" == ipAddrStr) {
    ipAddrStr = "0.0.0.0/0";
  }
  // Convert various "any" address to IP addrs.
  else if ("any6" == ipAddrStr) {
    ipAddrStr = "::";
  }
  else if ("any" == ipAddrStr) {
    ipAddrStr = "0.0.0.0";
  }

  return std::tuple<nmdo::IpAddress, std::string>{
    nmdo::IpAddress{ipAddrStr}, vrfId
  };
}


void
Parser::parseLtmVirtualAddress(const nlohmann::ordered_json& doc)
{
  for (const auto& itemJson : doc.at("items")) {
    const std::string logicalSystemName{
      nmcu::toLower(itemJson.at("partition").get<std::string>())
    };
    auto& logicalSystem{data.logicalSystems[logicalSystemName]};
    logicalSystem.name = logicalSystemName;

    const std::string ifaceName{
      itemJson.at("name").get<std::string>()
    };
    logicalSystem.ifaces[ifaceName].setName(ifaceName);

    const std::string description{
      itemJson.at("fullPath").get<std::string>()
    };
    logicalSystem.ifaces[ifaceName].setDescription(description);

    const auto [ipMask, maskVrfId] =
      parseIpAddrVrfStr(itemJson.at("mask").get<std::string>());
    auto [ipAddr, vrfId] =
      parseIpAddrVrfStr(itemJson.at("address").get<std::string>());
    ipAddr.setNetmask(ipMask);

    logicalSystem.ifaces[ifaceName].addIpAddress(ipAddr);
    logicalSystem.vrfs[vrfId].setId(vrfId);
    logicalSystem.vrfs[vrfId].addIface(ifaceName);
  }
}


void
Parser::parseNetArpNdp(const nlohmann::ordered_json& doc)
{
  for (const auto& itemJson : doc.at("items")) {
    const std::string logicalSystemName{
      nmcu::toLower(itemJson.at("partition").get<std::string>())
    };
    auto& logicalSystem{data.logicalSystems[logicalSystemName]};
    logicalSystem.name = logicalSystemName;

    const std::string ifaceName{
      itemJson.at("name").get<std::string>()
    };
    logicalSystem.ifaces[ifaceName].setName(ifaceName);

    const std::string description{
      itemJson.at("fullPath").get<std::string>()
    };
    logicalSystem.ifaces[ifaceName].setDescription(description);

    const auto [peerIpAddr, vrfId] =
      parseIpAddrVrfStr(itemJson.at("ipAddress").get<std::string>());
    nmdo::MacAddress peerMacAddr{
      itemJson.at("macAddress").get<std::string>()
    };
    peerMacAddr.setResponding(true);
    peerMacAddr.addIp(peerIpAddr);

    logicalSystem.ifaces[ifaceName].addReachableMac(peerMacAddr);
    logicalSystem.vrfs[vrfId].setId(vrfId);
    logicalSystem.vrfs[vrfId].addIface(ifaceName);
  }
}


void
Parser::parseNetSelf(const nlohmann::ordered_json& doc)
{
  for (const auto& itemJson : doc.at("items")) {
    const std::string logicalSystemName{
      nmcu::toLower(itemJson.at("partition").get<std::string>())
    };
    auto& logicalSystem{data.logicalSystems[logicalSystemName]};
    logicalSystem.name = logicalSystemName;

    const std::string ifaceName{
      itemJson.at("name").get<std::string>()
    };
    logicalSystem.ifaces[ifaceName].setName(ifaceName);

    const std::string description{
      itemJson.at("fullPath").get<std::string>()
    };
    logicalSystem.ifaces[ifaceName].setDescription(description);

    const auto [ipAddr, vrfId] =
      parseIpAddrVrfStr(itemJson.at("address").get<std::string>());

    logicalSystem.ifaces[ifaceName].addIpAddress(ipAddr);
    logicalSystem.vrfs[vrfId].setId(vrfId);
    logicalSystem.vrfs[vrfId].addIface(ifaceName);
  }
}


void
Parser::parseNetRoute(const nlohmann::ordered_json& doc)
{
  for (const auto& itemJson : doc.at("items")) {
    const std::string logicalSystemName{
      nmcu::toLower(itemJson.at("partition").get<std::string>())
    };
    auto& logicalSystem{data.logicalSystems[logicalSystemName]};
    logicalSystem.name = logicalSystemName;

    nmdo::Route route;

    const std::string tableId{
      itemJson.at("name").get<std::string>()
    };
    route.setTableId(tableId);

    const std::string description{
      itemJson.at("fullPath").get<std::string>()
    };
    route.setDescription(description);

    const auto [dstIpNet, unusedVrfId] =
      parseIpAddrVrfStr(itemJson.at("network").get<std::string>());
    route.setDstIpNet(dstIpNet);

    const auto [nextHopIpAddr, vrfId] =
      parseIpAddrVrfStr(itemJson.at("gw").get<std::string>());
    route.setNextHopIpAddr(nextHopIpAddr);
    route.setVrfId(vrfId);

    logicalSystem.vrfs[vrfId].setId(vrfId);
    logicalSystem.vrfs[vrfId].addRoute(route);
  }
}


void
Parser::parseUnsupported(const nlohmann::ordered_json& doc)
{
  const std::string docKind{doc.at("kind").get<std::string>()};
  data.observations.addUnsupportedFeature(docKind);
}
