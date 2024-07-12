// =============================================================================
// Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC
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
    +(( noDetailConfig
      | detailConfig
      ) [(qi::_val = pnx::bind(&Parser::getData, this))]
    | ignoredLine
    )
    ;

  // ----- no detail, IOS or NXOS -----

  noDetailConfig =
    noDetailCapabilityCodes
    > noDetailHeader
    > *noDetailEntry [(pnx::bind(&Parser::finalizeData, this))]
    > *qi::eol
    > -noDetailEntryCount
    > *qi::eol
    ;

  noDetailCapabilityCodes =
    qi::lit("Capability Codes: ") > ignoredLine
    > *(!&qi::lit("Device") >> ignoredLine)
    ;

  noDetailHeader =
    (qi::lit("Device-ID") | qi::lit("Device ID"))
    > qi::lit("Local Intrfce")
    > (qi::lit("Holdtme") | qi::lit("Hldtme"))
    > qi::lit("Capability")
    > qi::lit("Platform")
    > qi::lit("Port ID")
    > qi::eol
    ;

  noDetailEntry =
    noDetailDeviceId [(pnx::ref(nd.curHostname) = qi::_1)]
    > noDetailLocalIface
    > noDetailHoldtime
    > noDetailCapability
    > noDetailPlatform [(pnx::ref(nd.curModel) = qi::_1)]
    > noDetailPortId [(pnx::ref(nd.curIfaceName) = qi::_1)]
    > -qi::eol
    ;

  noDetailEntryCount =
    qi::lit("Total ") > ignoredLine
    ;

  noDetailDeviceId =
    token
    > -qi::eol
    ;

  noDetailLocalIface =
    token
    > -( qi::ascii::blank
      >> (+qi::ascii::digit >> +(qi::char_('/') >> +qi::ascii::digit))
      )
    ;

  noDetailHoldtime =
    qi::uint_
    ;

  noDetailCapability =
    *(qi::ascii::graph >> qi::ascii::blank)
    ;

  // Variable size field of chars and whitespace, best guess min of "Platform"
  noDetailPlatform =
    qi::repeat(1,8)[ (qi::ascii::graph >> &qi::ascii::graph)
                   | (qi::ascii::graph >> &qi::ascii::blank)
                   | (qi::ascii::blank >> &qi::ascii::graph)
                   ]
    > -(&qi::ascii::graph > token)
    ;

  noDetailPortId =
    +((qi::ascii::graph)
    | (qi::ascii::blank >> &qi::ascii::graph)
    )
    ;

  // ----- detail, IOS or NXOS -----

  detailConfig =
    +( detailHeader
    >> detailEntry
    ) [(pnx::bind(&Parser::finalizeData, this))]
    ;

  detailHeader =
    +qi::char_('-') > qi::eol
    ;

  detailEntry =
    +(detailDeviceId
    | detailIpAddress
    | detailPlatform
    | detailInterface
    | ((!detailHeader) >> ignoredLine)
    | qi::eol
    )
    ;

  detailDeviceId =
    ( qi::lit("Device ID:")
    | qi::lit("Device-ID:")
    | qi::lit("SysName:")
    ) > token [(pnx::ref(nd.curHostname) = qi::_1)]
    > qi::eol
    ;

  detailIpAddress =
    qi::lit("IP")
    >> -(qi::lit("v4") | qi::lit("v6"))
    >> -(qi::char_("aA") >> qi::lit("ddress:"))
    > ipAddr
      [(pnx::bind([&](nmdo::IpAddress val){nd.ipAddrs.push_back(val);}, qi::_1))]
    > *token
    > qi::eol
    ;

  detailPlatform =
    qi::lit("Platform: ")
    > ( (csvToken >> !qi::lit(',') >> csvToken)
          [(pnx::ref(nd.curVendor) = qi::_1
          , pnx::ref(nd.curModel)  = qi::_2
          )]
      | (csvToken) [(pnx::ref(nd.curModel) = qi::_1)]
      )
    > *token
    > qi::eol
    ;

  detailInterface =
    qi::lit("Interface: ") > token
    > qi::lit("Port ID (outgoing port): ")
    > token [(pnx::ref(nd.curIfaceName) = qi::_1)]
    > qi::eol
    ;

  // ----- common usage -----

  token =
    +qi::ascii::graph
    ;

  csvToken =
    +(qi::ascii::graph - qi::char_(','))
    ;

  ignoredLine =
      (+token > qi::eol)
    | (qi::eol)
    ;

  BOOST_SPIRIT_DEBUG_NODES(
      //(start)
      (detailConfig)
        (detailHeader)
          (detailEntry)
            (detailDeviceId)
            (detailInterface)
            (detailIpAddress)
            (detailPlatform)
        (noDetailConfig)
          (noDetailCapabilityCodes)
          (noDetailHeader)
          (noDetailEntry)
            (noDetailDeviceId)
            (noDetailLocalIface)
            (noDetailHoldtime)
            (noDetailCapability)
            (noDetailPlatform)
            (noDetailPortId)
          (noDetailEntryCount)
      //(ignoredLine)
      //(token)
      //(csvToken)
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

  for (const auto& ip : nd.ipAddrs) {
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
    // normalize hostname
    auto temp {nd.curHostname};
    auto pos = temp.find('(');
    if (std::string::npos != pos) {
      temp = temp.substr(0, pos);
    }
    nd.curHostname = nmcu::toLower(temp);

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

  if (d != Data()) {
    r.push_back(d);
  }

  return r;
}
