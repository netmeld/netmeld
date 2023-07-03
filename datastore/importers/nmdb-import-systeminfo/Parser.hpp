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

#ifndef PARSER_HPP
#define PARSER_HPP

// #include <netmeld/datastore/objects/DeviceInformation.hpp>
// #include <netmeld/datastore/objects/OperatingSystem.hpp>
// #include <netmeld/datastore/objects/Interface.hpp>
// #include <netmeld/datastore/parsers/ParserIpAddress.hpp>
// #include <netmeld/datastore/parsers/ParserMacAddress.hpp>
#include <netmeld/datastore/objects/ToolObservations.hpp>
#include <netmeld/datastore/parsers/ParserHelper.hpp>

namespace nmdp = netmeld::datastore::parsers;
namespace nmdo = netmeld::datastore::objects;

// =============================================================================
// Data containers
// =============================================================================
struct Data
{
  nmdo::ToolObservations                  observations;
};
typedef std::vector<Data> Result;

struct Systeminfo
{
  std::string host_name;
  std::string os_name;
  std::string os_version;
  std::string os_manufacturer;
  std::string os_configuration;
  std::string build_type;
  std::string registered_owner;
  std::string registered_organization;
  std::string product_id;
  std::string original_install_date;
  std::string system_boot_time;
  std::string system_manufacturer;
  std::string system_model;
  std::string system_type;
  std::string processor;
  std::string system_directory;
  std::string locale;
  std::string domain;
  std::string time_zone;
  std::string total_physical_memory;
  std::string available_physical_memory;
  std::string virtual_memory_max_size;
  std::string virtual_memory_in_use;
  std::string page_file_location;
};

// =============================================================================
// Parser definition
// =============================================================================
class Parser :
  public qi::grammar<nmdp::IstreamIter, Result(), qi::ascii::blank_type>
{
  // ===========================================================================
  // Variables
  // ===========================================================================
  private:
    // Supporting data structures
    Data data;

  protected:
    // Rules
    qi::rule<nmdp::IstreamIter, Result(), qi::ascii::blank_type>
      start;

    qi::rule<nmdp::IstreamIter, qi::ascii::blank_type>
     ignoredLine,
     token;

    qi::rule<nmdp::IstreamIter, qi::ascii::blank_type>
      systeminfo;

    qi::rule<nmdp::IstreamIter, std::string(), qi::ascii::blank_type>
      host_name,
      os_name,
      os_version,
      os_manufacturer,
      os_configuration,
      os_build_type,
      registered_owner,
      registered_organization,
      product_id,
      original_install_date,
      system_boot_time,
      system_manufacturer,
      system_model,
      system_type,
      processors,
      bios_version,
      windows_directory,
      system_directory,
      boot_device,
      system_locale,
      input_locale,
      time_zone,
      total_physical_memory,
      available_physical_memory,
      virtual_memory_max,
      virtual_memery_available,
      virtual_memory_in_use,
      page_file_locations,
      domain,
      logon_server,
      hotfixs,
      network_cards,
      hyper_v;
  // ===========================================================================
  // Constructors
  // ===========================================================================
  public:
    Parser();

  // ===========================================================================
  // Methods
  // ===========================================================================
  private:
    Result getData();
    void setHostname(const std::string&);
    Systeminfo sysinfo_;
};
#endif // PARSER_HPP