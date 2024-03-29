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

#include <regex>

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
    *(  system
      | applications
      | interfaces
      | routingOptions
      | security
      | groups
      | vlans
      | logicalSystems
      | routingInstances
      | ignoredBlock
      | garbageLine
     )
    ;

  // START OF: system {...}
  system =
    qi::lit("system") >>
    startBlock >>
    *(  (qi::lit("host-name") > token)
//          [(pnx::bind(&Parser::updateDeviceId, this, qi::_1))]
        // domain-name/search
        // name-server
        // ntp
        // ...
      | ignoredBlock
    ) >>
    stopBlock
    ;
  // END OF: system {...}

  // START OF: applications {...}
  applications =
    qi::lit("applications")
      [(pnx::bind(&Parser::updateTgtZone, this, DEFAULT_ZONE))] >>
    startBlock >>
    *(  applicationSet
      | application
      | comment >> qi::eol //ignoredBlock
    ) >>
    stopBlock
    ;

  applicationSet =
    qi::lit("application-set") >> token
      [(pnx::bind(&Parser::updateBookName, this, qi::_1))] >>
    startBlock >>
    *( (qi::lit("application") >> token >> qi::eol)
         [(pnx::bind(&Parser::updateSrvcBookGroup, this, qi::_1))]
     | ignoredBlock
    ) >>
    stopBlock
    ;

  application =
    qi::lit("application") >> token
      [(pnx::bind(&Parser::updateBookName, this, qi::_1))] >>
      ( appMultiLine | appSingleLine | ignoredBlock )
        [(pnx::bind(&Parser::updateSrvcBook, this))]
    ;

  appMultiLine =
    startBlock >>
    *( (token >> token >> qi::eol)
         [(pnx::bind(&Parser::updateSrvcData, this, qi::_1, qi::_2))]
     | (appSingleLine)
         [(pnx::bind(&Parser::updateSrvcBook, this))]
    ) >>
    stopBlock
    ;

  appSingleLine =
    *(token >> token)
      [(pnx::bind(&Parser::updateSrvcData, this, qi::_1, qi::_2))] >>
    qi::eol
    ;
  // END OF: applications {...}

  // START OF: interfaces {...}
  interfaces =
    qi::lit("interfaces") >> startBlock >> *interface >> stopBlock
    ;

  interface =
    ( (qi::lit("interface-range") >> ignoredBlock)
        [(pnx::bind(&Parser::unsup, this, "interface-range"))]
    | (typeSlot >> startBlock >>
      *((  unit
         | (qi::lit("disable") >> semicolon)
              [(pnx::bind(&Parser::tgtIfaceUp, this) = false)]
         // description tokens
         | ignoredBlock
        ) >> -qi::eol
       ) >> stopBlock
          [(pnx::bind(&Parser::updateIfaceNameTypeState, this),
            pnx::bind(&Parser::tgtIfaceUp, this) = true)]
      )
    | ignoredBlock
    )
    ;

  typeSlot =
      (qi::as_string[+qi::ascii::alpha] >>
       qi::as_string[+qi::ascii::graph | qi::attr("")]
      ) [(pnx::bind(&Parser::updateIfaceTypeSlot, this, qi::_1, qi::_2))]
    ;

  unit =
    qi::lit("unit") >> qi::uint_
      [(pnx::bind(&Parser::updateIfaceUnit, this, qi::_1))] >>
    startBlock >>
    *((  family
       | (qi::lit("vlan-id") >> qi::ushort_ >> semicolon)
            [(pnx::bind(&Parser::addIfaceVlan, this, qi::_1))]
       | (qi::lit("disable") >> semicolon)
            [(pnx::bind(&Parser::tgtIfaceUp, this) = (tgtIfaceUp && false))]
       | ignoredBlock
      ) >> -qi::eol
     ) >> stopBlock [(pnx::bind(&Parser::tgtIfaceUp, this) = (tgtIfaceUp && true))]
    ;

  family =
    qi::lit("family") >> token >> startBlock >>
    *((  (qi::lit("address") >> ipAddr >> ((startBlock >> ignoredBlock >> stopBlock) | semicolon))
           [(pnx::bind(&Parser::addIfaceIpAddr, this, qi::_1))]
       | (qi::lit("port-mode") >> token)
           [(pnx::bind(&Parser::updateIfaceMode, this, "L2 " + qi::_1))]
       | (qi::lit("native-vlan-id") >> qi::ushort_ >> semicolon)
           [(pnx::bind(&Parser::addIfaceVlan, this, qi::_1))]
       | ifaceVlan
       | ignoredBlock
      ) >> -qi::eol
     ) >> stopBlock
    ;

  ifaceVlan =
    qi::lit("vlan") >> startBlock >>
    qi::lit("members") >> -qi::lit('[') >>
    +(!qi::lit(']') >>
     (  (qi::lit("all"))
          [(pnx::bind(&Parser::addIfaceVlanMembers, this, "all"))]
      | (qi::ushort_ >> qi::lit('-') >> qi::ushort_)
          [(pnx::bind(&Parser::addIfaceVlanRange, this, qi::_1, qi::_2))]
      | (qi::ushort_ )
          [(pnx::bind(&Parser::addIfaceVlan, this, qi::_1))]
      | token
          [(pnx::bind(&Parser::addIfaceVlanMembers, this, qi::_1))]
     )
    ) >> -qi::lit(']') >> semicolon >> qi::eol >> stopBlock
    ;
  // END OF: interfaces {...}

  // START OF: routing-options {...}
  routingOptions =
    qi::lit("routing-options") >> startBlock >>
    *((  routeStatic
       | ignoredBlock
      ) >> -qi::eol
     ) >> stopBlock
    ;
  routeStatic =
    qi::lit("static") >> startBlock >>
    *((  route
           [(pnx::bind(&Parser::addRoute, this, qi::_1))]
       | ignoredBlock
      ) >> -qi::eol
     ) >> stopBlock
    ;
  route =
    qi::lit("route") >> ipAddr
      [(  pnx::bind(&nmdo::Route::setDstIpNet, &qi::_val, qi::_1)
        , pnx::bind(&nmdo::Route::setNextHopIpAddr, &qi::_val
                   , pnx::bind([](nmdo::IpAddress& ip)
                                {
                                  if (ip.isV4()) {
                                    return nmdo::IpAddress::getIpv4Default();
                                  } else {
                                    return nmdo::IpAddress::getIpv6Default();
                                  }
                                }
                              , qi::_1
                              )
                   )
      )] >>
    qi::lit("next-hop") >>
    (  (ipAddr >> semicolon)
         [(pnx::bind(&nmdo::Route::setNextHopIpAddr, &qi::_val, qi::_1))]
     | token
         [(pnx::bind(&nmdo::Route::setOutIfaceName, &qi::_val, qi::_1))]
    ) >> qi::eol
    ;
  // END OF: routing-options {...}

  // START OF: security {...}
  security =
    qi::lit("security") >> startBlock >>
    *((  policies
       | zones
       | addressBook
       | ignoredBlock
      ) >> -qi::eol
     ) >> stopBlock
    ;

  policies =
    qi::lit("policies") >> startBlock >>
    *((  policyFromTo
       | ignoredBlock
      ) >> -qi::eol
     ) >> stopBlock
    ;

  policyFromTo =
    (qi::lit("from-zone") >> token >> qi::lit("to-zone") >> token)
       [(pnx::bind(&Parser::updateZones, this, qi::_1, qi::_2))] >>
    startBlock >>
    *((  policy
       | ignoredBlock
      ) >> -qi::eol
     ) >> stopBlock
         [(pnx::bind(&Parser::updateZones, this, DEFAULT_ZONE, DEFAULT_ZONE))]
    ;

  policy =
    qi::lit("policy") >> token
      [(pnx::bind(&Parser::updateCurRuleId, this, qi::_1))] >>
    startBlock >>
    *((  policyMatch
       | policyThen
       | ignoredBlock
      ) >> -qi::eol
     ) >> stopBlock
    ;

  policyMatch =
    qi::lit("match") >> startBlock >>
    *((  (token >> (tokenList | +token))
           [(pnx::bind(&Parser::updateRule, this, qi::_1, qi::_2))]
       | ignoredBlock
      ) >> -qi::eol
     ) >> stopBlock
    ;

  policyThen =
    qi::lit("then") >> startBlock >>
    *((  (+token >> qi::eol)
            [(pnx::bind(&Parser::updateRule, this, "action", qi::_1))]
       | logBlock
           [(pnx::bind(&Parser::updateRule, this, "action", qi::_1))]
       | (+token > startBlock > +ignoredBlock > stopBlock)
            [(pnx::bind(&Parser::updateRule, this, "action", qi::_1))]
       | (ignoredBlock)
      ) >> -qi::eol
     ) >> stopBlock
    ;

  logBlock =
    qi::string("log") >> startBlock >> +(token >> qi::eol) >> stopBlock
    ;

  zones =
    qi::lit("zones") >> startBlock >>
    *((  zone
       | ignoredBlock
      ) >> -qi::eol
     ) >> stopBlock
    ;

  zone =
    (qi::lit("security-zone") >> token)
      [(pnx::bind(&Parser::updateTgtZone, this, qi::_1))] >>
    startBlock >>
    *((  addressBook
       | zoneIface
       | ignoredBlock
      ) >> -qi::eol
     ) >> stopBlock
        [(pnx::bind(&Parser::updateTgtZone, this, DEFAULT_ZONE))]
    ;

  addressBook =
    qi::lit("address-book") >> startBlock >>
    (  (qi::hold[(!qi::lit("attach") >> token >> startBlock)]
          [(pnx::bind(&Parser::updateTgtZone, this, qi::_1))] >>
        addressBookData >> stopBlock
       )
     | addressBookData
    ) >> stopBlock
    ;

  addressBookData =
    *((  address
       | addressSet
       // attach { zone zone-name; }
       | (qi::lit("attach") >> startBlock >> +token >> -qi::eol >> stopBlock)
            [(pnx::bind(&Parser::unsup, this, "address-book attach zone"))]
       // description text;
       | ignoredBlock
      ) >> -qi::eol)
    ;

  address =
    qi::lit("address") >> token
      [(qi::_a = qi::_1)] >>
    (  (ipAddr >> semicolon >> qi::eol)
          [(pnx::bind(&Parser::updateNetBookIp, this, qi::_a, qi::_1))]
     | (startBlock >>
       *((  (ipAddr >> semicolon)
              [(pnx::bind(&Parser::updateNetBookIp, this, qi::_a, qi::_1))]
          // dns-name domain-name { ipv4-only; ipv6-only; }
          | (qi::lit("dns-name") >> fqdn >> -ignoredBlock)
              [(pnx::bind(&Parser::updateNetBookStr, this, qi::_a, qi::_1))]
          // range-address lower-limit to upper-limit;
          | (qi::lit("range-address") >> token >> qi::lit("to") >> token)
              [(pnx::bind(&Parser::updateNetBookStr, this, qi::_a,
                          qi::_1+"-"+qi::_2))]
          // wildcard-address ipv4-address/wildcard-mask;
          | (qi::lit("wildcard-address") >> token)
              [(pnx::bind(&Parser::updateNetBookStr, this, qi::_a, qi::_1))]
          // description text;
          | (ignoredBlock)
         ) >> -qi::eol
        ) >> stopBlock)
    )
    ;

  addressSet =
    qi::lit("address-set") >> token
      [(qi::_a = qi::_1)] >>
    startBlock >>
      *((  (qi::lexeme[qi::lit("address") >> -qi::lit("-set")] >> token)
              [(pnx::bind(&Parser::updateNetBookGroup, this, qi::_a, qi::_1))]
         // description text;
         | (ignoredBlock)
        ) >> -qi::eol
       ) >> stopBlock
    ;

  zoneIface =
    qi::lit("interfaces") >> startBlock >>
    *((  (token >> -(startBlock >> ignoredBlock >> stopBlock))
           [(pnx::bind(&Parser::addZoneIface, this, qi::_1))]
       | ignoredBlock
      ) >> -qi::eol
     ) >> stopBlock
    ;
  // END OF: security {...}

  // START OF: groups {...}
  groups =
    qi::lit("groups") >> startBlock >>
    *((  group
       | ignoredBlock
      ) >> -qi::eol
     ) >> stopBlock
    ;

  group =
    (token >> startBlock)
      [(pnx::bind(&Parser::addGroup, this, qi::_1, qi::_1))] >>
    *((  system
       | applications
       | interfaces
       | routingOptions
       | security
       | ignoredBlock
      ) >> -qi::eol
    ) >> stopBlock
      [(pnx::bind(&Parser::resetDevice, this))]
    ;
  // END OF: groups {...}

  // START OF: vlans {...}
  vlans =
    qi::lit("vlans") >> startBlock >>
    *((  vlan
       | ignoredBlock
      ) >> -qi::eol
    ) >> stopBlock
    ;

  vlan =
    token
      [(qi::_a = qi::_1,
        pnx::bind(&Parser::updateVlanDescription, this, qi::_a, qi::_a))] >>
    startBlock >>
    *((  (qi::lit("vlan-id") >> qi::ushort_)
           [(pnx::bind(&Parser::addVlan, this, qi::_a, qi::_1))]
       | (qi::lit("l3-interface") >> token >> -startBlock)
           [(pnx::bind(&Parser::tgtIfaceName, this) = qi::_1,
             pnx::bind(&Parser::updateIfaceMode, this, "L3"))]
       | (qi::lit("interface") >> token)
           [(pnx::bind(&Parser::tgtIfaceName, this) = qi::_1,
             pnx::bind(&Parser::unsup, this, "vlans{vlan{interface}}"))]
       | (qi::lit("description") >> token)
           [(pnx::bind(&Parser::updateVlanDescription, this, qi::_a, qi::_1))]
       | ignoredBlock
       // https://www.juniper.net/documentation/en_US/junos/topics/topic-map/bridging-and-vlans.html#id-configuring-a-vlan
         // domain-type
         // vlan-id-list [ vlan-id-numbers ]
         // vlan-tags
      ) >> -qi::eol
    ) >> stopBlock
    ;
  // END OF: vlans {...}


  // START OF: logical-systems {...}
  logicalSystems =
    qi::lit("logical-systems") >> startBlock >>
    *((logicalSystem | ignoredBlock) >> -qi::eol) >>
    stopBlock
    ;

  logicalSystem =
    (token >> startBlock)
      [(pnx::bind(&Parser::addDevice, this, qi::_1, "logical-system"))] >>
    *((  applications
       | interfaces
       | routingOptions
       | security
       | ignoredBlock
      ) >> -qi::eol
     ) >> stopBlock
    [(pnx::bind(&Parser::resetDevice, this))]
    ;
  // END OF: logical-systems {...}


  // START OF: routing-instances {...}
  routingInstances =
    qi::lit("routing-instances") >> startBlock >>
    *((routingInstance | ignoredBlock) >> -qi::eol) >>
    stopBlock
    ;

  routingInstance =
    (token >> startBlock)
      [(pnx::bind(&Parser::addDevice, this, qi::_1, "routing-instance"))] >>
    *((  (qi::lit("interface") > token > qi::eol)
           [(pnx::bind(&Parser::addInvalidIface, this, qi::_1))]
       | routingOptions
       | ignoredBlock
      ) >> -qi::eol
     ) >> stopBlock
    [(pnx::bind(&Parser::resetDevice, this))]
    ;
  // END OF: routing-instances {...}


  ignoredBlock =
      (+(token - startBlock) >> startBlock >> *ignoredBlock >> stopBlock)
    | +token >> qi::eol
    | comment >> qi::eol
    | +qi::eol >> "{" >> token >> "}" >> +qi::eol
    | +qi::eol
    ;

  garbageLine =
    +(qi::char_ - qi::eol) > -qi::eol
    ;

  token =
    (  (qi::lit('"') >>
        +(qi::lit("\\\"") | (qi::char_ - qi::char_('"'))) >>
        qi::lit('"'))
     | +(qi::ascii::graph - qi::char_(";{}#"))
    ) >> -semicolon
    ;

  tokenList =
    qi::lit("[") >> +(token - qi::lit("]")) >>
    qi::lit("]") >> semicolon
    ;

  comment =
    +qi::lit('#') >> *(qi::char_ - qi::eol)
    ;

  startBlock =
    qi::lit("{") >> -comment >> qi::eol
    ;

  stopBlock =
    qi::lit("}") >> qi::eol
    ;

  semicolon =
    qi::skip(qi::ascii::blank)[qi::lit(';') >> -comment]
    ;

  BOOST_SPIRIT_DEBUG_NODES(
    //(start)
    (config)
    (system)
    (applications)(application)(applicationSet)(appMultiLine)(appSingleLine)
    (interfaces)(interface)(typeSlot)(unit)(family)
    (routingOptions)(routeStatic)(route)
    (security)
    (policies)(policyFromTo)(policy)(policyMatch)(policyThen)(logBlock)
    (zones)(zone)(zoneIface)
    (addressBook)(addressBookData)(address)(addressSet)
    (groups)(group)
    (logicalSystems)(logicalSystem)
    (routingInstances)(routingInstance)
    (ignoredBlock)
    (token)(tokenList)(comment)
    (garbageLine)
    //(startBlock)(stopBlock)(semicolon)
    );
}


// =============================================================================
// Parser helper methods
// =============================================================================
// Device related
void
Parser::addDevice(const std::string& _devId, const std::string& _devType)
{
  devices.emplace_back();
  d = &devices.back();
  updateDeviceId(_devId);
  d->devInfo.setDeviceType(_devType);
}

void
Parser::addGroup(const std::string& _devId, const std::string& _devType)
{
  const std::regex namedGroup(
      "^(chassis|member|model|node|peers|routing-engine|time|re)[0-9].*"
      );
  if (std::regex_match(_devId, namedGroup)) {
    addDevice(_devId, _devType);
  } else {
    d = &deviceMetadata[_devId];
  }
}

void
Parser::updateDeviceId(const std::string& _devId)
{
  d->devInfo.setDeviceId(_devId);
}

void
Parser::resetDevice()
{
  d = &devices.front();
}

void
Parser::addRoute(nmdo::Route& _route)
{
  d->routes.push_back(_route);
}

void
Parser::addVlan(const std::string& _name, unsigned short _id)
{
  auto& vl {d->vlans[_name]};
  vl.setId(_id);
}

void
Parser::updateVlanDescription(const std::string& _name, const std::string& _desc)
{
  auto& vl {d->vlans[_name]};
  vl.setDescription(_desc);
}

// Interface related
void
Parser::updateIfaceTypeSlot(const std::string& _type, const std::string& _slot)
{
  tgtIfaceType = _type;
  tgtIfaceSlot = _slot;
  tgtIfaceName = _type + _slot;
}

void
Parser::updateIfaceNameTypeState()
{
  auto& iface {d->ifaces[tgtIfaceName]};
  iface.setName(tgtIfaceName);
  iface.setMediaType(tgtIfaceType);
  iface.setState(tgtIfaceUp);
}

void Parser::addInvalidIface(const std::string& _name) {
  d->ifaces[_name];
}

void
Parser::updateIfaceUnit(const size_t _unit)
{
  tgtIfaceUnit = _unit;

  std::ostringstream oss;
  oss << tgtIfaceType << tgtIfaceSlot << "." << tgtIfaceUnit;
  tgtIfaceName = oss.str();

  updateIfaceNameTypeState();
  updateIfaceMode("l2 access");
}

void
Parser::addIfaceIpAddr(const nmdo::IpAddress& ipAddr)
{
  d->ifaces[tgtIfaceName].addIpAddress(ipAddr);
}

void
Parser::addIfaceVlan(const uint16_t vlanId)
{
  d->ifaces[tgtIfaceName].addVlan(vlanId);
}

void
Parser::addIfaceVlanRange(const uint16_t start, const uint16_t end)
{
  d->ifaces[tgtIfaceName].addVlanRange(start, end);
}

void
Parser::addIfaceVlanMembers(const std::string& _vlanName)
{
  ifaceVlanMembers.emplace(tgtIfaceName, _vlanName);
}

void
Parser::updateIfaceMode(const std::string& _mode)
{
  d->ifaces[tgtIfaceName].setSwitchportMode(_mode);
}

void
Parser::addZoneIface(const std::string& _ifaceName)
{
  for (auto& [name, book] : d->ruleBooks) {
    auto delimPos {name.find("->")};
    const auto& prefix {name.substr(0, delimPos)};
    const auto& suffix {name.substr(delimPos+2)};

    if (prefix == tgtZone) {
      for (auto& [_, rule] : book) {
        rule.addSrcIface(_ifaceName);
      }
    }
    if (suffix == tgtZone) {
      for (auto& [_, rule] : book) {
        rule.addDstIface(_ifaceName);
      }
    }
  }
}


// Access control related
void
Parser::updateTgtZone(const std::string& _tgtZone)
{
  tgtZone = _tgtZone;
}

void
Parser::updateZones(const std::string& _srcZone,
                    const std::string& _dstZone)
{
  srcZone = _srcZone;
  dstZone = _dstZone;
  updateTgtZone(dstZone);
  updateBookName(srcZone+"->"+dstZone);
}

void
Parser::updateCurRuleId(const std::string& _ruleName)
{
  curRuleId = ruleIds[bookName]++;
  d->ruleBooks[bookName][curRuleId].setRuleId(curRuleId);
  d->ruleBooks[bookName][curRuleId].setRuleDescription(_ruleName);
}

void
Parser::updateBookName(const std::string& _bookName)
{
  bookName = _bookName;
  proto = "";
  srcPort = "";
  dstPort = "";
}

void
Parser::updateSrvcData(const std::string& _key, const std::string& _value)
{
  if ("protocol" == _key) {
    proto = _value;
  } else if ("source-port" == _key) {
    srcPort = _value;
  } else if ("destination-port" == _key) {
    dstPort = _value;
  } else if ("rpc-program-number" == _key) {
    if (!dstPort.empty()) {
      LOG_WARN << "Parser::UpdateSrvcData: rpc overwriting non-empty dstPort: "
               << nmcu::getSrvcString(proto, srcPort, dstPort) << std::endl;
    }
    proto += "-sun-rpc";
    srcPort = "";
    dstPort = _value;
  } else if ("uuid" == _key) {
    if (!dstPort.empty()) {
      LOG_WARN << "Parser::UpdateSrvcData: rpc overwriting non-empty dstPort: "
               << nmcu::getSrvcString(proto, srcPort, dstPort) << std::endl;
    }
    proto += "-ms-rpc";
    srcPort = "";
    dstPort = _value;
  }
}

void
Parser::updateSrvcBook()
{
  if (proto.empty() && srcPort.empty() && dstPort.empty()) {
    return;
  }
  const auto& srvcData {nmcu::getSrvcString(proto, srcPort, dstPort)};
  d->serviceBooks[tgtZone][bookName].addData(srvcData);
  proto = "";
  srcPort = "";
  dstPort = "";
}

void
Parser::updateSrvcBookGroup(const std::string& _bookOther)
{
  const auto& tgtData {d->serviceBooks[tgtZone][_bookOther].getData()};
  if (0 == tgtData.size()) {
    LOG_DEBUG << "Parser::updateSrvcBookGroup: set defined after use ["
              << tgtZone << "][" << bookName << "]: " << _bookOther << "\n";
    d->serviceBooks[tgtZone][bookName].addData(_bookOther);
  } else {
    d->serviceBooks[tgtZone][bookName].addData(tgtData);
  }
}

void
Parser::updateNetBookIp(const std::string& _bookName,
                        const nmdo::IpAddress& _ip)
{
  d->networkBooks[tgtZone][_bookName].addData(_ip.toString());
}

void
Parser::updateNetBookStr(const std::string& _bookName,
                         const std::string& _addr)
{
  d->networkBooks[tgtZone][_bookName].addData(_addr);
}

void
Parser::updateNetBookGroup(const std::string& _bookName,
                           const std::string& _bookOther)
{
  const auto& tgtData {d->networkBooks[tgtZone][_bookOther].getData()};
  if (0 == tgtData.size()) {
    LOG_DEBUG << "Parser::updateNetBookGroup: set defined after use ["
              << tgtZone << "][" << _bookName << "]: " << _bookOther << "\n";
    d->networkBooks[tgtZone][_bookName].addData(_bookOther);
  } else {
    d->networkBooks[tgtZone][_bookName].addData(tgtData);
  }
}


void
Parser::updateRule(const std::string& _key,
                   const std::vector<std::string>& _values)
{
  auto& rule {d->ruleBooks[bookName][curRuleId]};
  if ("source-address" == _key) {
    for (const auto& value : _values) {
      rule.addSrc(value);
    }
    rule.setSrcId(srcZone);
  } else if ("destination-address" == _key) {
    for (const auto& value : _values) {
      rule.addDst(value);
    }
    rule.setDstId(dstZone);
  } else if ("application" == _key) {
    for (const auto& value : _values) {
      rule.addService(value);
    }
  } else if ("action" == _key) {
    std::ostringstream oss;
    for (const auto& value : _values) {
      oss << value << " ";
    }
    auto value {oss.str()};
    value.pop_back();
    rule.addAction(value);
  } else {
    for (const auto& value : _values) {
      unsup("Parser::updateRule: unsupported key-value: " +
            _key + "--" + value);
    }
  }
}


void
Parser::unsup(const std::string& _value)
{
  d->observations.addUnsupportedFeature(_value);
}

Result
Parser::getData()
{
  for (auto& device : devices) {
    for (const auto& [nsi, nsb] : device.networkBooks) {
      for (const auto& [ns, nsd] : nsb) {
        nmdu::expanded(device.networkBooks, nsi, ns, DEFAULT_ZONE);
      }
    }
    for (const auto& [nsi, nsb] : device.serviceBooks) {
      for (const auto& [ns, nsd] : nsb) {
        nmdu::expanded(device.serviceBooks, nsi, ns, DEFAULT_ZONE);
      }
    }

    // all - iface gets all vlan ids
    // VLAN_NAME - iface gets VLAN_NAME ids
    for (const auto& [iface, members] : ifaceVlanMembers) {
      if ("all" == members) {
        for (const auto& [_, vl] : device.vlans) {
          device.ifaces[iface].addVlan(vl.getVlanId());
        }
        for (const auto& [_, vl] : devices.at(0).vlans) {
          device.ifaces[iface].addVlan(vl.getVlanId());
        }
      } else {
        const auto& searchLocal    {device.vlans.find(members)};
        const auto& searchDefault  {devices.at(0).vlans.find(members)};
        if (device.vlans.end() == searchLocal) {
          if (devices.at(0).vlans.end() == searchDefault) {
            device.observations.addNotable("Unknown VLAN member name: "
                                           + members);
          } else {
            device.ifaces[iface].addVlan(searchDefault->second.getVlanId());
          }
        } else {
          device.ifaces[iface].addVlan(searchLocal->second.getVlanId());
        }
      }
    }
  }

  bool first {true};
  auto& deviceRoot {devices.at(0)};
  for (auto& device : devices) {
    if (first) { first = false; continue; }

    for (auto& [name,iface] : device.ifaces) {
      const auto& search {deviceRoot.ifaces.find(name)};
      if (   search == device.ifaces.end()
          || iface.isValid())
      { continue; }

      device.ifaces[name] = deviceRoot.ifaces[name];
      deviceRoot.ifaces.erase(name);
    }
  }

  return devices;
}
