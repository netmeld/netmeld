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
      ethSrc {"\"eth.src\": "},
      sllSrc {"\"sll.src.eth\": "},
      vlanId {"\"vlan.id\": "},
      stpRootHw {"\"stp.root.hw\": "},
      stpBridgeHw {"\"stp.bridge.hw\": "},
      dtpSenderid {"\"dtp.senderid\": "},
      vtp {"\"vtp\": "},
      arpSrcHwMac {"\"arp.src.hw_mac\": "},
      arpSrcProtoIpv4 {"\"arp.src.proto_ipv4\": "},
      cdpDeviceid {"\"cdp.deviceid\": "},
      cdpPlatform {"\"cdp.platform\": "},
      cdpSoftwareVersion {"\"cdp.software_version\": "},
      cdpNrgyzIpAddress {"\"cdp.nrgyz.ip_address\": "},
      cdpPortid {"\"cdp.portid\": "},
      cdpNativeVlan {"\"cdp.native_vlan\": "},
      lldpChassisIdMac {"\"lldp.chassis.id.mac\": "},
      lldpTlvSystemName {"\"lldp.tlv.system.name\": "},
      lldpTlvSystemDesc {"\"lldp.tlv.system.desc\": "},
      lldpPortId {"\"lldp.port.id\": "},
      lldpPortDesc {"\"lldp.port.desc\": "},
      lldpIeee8021PortVlanId {"\"lldp.ieee.802_1.port_vlan.id\": "},
      ipSrc {"\"ip.src\": "},
      //ipProto {"\"ip.proto\": "},
      ipv6Src {"\"ipv6.src\": "},
      ipv6SrcSaMac {"\"ipv6.src_sa_mac\": "},
      //ipv6Nxt {"\"ipv6.nxt\": "},
      //udpSrcPort {"\"udp.srcport\": "},
      dnsRespName {"\"dns.resp.name\": "},
      dnsAaaa {"\"dns.aaaa\": "},
      dnsA {"\"dns.a\": "},
      dnsCname {"\"dns.cname\": "},
      dnsNs {"\"dns.ns\": "},
      dnsSoaMname {"\"dns.soa.mname\": "},
      dnsSoaRname {"\"dns.soa.rname\": "},
      bootpOptionSubnetMask {"\"bootp.option.subnet_mask\": "},
      bootpOptionDomainNameServer {"\"bootp.option.domain_name_server\": "},
      bootpOptionDhcpServerId {"\"bootp.option.dhcp_server_id\": "},
      bootpOptionRouter {"\"bootp.option.router\": "},
      bootpOptionDomainName {"\"bootp.option.domain_name\": "},
      bootpOptionTftpServerAddress {"\"bootp.option.tftp_server_address\": "},
      bootpOptionSipServerAddress {"\"bootp.option.sip_server.address\": "},
      bootpOptionHostname {"\"bootp.option.hostname\": "}, // client name
      bootpIpYour {"\"bootp.ip.your\": "}, // client ip
      bootpIpRelay {"\"bootp.ip.relay\": "}, // tricky
      bootpHwMacAddr {"\"bootp.hw.mac_addr\": "}, // client mac
      // what to do with these two, macs and subnets
      dhcp6IaprefixPrefAddr {"\"dhcp6.iaprefix.pref_addr\": "}, // "subnet" for c&s
      dhcpv6DuidlltLinkLayerAddr {"\"dhcpv6.duidllt.link_layer_addr\": "}, // client&server
      ntpRefid {"\"ntp.refid\": "} // ntp source, bytes, see "not you"
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
