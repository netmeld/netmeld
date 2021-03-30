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

#include <netmeld/datastore/utils/AcBookUtilities.hpp>
#include <netmeld/core/utils/StringUtilities.hpp>

#include "Parser.hpp"

namespace nmcu = netmeld::core::utils;
namespace nmdu = netmeld::datastore::utils;


// =============================================================================
// Parser logic
// =============================================================================
Parser::Parser() : Parser::base_type(start)
{
  start =
    config
      [(qi::_val = pnx::bind(&Parser::getData, this))]
    ;

  config =
    *(  (qi::lit("set") >>
         (// show configuration | display set
            ("interfaces" >> (interfaces | qi::omit[*token]))
          | ("routing-options" >> (routes | qi::omit[*token]))
          // ScreenOS: get config
          | (qi::lit("interface") >> (interface | qi::omit[*token]))
          | (qi::lit("service") >> (service | qi::omit[*token]))
          | (qi::lit("zone") >> (zone | qi::omit[*token]))
          | (qi::lit("address") >> (address | qi::omit[*token]))
          | (qi::lit("group") >> (group | qi::omit[*token]))
          | (qi::lit("policy") >> (policy | qi::omit[*token]))
          | (qi::lit("route") >> (route | qi::omit[*token]))
         ) >> +qi::eol)
     | (qi::omit[+token] >> -(+qi::eol))
     | +qi::eol
    )
    ;

  // interface { ip|ipv6|mip|tag|zone }
  //   ip {ip_addr/prefix|unnumbered interface interface}
  //   ipv6 ip ip_addr/prefix
  //   mip ip_addr host ip_addr [netmask mask] [vrouter vrouter]
  //   tag id_num zone zone
  //   zone zone
  interface =
    ifaceTypeName
      [(pnx::bind(&Parser::initIface, this, qi::_1[0], qi::_1[1]))] >>
    (  (qi::lexeme[qi::lit("ip") >> -qi::lit("v6")] >> ipAddr)
         [(pnx::bind(&Parser::updateIfaceIp, this, qi::_1))]
     | (qi::lit("ip unnumbered interface") >> token)
         [(pnx::bind(&Parser::unsup, this, "iface alias: " + qi::_1))]
     // NOTE: mip == host, maybe could do withe network book...but appling
     //       a netmask means the entire, individual, range is aliased
     | ((qi::lit("mip") >> ipAddr >> qi::lit("host") >> ipAddr) >>
        -(qi::lit("netmask") >> ipAddr) >> -vrouter)
         [(pnx::bind(&Parser::unsup, this, "mip defined"))]
     | (-(qi::lit("tag") >> qi::uint_) >> qi::lit("zone") >> token)
         [(pnx::bind(&Parser::updateZoneIfaceBook, this, qi::_2))]
    )
    ;

  interfaces =
    (  (ifaceTypeName >> "unit" >> qi::as_string[+qi::digit])
         [(pnx::bind(&Parser::initIface, this, qi::_1[0],
                     qi::_1[1] + '.' + qi::_2))]
     | (ifaceTypeName)
         [(pnx::bind(&Parser::initIface, this, qi::_1[0],
                     qi::_1[1] + ".0"))] // if no unit, assume 0
     | (qi::as_string[qi::lexeme[+qi::graph]] >>
        "unit" >> qi::as_string[+qi::digit])
         [(pnx::bind(&Parser::initIface, this, qi::_1, '.' + qi::_2))]
     | (qi::as_string[qi::lexeme[+qi::graph]])
         [(pnx::bind(&Parser::initIface, this, qi::_1, ""))] // no slot or unit
    ) >>
    -( ("family" >> (qi::lit("inet6") | qi::lit("inet")) >> "address" >> ipAddr)
         [(pnx::bind(&Parser::updateIfaceIp, this, qi::_1))]
     | (qi::lit("disable")) [(pnx::bind(&Parser::disableIface, this))]
    )
    ;

  ifaceTypeName =
      (qi::lit('"') >>
       +qi::ascii::alpha >> +(qi::char_ - qi::char_('"')) >>
       qi::lit('"'))
    | (+qi::ascii::alpha >> +qi::ascii::graph)
    ;

  // name protocol|+ proto [src-port num-num dst-port num-num]
  service =
    (token >> (qi::lit("protocol") | qi::lit("+")) >> token >>
     srvcSrcPort >> srvcDstPort)
      [(pnx::bind(&Parser::updateSrvcBook, this, qi::_1,
          pnx::bind(&nmcu::getSrvcString, qi::_2, qi::_3, qi::_4)))]
    ;

  srvcSrcPort =
    (qi::lit("src-port") >> token) | qi::attr("")
    ;

  srvcDstPort =
    (qi::lit("dst-port") >> token) | qi::attr("")
    ;


  // [id num] name [vrouter vr_name]
  zone =
    -(qi::lit("id") >> qi::uint_) >> token >> -vrouter
    ;

  // zone name {ip mask|ip/prefix|fqdn} [comment]
  address =
    (token >> token >> ipAddrOrFqdn >> *token)
      [(pnx::bind(&Parser::updateNetBook, this, qi::_1, qi::_2, qi::_3))]
    ;

  ipAddrOrFqdn =
    (  (ipAddr >> ipAddr)
         [(pnx::bind(&nmdo::IpAddress::setNetmask, &qi::_1, qi::_2),
           qi::_val = pnx::bind(&nmdo::IpAddress::toString, &qi::_1))]
     | (ipAddr)
         [(qi::_val = pnx::bind(&nmdo::IpAddress::toString, &qi::_1))]
     | (fqdn)
         [(qi::_val = qi::_1)]
    )
    ;

  // {{address zone}|service} grpName [add name] [comment]
  group =
    (  (qi::lit("address") >> token >> token >> qi::lit("add") >> token)
         [(pnx::bind(&Parser::updateNetBookGroup, this, qi::_1, qi::_2, qi::_3))]
     | (qi::lit("service") >> token >> qi::lit("add") >> token)
         [(pnx::bind(&Parser::updateSrvcBookGroup, this, qi::_1, qi::_2))]
     | (qi::omit[token >> token]) // ignore: {address|service} grpName
    ) >> qi::omit[*token] // comments
    ;

  // id number from zone1 to zone2 src dst service action(s)
  // id number disable
  // id number set src-address|dst-address|service|log value
  policy =
    (qi::lit("id") >> qi::uint_)
      [(pnx::bind(&Parser::updateCurRuleId, this, qi::_1))] >>
    (  (qi::lit("from") >> token >> qi::lit("to") >> token >>
        token >> token >> token >> +token)
          [(pnx::bind(&Parser::addRule, this,
                      qi::_1, qi::_2, qi::_3, qi::_4, qi::_5, qi::_6))]
     | (qi::lit("disable"))
         [(pnx::bind(&Parser::disableRule, this))]
     | (qi::eol >>
        *(qi::lit("set") >> token >> (token | qi::as_string[qi::attr("")]) >>
          qi::omit[*token] >> qi::eol)
            [(pnx::bind(&Parser::updateRule, this, qi::_1, qi::_2))] >>
        qi::lit("exit")
       )
     | (token >> qi::omit[*token] >> qi::eol)
        [(pnx::bind(&Parser::unsup, this, qi::_1))]
    )
    ;

  // route ip/prefix interface ifaceName [gateway ip]
  route =
    (ipAddr >> qi::lit("interface") >> token [(qi::_a = qi::_1)])
       [(pnx::bind(&Parser::setIfaceRoute, this, qi::_a, qi::_1))] >>
    (-qi::lit("gateway") >> ipAddr)
      [(pnx::bind(&Parser::setIfaceGateway, this, qi::_a, qi::_1))]
    ;

  routes =
    ("static route" >> ipAddr >> "next-hop" >> ipAddr)
      [(pnx::bind(&Parser::setIfaceRoute, this, "", qi::_1),
        pnx::bind(&Parser::setIfaceGateway, this, "", qi::_2))]
    ;

  vrouter =
    (qi::lit("vrouter") | qi::lit("vr")) >> token
    ;

  token =
      (qi::lit('"') >> +(qi::char_ - qi::char_('"')) >> qi::lit('"'))
    | +(qi::ascii::graph)
    ;

  BOOST_SPIRIT_DEBUG_NODES(
      //(start)
      (config)
      (interface)(service)(zone)(address)(group)(policy)
      (interfaces)(routes)
      (route)
      (ifaceTypeName)
      (ipAddrOrFqdn)
      (srvcSrcPort)(srvcDstPort)
      (vrouter)
      //(token)
      );
}


// =============================================================================
// Parser helper methods
// =============================================================================
void
Parser::setIfaceRoute(const std::string& _iface, nmdo::IpAddress& _ip)
{
  d.routes[_iface].setDstNet(_ip);
}

void
Parser::setIfaceGateway(const std::string& _iface, nmdo::IpAddress& _ip)
{
  d.routes[_iface].setRtrIp(_ip);
}

void
Parser::initIface(const std::string& _type, const std::string& _slotUnit)
{
  tgtIface = {_type + _slotUnit};
  if (!d.ifaces.count(tgtIface)) {
    auto& iface {d.ifaces[tgtIface]};
    iface.setName(tgtIface);
    iface.setState(true);
    iface.setMediaType(_type);
  }
}

void
Parser::disableIface()
{
  d.ifaces[tgtIface].setState(false);
}

void
Parser::updateIfaceIp(nmdo::IpAddress& _ip)
{
  d.ifaces[tgtIface].addIpAddress(_ip);
}

void
Parser::updateZoneIfaceBook(const std::string& _zone)
{
  zoneIfaceBook[_zone] = tgtIface;
}

void
Parser::updateNetBook(const std::string& _zone, const std::string& _bookName,
                      const std::string& _ip)
{
  d.networkBooks[_zone][_bookName].addData(_ip);
}

void
Parser::updateNetBookGroup(const std::string& _zone,
                           const std::string& _bookName,
                           const std::string& _zoneOther)
{
  const auto& tgtData {d.networkBooks[_zone][_zoneOther].getData()};
  if (0 == tgtData.size()) {
    LOG_DEBUG << "Parser::updateNetBookGroup: set defined after use ["
              << _zone << "][" << _bookName << "]: " << _zoneOther << "\n";
    d.networkBooks[_zone][_bookName].addData(_zoneOther);
  } else {
    d.networkBooks[_zone][_bookName].addData(tgtData);
  }
}

void
Parser::updateSrvcBook(const std::string& _bookName, const std::string& _port)
{
  d.serviceBooks[DEFAULT_ZONE][_bookName].addData(_port);
}

void
Parser::updateSrvcBookGroup(const std::string& _bookName,
                            const std::string& _bookOther)
{
  const auto& tgtData {d.serviceBooks[DEFAULT_ZONE][_bookOther].getData()};
  if (0 == tgtData.size()) {
    LOG_DEBUG << "Parser::updateSrvcBookGroup: set defined after use ["
              << DEFAULT_ZONE << "][" << _bookName << "]: " << _bookOther << "\n";
    d.serviceBooks[DEFAULT_ZONE][_bookName].addData(_bookOther);
  } else {
    d.serviceBooks[DEFAULT_ZONE][_bookName].addData(tgtData);
  }
}

void
Parser::updateCurRuleId(const size_t _id)
{
  curRuleId = _id;
  d.ruleBooks[bookName][_id].setRuleId(curRuleId);
}

void
Parser::disableRule()
{
  d.ruleBooks[bookName][curRuleId].disable();
}

void
Parser::updateRule(const std::string& _key, const std::string& _val)
{
  auto& rule {d.ruleBooks[bookName][curRuleId]};
  if ("src-address" == _key) {
    rule.addSrc(_val);
  } else if ("dst-address" == _key) {
    rule.addDst(_val);
  } else if ("service" == _key) {
    rule.addService(_val);
  } else if ("log" == _key) {
    rule.addAction(_val);
  } else {
    LOG_WARN << "Parser::updateRule: unsupported key-value: "
             << _key << "--" << _val << std::endl;
  }
}

void
Parser::addRule(const std::string& _srcId, const std::string& _dstId,
                const std::string& _src, const std::string& _dst,
                const std::string& _srvc,
                const std::vector<std::string>& _action)
{
  auto& rule {d.ruleBooks[bookName][curRuleId]};

  rule.setSrcId(_srcId);
  rule.addSrc(_src);
  rule.setDstId(_dstId);
  rule.addDst(_dst);
  rule.addService(_srvc);
  for (const auto& action : _action) {
    rule.addAction(action);
  }
}

void
Parser::unsup(const std::string& _value)
{
  d.observations.addUnsupportedFeature(_value);
}

Result
Parser::getData()
{
  for (const auto& [nsi, nsb] : d.networkBooks) {
    for (const auto& [ns, nsd] : nsb) {
      nmdu::expanded(d.networkBooks, nsi, ns, DEFAULT_ZONE);
    }
  }
  for (const auto& [nsi, nsb] : d.serviceBooks) {
    for (const auto& [ns, nsd] : nsb) {
      nmdu::expanded(d.serviceBooks, nsi, ns, DEFAULT_ZONE);
    }
  }

  for (const auto& [z, m] : d.ruleBooks) {
    for (const auto& [i, b] : m) {
      auto& rule {d.ruleBooks[z][i]};

      const auto& srcId {rule.getSrcId()};
      if (zoneIfaceBook.count(srcId)) {
        rule.addSrcIface(zoneIfaceBook[srcId]);
      }

      const auto& dstId {rule.getDstId()};
      if (zoneIfaceBook.count(dstId)) {
        rule.addDstIface(zoneIfaceBook[dstId]);
      }
    }
  }

  Result r;
  r.push_back(d);

  return r;
}

