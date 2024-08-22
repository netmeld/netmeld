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

#include <netmeld/datastore/objects/DeviceInformation.hpp>
#include <netmeld/datastore/objects/IpAddress.hpp>
#include <netmeld/datastore/objects/InterfaceNetwork.hpp>
#include <netmeld/datastore/parsers/ParserDomainName.hpp>
#include <netmeld/datastore/parsers/ParserIpAddress.hpp>

namespace nmdo = netmeld::datastore::objects;
namespace nmdp = netmeld::datastore::parsers;


// =============================================================================
// Data containers
// =============================================================================
struct Data {
  std::vector<nmdo::IpAddress>         ipAddrs;
  std::vector<nmdo::DeviceInformation> devInfos;

  std::vector<std::pair<nmdo::InterfaceNetwork, std::string>> interfaces;

  auto operator<=>(const Data&) const = default;
  bool operator==(const Data&) const = default;
};
typedef std::vector<Data>  Result;

struct NeighborData {
  std::string curHostname   {""}; // DeviceInformation.deviceId
  std::string curIfaceName     {""}; // InterfaceNetwork.name
  std::string curDeviceType {""}; // DeviceInformation.deviceType
  std::string curSysDescription{""}; // DeviceInformation.description
  std::string curPortDescription  {""}; // InterfaceNetworkDescription
  std::string curMacAddr       {""}; // InterfaceNetwork.macAddr
  std::vector<nmdo::IpAddress>  ipAddrs;
  // vlans for later implimentation


  auto operator<=>(const NeighborData&) const = default;
  bool operator==(const NeighborData&) const = default;
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
  protected:
    Data d;
    NeighborData nd;

    // Rules
    qi::rule<nmdp::IstreamIter, Result(), qi::ascii::blank_type>
      start;

    // from what I understand, these save data in their definition
    qi::rule<nmdp::IstreamIter, qi::ascii::blank_type>
        detailConfig
      , detailCommon
      , detailHeader
      , detailBody
      , detailAge
      , detailChange
      , detailChassisInfo
      , detailPortInfo
      , detailSystemName
      , detailCapabilities
      , detailSystemDescription
      , noDetailConfig
      , noDetailTableInfo
      , noDetailHeader
      , noDetailEntry
      , ignoredLine
      ;
    // from what I understand, these process data that is returned THEN saved
    qi::rule<nmdp::IstreamIter, std::string()>
        detailTTL
      , detailPortDescription
      , noDetailPort
      , noDetailDeviceId
      , port
      , noDetailTTL
      , restOfLine
      , inQuotes
      , token
      , csvToken
      ;

    // from what I understand, these process the data that is never saved
    qi::rule<nmdp::IstreamIter>
        noDetailCapability
      , noDetailHoldtime
      , noDetailLocalIface
      ;

  // ===========================================================================
  // Constructors
  // ===========================================================================
  public: // Constructor is only default and must be public
    Parser();

  // ===========================================================================
  // Methods
  // ===========================================================================
  private:
    std::string getDevice(const std::string&);
    void print(const std::string&, const std::string&);
    void printUint(const std::string&, const uint);
    void printInfo();

    void updateIpAddrs();
    void updateInterfaces();
    void updateDeviceInformation();
    void finalizeData();

    std::string infoString;
    // Object return
    Result getData();
};
#endif // PARSER_HPP
