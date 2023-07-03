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
  start = (
    systeminfo
    ) [(qi::_val = pnx::bind(&Parser::getData, this))]
  ;
  token =
    +(qi::ascii::print)
  ;

  host_name = 
    qi::lit("Host Name:") >> (token - '\n') >> qi::eol
  ;
  os_name = 
    qi::lit("OS Name:") >> (token - '\n') >> qi::eol
  ;
  os_version = 
    qi::lit("OS Version:") >> (token - '\n') >> qi::eol
  ;
  os_manufacturer = 
    qi::lit("OS Manufacturer:") >> (token - '\n') >> qi::eol
  ;
  os_configuration  =
    qi::lit("OS Configuration:") >> (token - '\n') >> qi::eol
  ;
  os_build_type  =
    qi::lit("OS Build Type:") >> (token - '\n') >> qi::eol
  ;
  registered_owner  =
    qi::lit("Registered Owner:") >> (token - '\n') >> qi::eol
  ;
  registered_organization  =
    qi::lit("Registered Organization:") >> (token - '\n') >> qi::eol
  ;
  product_id  =
    qi::lit("Product ID:") >> (token - '\n') >> qi::eol
  ;
  original_install_date  =
    qi::lit("Original Install Date:") >> (token - '\n') >> qi::eol
  ;
  system_boot_time  =
    qi::lit("System Boot Time:") >> (token - '\n') >> qi::eol
  ;
  system_manufacturer  =
    qi::lit("System Manufacturer:") >> (token - '\n') >> qi::eol
  ;
  system_model  =
    qi::lit("System Model:") >> (token - '\n') >> qi::eol
  ;
  system_type  =
    qi::lit("System Type:") >> (token - '\n') >> qi::eol
  ;
  processors  =
    qi::lit("Processor(s):") >> (token - '\n') >> qi::eol
  ;
  bios_version  =
    qi::lit("BIOS Version:") >> (token - '\n') >> qi::eol
  ;
  windows_directory  =
    qi::lit("Windows Directory:") >> (token - '\n') >> qi::eol
  ;
  system_directory  =
    qi::lit("System Directory:") >> (token - '\n') >> qi::eol
  ;
  boot_device  =
    qi::lit("Boot Device:") >> (token - '\n') >> qi::eol
  ;
  system_locale  =
    qi::lit("System Locale:") >> (token - '\n') >> qi::eol
  ;
  input_locale  =
    qi::lit("Input Locale:") >> (token - '\n') >> qi::eol
  ;
  time_zone  =
    qi::lit("Time Zone:") >> (token - '\n') >> qi::eol
  ;
  total_physical_memory  =
    qi::lit("Total Physical Memory:") >> (token - '\n') >> qi::eol
  ;
  available_physical_memory  =
    qi::lit("Available Physical Memory:") >> (token - '\n') >> qi::eol
  ;
  virtual_memory_max  =
    qi::lit("Virtual Memory: Max Size:") >> (token - '\n') >> qi::eol
  ;
  virtual_memery_available  =
    qi::lit("Virtual Memory: Available:") >> (token - '\n') >> qi::eol
  ;
  virtual_memory_in_use  =
    qi::lit("Virtual Memory: In Use:") >> (token - '\n') >> qi::eol
  ;
  page_file_locations  =
    qi::lit("Page File Location(s):") >> (token - '\n') >> qi::eol
  ;
  domain  =
    qi::lit("Domain:") >> (token - '\n') >> qi::eol
  ;
  logon_server  =
    qi::lit("Logon Server:") >> (token - '\n') >> qi::eol
  ;
  hotfixs  =
    qi::lit("Hotfix(s):") >> (token - '\n') >> qi::eol
  ;
  network_cards  =
    qi::lit("Network Card(s):") >> (token - '\n') >> qi::eol
  ;
  hyper_v = 
    qi::lit("Hyper-V Requirements:") >> (token - '\n') >> qi::eol
  ;
  systeminfo =
      host_name >>
      os_name >>
      os_version >>
      os_manufacturer >>
      os_configuration >>
      os_build_type >>
      registered_owner >>
      registered_organization >>
      product_id >>
      original_install_date >>
      system_boot_time >>
      system_manufacturer >>
      system_model >>
      system_type;
      // processors >>
      // bios_version >>
      // windows_directory >>
      // system_directory >>
      // boot_device >>
      // system_locale >>
      // input_locale >>
      // time_zone >>
      // total_physical_memory >>
      // available_physical_memory >>
      // virtual_memory_max >>
      // virtual_memery_available >>
      // virtual_memory_in_use >>
      // page_file_locations >>
      // domain >>
      // logon_server >>
      // hotfixs >>
      // network_cards >>
      // hyper_v;
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
      // (processors)
      // (bios_version)
      // (windows_directory)
      // (system_directory)
      // (boot_device)
      // (system_locale)
      // (input_locale)
      // (time_zone)
      // (total_physical_memory)
      // (available_physical_memory)
      // (virtual_memory_max)
      // (virtual_memery_available)
      // (virtual_memory_in_use)
      // (page_file_locations)
      // (domain)
      // (logon_server)
      // (hotfixs)
      // (network_cards)
      // (hyper_v)
      (token)
      // (systeminfo)
      );
}

// =============================================================================
// Parser helper methods
// =============================================================================
void
Parser::setHostname(const std::string& _string)
{
  sysinfo_.host_name = _string;
}

Result
Parser::getData()
{
  Result r;
  r.push_back(data);
  return r;
}
