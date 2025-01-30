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
  using DevInfo = nmdo::DeviceInformation;
  using OS      = nmdo::OperatingSystem;

  start =
    (*qi::eol > systemInfo > -qi::eol)
      [(qi::_val = pnx::bind(&Parser::getData, this))]
    ;

  restOfLine =
    +qi::ascii::print > qi::eol
    ;

  hostName =
    qi::lit("Host Name: ")
    > restOfLine [pnx::bind(&DevInfo::setDeviceId, &data.devInfo, qi::_1)]
    ;

  osName =
    qi::lit("OS Name: ")
    > -qi::lit("Microsoft")
    > restOfLine [pnx::bind(&OS::setProductName, &data.os, qi::_1)]
    ;

  osVersion =
    qi::lit("OS Version: ")
    > restOfLine [pnx::bind(&OS::setProductVersion, &data.os, qi::_1)]
    ;

  osManufacturer =
    qi::lit("OS Manufacturer: ")
    > ( (qi::lit("Microsoft Corporation") > qi::eol)
          [pnx::bind(&OS::setVendorName, &data.os, "microsoft")]
      | (restOfLine)
          [pnx::bind(&OS::setVendorName, &data.os, qi::_1)]
      )
    ;

  osConfiguration =
    qi::lit("OS Configuration: ")
    > restOfLine [pnx::bind(&DevInfo::setDescription, &data.devInfo, qi::_1)]
    ;

  systemManufacturer =
    qi::lit("System Manufacturer: ")
    > restOfLine [pnx::bind(&DevInfo::setVendor, &data.devInfo, qi::_1)]
    ;

  systemModel =
    qi::lit("System Model: ")
    > restOfLine [pnx::bind(&DevInfo::setModel, &data.devInfo, qi::_1)]
    ;

  systemType =
    qi::lit("System Type: ")
    > restOfLine [pnx::bind(&DevInfo::setDeviceType, &data.devInfo, qi::_1)]
    ;

  domain =
    qi::lit("Domain: ")
    > restOfLine
    ;

  bracketedNumber =
    qi::lit("[") >> +qi::ascii::digit >> qi::lit("]:")
    ;

  hotfix =
    bracketedNumber //"[" >> +qi::ascii::digit >> "]:"
    >> qi::as_string[qi::string("KB") >> +qi::ascii::digit]
        [pnx::bind(&Parser::addHotfix, this, qi::_1)]
    ;

  hotfixes =
    qi::lit("Hotfix(s): ")
    > restOfLine
    > +((hotfix > qi::eol)
      | (&qi::lit("[") > +qi::graph > qi::eol)
      )
    ;

  dhcpEnabledStatus =
    qi::lit("DHCP Enabled:")
    > (qi::lit("Yes") | qi::lit("No"))
    > qi::eol
    ;

  networkCard =
    bracketedNumber //"[" >> +qi::ascii::digit >> "]:"
    > restOfLine [pnx::ref(curIfaceName) = qi::_1]
    > qi::lit("Connection Name: ")
    > restOfLine [(pnx::bind(&Parser::addIfaceConnectName, this, qi::_1))]
    > -dhcpEnabledStatus
    > -(qi::lit("DHCP Server: ") > restOfLine)
    > -( qi::lit("IP address(es)") > qi::eol
      >> +( bracketedNumber //"[" >> +qi::ascii::digit >> "]:"
         >> ipAddr [(pnx::bind(&Parser::addIfaceIp, this, qi::_1))]
         > qi::eol
         )
      )
    > -( qi::lit("Status: ")
      > restOfLine [(pnx::bind(&Parser::setIfaceStatus, this, qi::_1))]
      )
    ;

  networkCards =
    qi::lit("Network Card(s):")
    > restOfLine
    > +networkCard
    ;

  systemInfo =
      hostName
    > osName
    > osVersion
    > osManufacturer
    > osConfiguration
    // Skip to system manufacturer
    > *(qi::char_ - qi::lit("System Manufacturer:"))
    > systemManufacturer
    > systemModel
    > systemType
    // Skip to domain
    > *(qi::char_ - qi::lit("Domain:"))
    > -domain [pnx::ref(curDomain) = qi::_1]
    // Skip to hotfixes
    > *(qi::char_ - qi::lit("Hotfix"))
    > -hotfixes
    > -networkCards
    // omit the rest of sysinfo (e.g., hyper-v)
    > qi::omit [*(qi::char_ - qi::eoi)]
  ;

  // Allows for error handling and debugging of qi.
  BOOST_SPIRIT_DEBUG_NODES(
      (start)
      (systemInfo)
      (hostName)
      (osName)
      (osVersion)
      (osManufacturer)
      (osConfiguration)
      (systemManufacturer)
      (systemModel)
      (systemType)
      (domain)
      (hotfixes)
      (hotfix)
      (networkCards)
      (networkCard)
      (dhcpEnabledStatus)
      (networkCardConnectionName)

      (bracketedNumber)
    );
}

// =============================================================================
// Parser helper methods
// =============================================================================

void Parser::addHotfix(const std::string& _string)
{
  data.hotfixes.push_back(_string);
}

void Parser::addInterface(const std::string& _name)
{
  nmdo::Interface iface {_name};
  curIfaceName = iface.getName();
  data.networkCards[curIfaceName] = iface;
}

void Parser::addIfaceConnectName(const std::string& _connectionname)
{
  auto& iface {data.networkCards[curIfaceName]};
  if (_connectionname.find("Wi-Fi") != std::string::npos) {
    iface.setMediaType("wi-fi");
  } else if (_connectionname.find("Bluetooth") != std::string::npos) {
    iface.setMediaType("bluetooth");
  } else { // default to ethernet
    iface.setMediaType("ethernet");
  }
  iface.setName(_connectionname);
  iface.setUp();
  iface.setDescription(curIfaceName);
}

void Parser::addIfaceIp(nmdo::IpAddress& _ipAddr)
{
  _ipAddr.addAlias( std::format("{}.{}", data.devInfo.getDeviceId(), curDomain)
                  , "from systeminfo"
                  );

  data.os.setIpAddr(_ipAddr);

  data.networkCards[curIfaceName].addIpAddress(_ipAddr);
}

void Parser::setIfaceStatus(const std::string& _status)
{
  data.networkCards[curIfaceName].setDown();
}

Result
Parser::getData()
{
  Result r;
  if (Data() != data) {
    data.os.setCpe();
    data.os.setAccuracy(1.0); // this is a definitive source

    r.push_back(data);
  }
  return r;
}
