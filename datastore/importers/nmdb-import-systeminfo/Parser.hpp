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
// Data Struct definition
// =============================================================================
struct Data
{
  auto operator<=>(const Data&) const = default;
  bool operator==(const Data&) const = default;

  nmdo::DeviceInformation                 devInfo;
  nmdo::OperatingSystem                   os;
  std::vector<std::string>                hotfixes;
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

    qi::rule<nmdp::IstreamIter, std::string(), qi::ascii::blank_type>
      hotfixes,
      networkCardName,
      networkCardConnectionName,
      dhcpServer,
      networkCardStatus,
      domain,
      token;

    qi::rule<nmdp::IstreamIter, bool(), qi::ascii::blank_type>
      dhcpEnabledStatus;

    qi::rule<nmdp::IstreamIter, qi::ascii::blank_type>
      ignoredLine,
      hostName,
      osName,
      osVersion,
      osManufacturer,
      osConfiguration,
      systemManufacturer,
      systemModel,
      systemType,
      hotfix,
      hyperV,
      networkCard,
      networkCards,
      systemInfo;

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
    void addHotfix(const std::string&);
    void addInterface(const std::string&);
    void addIfaceConnectName(const std::string&);
    void addIfaceIp(nmdo::IpAddress&);
    void setIfaceStatus(const std::string&);
};
#endif // PARSER_HPP
