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

#include <netmeld/core/utils/AcBookUtilities.hpp>
#include <netmeld/core/utils/StringUtilities.hpp>

#include "Parser.hpp"

namespace nmcu = netmeld::core::utils;


// =============================================================================
// Parser logic
// =============================================================================
Parser::Parser() : Parser::base_type(start)
{
  start =
    (config)
      [pnx::bind([&](){d.devInfo.setVendor("cisco");}),
       qi::_val = pnx::bind(&Parser::getData, this)]
    ;

  config =
    *(
        (qi::lit("hostname") >> fqdn >> qi::eol)
           [pnx::bind([&](const std::string& val)
                      {d.devInfo.setDeviceId(val);}, qi::_1)]
      | (qi::lit("domain-name") >> fqdn >> qi::eol)
           [pnx::bind(&Parser::unsup, this, "domain-name")]
      | (qi::lit("name") >> ipAddr >> fqdn
           [pnx::bind(&Parser::tgtBook, this) = qi::_1] >>
         -(qi::lit("description") >> tokens) >> qi::eol)
           [pnx::bind(&Parser::updateNetBookIp, this, qi::_1)]
      | (qi::lit("name") >> tokens >> qi::eol)
           [pnx::bind(&Parser::unsup, this, "name")]
      | (asaIface)
      | (qi::lit("object-group") > (objGrpNet | objGrpSrv | objGrpProto))
      | (qi::lit("object") > (objNet | objSrv))
      | (qi::lit("access-list") >> policy)
      // access-group ac-list {in|out} interface ifaceName
      | (qi::lit("access-group") >> token >> token >>
         qi::lit("interface") >> token)
           [pnx::bind(&Parser::assignRules, this, qi::_1, qi::_2, qi::_3)]
      | ignoredLine
     )
    ;

  asaIface =
    (qi::lit("interface") >> token >> qi::eol)
       [pnx::bind(&Parser::ifaceInit, this, qi::_1)] >>
    // Explicitly check to ensure still in interface scope
    *(qi::no_skip[+qi::char_(' ')] >>
      (
       // Capture general settings
         (qi::lit("description") >> tokens)
            [pnx::bind(&nmco::InterfaceNetwork::setDescription,
                       pnx::bind(&Parser::tgtIface, this), qi::_1)]
       | (qi::lit("nameif") >> token)
            [pnx::bind([&](const std::string& val)
                       {ifaceAliases.emplace(val, *tgtIface);}, qi::_1)]
       | qi::lit("ipv6 address") >>
         (  (ipAddr)
               [pnx::bind(&nmco::InterfaceNetwork::addIpAddress,
                          pnx::bind(&Parser::tgtIface, this), qi::_1)]
          | (fqdn)
               [pnx::bind(&Parser::unsup, this, "ipv6 address DOMAIN-NAME")]
         )
       | qi::lit("ip address") >>
         (  (ipAddr >> ipAddr)
               [pnx::bind(&nmco::IpAddress::setNetmask, &qi::_1, qi::_2),
                pnx::bind(&nmco::InterfaceNetwork::addIpAddress,
                          pnx::bind(&Parser::tgtIface, this), qi::_1)]
          | (fqdn >> ipAddr)
               [qi::_a = pnx::bind(&Parser::getIpFromAlias, this, qi::_1),
                pnx::bind(&nmco::IpAddress::setNetmask, &qi::_a, qi::_2),
                pnx::bind(&nmco::InterfaceNetwork::addIpAddress,
                          pnx::bind(&Parser::tgtIface, this), qi::_a)]
         )// [pnx::bind(&nmco::IpAddress::setNetmask, &qi::_1, qi::_2),
          //  pnx::bind(&nmco::InterfaceNetwork::addIpAddress,
          //            pnx::bind(&Parser::tgtIface, this), qi::_1)]
       // Ignore all other settings
       | (qi::omit[+token])
       | (&qi::eol) // space prefixed blank line
      ) >> qi::eol
    )
    ;


  // object *

  // object network val_name
  //  { host val_ip | subnet val_ip val_mask | range val_ip val_ip }
  objNet =
    (qi::lit("network") >> token >> qi::eol)
      [pnx::bind(&Parser::tgtBook, this) = qi::_1] >>
    // Explicitly check to ensure still in interface scope
    *(qi::no_skip[+qi::char_(' ')] >>
      (  (qi::lit("subnet") >> ipAddr >> ipAddr)
           [pnx::bind(&nmco::IpAddress::setNetmask, &qi::_1, qi::_2),
            pnx::bind(&Parser::updateNetBookIp, this, qi::_1)]
       | (qi::lit("subnet") >> ipAddr)
           [pnx::bind(&Parser::updateNetBookIp, this, qi::_1)]
       | (qi::lit("host") >> ipAddr)
           [pnx::bind(&Parser::updateNetBookIp, this, qi::_1)]
       | (qi::lit("range") >> token >> token)
           [pnx::bind(&Parser::updateNetBookRange, this, qi::_1, qi::_2)]

       // Ignore all other settings
       | (qi::omit[+token])
       | (&qi::eol) // space prefixed blank line
      ) >> qi::eol
    )
    ;

  // object service val_name
  //  service { val_proto | icmp val_type | icmp6 val_type | { tcp | udp } [ source operator val_port ] [ destination operator val_port ]}
  // NOTE: operator = { eq | neq | lt | gt | range }
  objSrv =
    (qi::lit("service") >> token >> qi::eol)
      [pnx::bind(&Parser::tgtBook, this) = qi::_1] >>
    // Explicitly check to ensure still in scope
    *(qi::no_skip[+qi::char_(' ')] >>
      (
       // Capture general settings
         (qi::lit("service") >> objSrvProto >> objSrvSrc >> objSrvDst)
           [pnx::bind(&Parser::updateSrvcBook, this,
                      qi::_1+":"+qi::_2+":"+qi::_3)]

       // Ignore all other settings
       | (qi::omit[+token])
       | (&qi::eol) // space prefixed blank line
      ) >> qi::eol
    )
    ;

  objSrvProto =
    (  ((qi::string("icmp6") | qi::string("icmp")) >> qi::omit[-tokens])
     | (qi::string("tcp"))
     | (qi::string("udp"))
     | (token)
    )
    ;

  objSrvSrc =
    (qi::lit("source") >> portArgument) | qi::attr("")
    ;

  objSrvDst =
    (qi::lit("destination") >> portArgument) | qi::attr("")
    ;

  portArgument =
    (  (qi::lit("eq") >> token)
          [qi::_val = qi::_1]
     | (qi::lit("range") >> srvRngVal)
          [qi::_val = qi::_1]
     | (qi::lit("neq") >> qi::omit[token])
          [pnx::bind(&Parser::unsup, this, "neq PORT")]
     | (qi::lit("lt") >> qi::omit[token])
          [pnx::bind(&Parser::unsup, this, "lt PORT")]
     | (qi::lit("gt") >> qi::omit[token])
          [pnx::bind(&Parser::unsup, this, "gt PORT")]
    )
    ;

  srvRngVal =
    (token >> token)
      [qi::_val = qi::_1 + "-" + qi::_2]
    ;

  // object-group *

  // object-group network val_name
  //  network-object { object val_name | host val_ip | val_ip val_mask}
  //  group-object val_name
  objGrpNet =
    (qi::lit("network") >> token >> qi::eol)
      [pnx::bind(&Parser::tgtBook, this) = qi::_1] >>
    *(qi::no_skip[+qi::char_(' ')] >>
      (  (qi::lit("network-object object") >> token)
           [pnx::bind(&Parser::updateNetBookGroup, this, qi::_1)]
       | (qi::lit("network-object host") >> ipAddr)
           [pnx::bind(&Parser::updateNetBookIp, this, qi::_1)]
       | (qi::lit("network-object") >> ipAddr >> ipAddr)
           [pnx::bind(&nmco::IpAddress::setNetmask, &qi::_1, qi::_2),
            pnx::bind(&Parser::updateNetBookIp, this, qi::_1)]
       | (qi::lit("network-object") >> token >> ipAddr)
           [pnx::bind(&Parser::updateNetBookGroupMask, this, qi::_1, qi::_2)]
       | (qi::lit("group-object") >> token)
           [pnx::bind(&Parser::updateNetBookGroup, this, qi::_1)]

       // Ignore all other settings
       | (qi::omit[+token])
       | (&qi::eol) // space prefixed blank line
      ) >> qi::eol
    )
    ;


  // object-group service val_name { tcp | udp | tcp-udp }
  //  port-object { eq val_port | range val_start val_end }
  //  group-object val_name
  //  service-object { val_proto | icmp val_type | icmp6 val_type | { tcp | udp } [ source operator val_port ] [ destination operator val_port ]}
  // NOTE: operator = { eq | neq | lt | gt | range }
  objGrpSrv =
    (qi::lit("service") >>
     token [pnx::bind(&Parser::tgtBook, this) = qi::_1] >>
     -token [qi::_a = qi::_1] >>
     qi::eol
    ) >
    // Explicitly check to ensure still in interface scope
    *(qi::no_skip[+qi::char_(' ')] >>
      (  (qi::lit("port-object") >> portArgument)
           [pnx::bind(&Parser::updateSrvcBook, this, qi::_a+"::"+qi::_1)]
       | (qi::lit("group-object") >> token)
           [pnx::bind(&Parser::updateSrvcBookGroup, this, qi::_1)]
       | (qi::lit("service-object object") >> token)
           [pnx::bind(&Parser::updateSrvcBookGroup, this, qi::_1)]
       | (qi::lit("service-object") >> objSrvProto >> objSrvSrc >> objSrvDst)
           [pnx::bind(&Parser::updateSrvcBook, this,
                      qi::_1+":"+qi::_2+":"+qi::_3)]
 
       // Ignore all other settings
       | (qi::omit[+token])
       | (&qi::eol) // space prefixed blank line
      ) >> qi::eol
    )
    ;


  // object-group protocol val_name
  //  protocol-object val_proto
  //  group-object val_name
  objGrpProto =
    (qi::lit("protocol") >> token >> qi::eol)
       [pnx::bind(&Parser::tgtBook, this) = qi::_1] >>
    // Explicitly check to ensure still in interface scope
    *(qi::no_skip[+qi::char_(' ')] >>
      (  (qi::lit("protocol-object") >> token)
           [pnx::bind(&Parser::updateSrvcBook, this, qi::_1)]
       | (qi::lit("group-object") >> token)
           [pnx::bind(&Parser::updateSrvcBookGroup, this, qi::_1)]

       // Ignore all other settings
       | (qi::omit[+token])
       | (&qi::eol) // space prefixed blank line
      ) >> qi::eol
    )
    ;

  policy =
    (
// access-list access_list_name
//   standard {deny | permit}
//   {any4 | host ip_address | ip_address mask }
       (token >> qi::lit("standard"))
          [pnx::bind(&Parser::unsup, this, "access-list NAME standard")]
          /* Get AcBook so can add rule after defined */
          /* AcRule.addAction, AcRule.addDst */

// access-list access_list_name
//   remark {tokens}
    | (token >> qi::lit("remark") >>
       tokens [pnx::bind(&Parser::curRuleDescription, this) = qi::_1]
      )

// access-list access_list_name [line line_number]
//   extended {deny | permit}
//   protocol_argument
//   source_address_argument [port_argument]
//   dest_address_argument [port_argument]
//   [log [[level] [interval secs] | disable | default]]
//   [time-range time_range_name]
//   [inactive]
// NOTE: port_arguments only seem to appear if protocol_argument is not an
//       object-group of type protocol
     | (token
          [qi::_a = qi::_1] >>
        qi::lit("extended") >> token
          [pnx::bind(&Parser::updateCurRuleId, this, qi::_a),
           pnx::bind(&Parser::updateRuleAction, this, qi::_a, qi::_1)] >>
        // protocol_argument
        ((  (qi::lit("object-group") >> token)
          | (qi::lit("object") >> token)
         ) [pnx::bind(&Parser::updateTgtProto, this, qi::_1, true)]
         | (token)
             [pnx::bind(&Parser::updateTgtProto, this, qi::_1, false)]
        ) >>
        // source_address_argument
        (addressArgument)
          [pnx::bind(&Parser::updateRuleSrc, this, qi::_a, qi::_1)] >>
        -(portArgument)
          // NOTE: New logic needed if this can also be an object-group
          [pnx::bind(&Parser::updateTgtSrcPort, this, qi::_1, false)] >>
        // dest_address_argument
        (addressArgument)
          [pnx::bind(&Parser::updateRuleDst, this, qi::_a, qi::_1)] >>
        (-(  (portArgument)
              [pnx::bind(&Parser::updateTgtDstPort, this, qi::_1, false)]
          | (qi::lit("object-group") >> token)
              [pnx::bind(&Parser::updateTgtDstPort, this, qi::_1, true)]
         )
        ) [pnx::bind(&Parser::updateRuleService, this, qi::_a)] >>
        // other
        -(logVal
            [pnx::bind(&Parser::updateRuleAction, this, qi::_a, qi::_1)]
         ) >>
        -(qi::string("time-range") >> token)
            [pnx::bind(&Parser::updateRuleAction, this,
                       qi::_a, qi::_1+":"+qi::_2)] >>
        -(qi::lit("inactive"))
            [pnx::bind(&Parser::disableRule, this, qi::_a)]
       )
    ) >> qi::eol
    ;

  addressArgument =
    (  (qi::lit("object-group") >> token)
     | (qi::lit("object") >> token)
     | (qi::lit("interface") >> token)
     | (qi::lit("host") >> ipAddrStr)
     | (qi::string("any4"))
     | (qi::string("any6"))
     | (qi::string("any"))
     | (ipAddrStr)
    )
    ;

  logVal =
    qi::as_string[
      qi::string("log") >>
      *(qi::blank >>
        !&(qi::lit("time-range") | qi::lit("inactive")) >> token
       )
    ]
    ;

  ipAddrStr =
    (  (ipAddr >> ipAddr)
         [pnx::bind(&nmco::IpAddress::setNetmask, &qi::_1, qi::_2),
          qi::_val = pnx::bind(&nmco::IpAddress::toString, &qi::_1)]
     | (ipAddr)
         [qi::_val = pnx::bind(&nmco::IpAddress::toString, &qi::_1)]
    )
    ;

  ignoredLine =
      (+token >> -qi::eol) // ignored config lines
    | (+qi::eol)          // ignored empty lines
    ;

  tokens =
    qi::as_string[+(token >> *qi::blank)]
    ;

  token =
    +(qi::ascii::graph)
    ;

  BOOST_SPIRIT_DEBUG_NODES(
      //(start)
      (config)
      (asaIface)
      (objNet)(objGrpNet)
      (objSrv)(objSrvProto)(objSrvSrc)(objSrvDst)
      (objGrpSrv)(objGrpProto)
      (ipAddrStr)(policy)(srvRngVal)(portArgument)(logVal)(addressArgument)
      (ignoredLine)
      //(tokens) (token)
      );
}

// =============================================================================
// Parser helper methods
// =============================================================================
// Interface related
void
Parser::ifaceInit(const std::string& _name)
{
  tgtIface = &d.ifaces[_name];
  tgtIface->setName(_name);
  tgtIface->setState(true);
}

nmco::IpAddress
Parser::getIpFromAlias(const std::string& _bookName)
{
  if (!d.networkBooks[ZONE].count(_bookName)) {
    // NOTE: not found is a major logic problem
    LOG_ERROR << "Parser::getIpFromAlias: Alias not defined: "
              << _bookName << std::endl;
    return nmco::IpAddress();
  }

  const auto& book {d.networkBooks[ZONE][_bookName].getData()};
  if (book.size() > 1) {
    LOG_WARN << "Parser::getIpFromAlias: More than one alias known for: "
             << _bookName << std::endl;
  }
  return nmco::IpAddress(*(book.begin()));
}


// AC related
void
Parser::updateNetBookIp(const nmco::IpAddress& _ip)
{
  d.networkBooks[ZONE][tgtBook].addData(_ip.toString());
}

void
Parser::updateNetBookRange(const std::string& _ip1, const std::string& _ip2)
{
  std::ostringstream oss;
  oss << _ip1 << "-" << _ip2;
  d.networkBooks[ZONE][tgtBook].addData(oss.str());
}

void
Parser::updateNetBookGroup(const std::string& _bookOther)
{
  const auto& tgtData {d.networkBooks[ZONE][_bookOther].getData()};
  if (0 == tgtData.size()) {
    LOG_DEBUG << "Parser::updateNetBookGroup: set defined after use ["
              << ZONE << "][" << tgtBook << "]: " << _bookOther << "\n";
    d.networkBooks[ZONE][tgtBook].addData(_bookOther);
  } else {
    d.networkBooks[ZONE][tgtBook].addData(tgtData);
  }
}

void
Parser::updateNetBookGroupMask(const std::string& _bookOther,
                               const nmco::IpAddress& _mask)
{
  for (const auto& _ip : d.networkBooks[ZONE][_bookOther].getData()) {
    if (std::string::npos != _ip.find('-')) {
      LOG_DEBUG << "Parser::updateNetBookGroupMask: attempt to apply netmask"
                << " to net range: " << _ip << "--" << _mask
                << "\n";
      continue;
    }
    nmco::IpAddress ip {_ip};
    ip.setNetmask(_mask);
    d.networkBooks[ZONE][tgtBook].addData(ip.toString());
  }
}

void
Parser::updateSrvcBook(const std::string& _ports)
{
  d.serviceBooks[ZONE][tgtBook].addData(_ports);
}

void
Parser::updateSrvcBookGroup(const std::string& _bookOther)
{
  const auto& tgtData {d.serviceBooks[ZONE][_bookOther].getData()};
  if (0 == tgtData.size()) {
    LOG_DEBUG << "Parser::updateSrvcBookGroup: set defined after use ["
              << ZONE << "][" << tgtBook << "]: " << _bookOther << "\n";
    d.networkBooks[ZONE][tgtBook].addData(_bookOther);
  } else {
    d.serviceBooks[ZONE][tgtBook].addData(tgtData);
  }
}

void
Parser::updateCurRuleId(const std::string& _bookName)
{
  curRuleId = ruleIds[_bookName]++;
  d.ruleBooks[_bookName][curRuleId].setRuleId(curRuleId);
  d.ruleBooks[_bookName][curRuleId].setRuleDescription(curRuleDescription);
  curRuleDescription = "";
}

void
Parser::updateRuleAction(const std::string& _bookName,
                         const std::string& _action)
{
  d.ruleBooks[_bookName][curRuleId].addAction(_action);
}

void
Parser::updateRuleSrc(const std::string& _bookName,
                      const std::string& _src)
{
  d.ruleBooks[_bookName][curRuleId].setSrcId(ZONE);
  d.ruleBooks[_bookName][curRuleId].addSrc(_src);
}

void
Parser::updateRuleDst(const std::string& _bookName,
                      const std::string& _dst)
{
  d.ruleBooks[_bookName][curRuleId].setDstId(ZONE);
  d.ruleBooks[_bookName][curRuleId].addDst(_dst);
}

void
Parser::updateTgtProto(const std::string& _proto, const bool _val)
{
  tgtProto   = std::make_pair(_val, _proto);
  tgtSrcPort = std::make_pair(false, "");
  tgtDstPort = std::make_pair(false, "");
}

void
Parser::updateTgtSrcPort(const std::string& _port, const bool _val)
{
  tgtSrcPort = std::make_pair(_val, _port);
}

void
Parser::updateTgtDstPort(const std::string& _port, const bool _val)
{
  tgtDstPort = std::make_pair(_val, _port);
}

void
Parser::updateRuleService(const std::string& _bookName)
{
  // 4 cases:
  //   tgtProto = serviceBook entry && tgt(Src|Dst)Port = none
  //   tgtProto = serviceBook entry && tgt(Src|Dst)Port = raw port
  //   tgtProto = raw proto && tgtDstPort = serviceBook entry
  //   tgtProto = raw proto && tgt(Src|Dst)Port = raw port

  const auto& tpb  {std::get<0>(tgtProto)};
  const auto& tp   {std::get<1>(tgtProto)};
  const auto& tspb {std::get<0>(tgtSrcPort)};
  const auto& tsp  {std::get<1>(tgtSrcPort)};
  const auto& tdpb {std::get<0>(tgtDstPort)};
  const auto& tdp  {std::get<1>(tgtDstPort)};

  auto& rule {d.ruleBooks[_bookName][curRuleId]};

  if (tpb) {
    if (!(tspb || tdpb) && tsp.empty() && tdp.empty()) {
      rule.addService(tp);
    } else {
      std::ostringstream oss;
      const auto& tgtData {d.serviceBooks[ZONE].at(tp).getData()};
      auto iter {tgtData.begin()};
      oss << *iter;
      ++iter;
      for (; iter != tgtData.end(); ++iter) {
        oss << '-' << *iter;
      }
      rule.addService(nmcu::getSrvcString(oss.str(), tsp, tdp));
    }
  } else {
    if (tdpb) {
      rule.addService(tdp);
    } else {
      rule.addService(nmcu::getSrvcString(tp, tsp, tdp));
    }
  }
}

void
Parser::disableRule(const std::string& _bookName)
{
  d.ruleBooks[_bookName][curRuleId].disable();
}

void
Parser::assignRules(const std::string& _bookName,
                    const std::string& _dir,
                    const std::string& _iface)
{
  if ("in" == _dir) {
    for (auto& [id, rule] : d.ruleBooks[_bookName]) {
      // in: filter traffic entering iface, regardless destination
      rule.addSrcIface(ifaceAliases[_iface].getName());
      rule.addDstIface("any");
    }
  } else if ("out" == _dir) {
    for (auto& [id, rule] : d.ruleBooks[_bookName]) {
      // out: filter traffic leaving iface, regardless origin
      rule.addSrcIface("any");
      rule.addDstIface(ifaceAliases[_iface].getName());
    }
  } else {
    LOG_ERROR << "Parser::assignRules: Unknown rule direction parsed: "
              << _dir << std::endl;
  }
}


// Unsupported
void
Parser::unsup(const std::string& _val)
{
  d.observations.addUnsupportedFeature(_val);
}


// Object return
Result
Parser::getData()
{
  for (const auto& [nsi, nsb] : d.networkBooks) {
    for (const auto& [ns, nsd] : nsb) {
      nmcu::expanded(d.networkBooks, nsi, ns, ZONE);
    }
  }
  for (const auto& [nsi, nsb] : d.serviceBooks) {
    for (const auto& [ns, nsd] : nsb) {
      nmcu::expanded(d.serviceBooks, nsi, ns, ZONE);
    }
  }

  Result r;
  r.push_back(d);
  return r;
}
