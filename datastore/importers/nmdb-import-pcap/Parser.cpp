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

#include <memory>
#include <pcap/pcap.h>

#include <netmeld/core/parsers/ParserHelper.hpp>
#include <netmeld/core/utils/Exit.hpp>

#include "Parser.hpp"

namespace nmcu = netmeld::core::utils;


// =============================================================================
// Parser constructor
// =============================================================================
Parser::Parser()
{}

// =============================================================================
// Parser logic
// =============================================================================
Result
Parser::processFile(const std::string& _filePath)
{
  char pcapErrBuf[PCAP_ERRBUF_SIZE];
  std::shared_ptr<pcap_t> pcapHandle
      {pcap_open_offline(_filePath.c_str(), pcapErrBuf), pcap_close};

  if (nullptr == pcapHandle.get()) {
    LOG_ERROR << "Failed to read pcap file" << std::endl;
    std::exit(nmcu::Exit::FAILURE);
  }

  processPackets(pcapHandle);

  Result r;
  r.push_back(d);

  return r;
}

void
Parser::processPackets(std::shared_ptr<pcap_t>& _handle)
{
  pcap_pkthdr* packetHeader = nullptr;
  uint8_t const* packetData = nullptr;

  //while (1 == pcap_next_ex(_handle.get(), &packetHeader, &packetData)) {
  for (int retVal = pcap_next_ex(_handle.get(), &packetHeader, &packetData);
       -2 != retVal;
       retVal = pcap_next_ex(_handle.get(), &packetHeader, &packetData)) {
    if (-1 == retVal) {
      LOG_DEBUG << pcap_geterr(_handle.get()) << '\n';
      continue;
    }
    if (0 == retVal) {
      continue;
    }

    bool done {false}; // for packet process looping
    packetSizeLeft = (packetHeader->caplen); // caplen <= len
    offset = packetData;

    // See https://www.tcpdump.org/linktypes.html
    int linkType {pcap_datalink(_handle.get())};
    uint16_t payloadType;
    if (1 == linkType) {
      const auto* eh {reinterpret_cast<EthernetHeader const*>(packetData)};
      if (processEthernetHeader(eh)) { // true if no further processing needed
        continue;
      }
      if (!isOffsetOk<EthernetHeader>()) { continue; };
      payloadType = ph.getPayloadProtocol(eh);
    } else if (113 == linkType) {
      const auto* lch {reinterpret_cast<LinuxCookedHeader const*>(packetData)};
      if (processLinuxCookedHeader(lch)) { // true if no further processing needed
        continue;
      }
      if (!isOffsetOk<LinuxCookedHeader>()) { continue; };
      payloadType = ph.getPayloadProtocol(lch);
    } else {
      LOG_DEBUG << "Unknown link type (" << linkType << "), skipping\n";
      continue;
    }

    /********** Payload Processing Notes **********
       - VLAN packets
         - VLAN adds another layer to the packet structure so we have to
           unwrap that and attempt another process pass; since we can have
           multiple VLAN wrappings, it needs to be nested
       - IPvX packets
         - Cannot associate an IP to a MAC as a router will substitute it's
           MAC for the IPs it routes to and from
    **********************************************/
    while (!done) {
      switch (payloadType) {
        case 0x8100: // 802.1Q VLAN tag
          {
            LOG_DEBUG << "Packet type: VLAN tag\n";
            const auto* vh {reinterpret_cast<VlanHeader const*>(offset)};
            processVlanHeader(vh);

            // Update and reset payload processing to handle VLAN payload
            payloadType = ph.getPayloadProtocol(vh);
            if (isOffsetOk<VlanHeader>()) {
              done = false;
            } else {
              done = true;
            }

            break;
          }
        case 0x0806: // ARP
          {
            LOG_DEBUG << "Packet type: ARP\n";
            processArpHeader(reinterpret_cast<ArpHeader const*>(offset));
            done = true;
            break;
          }
        case 0x0800: // IPv4
          {
            LOG_DEBUG << "Packet type: IPv4\n";
            processIpv4Header(reinterpret_cast<Ipv4Header const*>(offset));
            done = true;
            break;
          }
        case 0x86DD: // IPv6
          {
            LOG_DEBUG << "Packet type: IPv6\n";
            processIpv6Header(reinterpret_cast<Ipv6Header const*>(offset));
            done = true;
            break;
          }
        default:
          {
            // EtherType values must be >= 1536
            // 1500 <= are payload size values (MTU)
            // 1501-1535 are undefined
            if (1536 <= payloadType) {
              LOG_DEBUG << "Packet type: UNK -- "
                        << "dec: " << payloadType
                        << ", hex: 0x"
                        << std::hex << payloadType << std::dec
                        << std::endl;
            }
            done = true;
            break;
          }
      } // end of switch
    } // end of while
  } // end of while
}

// =============================================================================
// Parser helper methods
// =============================================================================
bool
Parser::isOffsetOk(size_t _size)
{
  if (packetSizeLeft >= _size) {
    offset += _size;
    packetSizeLeft -= _size;
    return true; // offset still under max packet size
  } else {
    LOG_DEBUG << "Offset larger than max packet size" << std::endl;
    return false;
  }
}
nmco::IpAddress&
Parser::getIpAddrLoc(const nmco::IpAddress& _ipAddr)
{
  const auto& ipAddrStr {_ipAddr.toString()};
  if (!d.ipAddrs.count(ipAddrStr)) {
    d.ipAddrs[ipAddrStr] = _ipAddr;
  }
  return (d.ipAddrs[ipAddrStr]);
}

bool
Parser::processEthernetHeader(const EthernetHeader* _eh)
{
  bool skip {false};

  auto srcMacAddr {ph.getSrcMacAddr(_eh)};
  d.macAddrs[srcMacAddr] = srcMacAddr;
  d.macAddrs[srcMacAddr].setResponding(true);

  auto dstMacAddr {ph.getDstMacAddr(_eh)};
  const auto& testVal {dstMacAddr.toString()};
  auto payloadType {ph.getPayloadProtocol(_eh)};

  if (stpMacs.count(testVal)) {
    std::ostringstream oss;
    oss << "Probable STP from MAC: " << srcMacAddr.toString();
    addObservation(oss.str());
    skip = true;
  }

  if (discProto.count(testVal) &&
      // this mac and size means it was STP, not CDP
      (0x26 != payloadType && "01:80:c2:00:00:00" != testVal)) {

    processDiscProtoPacket(_eh);

    std::ostringstream oss;
    oss << "Probable {C|LL}DP from MAC: " << srcMacAddr.toString();
    addObservation(oss.str());
    skip = true;
  }

  return skip;
}

bool
Parser::processLinuxCookedHeader(const LinuxCookedHeader* _lch)
{
  bool skip {false};

  auto srcMacAddr {ph.getSrcMacAddr(_lch)};
  d.macAddrs[srcMacAddr] = srcMacAddr;
  d.macAddrs[srcMacAddr].setResponding(true);

  return skip;
}

void
Parser::processDiscProtoPacket(const EthernetHeader*)
{
  // TODO 17FEB19 We can get a lot of data from a CDP/LLDP packets
  //      https://wiki.wireshark.org/LinkLayerDiscoveryProtocol
  //      https://wiki.wireshark.org/CDP
}

void
Parser::processVlanHeader(const VlanHeader* _vh)
{
  auto vlan {ph.getVlan(_vh)};
  vlan.setDescription(PCAP_REASON);
  d.vlans[vlan] = vlan;
}

void
Parser::processArpHeader(const ArpHeader* _ah)
{
  auto macAddr {ph.getSrcMacAddr(_ah)};
  d.macAddrs[macAddr] = macAddr;

  auto ipAddr {ph.getSrcIpAddr(_ah)};
  ipAddr.setReason(PCAP_REASON);

  d.macAddrs[macAddr].addIp(ipAddr);
  LOG_DEBUG << macAddr << "--" << ipAddr << std::endl;
}

// TODO Look into adding more IPv4/6 processing (only if useful)
//      - DNS....somewhat done, maybe add more types/notifications
//      - NetBIOS (WPAD) - very similar to DNS, so should be fairly easy
//      - CAPWAP (maybe)
//      - BOOTP/DHCPv6
//      - SMB
//      - SNMP
//      - SMTP
void
Parser::processIpv4Header(const Ipv4Header* _iph)
{
  auto ipAddr {ph.getSrcIpAddr(_iph)};
  auto& ipAddrEntry {getIpAddrLoc(ipAddr)};
  ipAddrEntry.setResponding(true);
  ipAddrEntry.setReason(PCAP_REASON);

  auto protoId {ph.getPayloadProtocol(_iph)};
  if (!isOffsetOk<Ipv4Header>()) { return; }

  // https://en.wikipedia.org/wiki/List_of_IP_protocol_numbers
  switch (protoId) {
    case 0x06: // TCP
      {
        processTcpHeader(reinterpret_cast<TcpHeader const*>(offset));
        break;
      }
    case 0x11: // UDP
      {
        processUdpHeader(reinterpret_cast<UdpHeader const*>(offset));
        break;
      }
    case 0x01: // ICMP
    case 0x02: // IGMP
    case 0x58: // EIGRP
    case 0x67: // PIM
    case 0x70: // VRRP
      { break; } // Known and ignored protocols
    default:  // Flag unknown/unevaluated protocols
      {
        LOG_DEBUG << "Protocol unhandled: "
                  << std::hex << std::showbase << static_cast<int>(protoId)
                  << std::dec << std::endl;
        break;
      }
  }
}

void
Parser::processIpv6Header(const Ipv6Header* _iph)
{
  auto ipAddr {ph.getSrcIpAddr(_iph)};
  auto& ipAddrEntry {getIpAddrLoc(ipAddr)};
  ipAddrEntry.setResponding(true);
  ipAddrEntry.setReason(PCAP_REASON);

  //LOG_DEBUG << "payloadProtocol: " << ntohs(_iph->payloadProtocol) << std::endl;
}

void
Parser::processUdpHeader(const UdpHeader* _up)
{
  auto length {ph.getLength(_up)};
  if (!isOffsetOk<UdpHeader>()) { return; }

  // Skip IPv6 jumbograms (0), UDP header only (8), or malformed (1-7)
  if (8 >= length) { return; }

  auto srcPort {ph.getSrcPort(_up)};
  auto dstPort {ph.getDstPort(_up)};
  if (53 == srcPort) { // dns
    // TODO add as known network service if successful
//    LOG_DEBUG << "Parser::processUdpHeader" << std::endl;
    processDnsHeader(reinterpret_cast<DnsHeader const*>(offset));
  }
  else if (67 == srcPort && 68 == dstPort) { // dhcp
    // TODO add as known network service if successful
    processDhcpHeader(reinterpret_cast<DhcpHeader const*>(offset));
  }
  else if (137 == dstPort) { // netbios-ns
  }
  else if (138 == dstPort) { // netbios-dgm
  }
  else if (139 == dstPort) { // netbios-ssn
  }
  else if (445 == dstPort) { // microsoft-ds
  }
}

void
Parser::processTcpHeader(const TcpHeader* _tp)
{
  auto length {ph.getLength(_tp)};
  if (!isOffsetOk<TcpHeader>()) { return; }

  // min/max 20/60 bytes, ignore others
  if (20 > length || 60 < length) { return; }

  auto srcPort {ph.getSrcPort(_tp)};
  //auto dstPort {ph.getDstPort(_tp)};
  if (53 == srcPort) { // dns
    // TODO add as known network service if successful
    offset += 2U; // ignore 2 byte length field, potentially check for bounding
//    LOG_DEBUG << "Parser::processTcpHeader" << std::endl;
    processDnsHeader(reinterpret_cast<DnsHeader const*>(offset));
  }
}

void
Parser::processDnsHeader(const DnsHeader* _dh)
{
  auto dnsPacketStart {offset};
  if (!isOffsetOk<DnsHeader>()) { return; }
  if (!ph.isResponse(_dh)) { return; } // only want responses

  // jump past the queries
  auto queryOffset {ph.getQueryOffset(dnsPacketStart, _dh, offset)};
//  LOG_DEBUG << "Parser::processDnsHeader::queryOffset: " << queryOffset << std::endl;
  if (!isOffsetOk(queryOffset)) { return; }

  // process answers
  auto ips {ph.getDnsARecords(dnsPacketStart, _dh, offset)};
  for (auto& ip : ips) {
    nmco::IpAddress ipAddr(std::get<0>(ip));
    auto ipAddrStr {ipAddr.toString()};
    if (!d.ipAddrs.count(ipAddrStr)) {
      d.ipAddrs[ipAddrStr] = ipAddr;
    }
    d.ipAddrs[ipAddrStr].addAlias(std::get<1>(ip), PCAP_REASON);
    LOG_DEBUG << "Added: " << d.ipAddrs[ipAddrStr] << std::endl;
  }
}

void
Parser::processDhcpHeader(const DhcpHeader* /*_dh*/)
{
  if (!isOffsetOk<DhcpHeader>()) { return; }

}


void
Parser::addObservation(const std::string& _val)
{
  d.observations.addNotable(_val);
}
