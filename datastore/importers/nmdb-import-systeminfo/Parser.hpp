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

#include <netmeld/datastore/objects/ToolObservations.hpp>
#include <netmeld/datastore/parsers/ParserHelper.hpp>
#include <netmeld/datastore/objects/Interface.hpp>
#include <netmeld/datastore/parsers/ParserIpAddress.hpp>
#include <netmeld/datastore/parsers/ParserDomainName.hpp>
#include <netmeld/datastore/objects/DeviceInformation.hpp>
#include <netmeld/datastore/objects/OperatingSystem.hpp>

namespace nmdp = netmeld::datastore::parsers;
namespace nmdo = netmeld::datastore::objects;

// =============================================================================
// Data containers
// =============================================================================
struct Systeminfo
{
  std::string host_name; //deviceinfo deviceId
  // std::string os_name; //os
  // std::string os_version; //os
  // std::string os_manufacturer; //os
  // std::string os_configuration; //deviceinfo device type
  std::string build_type; //nohome
  std::string registered_owner; //nohome
  std::string registered_organization; //can be blank //nohome
  std::string product_id; //nohome
  std::string original_install_date; //nohome
  std::string system_boot_time; //nohome
  // std::string system_manufacturer;  //deviceinfo
  // std::string system_model; //deviceinfo
  // std::string system_type; //deviceinfo
  std::string processor; //nohome //nohome
  std::string bios_version; // optional //nohome
  std::string windows_directory; // optional //nohome
  std::string system_directory; //nohome
  std::string boot_device; // optional //nohome
  std::string domain; //nohome
  // std::map<std::string, nmdo::Interface> network_cards; //interface
  std::string hyper_v; //nohome

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
      oss << "HostName: " << host_name << ",\n ";
      oss << "OS Build Type: " << build_type << ",\n ";
      oss << "Registered Owner: " << registered_owner << ",\n ";
      oss << "Registered Organization: " << registered_organization << ",\n ";
      oss << "Product ID: " << product_id << ",\n ";
      oss << "Original Install Date: " << original_install_date << ",\n ";
      oss << "System Boot Time: " << system_boot_time << ",\n ";
      oss << "Processor(s):" << processor << ",\n ";
      oss << "BIOS Version:" << bios_version << ",\n ";
      oss << "Windows Directory:" << windows_directory << ",\n ";
      oss << "System Directory:" << system_directory << ",\n ";
      oss << "Boot Device:" << boot_device << ",\n ";
      oss << "Hyper-V Requirements:" << hyper_v << ",\n ";
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
  nmdo::DeviceInformation                 devInfo;
  nmdo::OperatingSystem                   os;
  std::vector<std::string>                hotfixs;
  std::map<std::string, nmdo::Interface>  network_cards;
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
    std::string curHostname;
    std::string curIfaceName;
    nmdp::ParserIpAddress   ipAddr;
    std::string curDomain;

  protected:
    // Rules
    qi::rule<nmdp::IstreamIter, Result(), qi::ascii::blank_type>
      start;

    qi::rule<nmdp::IstreamIter, qi::ascii::blank_type>
      ignoredLine;

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

    qi::rule<nmdp::IstreamIter, qi::ascii::blank_type>
     network_card;

    qi::rule<nmdp::IstreamIter, qi::ascii::blank_type>
     network_cards;

    qi::rule<nmdp::IstreamIter, std::string(), qi::ascii::blank_type>
      host_name,
      os_name,
      os_version,
      os_manufacturer,
      os_configuration,
      system_manufacturer,
      system_model,
      system_type,
      domain,
      hotfix,
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
    void setDomain(const std::string&);
    void addHotfix(const std::string&);
    void addInterface(const std::string&);
    void addIfaceConnectName(const std::string&);
    void addIfaceIp(nmdo::IpAddress&);
    void setIfaceStatus(const std::string&);
};
#endif // PARSER_HPP