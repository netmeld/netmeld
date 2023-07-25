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
  std::string bios_version; // optional
  std::string windows_directory; // optional
  std::string system_directory;
  std::string boot_device; // optional
  std::string locale;
  std::string domain;
  std::string time_zone;
  std::string total_physical_memory;
  std::string available_physical_memory;
  std::string virtual_memory_max_size;
  std::string virtual_memory_available;
  std::string page_file_location;
  std::string logon_server;
  std::vector<std::string> hotfixs;
  std::vector<std::string> network_cards;
  std::string hyper_v;

  void
  save(pqxx::transaction_base& t, const nmco::Uuid& toolRunId, const std::string& deviceId)
  {
      if(!isValid() && !deviceId.empty()){
          LOG_DEBUG << "Package object is not saving: " << toDebugString()
              << std::endl;
      return;
      }

      // t.exec_prepared("insert_raw_packages",
      //     toolRunId,
      //     state,
      //     name,
      //     version,
      //     architecture,
      //     description);
  }

  std::string
  toDebugString() const
  {
      std::ostringstream oss;
      oss << "["; // opening bracket
      oss << "Host Name: " << host_name << ",\n ";
      oss << "OS Name: " << os_name << ",\n ";
      oss << "OS Version: " << os_version << ",\n ";
      oss << "OS Manufacturer: " << os_manufacturer << ",\n ";
      oss << "OS Configuration: " << os_configuration << ",\n ";
      oss << "OS Build Type: " << build_type << ",\n ";
      oss << "Registered Owner: " << registered_owner << ",\n ";
      oss << "Registered Organization: " << registered_organization << ",\n ";
      oss << "Product ID: " << product_id << ",\n ";
      oss << "Original Install Date: " << original_install_date << ",\n ";
      oss << "System Boot Time: " << system_boot_time << ",\n ";
      oss << "System Manufacturer: " << system_manufacturer << ",\n ";
      oss << "System Model: " << system_model << ",\n ";
      oss << "System Type: " << system_type << ",\n ";
      oss << "Processor(s): " << processor << ",\n ";
      oss << "]"; // closing bracket
      return oss.str();
  }

  bool
  isValid() const
  {
      return true;
  }
};

struct Data
{
  Systeminfo                              sysinfo_;
  nmdo::ToolObservations                  observations;
};
typedef std::vector<Data> Result;

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
      network_cards;

    qi::rule<nmdp::IstreamIter, Systeminfo(), qi::ascii::blank_type>
      systeminfo;

    qi::rule<nmdp::IstreamIter, std::string(), qi::ascii::blank_type>
      hotfixs,
      networkCardName,
      networkCardConnectionName,
      dhcpServer,
      networkCardStatus,
      ipAddressLine
      ;

    qi::rule<nmdp::IstreamIter, std::vector<std::string>(), qi::ascii::blank_type>
      ipaddresssection
    ;

    qi::rule<nmdp::IstreamIter, bool(), qi::ascii::blank_type>
      dhcpEnabledStatus;

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
      virtual_memory_available,
      virtual_memory_in_use,
      page_file_locations,
      domain,
      logon_server,
      hotfix,
      network_card,
      hyper_v,
      token;
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
    void setOS(const std::string&);
    void setOSVersion(const std::string&);
    void setOSManufacturer(const std::string&);
    void setOSConfiguration(const std::string&);
    void setOSBuildType(const std::string&);
    void setRegisteredOwner(const std::string&);
    void setRegisteredOrganization(const std::string&);
    void setProductID(const std::string&);
    void setOriginalInstallDate(const std::string&);
    void setSystemBootTime(const std::string&);
    void setSystemManufacturer(const std::string&);
    void setSystemModel(const std::string&);
    void setSystemType(const std::string&);
    void setProcessors(const std::string&);
    void setBIOSVersion(const std::string&);
    void setWindowsDirectory(const std::string&);
    void setSystemDirectory(const std::string&);
    void setBootDevice(const std::string&);
    void setSystemLocale(const std::string&);
    void setInputLocale(const std::string&);
    void setTimeZone(const std::string&);
    void setTotalPhysicalMemory(const std::string&);
    void setAvailablePhysicalMemory(const std::string&);
    void setVirtualMemoryMax(const std::string&);
    void setVirtualMemoryAvailable(const std::string&);
    void setVirtualMemoryInUse(const std::string&);
    void setPageFileLocations(const std::string&);
    void setDomain(const std::string&);
    void setLogonServer(const std::string&);
    void setHotfixs(const std::string&);
    void setNetworkCards(const std::string&);
    void setHyperV(const std::string&);
    void addSysteminfo(Systeminfo&);
};
#endif // PARSER_HPP