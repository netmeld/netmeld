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

#include <netmeld/datastore/objects/AcRule.hpp>
#include <netmeld/datastore/objects/DeviceInformation.hpp>
#include <netmeld/datastore/objects/InterfaceNetwork.hpp>
#include <netmeld/datastore/objects/Route.hpp>
#include <netmeld/datastore/objects/Service.hpp>
#include <netmeld/datastore/objects/ToolObservations.hpp>
#include <netmeld/datastore/objects/Vlan.hpp>
#include <netmeld/datastore/parsers/ParserDomainName.hpp>
#include <netmeld/datastore/parsers/ParserIpAddress.hpp>
#include <netmeld/datastore/parsers/ParserMacAddress.hpp>

namespace nmdo = netmeld::datastore::objects;
namespace nmdp = netmeld::datastore::parsers;

typedef nmdo::DeviceInformation DevInfo;

struct PowerSupplySlotEntry {
  int slot;
  std::string model;
  std::string serialNumber;
};

struct FanModuleEntry {
  int moduleNumber;
  int numberOfFans;
  std::string model;
  std::string serialNumber;
};

struct PortsEntry {
  std::string type;
  int count;
};

struct TransceiverSlotEntry {
  int port;
  std::string manufacturer;

};

struct StorageDeviceEntry {
  std::string mount;
  std::string type;
  std::string model;
  std::string serialNumber;
  std::string rev;
  int sizeGB;
    };

struct ParserOutput {
  DevInfo devInfo;
  std::vector<PowerSupplySlotEntry> powerSupplySlots;
  std::vector<FanModuleEntry> fanModules;
  std::vector<PortsEntry> ports;
  std::vector<TransceiverSlotEntry> transceiverSlots;
  std::vector<StorageDeviceEntry> storageDevices;

  std::strong_ordering operator<=>(const ParserOutput&) const;
  bool operator==(const ParserOutput&) const;
};

typedef std::vector<ParserOutput> Result;
typedef std::vector<std::unordered_map<std::string, std::string>> KeyValVec;

class Parser:
  public qi::grammar<nmdp::IstreamIter, Result(), qi::ascii::blank_type>
{
  private:
    std::string VENDOR = "Arista";

    KeyValVec 
      systemInformationEntry,
      powerSupplyEntries,
      fanModuleEntries,
      portEntries,
      transceiverSlotEntries,
      storageDeviceEntries
    ;


    

  public:

    Parser();

    qi::rule<nmdp::IstreamIter, Result(), qi::ascii::blank_type>
      start;

    qi::rule<nmdp::IstreamIter, ParserOutput(), qi::ascii::blank_type>
      deviceInfo;

    qi::rule<nmdp::IstreamIter, DevInfo(), qi::ascii::blank_type>
      systemInformationStart
    ;

    qi::rule<nmdp::IstreamIter, qi::ascii::blank_type>
      systemInformation
    ;

    qi::rule<nmdp::IstreamIter, KeyValVec(), qi::ascii::blank_type>
      systemInformationEntryRule
    ;

    qi::rule<nmdp::IstreamIter, qi::ascii::blank_type>
      systemPowerSupplyStart, systemPowerSupply;

    qi::rule<nmdp::IstreamIter, PowerSupplySlotEntry(), qi::ascii::blank_type>
      systemPowerSupplyEntryRule;

    qi::rule<nmdp::IstreamIter, qi::ascii::blank_type>
      systemFanModuleStart, systemFanModule;

    qi::rule<nmdp::IstreamIter, FanModuleEntry(), qi::ascii::blank_type>
      systemFanModuleEntryRule;

    qi::rule<nmdp::IstreamIter, qi::ascii::blank_type>
      systemPortStart, systemPort;

    qi::rule<nmdp::IstreamIter, PortsEntry(), qi::ascii::blank_type>
      systemPortEntryRule;

    qi::rule<nmdp::IstreamIter, qi::ascii::blank_type>
      systemTransceiverStart, systemTransceiver;

    qi::rule<nmdp::IstreamIter, TransceiverSlotEntry(), qi::ascii::blank_type>
      systemTransceiverEntryRule;

    qi::rule<nmdp::IstreamIter, qi::ascii::blank_type>
      systemStorageStart, systemStorage;

    qi::rule<nmdp::IstreamIter, StorageDeviceEntry(), qi::ascii::blank_type>
      systemStorageEntryRule;

    qi::rule<nmdp::IstreamIter, std::string()>
      grabLine
    ;

    qi::rule
    <
            nmdp::IstreamIter,
            std::tuple<std::string, std::vector<int>, std::vector<std::string>>,
            qi::ascii::blank_type
    >
      entryRuleBase
    ;

    qi::rule<nmdp::IstreamIter, int(), qi::locals<unsigned int>>
      countDashes
    ;

    qi::rule<nmdp::IstreamIter, std::string()>
      token;

    qi::rule<nmdp::IstreamIter, qi::ascii::blank_type>
      ignoredLine;
};