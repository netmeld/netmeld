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
Parser::Parser() : Parser::base_type(start)
{
  start =
    ((systeminfo >> -qi::eol
    | qi::eol
    )) [(qi::_val = pnx::bind(&Parser::getData, this))]
  ;
  token =
    +qi::ascii::graph
  ;

  host_name =
    "Host Name: " > token > qi::eol
  ;

  os_name  =
    "OS Name: " > token > qi::eol
  ;

  os_version =
      ("OS Version: " > token > qi::eol)
    ;

  os_manufacturer =
      ("OS Manufacturer: " > token > qi::eol)
    ;

  os_configuration =
      ("OS Configuration: " > token > qi::eol)
    ;

  os_build_type =
      ("OS Build Type: " > token > qi::eol)
    ;

  registered_owner =
      ("Registered Owner: " > token > qi::eol)
    ;

  registered_organization =
      ("Registered Organization: " > token > qi::eol)
    ;
  product_id =
      ("Product ID: " > token > qi::eol)
    ;

  original_install_date =
      ("Original Install Date: " > token > qi::eol)
    ;

  system_boot_time =
      ("System Boot Time: " > token > qi::eol)
    ;

  system_manufacturer =
      ("System Manufacturer: " > token > qi::eol)
    ;

  system_model =
      ("System Model: " > token > qi::eol)
    ;

  system_type =
      ("System Type: " > token > qi::eol)
    ;

  processors =
      ("Processor(s): " > token > qi::omit[(qi::eol)] > token > qi::eol) //parse until we hit bios
    ;

  bios_version =
      ("BIOS Version: " > token > qi::eol)
    ;

  windows_directory =
      ("Windows Directory: " > token > qi::eol)
    ;

  system_directory =
      ("System Directory: " > token > qi::eol)
    ;

  boot_device =
      ("Boot Device: " > token > qi::eol)
    ;

  domain =
      ("Domain: " > token > qi::eol)
    ;

  hotfix =
      +(token - qi::omit[qi::eol])// add hotfixes here
  ;
  hotfixs =
      ("Hotfix(s): " > *(hotfix [pnx::bind(&Parser::addHotfixs, this, qi::_1)] > qi::eol) )
    ;
  networkCardName =
    qi::lexeme[+(qi::ascii::char_ - qi::eol)]
  ;
  networkCardConnectionName =
    qi::lexeme[+(qi::ascii::char_ - qi::eol)]
  ;

  ipAddressLine =
    qi::int_ >> '.' >> qi::int_ >> '.' >> qi::int_ >> '.' >> qi::int_
  ;

  ipaddresssection =
    +qi::ascii::graph
  ;
  dhcpServer =
    qi::lexeme[+(qi::char_ - qi::eol)];
  ;
  dhcpEnabledStatus =
    qi::lit("DHCP Enabled:") >> ( qi::lit("Yes") |  qi::lit("No"))
  ;

  networkCardStatus =
    +qi::ascii::graph
  ;
  network_card =
  (
        '[' >> qi::lexeme[+qi::char_("0-9")]
        >> ']' >> qi::lit(':')
        >> networkCardName [(pnx::bind(&Parser::addInterface, this, qi::_1))] >> qi::eol
        >> "Connection Name: " > networkCardConnectionName [(pnx::bind(&Parser::addIfaceConnectName, this, qi::_1))] >> qi::eol
        >> -(dhcpEnabledStatus >> qi::eol)
        >> -("DHCP Server: " >> dhcpServer >> qi::eol)
        >> -("IP address(es)" >> qi::eol >> ('[' >> qi::lexeme[+qi::char_("0-9")] >> "]: " >> ipAddr [(pnx::bind(&Parser::addIfaceIp, this, qi::_1))] ) >> qi::eol)
        >> -("Status: " >> networkCardStatus [(pnx::bind(&Parser::setIfaceStatus, this, qi::_1))] >> qi::eol)
    );
  ;
  network_cards =
      qi::lit("Network Card(s):") >> +qi::ascii::print > qi::eol
      > +network_card
    ;

  hyper_v =
      ("Hyper-V Requirements: " > token > qi::eol)
    ;
  systeminfo =
      // host_name [pnx::bind(&nmdo::DeviceInformation::setDeviceId, pnx::ref(data.devInfo), pnx::bind(&std::to_string, qi::_1))]
      host_name [(pnx::bind(&Parser::setHostname, this, qi::_1))]
      > os_name [(pnx::bind(&Parser::setOS, this, qi::_1))]
      > os_version [pnx::bind(&Parser::setOSVersion, this, qi::_1)]
      > os_manufacturer [pnx::bind(&Parser::setOSManufacturer, this, qi::_1)]
      > os_configuration [pnx::bind(&Parser::setOSConfiguration, this, qi::_1)]
      > os_build_type [pnx::bind(&Parser::setOSBuildType, this, qi::_1)]
      > registered_owner [pnx::bind(&Parser::setRegisteredOwner, this, qi::_1)]
      > registered_organization [pnx::bind(&Parser::setRegisteredOrganization, this, qi::_1)]
      > product_id [pnx::bind(&Parser::setProductID, this, qi::_1)]
      > original_install_date [pnx::bind(&Parser::setOriginalInstallDate, this, qi::_1)]
      > system_boot_time [pnx::bind(&Parser::setSystemBootTime, this, qi::_1)]
      > system_manufacturer [pnx::bind(&Parser::setSystemManufacturer, this, qi::_1)]
      > system_model [pnx::bind(&Parser::setSystemModel, this, qi::_1)]
      > system_type [pnx::bind(&Parser::setSystemType, this, qi::_1)]
      > processors [pnx::bind(&Parser::setProcessors, this, qi::_1)]
      > -bios_version [pnx::bind(&Parser::setBIOSVersion, this, qi::_1)]
      > -windows_directory [pnx::bind(&Parser::setWindowsDirectory, this, qi::_1)]
      > system_directory [pnx::bind(&Parser::setSystemDirectory, this, qi::_1)]
      > -boot_device [pnx::bind(&Parser::setBootDevice, this, qi::_1)]
      > *(qi::char_ - qi::lit("Domain:"))
      > domain [pnx::bind(&Parser::setDomain, this, qi::_1)]
      > *(qi::char_ - qi::lit("Hotfix"))
      > hotfixs
      > network_cards
      > hyper_v [pnx::bind(&Parser::setHyperV, this, qi::_1)];
  ;
  ignoredLine =
    (qi::ascii::print > -qi::eol) | +qi::eol
  ;

  // Allows for error handling and debugging of qi.
  BOOST_SPIRIT_DEBUG_NODES(
      (start)
      (systeminfo)
      (host_name)
      (os_name)
      (os_version)
      (os_manufacturer)
      (os_configuration)
      (os_build_type)
      (registered_owner)
      (registered_organization)
      (product_id)
      (original_install_date)
      (system_boot_time)
      (system_manufacturer)
      (system_model)
      (system_type)
      (processors)
      (bios_version)
      (windows_directory)
      (system_directory)
      (boot_device)
      (domain)
      (hotfixs)
      (hotfix)
      (network_cards)
      (network_card)
      (dhcpServer)
      (dhcpEnabledStatus)
      (ipAddressLine)
      (networkCardName)
      (networkCardConnectionName)
      (networkCardStatus)
      (hyper_v)
      // (token)
      );
}

// =============================================================================
// Parser helper methods
// =============================================================================
void
Parser::setHostname(const std::string& _string)
{
  data.devInfo.setDeviceId(_string);
}

void
Parser::setOS(const std::string& _string)
{
  data.os.setProductName(_string);
}

void
Parser::setOSVersion(const std::string& _string)
{
  data.os.setProductVersion(_string);
}

void
Parser::setOSManufacturer(const std::string& _string)
{
  data.os.setVendorName(_string);
}

void
Parser::setOSConfiguration(const std::string& _string)
{
  data.devInfo.setDescription(_string);
}

void
Parser::setOSBuildType(const std::string& _string)
{
  data.sysinfo_.build_type = _string;
}

void
Parser::setRegisteredOwner(const std::string& _string)
{
  data.sysinfo_.registered_owner = _string;
}

void
Parser::setRegisteredOrganization(const std::string& _string)
{
  data.sysinfo_.registered_organization = _string;
}

void
Parser::setProductID(const std::string& _string)
{
  data.sysinfo_.product_id = _string;
}

void
Parser::setOriginalInstallDate(const std::string& _string)
{
  data.sysinfo_.original_install_date = _string;
}

void
Parser::setSystemBootTime(const std::string& _string)
{
  data.sysinfo_.system_boot_time = _string;
}

void
Parser::setSystemManufacturer(const std::string& _string)
{
  data.devInfo.setVendor(_string);
}

void
Parser::setSystemModel(const std::string& _string)
{
  data.devInfo.setModel(_string);
}

void
Parser::setSystemType(const std::string& _string)
{
  data.devInfo.setDeviceType(_string);
}

void
Parser::setProcessors(const std::string& _string)
{
  data.sysinfo_.processor = _string;
}

void
Parser::setBIOSVersion(const std::string& _string)
{
  data.sysinfo_.bios_version = _string;
}

void
Parser::setWindowsDirectory(const std::string& _string)
{
  data.sysinfo_.windows_directory = _string;
}

void
Parser::setSystemDirectory(const std::string& _string)
{
  data.sysinfo_.system_directory = _string;
}

void
Parser::setBootDevice(const std::string& _string)
{
  data.sysinfo_.boot_device = _string;
}

void
Parser::setDomain(const std::string& _string)
{
  data.sysinfo_.domain = _string;
}

void
Parser::addHotfixs(const std::string& _string)
{
  nmdo::Hotfix hotfix;
  hotfix.setHotfix(_string);
  data.hotfixs.push_back(hotfix);
}

void
Parser::addInterface(const std::string& _name)
{
  nmdo::Interface iface;
  iface.setName(_name);
  curIfaceName = iface.getName();
  data.sysinfo_.network_cards[curIfaceName] = iface;
}

void
Parser::addIfaceConnectName(const std::string& _connectionname)
{
  auto& iface {data.sysinfo_.network_cards[curIfaceName]};
  if(_connectionname.find("Wi-Fi") != std::string::npos || _connectionname.find("Bluetooth") != std::string::npos)
  {
    iface.setMediaType(_connectionname);
  }
  iface.setUp();
  iface.setDescription(_connectionname);
}

void
Parser::addIfaceIp(nmdo::IpAddress& _ipAddr)
{
  auto& iface {data.sysinfo_.network_cards[curIfaceName]};

  _ipAddr.addAlias(data.sysinfo_.host_name, "systeminfo");
  iface.addIpAddress(_ipAddr);
}

void
Parser::setIfaceStatus(const std::string& _status)
{
  auto& iface {data.sysinfo_.network_cards[curIfaceName]};
  iface.setDown();

}

void
Parser::setHyperV(const std::string& _string)
{
  data.sysinfo_.hyper_v = _string;
}

// void
// Parser::addSysteminfo(Systeminfo& systeminfo)
// {
//   // data.sysinfo_.os_name = ;
//   // systeminfo.os_name = "test";
//   // data.sysinfo_.host_name = systeminfo.os_name;
//   // data.sysinfo_.os_name = "TestName";
//   // data.sysinfo_ = systeminfo;
// }

Result
Parser::getData()
{
  Result r;
  r.push_back(data);
  return r;
}
