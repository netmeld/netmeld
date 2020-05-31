// =============================================================================
// Copyright 2017 National Technology & Engineering Solutions of Sandia, LLC
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

#include <memory>
#include <pcap/pcap.h>

#include <netmeld/datastore/objects/IpAddress.hpp>
#include <netmeld/datastore/objects/MacAddress.hpp>
#include <netmeld/datastore/objects/ToolObservations.hpp>
#include <netmeld/datastore/objects/Vlan.hpp>
#include <netmeld/datastore/parsers/ParserHelper.hpp>

#include "PacketHelper.hpp"

namespace nmdo = netmeld::datastore::objects;

// The following map is using the same type of object for the keys and values
// This is an optimization due to how this data is extracted from packets, it
// the key is a raw piece of data extracted (e.g. a MacAddress with only the
// address member set). The value is the object that is modified and eventually
// saved. This allows us to grab the objects we're modifying easily without
// needing to cast/convert data around to get a more "normal" key. We cannot
// use the object being modifed (e.g. in a set) as the key because our compare
// operator does a deep compare and would not work with the raw objects.
struct Data {
  std::map<nmdo::Vlan, nmdo::Vlan>              vlans;

  std::map<nmdo::MacAddress, nmdo::MacAddress>  macAddrs;
  std::map<std::string, nmdo::IpAddress>    ipAddrs;

  nmdo::ToolObservations observations;
};

typedef std::vector<Data>  Result;


// =============================================================================
// Parser definition
// =============================================================================
class Parser
{
  // ===========================================================================
  // Variables
  // ===========================================================================
  private:
    const std::string PCAP_REASON {"from pcap import"};

    const std::set<std::string> stpMacs {
      "01:00:0c:cc:cc:cd",
      "01:80:c2:00:00:00", "01:80:c2:00:00:08"
    };
    const std::set<std::string> discProto {
      "01:00:0c:cc:cc:cc",
      "01:80:c2:00:00:00", "01:80:c2:00:00:03", "01:80:c2:00:00:0e"
    };

    PacketHelper ph;
    Data d;

    const uint8_t* offset {nullptr};
    size_t packetSizeLeft {0};

  // ===========================================================================
  // Constructors
  // ===========================================================================
  public: // Constructor is only default and must be public
    Parser();

  // ===========================================================================
  // Methods
  // ===========================================================================
  private:
    void processPackets(std::shared_ptr<pcap_t>&);

    bool isOffsetOk(size_t);
    template<typename T>
    bool isOffsetOk()
    {
      size_t size {sizeof(T)};
      return isOffsetOk(size);
    }
    nmdo::IpAddress& getIpAddrLoc(const nmdo::IpAddress&);

    bool processEthernetHeader(const EthernetHeader*);
    void processDiscProtoPacket(const EthernetHeader*);

    bool processLinuxCookedHeader(const LinuxCookedHeader*);

    void processVlanHeader(const VlanHeader*);

    void processArpHeader(const ArpHeader*);

    void processIpv4Header(const Ipv4Header*);
    void processIpv6Header(const Ipv6Header*);

    void processUdpHeader(const UdpHeader*);
    void processTcpHeader(const TcpHeader*);

    void processDnsHeader(const DnsHeader*);
    void processDhcpHeader(const DhcpHeader*);

    void addObservation(const std::string&);

  protected:
  public:
    Result processFile(const std::string&);
};
#endif // PARSER_HPP
