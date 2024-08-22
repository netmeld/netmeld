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
    > *qi::eol
    > noDetailHeader
    > *noDetailEntry
    > *qi::eol
    ;

  noDetailTableInfo = 
    qi::lit("Last table change time") > ':' > (token >> qi::lit("ago") >> qi::eol) [(pnx::bind(&Parser::print, this, "Change time", qi::_1))]
    > qi::lit("Number of table inserts") > ':'  > (qi::uint_ >> qi::eol) [(pnx::bind(&Parser::printUint, this, "Table inserts", qi::_1))]
    > qi::lit("Number of table deletes") > ':' > (qi::uint_ >> qi::eol) [(pnx::bind(&Parser::printUint, this, "Table deletes", qi::_1))]
    > qi::lit("Number of table drops") > ':' > (qi::uint_ >> qi::eol) [(pnx::bind(&Parser::printUint, this, "Table drops", qi::_1))]
    > qi::lit("Number of table age-outs") > ':' > (qi::uint_ >> qi::eol) [(pnx::bind(&Parser::printUint, this, "Age-outs", qi::_1))]
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
    token [(pnx::bind(&Parser::print, this, "Port", qi::_1))]
    > token [(pnx::bind(&Parser::print, this, "Device Id", qi::_1))]
    > port [(pnx::bind(&Parser::print, this, "Port Id", qi::_1))]
    > token [(pnx::bind(&Parser::print, this, "TTL", qi::_1))]
    > qi::eol
    ;

  // noDetailPort =
  //   token
  //   ;

  // noDetailDeviceId =
  //   token
  //   ;

  port =
    token
    > -( qi::ascii::blank
      >> (+qi::ascii::digit >> +(qi::char_('/') >> +qi::ascii::digit))
      )
    ;

  // noDetailTTL =
  //   qi::uint_
  //   ;

  // ----- detail -----

  detailConfig =
    (detailHeader
    >> (detailBody | *qi::eol)
    ) [(pnx::bind(&Parser::finalizeData, this))]
    ;

  detailHeader =
    qi::lit("Interface")
    >> port [(pnx::bind(&Parser::print, this, "Port scanned: ", qi::_1))]
    >> ignoredLine
    > qi::eol
    ;

  detailBody =
    detailAge
    > detailChange
    > detailCommon
    > (*( detailSystemName
       | detailCapabilities
       | detailSystemDescription
       | (ignoredLine - qi::eol)
       )) [pnx::bind(&Parser::finalizeData, this)]
    ;

  detailAge =
    qi::lit("Neighbor")
    > port
    > qi::lit("age")
    > ignoredLine
    ;

  detailChange =
    qi::lit("Discovered")
    > qi::uint_
    > qi::lit("days,")
    > token
    > ignoredLine
    ;

  detailCommon =
    qi::lit("- Chassis ID type:") > restOfLine
    > qi::lit("Chassis ID     :") > restOfLine [(pnx::ref(nd.curMacAddr) = qi::_1)]
    > qi::lit("- Port ID type:") > restOfLine
    > qi::lit("Port ID     :") > restOfLine [(pnx::ref(nd.curIfaceName) = qi::_1)]
    > qi::lit("- Time To Live:") > qi::uint_
    > qi::lit("seconds") > qi::eol
    > qi::lit("- Port Description:") > restOfLine [(pnx::ref(nd.curPortDescription) = qi::_1)]
    ;

  detailSystemName =
    qi::lit("- System Name:")
    >> inQuotes [(pnx::ref(nd.curHostname) = qi::_1)]
    > qi::eol
    ;

  detailCapabilities =
    (qi::lit("- System Capabilities :") | qi::lit("Enabled Capabilities:"))
    > restOfLine [(pnx::ref(nd.curDeviceType) = qi::_1)]
    ;

  detailSystemDescription =
    qi::lit("- System Description:")
    > restOfLine [(pnx::ref(nd.curSysDescription) = qi::_1)]
    ;

  // ----- common usage -----
  restOfLine =
    *(qi::char_ - qi::eol) 
    >> qi::eol
    ;

  inQuotes =
    '"'
    > +(qi::char_ - '"')
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
          (noDetailPort)
          (noDetailDeviceId)
          (port)
          (noDetailTTL)
      (detailConfig)
        (detailHeader)
        (detailBody)
          (detailCommon)
          (detailAge)
          (detailChange)
          (detailSystemName)
          (detailCapabilities)
          (detailSystemDescription)
      //(restOfLine)
      //(inQuotes)
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
Parser::print(const std::string& label, const std::string& str)
{
    infoString += "DATA: " + label + " : " + str + '\n';
}

void
Parser::printUint(const std::string& label, const uint val)
{
    infoString += "DATA: " + label + " : " + std::to_string(val) + '\n';
}

void
Parser::printInfo()
{
  std::cerr << infoString;
}

// void
// Parser::updateIpAddrs()
// {
//   for (auto ip : nd.ipAddrs) {
//     ip.addAlias(nd.curHostname, "lldp import");
//     d.ipAddrs.push_back(ip);
//   }
// }

void
Parser::updateInterfaces()
{
  nmdo::InterfaceNetwork iface {nd.curIfaceName};
  iface.setPartial(true);
  iface.setState(true);
  iface.setDiscoveryProtocol(true);
  iface.setDescription(nd.curPortDescription);

  nmdo::MacAddress macAddr {nd.curMacAddr};
  iface.setMacAddress(macAddr);

  std::cerr << iface.toDebugString();
  d.interfaces.emplace_back(iface, getDevice(nd.curHostname));
}

void
Parser::updateDeviceInformation()
{
  nmdo::DeviceInformation devInfo;

  devInfo.setDeviceId(getDevice(nd.curHostname));
  devInfo.setDeviceType(nd.curDeviceType);
  devInfo.setDescription(nd.curSysDescription);

  std::cerr << devInfo.toDebugString();
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

    // updateIpAddrs();
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
