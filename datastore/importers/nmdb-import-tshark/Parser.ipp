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

#include "DataContainerSingleton.hpp"

// =============================================================================
// Parser logic
// =============================================================================
template<typename Iter>
Parser<Iter>::Parser() : Parser::base_type(start)
{
  // snmp, wpad, netbios, etc
  dataLines.add
    (ethSrc, &value)
    (sllSrc, &value)
    (vlanId, &value)
    (stpRootHw, &value)
    (stpBridgeHw, &value)
    (dtpSenderid, &value)
    (vtp, &value)
    (arpSrcHwMac, &value)
    (arpSrcProtoIpv4, &value)
    (cdpDeviceid, &value)
    (cdpPlatform, &value)
    (cdpSoftwareVersion, &value)
    (cdpNrgyzIpAddress, &value)
    (cdpPortid, &value)
    (cdpNativeVlan, &value)
    (lldpChassisIdMac, &value)
    (lldpTlvSystemName, &value)
    (lldpTlvSystemDesc, &value)
    (lldpPortId, &value)
    (lldpPortDesc, &value)
    (lldpIeee8021PortVlanId, &value)
    (ipSrc, &value)
    (ipv6Src, &value)
    (ipv6SrcSaMac, &value)
    (dnsRespName, &value)
    (dnsAaaa, &value)
    (dnsA, &value)
    (dnsCname, &value)
    (dnsNs, &value)
    (dnsSoaMname, &value)
    (dnsSoaRname, &value)
    (bootpOptionSubnetMask, &value)
    (bootpOptionDomainNameServer, &value)
    (bootpOptionDhcpServerId, &value)
    (bootpOptionRouter, &value)
    (bootpOptionDomainName, &value)
    (bootpOptionTftpServerAddress, &value)
    (bootpOptionSipServerAddress, &value)
    (bootpOptionHostname, &value) // client name
    (bootpIpYour, &value) // client ip
    (bootpIpRelay, &value) // tricky
    (bootpHwMacAddr, &value) // client mac
    (dhcp6IaprefixPrefAddr, &value) // "subnet" for c&s
    (dhcpv6DuidlltLinkLayerAddr, &value) // client&server
    (ntpRefid, &value) // ntp source, bytes, see "not you"
    ;


  start =
    //+((+(qi::char_ - qi::eol) > qi::eol) | qi::eol)
    qi::eps [qi::_val = pnx::construct<Result>()] >
    jsonBlock [pnx::bind(&Parser::setDone, this)]
    ;
/*  multitime -q -n 10 nmdb-import-tshark tshark1.json
--blind slurp
              Mean        Std.Dev.    Min         Median      Max
real        0.579       0.010       0.568       0.578       0.600
--json,... (no skipper)
             Mean        Std.Dev.    Min         Median      Max
real        10.858      0.111       10.716      10.849      11.083
--json,packet,source,layers,frame
            Mean        Std.Dev.    Min         Median      Max
real        16.359      0.164       16.126      16.359      16.766
--json,... (blank skipper)
            Mean        Std.Dev.    Min         Median      Max
real        8.437       0.043       8.375       8.441       8.506
--json,... (symbol table)
            Mean        Std.Dev.    Min         Median      Max
real        7.514       0.055       7.447       7.500       7.607
--json,packet (symbol table)
            Mean        Std.Dev.    Min         Median      Max
real        0.457       0.014       0.446       0.453       0.497
--json,packet (symbol table, no debug)
            Mean        Std.Dev.    Min         Median      Max
real        0.255       0.007       0.244       0.254       0.267
 */
  jsonBlock =
    qi::lit("[") > +qi::eol >
    *(packetBlock [pnx::bind(&Parser::setNewPacket, this)]) >
    qi::lit("]") > -qi::eol
    ;

  packetBlock =
    qi::lit("{\n") >
    +( (&qi::as_string[qi::raw[dataLines]] [qi::_b = qi::_1] >
        dataLines [qi::_a = qi::_1] > qi::lazy(*qi::_a))
         [pnx::bind(&Parser::setPacketData, this, qi::_b, qi::_2)]
     | (jsonEob)
     | (iLine - (qi::lit('{') | jsonEob | qi::lit(']')))
    )
    ;

  jsonEob =
    (  (qi::lit("},") > +qi::eol)
       // cppcheck-suppress compareBoolExpressionWithInt
     | ((qi::lit('}') > +qi::eol) > -(qi::lit(',') > +qi::eol))
    )
    ;

  // helpers
  //  attribute generating
  value =
    (  (qi::lit("\"\""))
     | (qi::lit('"') >
        +((qi::lit('\\') > qi::char_) | (qi::char_ - qi::lit('"'))) >
        qi::lit('"'))
     | (+qi::graph)
    ) > -qi::lit(',') > qi::eol
    ;
  //  non-attribute generating
  iLine =
    +(qi::char_ - qi::eol) > qi::eol;
    ;

  BOOST_SPIRIT_DEBUG_NODES(
      //(start)
      (jsonBlock)(jsonEob)
      (packetBlock)
      (value)
      (iLine)
      );
}


// =============================================================================
// Parser helper methods
// =============================================================================
template<typename Iter>
void
Parser<Iter>::setDone()
{
  LOG_INFO << "Parser packets processed: " << parsedPacketCount << "\n";
}

template<typename Iter>
void
Parser<Iter>::setNewPacket()
{
  if (processPacket(pd)) {
    ++parsedPacketCount;
  }
  pd.clear();
}

template<typename Iter>
void
Parser<Iter>::setPacketData(const std::string& _key, const std::string& _val)
{
  if (0 == _key.find("\"dns.")) {
    if (dnsRespName == _key) {
      tempDnsRespName = _val;
    } else {
      if (!pd.count(_key)) {
        pd[_key] = VecTupStrStrType();
      }
      auto& vec = std::any_cast<VecTupStrStrType&>(pd[_key]);
      vec.push_back(std::make_tuple(tempDnsRespName, _val));
    }
  } else if (  bootpHwMacAddr == _key
            || cdpNrgyzIpAddress == _key
            )
  {
    if (pd.count(_key)) {
      const auto& oVal {std::any_cast<std::string>(pd[_key])};
      if (oVal != _val) {
        LOG_WARN << "Multiple " << _key << " values differ: "
                 << oVal << " vs " << _val << "\n";
      }
    }
    pd[_key] = _val;
  } else if (   0 == _key.find("\"vlan.")
             || bootpOptionDomainNameServer == _key
             || bootpOptionTftpServerAddress == _key
            )
  {
    if (!pd.count(_key)) {
      pd[_key] = VecStrType();
    }
    auto& vec = std::any_cast<VecStrType&>(pd[_key]);
    vec.push_back(_val);
  } else if (cdpSoftwareVersion == _key) {
    if (pd.count(_key)) {
      std::ostringstream oss;
      oss << pd[_key] << " " << _val;
      pd[_key] = oss.str();
    } else {
      pd[_key] = _val;
    }
  } else if (sllSrc == _key) {
    pd[ethSrc] = _val; // handle as ethSrc
  } else if (pd.count(_key)) {
    LOG_DEBUG << "multiple, unhandled values for key: " << _key << "\n";
    pd[_key] = _val;
  } else {
    pd[_key] = _val;
  }
}

template<typename Iter>
bool
Parser<Iter>::processPacket(PacketData& _pd)
{
  bool status {false};
  Data d;

  // Helpers to access the data easier
  auto s1 =
    [&_pd](const std::string& _key) {
      return std::any_cast<std::string&>(_pd[_key]);
    };
  auto v1 =
    [&_pd](const std::string& _key) {
      return std::any_cast<VecStrType&>(_pd[_key]);
    };
  auto v2 =
    [&_pd](const std::string& _key) {
      return std::any_cast<VecTupStrStrType&>(_pd[_key]);
    };

  // NOTE: Order matters when processing packets
  // ----- Start Of Packet Processing -----

  // ===== VTP =====
  if (_pd.count(vtp)) {
    std::ostringstream oss;
    oss << "VTP like traffic seen from: " << s1(ethSrc);
    d.observations.addNotable(oss.str());
    status = true;
  }

  // ===== DTP =====
  if (_pd.count(dtpSenderid)) {
    std::ostringstream oss;
    oss << "DTP like traffic seen from: " << s1(ethSrc)
        << " (Sender ID: " << s1(dtpSenderid)
        << ")";
    d.observations.addNotable(oss.str());
    status = true;
  }

  // ===== STP =====
  if (_pd.count(stpRootHw) | _pd.count(stpBridgeHw)) {
    std::ostringstream oss;
    oss << "STP like traffic seen from: " << s1(ethSrc)
        << " (Root HW: " << s1(stpRootHw)
        << ", Bridge HW: " << s1(stpBridgeHw)
        << ")";
    d.observations.addNotable(oss.str());
    status = true;
  }

  // ===== ARP =====
  if (_pd.count(arpSrcHwMac) | _pd.count(arpSrcProtoIpv4)) {
    const auto& macAddrStr {s1(arpSrcHwMac)};
    const auto& ipAddrStr {s1(arpSrcProtoIpv4)};
    auto& macAddr {d.macAddrs[macAddrStr]};
    macAddr.setMac(macAddrStr);
    nmdo::IpAddress ipAddr {ipAddrStr, TSHARK_REASON};
    ipAddr.setResponding(true);
    macAddr.addIp(ipAddr);
    macAddr.setResponding(true);
    status = true;
  }

  // ===== DNS =====

  //if (_pd.count(dnsNs)) {
  //  for (const auto& v : v2(dnsNs)) {
  //    const auto& name {std::get<0>(v)};
  //    const auto& ipStr {std::get<1>(v)};
  //    nmdo::Service serv;
  //    serv.addDstPort("53");
  //    serv.setDstAddress(nmdo::IpAddress(ipStr, TSHARK_REASON));
  //    serv.setServiceName("dns");
  //    serv.setServiceDescription(name);
  //    serv.setServiceReason(TSHARK_REASON);
  //    serv.setProtocol("udp");
  //    d.services.insert(serv);
  //  }
  //  status = true;
  //}
  if (_pd.count(dnsA)) {
    for (const auto& v : v2(dnsA)) {
      const auto& name {std::get<0>(v)};
      const auto& ipStr {std::get<1>(v)};
      auto& ipAddr {d.ipAddrs[ipStr]};
      ipAddr.setAddress(ipStr);
      ipAddr.setReason(TSHARK_REASON);
      ipAddr.addAlias(name, TSHARK_REASON);
    }
    status = true;
  }
  if (pd.count(dnsAaaa)) {
    for (const auto& v : v2(dnsAaaa)) {
      const auto& name {std::get<0>(v)};
      const auto& ipStr {std::get<1>(v)};
      auto& ipAddr {d.ipAddrs[ipStr]};
      ipAddr.setAddress(ipStr);
      ipAddr.setReason(TSHARK_REASON);
      ipAddr.addAlias(name, TSHARK_REASON);
    }
    status = true;
  }
 
  // ===== bootp (DHCP) =====
  if (_pd.count(bootpOptionSubnetMask)) {
    const auto& subnetMask {s1(bootpOptionSubnetMask)};
    const auto& ipYour {s1(bootpIpYour)};

    if ("0.0.0.0" != ipYour) {
      auto& ipAddr {d.ipAddrs[ipYour]};
      ipAddr.setAddress(ipYour);
      ipAddr.setNetmask(nmdo::IpNetwork(subnetMask));
      ipAddr.setReason(TSHARK_REASON);
      ipAddr.setResponding(true);
      if (_pd.count(bootpOptionHostname)) {
        ipAddr.addAlias(s1(bootpOptionHostname), TSHARK_REASON);
      }
      status = true;
    }
  }
  if (_pd.count(bootpOptionDomainNameServer)) {
    for (const auto& v : v1(bootpOptionDomainNameServer)) {
      nmdo::Service serv;
      serv.addDstPort("53");
      serv.setDstAddress(nmdo::IpAddress(v, TSHARK_REASON));
      serv.setServiceName("dns");
      serv.setServiceReason(TSHARK_REASON);
      serv.setProtocol("udp");
      d.services.insert(serv);
    }
    status = true;
  }
  if (_pd.count(bootpOptionTftpServerAddress)) {
    for (const auto& v : v1(bootpOptionTftpServerAddress)) {
      nmdo::Service serv;
      serv.addDstPort("69");
      serv.setDstAddress(nmdo::IpAddress(v, TSHARK_REASON));
      serv.setServiceName("tftp");
      serv.setServiceReason(TSHARK_REASON);
      serv.setProtocol("udp");
      d.services.insert(serv);
    }
    status = true;
  }
  if (_pd.count(bootpHwMacAddr)) {
    const auto& macAddrStr {s1(bootpHwMacAddr)};
    auto& macAddr {d.macAddrs[macAddrStr]};
    macAddr.setMac(macAddrStr);
    if (_pd.count(bootpOptionDhcpServerId)) {
      const auto& ipAddrStr {s1(bootpOptionDhcpServerId)};
      nmdo::IpAddress ipAddr {ipAddrStr, TSHARK_REASON};
      ipAddr.setResponding(true);
      macAddr.addIp(ipAddr);
    }
    macAddr.setResponding(true);
    status = true;
  }

  // ===== NTP =====
  if (_pd.count(ntpRefid)) {
    auto refidHex {s1(ntpRefid)};
    std::replace(refidHex.begin(), refidHex.end(), ':', ' ');
    int o1,o2,o3,o4;
    std::istringstream(refidHex) >> std::hex >> o1 >> o2 >> o3 >> o4;
    std::ostringstream oss;
    oss << o1 << '.' << o2 << '.' << o3 << '.' << o4;

    // ignore not you ips
    if (!(127 == o1 && 127 == o2 && 127 == o3 && (127 == o4 || 128 == o4))) {
      nmdo::Service serv;
      serv.addDstPort("123");
      serv.setDstAddress(nmdo::IpAddress(oss.str(), TSHARK_REASON));
      serv.setServiceName("ntp");
      serv.setServiceReason(TSHARK_REASON);
      serv.setProtocol("udp");
      d.services.insert(serv);
    }
    status = true;
  }

  // ===== CDP =====
  if (_pd.count(cdpDeviceid)) {
    const auto& ifaceName {s1(cdpDeviceid)};
    auto& iface {d.ifaces[ifaceName]};
    iface.setDiscoveryProtocol(true);
    iface.setState(true);
    iface.setDescription(
        s1(cdpPlatform) + " -- " + s1(cdpSoftwareVersion)
        );
    nmdo::MacAddress macAddr {s1(ethSrc)};
    iface.setMacAddress(macAddr);
    nmdo::IpAddress ipAddr {s1(cdpNrgyzIpAddress), TSHARK_REASON};
    iface.addIpAddress(ipAddr);
    iface.setName(s1(cdpPortid));
    iface.setMediaType(s1(cdpPortid));
    //iface.(s1(cdpNativeVlan));
    status = true;
  }

  // ===== VLAN =====
  if (_pd.count(vlanId)) {
    for (const auto& vId : std::any_cast<VecStrType>(_pd[vlanId])) {
      auto val {static_cast<uint16_t>(std::stoi(vId))};
      d.vlans[vId] = nmdo::Vlan(val, TSHARK_REASON);
    }
  }


  // ===== Ethernet =====
  if (_pd.count(ethSrc)) {
    const auto& macAddrStr {s1(ethSrc)};
    auto& macAddr {d.macAddrs[macAddrStr]};
    macAddr.setMac(macAddrStr);
    macAddr.setResponding(true);
    status = true;
// NOTE: Below is wrong as ethSrc may be the router not the end point.
//    // ===== IPv4 =====
//    if (_pd.count(ipSrc)) {
//      nmdo::IpAddress ipAddr {s1(ipSrc), TSHARK_REASON};
//      macAddr.addIp(ipAddr);
//    }
//    // ===== IPv6 =====
//    if (_pd.count(ipv6Src)) {
//      nmdo::IpAddress ipAddr {s1(ipv6Src), TSHARK_REASON};
//      macAddr.addIp(ipAddr);
//    }
  }

  // ----- End Of Packet Processing -----

  // Put data into container if successful in processing
  if (status) {
    DataContainerSingleton::getInstance().insert(d);
  }

  return status;
}
