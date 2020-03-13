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

#include "PacketHelper.hpp"


// ===========================================================================
// Constructors
// ===========================================================================
PacketHelper::PacketHelper()
{}

// ===========================================================================
// Methods
// ===========================================================================
uint16_t
PacketHelper::asUint16_t(const uint8_t* _memLoc)
{
  return ntohs(*(reinterpret_cast<const uint16_t*>(_memLoc)));
}


// Ethernet
uint16_t
PacketHelper::getPayloadProtocol(const EthernetHeader* ph)
{
  return ntohs(ph->payloadProtocol);
}
nmco::MacAddress
PacketHelper::getSrcMacAddr(const EthernetHeader* ph)
{
  std::vector<uint8_t> macAddr(std::begin(ph->srcMacAddr),
                               std::end(ph->srcMacAddr));
  return nmco::MacAddress(macAddr);
}
nmco::MacAddress
PacketHelper::getDstMacAddr(const EthernetHeader* ph)
{
  std::vector<uint8_t> macAddr(std::begin(ph->dstMacAddr),
                               std::end(ph->dstMacAddr));
  return nmco::MacAddress(macAddr);
}


// LinuxCooked
uint16_t
PacketHelper::getPayloadProtocol(const LinuxCookedHeader* ph)
{
  return ntohs(ph->payloadProtocol);
}
nmco::MacAddress
PacketHelper::getSrcMacAddr(const LinuxCookedHeader* ph)
{
  std::vector<uint8_t> macAddr(std::begin(ph->srcMacAddr),
                               std::end(ph->srcMacAddr));
  macAddr.resize(ntohs(ph->llAddrLength));
  return nmco::MacAddress(macAddr);
}


// VLAN
uint16_t
PacketHelper::getPayloadProtocol(const VlanHeader* ph)
{
  return ntohs(ph->payloadProtocol);
}
nmco::Vlan
PacketHelper::getVlan(const VlanHeader* ph)
{
  auto vlanId = ntohs(ph->tagControl) & 0x0FFF;
  return nmco::Vlan(vlanId);
}


// ARP
nmco::MacAddress
PacketHelper::getSrcMacAddr(const ArpHeader* ph)
{
  std::vector<uint8_t> macAddr(std::begin(ph->srcMacAddr),
                               std::end(ph->srcMacAddr));
  return nmco::MacAddress(macAddr);
}
nmco::IpAddress
PacketHelper::getSrcIpAddr(const ArpHeader* ph)
{
  std::vector<uint8_t> ipAddr(std::begin(ph->srcIpAddr),
                              std::end(ph->srcIpAddr));
  return nmco::IpAddress(ipAddr);
}


// IP
uint8_t
PacketHelper::getPayloadProtocol(const Ipv4Header* ph)
{
  return (ntohs(ph->payloadProtocol) >> 8);
}
uint8_t
PacketHelper::getPayloadProtocol(const Ipv6Header* ph)
{
  return (ntohs(ph->payloadProtocol) >> 8);
}
nmco::IpAddress
PacketHelper::getSrcIpAddr(const Ipv4Header* ph)
{
  std::vector<uint8_t> ipAddr(std::begin(ph->srcIpAddr),
                              std::end(ph->srcIpAddr));
  return nmco::IpAddress(ipAddr);
}

nmco::IpAddress
PacketHelper::getSrcIpAddr(const Ipv6Header* ph)
{
  std::vector<uint8_t> ipAddr(std::begin(ph->srcIpAddr),
                              std::end(ph->srcIpAddr));
  return nmco::IpAddress(ipAddr);
}


// UDP
uint16_t
PacketHelper::getSrcPort(const UdpHeader* _uh)
{
  return ntohs(_uh->srcPort);
}
uint16_t
PacketHelper::getDstPort(const UdpHeader* _uh)
{
  return ntohs(_uh->dstPort);
}
uint16_t
PacketHelper::getLength(const UdpHeader* _uh)
{
  return ntohs(_uh->length);
}


// TCP
uint16_t
PacketHelper::getSrcPort(const TcpHeader* _th)
{
  return ntohs(_th->srcPort);
}
uint16_t
PacketHelper::getDstPort(const TcpHeader* _th)
{
  return ntohs(_th->dstPort);
}
uint8_t
PacketHelper::getLength(const TcpHeader* _th)
{
  uint8_t length = ((_th->flags)>>4)&0x0F;
  return (length*4);;
}


// DNS
bool
PacketHelper::isResponse(const DnsHeader* _dh)
{
  // first bit of flags is query/response (0/1)
  return (0x8000 == (ntohs(_dh->flags) & 0xF000));
}
size_t
PacketHelper::getQueryOffset(const uint8_t* _dnsPacketStart,
                             const DnsHeader* _dh,
                             const uint8_t* _curLoc)
{
  size_t offset {0};

  for (auto i {ntohs(_dh->countQuestions)}; 0 < i; i--) {
    // Query name
    std::ostringstream oss;
    offset += getQueryName(_dnsPacketStart, _curLoc, oss);
    // Query type
    offset += 2U;
    // NOTE: we may want to do something with this (e.g. id zone transfer)
    // Query class
    offset += 2U;
  }

  return offset;
}
size_t
PacketHelper::getQueryName(const uint8_t* _dnsPacketStart, 
                           const uint8_t* _curLoc,
                           std::ostringstream& _oss)
{
  size_t offset  {0};
  uint8_t length {*_curLoc};
  while (0x00 != length) {
    if (0xC0 == (length & 0xF0)) { // it's a pointer
      auto nameOffset = (asUint16_t(_curLoc + offset) & 0x0FFF);
      offset += 
        getQueryName(_dnsPacketStart, {_dnsPacketStart + nameOffset}, _oss);
      length = 0x00;
    } else { // it's a value
      for (size_t i {1}; i <= length; i++) {
        _oss << static_cast<uint8_t>(*(_curLoc + offset + i));
      }
      _oss << ".";
      offset += (length + 1U);

      length = (*(_curLoc + offset));
    }
//    LOG_DEBUG << "PacketHelper::getQueryName::_oss: " << _oss.str() << std::endl;
    // Sanity check, 253 is max character count, so something is wrong
    if (253 < offset) {
      LOG_DEBUG << "Read longer than allowed (253): " << offset << std::endl;
      return SIZE_MAX;
    }
  }
  return (offset+1U);
}
std::vector<std::tuple<std::vector<uint8_t>, std::string>>
PacketHelper::getDnsARecords(const uint8_t* _dnsPacketStart,
                             const DnsHeader* _dh,
                             const uint8_t* _curLoc)
{
  std::vector<std::tuple<std::vector<uint8_t>, std::string>>
    data;

  size_t offset {0};
  for (auto i {ntohs(_dh->countAnswers)}; 0 < i; i--) {
    // name - var, first two bits are 11 then offset from start of DnsHeader
    std::ostringstream oss;
    uint8_t ptrVal {*(_curLoc + offset)};
    const uint8_t* nameOffset;
    if ((0xC0 == (ptrVal & 0xF0))) { // just offset
      nameOffset = (_dnsPacketStart + (asUint16_t(_curLoc + offset) & 0x0FFF));
      offset += 2U;
    } else { // prefix + offset
      nameOffset = (_curLoc + offset);
      while ((0xC0 != (ptrVal & 0xF0))) {
          offset += 1U;
          ptrVal = (*(_curLoc + offset));
      }
      offset += 2U;
    }

    // type - A (1) and AAAA (28)...but also NS (2), MX (15), etc...
    uint16_t rType {asUint16_t(_curLoc + offset)};
//    LOG_DEBUG << "PacketHelper::getDnsARecords::rType: "  << (int)rType << std::endl;
    offset += 2U;

    // class - for sure IN (1)...but what are others?
    uint16_t rClass {asUint16_t(_curLoc + offset)};
//    LOG_DEBUG << "PacketHelper::getDnsARecords::rClass: " << (int)rClass << std::endl;
    offset += 2U;

    // ttl - don't care
    offset += 4U;

    // rdlength - uint16_t, length of rddata field
    uint16_t rdLength {asUint16_t(_curLoc + offset)};
//    LOG_DEBUG << "PacketHelper::getDnsARecords::rdLength: " << (int)rdLength << std::endl;
    offset += 2U;

    // rddata - var, devinded by rdlength, ipv4 (4) and ipv6 (16)
    if (   (1 == rType || 28 == rType)
        && 1 == rClass)
    {
      // get name
      getQueryName(_dnsPacketStart, nameOffset, oss);

      std::vector<uint8_t> ipVec;
      for (size_t octet {0}; octet < rdLength; octet++) {
        ipVec.push_back(*(_curLoc + offset + octet));
      }

      // put it together, only for what we care about
      auto dataTuple {std::make_tuple(ipVec, oss.str())};
      data.push_back(dataTuple);
    }
    offset += rdLength;
  }

  return data;
}
