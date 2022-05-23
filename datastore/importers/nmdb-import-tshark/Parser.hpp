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

#include <any>

#include <netmeld/datastore/parsers/ParserDomainName.hpp>
#include <netmeld/datastore/parsers/ParserIpAddress.hpp>
#include <netmeld/datastore/parsers/ParserMacAddress.hpp>

#include "DataContainerSingleton.hpp" // needed for Result

namespace nmdo = netmeld::datastore::objects;
namespace nmdp = netmeld::datastore::parsers;

// =============================================================================
// Data containers
// =============================================================================
typedef std::vector<std::string> VecStrType;
// NOTE: could probably use a std::multimap<>, however the data size is
//       typically few so most likely not worth it
typedef std::tuple<std::string, std::string> TupStrStrType;
typedef std::vector<TupStrStrType> VecTupStrStrType;

// =============================================================================
// Parser definition
// =============================================================================
template<typename Iter>
class Parser :
  public qi::grammar<Iter, Result(), qi::ascii::blank_type>
{
  // ===========================================================================
  // Variables
  // ===========================================================================
  private: // Variables are always private
    // Rules
    qi::symbols<const char, qi::rule<Iter, std::string()>*>
      dataLines;

    qi::rule<Iter, Result(), qi::ascii::blank_type>
      start;

    qi::rule<Iter, qi::ascii::blank_type>
      jsonBlock, jsonEob;

    qi::rule<Iter, qi::ascii::blank_type,
             qi::locals<qi::rule<Iter, std::string()>*, std::string>>
      packetBlock;

    qi::rule<Iter, std::string()>
      value;

    qi::rule<Iter>
      iLine;

    // Data
    const std::string
        ethSrc {R"("eth.src": )"}
      , sllSrc {R"("sll.src.eth": )"}
      , vlanId {R"("vlan.id": )"}
      , stpRootHw {R"("stp.root.hw": )"}
      , stpBridgeHw {R"("stp.bridge.hw": )"}
      , dtpSenderid {R"("dtp.senderid": )"}
      , vtp {R"("vtp": )"}
      , arpSrcHwMac {R"("arp.src.hw_mac": )"}
      , arpSrcProtoIpv4 {R"("arp.src.proto_ipv4": )"}
      , cdpDeviceid {R"("cdp.deviceid": )"}
      , cdpPlatform {R"("cdp.platform": )"}
      , cdpSoftwareVersion {R"("cdp.software_version": )"}
      , cdpNrgyzIpAddress {R"("cdp.nrgyz.ip_address": )"}
      , cdpPortid {R"("cdp.portid": )"}
      , cdpNativeVlan {R"("cdp.native_vlan": )"}
      , lldpChassisIdMac {R"("lldp.chassis.id.mac": )"}
      , lldpTlvSystemName {R"("lldp.tlv.system.name": )"}
      , lldpTlvSystemDesc {R"("lldp.tlv.system.desc": )"}
      , lldpPortId {R"("lldp.port.id": )"}
      , lldpPortDesc {R"("lldp.port.desc": )"}
      , lldpIeee8021PortVlanId {R"("lldp.ieee.802_1.port_vlan.id": )"}
      , ipSrc {R"("ip.src": )"}
      //, ipProto {R"("ip.proto": )"}
      , ipv6Src {R"("ipv6.src": )"}
      , ipv6SrcSaMac {R"("ipv6.src_sa_mac": )"}
      //, ipv6Nxt {R"("ipv6.nxt": )"}
      //, udpSrcPort {R"("udp.srcport": )"}
      , dnsRespName {R"("dns.resp.name": )"}
      , dnsAaaa {R"("dns.aaaa": )"}
      , dnsA {R"("dns.a": )"}
      , dnsCname {R"("dns.cname": )"}
      , dnsNs {R"("dns.ns": )"}
      , dnsSoaMname {R"("dns.soa.mname": )"}
      , dnsSoaRname {R"("dns.soa.rname": )"}
      , bootpOptionSubnetMask {R"("bootp.option.subnet_mask": )"}
      , bootpOptionDomainNameServer {R"("bootp.option.domain_name_server": )"}
      , bootpOptionDhcpServerId {R"("bootp.option.dhcp_server_id": )"}
      , bootpOptionRouter {R"("bootp.option.router": )"}
      , bootpOptionDomainName {R"("bootp.option.domain_name": )"}
      , bootpOptionTftpServerAddress {R"("bootp.option.tftp_server_address": )"}
      , bootpOptionSipServerAddress {R"("bootp.option.sip_server.address": )"}
      , bootpOptionHostname {R"("bootp.option.hostname": )"} // client name
      , bootpIpYour {R"("bootp.ip.your": )"} // client ip
      , bootpIpRelay {R"("bootp.ip.relay": )"} // tricky
      , bootpHwMacAddr {R"("bootp.hw.mac_addr": )"} // client mac
        // what to do with these two, macs and subnets
      , dhcp6IaprefixPrefAddr {R"("dhcp6.iaprefix.pref_addr": )"} // "subnet" for c&s
      , dhcpv6DuidlltLinkLayerAddr {R"("dhcpv6.duidllt.link_layer_addr": )"} // client&server
      , ntpRefid {R"("ntp.refid": )"} // ntp source, bytes, see "not you"
    ;
    const std::map<std::string, std::string> ipProtoNums
    {
      {"6", "tcp"},
      {"17", "udp"}
    };

    const std::string TSHARK_REASON {"from tshark import"};

    typedef std::map<std::string, std::any>  PacketData;
    PacketData pd;

    size_t parsedPacketCount {0};

    std::string tempDnsRespName;


  // ===========================================================================
  // Constructors
  // ===========================================================================
  public: // Constructor is only default and must be public
    Parser();


  // ===========================================================================
  // Methods
  // ===========================================================================
  private:
    void setDone();
    void setNewPacket();
    void setPacketData(const std::string&, const std::string&);
    bool processPacket(PacketData&);

  public:
};

#include "Parser.ipp"
#endif // PARSER_HPP
