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

#ifndef PACKET_HELPER
#define PACKET_HELPER

#include <netmeld/datastore/objects/IpAddress.hpp>
#include <netmeld/datastore/objects/MacAddress.hpp>
#include <netmeld/datastore/objects/Vlan.hpp>

namespace nmdo = netmeld::datastore::objects;


//---------------------------------------------------------------------------
// Packet Headers
//---------------------------------------------------------------------------

struct EthernetHeader
{
  uint8_t  dstMacAddr[6];
  uint8_t  srcMacAddr[6];
  uint16_t payloadProtocol;
};

struct LinuxCookedHeader
{ // https://www.tcpdump.org/linktypes/LINKTYPE_LINUX_SLL.html
  uint16_t packetType;
  uint16_t llAddrType;
  uint16_t llAddrLength;
  uint8_t srcMacAddr[8];
  uint16_t payloadProtocol;
};

struct VlanHeader
{
  uint16_t tagControl; // control 1 byte, VLAN ID 3 bytes
  uint16_t payloadProtocol;
};

struct ArpHeader
{
  uint16_t hardwareType;
  uint16_t protocolType;
  uint8_t hardwareSize;
  uint8_t protocolSize;
  uint16_t opcode;
  uint8_t srcMacAddr[6];
  uint8_t srcIpAddr[4];
  uint8_t dstMacAddr[6];
  uint8_t dstIpAddr[4];
};

struct Ipv4Header
{
  uint8_t  version;
  uint8_t  typeOfService;
  uint16_t length;
  uint16_t id;
  uint16_t fragmentOffset;
  uint8_t  hopLimit;
  uint8_t  payloadProtocol;
  uint16_t checksum;
  uint8_t  srcIpAddr[4];
  uint8_t  dstIpAddr[4];
};

struct Ipv6Header
{
  uint8_t  version;
  uint8_t  trafficClass;
  uint16_t flowLabel;
  uint16_t payloadLength;
  uint8_t  payloadProtocol;
  uint8_t  hopLimit;
  uint8_t  srcIpAddr[16];
  uint8_t  dstIpAddr[16];
};

struct UdpHeader
{
  uint16_t srcPort;
  uint16_t dstPort;
  uint16_t length; // size 0 means packet is greater than 65535 bytes
  uint16_t checksum;
};

struct TcpHeader
{
  uint16_t srcPort;
  uint16_t dstPort;
  uint32_t sequenceNumber;
  uint32_t acknowledgementNumber;
  uint16_t flags; // data offset (4), reserved (3), flags (9)
  uint16_t windowSize;
  uint16_t checksum;
  uint16_t urgentPointer;
  // options (var, 0-320 bits, divisible by 32, length based on data offset)
};

struct DnsHeader
{
  uint16_t id;
  uint16_t flags;
  uint16_t countQuestions;
  uint16_t countAnswers;
  uint16_t countAuthorityRecords;
  uint16_t countAdditionalRecords;
};

struct DhcpHeader
{
  uint8_t  messageType;
  uint8_t  hardwareType;
  uint8_t  hardwareLength;
  uint8_t  hopLimit;
  uint32_t transactionId;
  uint16_t secondsElapsed;
  uint16_t flags;
  uint8_t  clientIpAddr[4];
  uint8_t  yourIpAddr[4];
  uint8_t  serverIpAddr[4];
  uint8_t  gatewayIpAddr[4];
  uint8_t  clientMacAddr[6];
  uint8_t  clientPadding[10];
};


//---------------------------------------------------------------------------
// General Helper
//---------------------------------------------------------------------------
class PacketHelper {
  // =========================================================================
  // Variables
  // =========================================================================
  private: // Variables will probably rarely appear at this scope
  protected: // Variables intended for internal/subclass API
  public: // Variables should rarely appear at this scope


  // =========================================================================
  // Constructors
  // =========================================================================
  private: // Constructors which should be hidden from API users
  protected: // Constructors part of subclass API
  public: // Constructors part of public API
    PacketHelper();

  // =========================================================================
  // Methods
  // =========================================================================
  private: // Methods which should be hidden from API users
    uint16_t asUint16_t(const uint8_t*);

  protected: // Methods part of subclass API
  public: // Methods part of public API
    // Ethernet
    uint16_t getPayloadProtocol(const EthernetHeader*);
    nmdo::MacAddress getSrcMacAddr(const EthernetHeader*);
    nmdo::MacAddress getDstMacAddr(const EthernetHeader*);

    // LinuxCooked
    uint16_t getPayloadProtocol(const LinuxCookedHeader*);
    nmdo::MacAddress getSrcMacAddr(const LinuxCookedHeader*);

    // VLAN
    uint16_t getPayloadProtocol(const VlanHeader*);
    nmdo::Vlan getVlan(const VlanHeader*);

    // ARP
    nmdo::MacAddress getSrcMacAddr(const ArpHeader*);
    nmdo::IpAddress getSrcIpAddr(const ArpHeader*);

    // IP
    uint8_t getPayloadProtocol(const Ipv4Header*);
    nmdo::IpAddress getSrcIpAddr(const Ipv4Header*);
    uint8_t getPayloadProtocol(const Ipv6Header*);
    nmdo::IpAddress getSrcIpAddr(const Ipv6Header*);

    // UDP
    uint16_t getSrcPort(const UdpHeader*);
    uint16_t getDstPort(const UdpHeader*);
    size_t getLength(const UdpHeader*);

    // TCP
    uint16_t getSrcPort(const TcpHeader*);
    uint16_t getDstPort(const TcpHeader*);
    size_t getLength(const TcpHeader*);

    // DNS
    bool isResponse(const DnsHeader*);
    size_t getQueryOffset(
        const uint8_t*, const DnsHeader*, const uint8_t*);
    std::vector<std::tuple<std::vector<uint8_t>, std::string>>
      getDnsARecords(const uint8_t*, const DnsHeader*, const uint8_t*);
    size_t getQueryName(
        const uint8_t*, const uint8_t*, std::ostringstream&);
};
#endif // PACKET_HELPER
