// =============================================================================
// Copyright 2017 National Technology & Engineering Solutions of Sandia, LLC
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

#include <netmeld/core/utils/StringUtilities.hpp>

#include "Parser.hpp"

namespace nmcu = netmeld::core::utils;

// =============================================================================
// Parser logic
// =============================================================================
Parser::Parser() : Parser::base_type(start)
{
  start =
    config
      [qi::_val = pnx::bind(&Parser::getData, this)]
    ;

  config =
    *(qi::eol)
    >> *(header >> deviceData) [pnx::bind(&Parser::finalizeData, this)]
    ;

  header =
    +qi::char_('-') > qi::eol
    ;

  deviceData =
    +( hostnameValue
     | ipAddressValue
     | platformValue
     | interfaceValue
     | ((!header) >> ignoredLine)
     | qi::eol
    )
    ;

  hostnameValue =
    (  (qi::lit("Device") > (+qi::blank | qi::lit('-')) > qi::lit("ID:"))
     | (qi::lit("SysName:"))
    ) > -(+qi::blank)
    > token [pnx::bind(&NeighborData::curHostname, &nd)
             = pnx::bind(&nmcu::toLower, qi::_1)]
    > qi::eol
    ;

  ipAddressValue =
    qi::lit("IP") > -(qi::lit("v") > qi::char_("46")) > +qi::blank
    > -(qi::char_("aA") > qi::lit("ddress: "))
    > ipAddr
      [pnx::bind([&](nmdo::IpAddress val){nd.ipAddrs.push_back(val);}, qi::_1)]
    // cppcheck-suppress compareBoolExpressionWithInt
    > *(+qi::blank > token)
    > qi::eol
    ;

  platformValue =
    qi::lit("Platform: ")
    > token [pnx::bind(&NeighborData::curVendor, &nd) = qi::_1]
    > +qi::blank
    > token [pnx::bind(&NeighborData::curModel, &nd) = qi::_1]
    // cppcheck-suppress compareBoolExpressionWithInt
    > *(+qi::blank > token)
    > qi::eol
    ;

  interfaceValue =
    qi::lit("Interface: ") > token
    > +qi::blank > qi::lit("Port ID (outgoing port): ")
    > token [pnx::bind(&NeighborData::curIfaceName, &nd) = qi::_1]
    > qi::eol
    ;

  token =
    +qi::ascii::graph
    ;

  ignoredLine =
    +token > qi::eol
    ;

  BOOST_SPIRIT_DEBUG_NODES(
      //(start)
      (config)
      (header) (deviceData)
      (hostnameValue)
      (ipAddressValue)
      (platformValue)
      (interfaceValue)
      (ignoredLine)
      //(token)
      );
}

// =============================================================================
// Parser helper methods
// =============================================================================
std::string
Parser::getDevice(const std::string& hostname)
{
  return hostname.substr(0, hostname.find("."));
}

void
Parser::updateIpAddrs()
{
  // NOTE: want a copy, else interface propagates wrong (alias, reason)
  for (auto ip : nd.ipAddrs) {
    ip.addAlias(nd.curHostname, "cdp import");
    d.ipAddrs.push_back(ip);
  }
}

void
Parser::updateInterfaces()
{
  nmdo::InterfaceNetwork iface {nd.curIfaceName};
  iface.setPartial(true);

  for (auto& ip : nd.ipAddrs) {
    iface.addIpAddress(ip);
  }

  d.interfaces.emplace_back(iface, getDevice(nd.curHostname));
}

void
Parser::updateDeviceInformation()
{
  nmdo::DeviceInformation devInfo;

  devInfo.setDeviceId(getDevice(nd.curHostname));

  if (',' == nd.curVendor.back()) {
    nd.curVendor.pop_back();
  } else if (nd.curModel.back() == ',') {
    nd.curModel.pop_back();
  }

  if (nd.curModel.empty()) {
    devInfo.setModel(nd.curVendor);
  } else {
    devInfo.setVendor(nd.curVendor);
    devInfo.setModel(nd.curModel);
  }

  d.devInfos.push_back(devInfo);
}

void
Parser::finalizeData()
{
  if (!nd.curHostname.empty()) {
    updateIpAddrs();
    updateInterfaces();
    updateDeviceInformation();
  }

  nd = NeighborData();
}

// Object return
Result
Parser::getData()
{
  Result r;
  r.push_back(d);

  return r;
}
