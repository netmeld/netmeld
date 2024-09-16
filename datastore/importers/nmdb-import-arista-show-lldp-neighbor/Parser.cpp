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

  // ----- no detail -----

  noDetailConfig =
    noDetailTableInfo
    >> noDetailHeader
    > *noDetailEntry [(pnx::bind(&Parser::finalizeData, this))]
    > *qi::eol
    ;

  noDetailTableInfo = 
    qi::lit("Last table change time")
    > *(!qi::lit("Port") >> ignoredLine)
    ;

  noDetailHeader =
    qi::lit("Port")
    > qi::lit("Neighbor Device ID")  
    > qi::lit("Neighbor Port ID")
    > qi::lit("TTL")
    > qi::eol
    > +qi::char_('-')
    > qi::eol
    ;

  noDetailEntry =
    token
    > token [(pnx::ref(nd.curHostname) = qi::_1)]
    > port  [(pnx::ref(nd.curIfaceName) = qi::_1)]
    > qi::uint_
    > qi::eol
    ;

  // ----- detail -----

  detailConfig =
    (detailHeader
    >> (detailEntry | *qi::eol)
    ) [(pnx::bind(&Parser::finalizeData, this))]
    ;

  detailHeader =
    qi::lit("Interface")
    >> port [(pnx::ref(nd.srcIfaceName) = qi::_1)]
    >> qi::lit("detected")
    > ignoredLine
    > qi::eol
    ;

  detailEntry =
    detailNeighborLine
    > detailDiscovered
    > +(detailChassisId
      | detailPortId
      | detailPortDescription
      | detailSystemName
      | detailSystemDescription
      | detailCapabilities
      | detailVlan
      | (!( detailHeader
          | detailNeighborLine
          ) >> ignoredLine - qi::eol
        )
      )
    ;

  detailNeighborLine =
    qi::lit("Neighbor")
    > +(qi::char_ - '/')
    > '/'
    > ( inQuotes | port)
    > qi::lit(", age")
    > ignoredLine
    ;

  detailDiscovered =
    qi::lit("Discovered")
    > qi::uint_
    > qi::lit("days,")
    > token
    > ignoredLine
    ;

  detailChassisId =
    qi::lit("- Chassis ID type:") > restOfLine
    > qi::lit("Chassis ID     :") > restOfLine [(pnx::ref(nd.curMacAddr) = qi::_1)]
    ;

  detailPortId =
    qi::lit("- Port ID type:") > restOfLine
    > qi::lit("Port ID     :") > inQuotes [(pnx::ref(nd.curIfaceName) = qi::_1)]
    > qi::eol
    ;

  detailPortDescription =
    qi::lit("- Port Description:")
    > inQuotes [(pnx::ref(nd.curPortDescription) = qi::_1)]
    > qi::eol
    ;

  detailSystemName =
    qi::lit("- System Name:")
    >> inQuotes [(pnx::ref(nd.curHostname) = qi::_1)]
    > qi::eol
    ;

  detailSystemDescription =
    qi::lit("- System Description:")
    > inQuotes [(pnx::ref(nd.curSysDescription) = qi::_1)]
    > qi::eol
    ;

  detailCapabilities =
    (qi::lit("- System Capabilities :")
    > restOfLine [(pnx::ref(nd.curDeviceType) = qi::_1)]
    > -(qi::lit("Enabled Capabilities:"))
    >> restOfLine) [(pnx::ref(nd.curDeviceType) = qi::_1)]
    ;

  detailVlan =
    qi::lit("VLAN ID: ")
    > qi::ushort_ [(pnx::bind(&Parser::addVlan, this, qi::_1))]
    > qi::lit(", VLAN Name: ")
    > inQuotes
    > qi::eol
    ;


  // ----- common usage -----
  port =
    +(!qi::lit(',') >> qi::ascii::graph)
    > -qi::hold[
        (qi::char_(' ')
        >> (+qi::ascii::digit >> *(qi::char_('/') >> +qi::ascii::digit))
        )]
    ;

  restOfLine =
    *(qi::char_ - qi::eol) 
    >> qi::eol
    ;

  inQuotes =
    '"'
    > *(qi::char_ - '"')
    > '"'
    ;

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
      (noDetailConfig)
        (noDetailTableInfo)
        (noDetailHeader)
        (noDetailEntry)
      (detailConfig)
        (detailHeader)
        (detailEntry)
          (detailNeighborLine)
          (detailDiscovered)
          (detailChassisId)
          (detailPortId)
          (detailPortDescription)
          (detailSystemName)
          (detailSystemDescription)
          (detailCapabilities)
      (port)
      //(restOfLine)
      (inQuotes)
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
Parser::addVlan(uint16_t vlanId)
{
  nd.curVlans.emplace_back(vlanId);
}

void
Parser::updateInterfaces()
{
  nmdo::InterfaceNetwork iface {nd.curIfaceName};
  iface.setPartial(true);
  iface.setState(true);
  iface.setDiscoveryProtocol(true);
  if (!nd.curPortDescription.empty()) {
    iface.setDescription(nd.curPortDescription);
  }

  if (!nd.curMacAddr.empty()) {
    nmdo::MacAddress macAddr {nd.curMacAddr};
    iface.setMacAddress(macAddr);
  }

  if (!nd.curVlans.empty()) {
    for (auto& vlanId : nd.curVlans) {
      iface.addVlan(vlanId);
    }
  }

  d.interfaces.emplace_back(iface, getDevice(nd.curHostname));
}

void
Parser::updatePhysicalConnection()
{
  nmdo::PhysicalConnection physCon;
  physCon.setSrcIfaceName(nd.srcIfaceName);
  physCon.setDstDeviceId(getDevice(nd.curHostname));
  physCon.setDstIfaceName(nd.curIfaceName);

  d.physConnections.emplace_back(physCon);
}

void
Parser::updateDeviceInformation()
{
  nmdo::DeviceInformation devInfo;

  devInfo.setDeviceId(getDevice(nd.curHostname));
  if (!nd.curDeviceType.empty()) {
    devInfo.setDeviceType(nd.curDeviceType);
  }
  if (!nd.curSysDescription.empty()) {
    devInfo.setDescription(nd.curSysDescription);
  }

  d.devInfos.push_back(devInfo);
}

void
Parser::finalizeData()
{
  if (!nd.curHostname.empty())  {
    // normalize hostname
    auto temp {nd.curHostname};
    auto pos = temp.find('(');
    if (std::string::npos != pos) {
      temp = temp.substr(0, pos);
    }
    nd.curHostname = nmcu::toLower(temp);

    updateInterfaces();
    updatePhysicalConnection();
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
