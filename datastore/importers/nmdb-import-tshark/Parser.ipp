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
/* NOTE: This parser currently requires two things for validity:
 *   1) The content is in a JSON object array, e.g., `[{},{},{}]`; and
 *   2) The content to be "saved" is a key value pair.
 * So long as those two conditions are met it will succeed.
*/
template<typename Iter>
Parser<Iter>::Parser() : Parser::base_type(start)
{
  // packet data to consume
  dataLines.add
    (frameNumber, &value)
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
    (dhcpOptionSubnetMask, &value)
    (dhcpOptionDomainNameServer, &value)
    (dhcpOptionDhcpServerId, &value)
    (dhcpOptionRouter, &value)
    (dhcpOptionDomainName, &value)
    (dhcpOptionTftpServerAddress, &value)
    (dhcpOptionSipServerAddress, &value)
    (dhcpOptionHostname, &value) // client name
    (dhcpIpYour, &value) // client ip
    (dhcpIpRelay, &value) // tricky
    (dhcpHwMacAddr, &value) // client mac
    (dhcp6IaprefixPrefAddr, &value) // "subnet" for c&s
    (dhcpv6DuidlltLinkLayerAddr, &value) // client&server
    // ===== NTP =====
    //(ntpRefid, &value) // ntp source, bytes, see "not you"
    (ntpMode, &value)
    (ntpCtrlFlags2R, &value)
    ;


  start =
    qi::eps [(qi::_val = pnx::construct<Result>())]
    > packetArray [(pnx::bind(&Parser::setDone, this))]
    ;

  packetArray =
    qi::lit("[") > +qi::eol
    > *(packetBlock [(pnx::bind(&Parser::setNewPacket, this))])
    > qi::lit("]") > -qi::eol
    ;

  packetBlock =
    qi::lit("{\n")
    > +( (&qi::as_string[qi::raw[dataLines]] [(qi::_b = qi::_1)]
          > dataLines [(qi::_a = qi::_1)] > qi::lazy(*qi::_a)
         ) [(pnx::bind(&Parser::setPacketData, this, qi::_b, qi::_2))]
       | (jsonEndOfObject)
       | (iLine - (qi::lit('{') | jsonEndOfObject | qi::lit(']')))
      )
    ;

  jsonEndOfObject =
    (  (qi::lit("},") > +qi::eol)
     | ((qi::lit('}') > +qi::eol) > -(qi::lit(',') > +qi::eol))
    )
    ;

  // helpers
  //  attribute generating
  value =
    (  (qi::lit(R"("")"))
     | (qi::lit('"')
        > +((qi::lit('\\') > qi::char_) | (qi::char_ - qi::lit('"')))
        > qi::lit('"')
       )
     | (+qi::ascii::graph)
    ) > -qi::lit(',') > qi::eol
    ;

  //  non-attribute generating
  iLine =
    +(qi::char_ - qi::eol) > qi::eol;
    ;

  BOOST_SPIRIT_DEBUG_NODES(
      //(start)
      (packetArray)
      (packetBlock)
      (jsonEndOfObject)
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
  ++processedPacketCount;
  if (processPacket(pd)) {
    ++parsedPacketCount;
  }
  pd.clear();
}

template<typename Iter>
void
Parser<Iter>::setPacketData(const std::string& _key, const std::string& _val)
{
  if (_key.starts_with("\"dns.")) {
    if (dnsRespName == _key) {
      tempDnsRespName = _val;
    } else {
      if (!pd.count(_key)) {
        pd[_key] = VecTupStrStrType();
      }
      auto& vec = std::any_cast<VecTupStrStrType&>(pd[_key]);
      vec.push_back(std::make_tuple(tempDnsRespName, _val));
    }
  } else if (  dhcpHwMacAddr == _key
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
  } else if (_key.starts_with("\"vlan.")
             || dhcpOptionDomainNameServer == _key
             || dhcpOptionTftpServerAddress == _key
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
    oss << "Probable VTP from MAC: " << s1(ethSrc);
    d.observations.addNotable(oss.str());
    status = true;
  }

  // ===== DTP =====
  if (_pd.count(dtpSenderid)) {
    std::ostringstream oss;
    oss << "Probable DTP from MAC: " << s1(ethSrc)
        << " (Sender ID: " << s1(dtpSenderid)
        << ")";
    d.observations.addNotable(oss.str());
    status = true;
  }

  // ===== STP =====
  if (_pd.count(stpRootHw) | _pd.count(stpBridgeHw)) {
    std::ostringstream oss;
    oss << "Probable STP from MAC: " << s1(ethSrc)
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
    macAddr.addIpAddress(ipAddr);
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

  // ===== DHCP =====
  if (_pd.count(dhcpOptionSubnetMask)) {
    const auto& subnetMask {s1(dhcpOptionSubnetMask)};
    const auto& ipYour {s1(dhcpIpYour)};

    if ("0.0.0.0" != ipYour) {
      auto& ipAddr {d.ipAddrs[ipYour]};
      ipAddr.setAddress(ipYour);
      ipAddr.setNetmask(nmdo::IpNetwork(subnetMask));
      ipAddr.setReason(TSHARK_REASON);
      ipAddr.setResponding(true);
      if (_pd.count(dhcpOptionHostname)) {
        ipAddr.addAlias(s1(dhcpOptionHostname), TSHARK_REASON);
      }
      status = true;
    }
  }
  if (_pd.count(dhcpOptionDomainNameServer)) {
    for (const auto& v : v1(dhcpOptionDomainNameServer)) {
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
  if (_pd.count(dhcpOptionTftpServerAddress)) {
    for (const auto& v : v1(dhcpOptionTftpServerAddress)) {
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
  if (_pd.count(dhcpHwMacAddr)) {
    const auto& macAddrStr {s1(dhcpHwMacAddr)};
    auto& macAddr {d.macAddrs[macAddrStr]};
    macAddr.setMac(macAddrStr);
    if (_pd.count(dhcpOptionDhcpServerId)) {
      const auto& ipAddrStr {s1(dhcpOptionDhcpServerId)};
      nmdo::IpAddress ipAddr {ipAddrStr, TSHARK_REASON};
      ipAddr.setResponding(true);
      macAddr.addIpAddress(ipAddr);
    }
    macAddr.setResponding(true);
    status = true;
  }

  // ===== NTP =====
  if (_pd.count(ntpMode)) {
    auto mode {s1(ntpMode)};
    std::string mode6Response =
        _pd.count(ntpCtrlFlags2R) ? s1(ntpCtrlFlags2R) : "";

    if (!("3" == mode || ("6" == mode && "1" != mode6Response))) {
      if ("6" == mode && "1" == mode6Response) {
        std::ostringstream oss;
        oss << "NTP control data present in frame: " << s1(frameNumber);
        d.observations.addNotable(oss.str());
      }
      nmdo::Service serv;
      serv.setServiceName("ntp");
      serv.setProtocol("udp"); // check for protocol? frame.protocols
      serv.addDstPort("123"); // udp.srcport
      auto ipAddr = getSrcIp(_pd);
      ipAddr.setReason(TSHARK_REASON);
      serv.setDstAddress(ipAddr);
      std::ostringstream oss;
      oss << TSHARK_REASON;
      if ("4" == mode) {
        oss << "; client/server variant";
      } else if ("1" == mode || "2" == mode) {
        oss << "; symmetric variant";
      } else if ("5" == mode || "6" == mode) {
        oss << "; broadcast variant";
      }
      oss << " (" << mode << ")";
      serv.setServiceReason(oss.str());
      d.services.insert(serv);

      status = true;
    }
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
      status = true;
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
//      macAddr.addIpAddress(ipAddr);
//    }
//    // ===== IPv6 =====
//    if (_pd.count(ipv6Src)) {
//      nmdo::IpAddress ipAddr {s1(ipv6Src), TSHARK_REASON};
//      macAddr.addIpAddress(ipAddr);
//    }
  }

  // ----- End Of Packet Processing -----

  // Put data into container if successful in processing
  if (status) {
    DataContainerSingleton::getInstance().insert(d);
  }

  return status;
}

template<typename Iter>
nmdo::IpAddress
Parser<Iter>::getSrcIp(PacketData& _pd) const
{
  std::string srcIp {""};

  if (_pd.count(ipSrc)) {
    srcIp = std::any_cast<std::string&>(_pd[ipSrc]);
  } else if (_pd.count(ipv6Src)) {
    srcIp = std::any_cast<std::string&>(_pd[ipv6Src]);
  }
  nmdo::IpAddress value {srcIp};
  return value;
}
