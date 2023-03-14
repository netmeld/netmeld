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

#include <netmeld/core/utils/StringUtilities.hpp>

#include "Parser.hpp"

// =============================================================================
// Parser logic
// =============================================================================
Parser::Parser() : Parser::base_type(start)
{
  start =
    (config) [(qi::_val = pnx::bind(&Parser::getData, this))]
    ;

  config =
    *( system
     | interfaces
     | firewall
//     | service
//     | port-forward
     | ignoredBlock
    )
    ;

  system =
    qi::lit("system") >> startBlock >
    *(  (qi::lit("host-name") > token)
          [(pnx::bind(&Parser::unsup, this, "host-name " + qi::_1))]
      | (qi::lit("domain-name") > fqdn)
          [(pnx::bind(&Parser::unsup, this, "domain-name " + qi::_1))]
      | (qi::lit("name-server") > ipAddr)
          [(pnx::bind(&Parser::serviceAddDns, this, qi::_1))]
      | (login)
//      | (ntp)
      | ignoredBlock
     ) > stopBlock
    ;

  login =
    qi::lit("login") >> startBlock >
    *(user [(pnx::bind(&Parser::noteCredentials, this))]) > stopBlock
    ;

  user =
    qi::lit("user") > token
      [(pnx::bind([&](const std::string& val){creds[1] = val;}, qi::_1))] >
    startBlock >
    *(  (qi::lit("level") > token > qi::eol)
           [(pnx::bind([&](const std::string& val){creds[0] = val;}, qi::_1))]
      | (qi::lit("authentication") > startBlock >
         *(  (qi::lit("encrypted-password") > token > qi::eol)
                [(pnx::bind([&](const std::string& val){creds[2] = val;}, qi::_1))]
           | (qi::lit("plaintext-password") > token > qi::eol)
                [(pnx::bind([&](const std::string& val){creds[3] = val;}, qi::_1))]
         ) > stopBlock)
    ) > stopBlock
    ;

  interfaces =
    qi::lit("interfaces") >> startBlock >
    *(  interface
      | ignoredBlock
     ) > stopBlock
    ;

  interface =
    (token > token)
      [(pnx::bind(&Parser::ifaceInit, this, qi::_2),
        pnx::bind(&nmdo::InterfaceNetwork::setMediaType,
                  pnx::bind(&Parser::tgtIface, this),
                  qi::_1))] >
    startBlock >
    *(  (qi::lit("address") >
         (  ipAddr
              [(pnx::bind(&nmdo::InterfaceNetwork::addIpAddress,
                          pnx::bind(&Parser::tgtIface, this),
                          qi::_1))]
          | token
              [(pnx::bind(&Parser::unsup, this, "address " + qi::_1))]
         )
        )
      | (qi::lit("description") > token)
          [(pnx::bind(&nmdo::InterfaceNetwork::setDescription,
                      pnx::bind(&Parser::tgtIface, this),
                      qi::_1))]
      | ifaceFirewall
      | ignoredBlock
    ) > stopBlock
    ;

  ifaceFirewall =
    qi::lit("firewall") >> startBlock >
    *( (qi::lit("out") > startBlock >
        *((qi::lit("ipv6-name") | qi::lit("name")) > token > qi::eol)
          [(pnx::bind(&Parser::ruleAddSrcIface, this, qi::_1))]
        > stopBlock)
     | ((qi::lit("in") | qi::lit("local")) > startBlock >
        *((qi::lit("ipv6-name") | qi::lit("name")) > token > qi::eol)
          [(pnx::bind(&Parser::ruleAddDstIface, this, qi::_1))]
        > stopBlock)
    ) > stopBlock
    ;

  firewall =
    qi::lit("firewall") >> startBlock >
    *(  group
      | ruleSets
      | ignoredBlock
    ) > stopBlock
    ;

  group =
    qi::lit("group") >> startBlock >
    *(  addressGroup
      | ignoredBlock
    ) > stopBlock
    ;

  addressGroup =
    qi::lit("address-group") > token
      [(pnx::bind(&Parser::tgtBook, this) = qi::_1)] >
    startBlock >
    *(  (qi::lit("address") > ipAddr)
          [(pnx::bind(&Parser::netBookAddAddr, this, qi::_1))]
      | ignoredBlock
    ) > stopBlock
    ;

  ruleSets =
    (qi::lit("ipv6-name")|qi::lit("name")) > token
      [(pnx::bind(&Parser::tgtZone, this) = qi::_1)] >
    startBlock >
    *(  rule
      | (qi::lit("default-action") > token)
          [(pnx::bind(&Parser::defaultAction, this) = qi::_1)]
      | (qi::lit("enable-default-log"))
      | ignoredBlock
    ) > stopBlock
    ;

  rule =
    qi::lit("rule") > qi::ulong_
      [(pnx::bind(&Parser::ruleInit, this, qi::_1))] >
    startBlock >
    *(  (qi::lit("action") > token)
          [(pnx::bind(&nmdo::AcRule::addAction,
                      pnx::bind(&Parser::tgtRule, this), qi::_1))]
      | (qi::lit("description") > token)
          [(pnx::bind(&nmdo::AcRule::setRuleDescription,
                      pnx::bind(&Parser::tgtRule, this), qi::_1))]
      | (qi::lit("state") > startBlock > +(token > qi::lit("enable") > qi::eol)
          [(pnx::bind([&](std::string val){states.push_back(val);},
                      qi::_1))] > stopBlock)
      | (qi::lit("protocol") > token)
          [(pnx::bind(&Parser::proto, this) = qi::_1)]
      | destination
      | source
      | (qi::lit("log") > token)
      | (qi::lit("disable"))
          [(pnx::bind(&nmdo::AcRule::disable,
                      pnx::bind(&Parser::tgtRule, this)))]
      | ignoredBlock
    ) > stopBlock
      [(pnx::bind(&Parser::ruleAddService, this))]
    ;

  destination =
    qi::lit("destination") > startBlock >
    *(  (qi::lit("port") > token)
          [(pnx::bind(&Parser::dstPort, this) = qi::_1)]
      | (qi::lit("group") > startBlock >
         qi::lit("address-group") > token
          [(pnx::bind(&nmdo::AcRule::addDst,
                      pnx::bind(&Parser::tgtRule, this), qi::_1))] >
         qi::eol > stopBlock)
      | (qi::lit("address") > token)
          [(pnx::bind(&nmdo::AcRule::addDst,
                      pnx::bind(&Parser::tgtRule, this), qi::_1))]
      | ignoredBlock
    ) > stopBlock
    ;

  source =
    qi::lit("source") > startBlock >
    *(  (qi::lit("port") > token)
          [(pnx::bind(&Parser::srcPort, this) = qi::_1)]
      | (qi::lit("group") > startBlock >
         qi::lit("address-group") > token
          [(pnx::bind(&nmdo::AcRule::addSrc,
                      pnx::bind(&Parser::tgtRule, this), qi::_1))] >
         qi::eol > stopBlock)
      | (qi::lit("address") > token)
          [(pnx::bind(&nmdo::AcRule::addSrc,
                      pnx::bind(&Parser::tgtRule, this), qi::_1))]
      | ignoredBlock
    ) > stopBlock
    ;

  ignoredBlock =
      (+(token - startBlock) >> startBlock > *ignoredBlock > stopBlock)
    | (+token > qi::eol)
    | (comment > qi::eol)
    | (+qi::eol)
    ;

  startBlock =
    qi::lit("{") > -comment > qi::eol
    ;

  stopBlock =
    qi::lit("}") > -qi::eol
    ;

  comment =
    +qi::lit('#') > *(qi::char_ - qi::eol)
    ;

  token =
     (qi::lit('"') > *(qi::lit("\\\"") | (qi::char_ - qi::char_('"'))) >
      qi::lit('"'))
    | +(qi::ascii::graph - qi::char_("{}#"))
    ;


  BOOST_SPIRIT_DEBUG_NODES(
      (start)
      (config)
      (system)(login)(user)
      (interfaces)(interface)(ifaceFirewall)
      (firewall)(group)(addressGroup)(ruleSets)(rule)(destination)(source)
      (startBlock)(stopBlock)(ignoredBlock)
      (token)(comment)
      );
}


// =============================================================================
// Parser helper methods
// =============================================================================
void
Parser::ifaceInit(const std::string& _name)
{
  tgtIface = &d.ifaces[_name];
  tgtIface->setName(_name);
}


void
Parser::serviceAddDns(const nmdo::IpAddress& _ipAddr)
{
  nmdo::Service service {"DNS", _ipAddr};
  service.addDstPort("53");
  service.setProtocol("udp");
  service.setServiceReason("VyOS device config");
  d.services.push_back(service);
}


void
Parser::netBookAddAddr(const nmdo::IpAddress& _ipAddr)
{
  d.networkBooks[tgtZone][tgtBook].addData(_ipAddr.toString());
}


void
Parser::ruleInit(size_t _ruleId)
{
  tgtRule = &d.ruleBooks[tgtZone][curRuleId];
  ++curRuleId;

  tgtRule->setRuleId(_ruleId);
  tgtRule->setDstId(DEFAULT_ZONE);
  tgtRule->setSrcId(DEFAULT_ZONE);
}

void
Parser::ruleAddDstIface(const std::string& _tgtZone)
{
  for (auto& [_, rbRule] : d.ruleBooks[_tgtZone]) {
    rbRule.addDstIface(tgtIface->getName());
  }
}

void
Parser::ruleAddSrcIface(const std::string& _tgtZone)
{
  for (auto& [_, rbRule] : d.ruleBooks[_tgtZone]) {
    rbRule.addSrcIface(tgtIface->getName());
  }
}

void
Parser::ruleAddService()
{
  if (proto.empty() && srcPort.empty() && dstPort.empty()) {
    tgtRule->addService("any");
  } else {
    tgtRule->addService(nmcu::getSrvcString(proto, srcPort, dstPort));
  }
  proto = srcPort = dstPort = "";

  if (!states.empty()) {
    tgtRule->addService("state:(" + nmcu::toString(states, ',') + ')');
  }
  states.clear();
}


void
Parser::noteCredentials()
{
  std::vector<std::string> temp(std::begin(creds), std::end(creds));
  d.observations.addNotable("Creds--" + nmcu::toString(temp, ':'));
}


void
Parser::unsup(const std::string& _value)
{
  d.observations.addUnsupportedFeature(_value);
}


Result
Parser::getData()
{
  for (auto& [zone, ruleBook] : d.ruleBooks) {
    for (auto& [ruleId, rbRule] : ruleBook) {
      if (0 == rbRule.getSrcs().size()) {
        rbRule.addSrc("any");
      }
      if (0 == rbRule.getSrcIfaces().size()) {
        rbRule.addSrcIface("any");
      }
      if (0 == rbRule.getDsts().size()) {
        rbRule.addDst("any");
      }
      if (0 == rbRule.getDstIfaces().size()) {
        rbRule.addDstIface("any");
      }
      if (0 == rbRule.getServices().size()) {
        rbRule.addService(nmcu::getSrvcString("all","",""));
      }
      if (0 == rbRule.getActions().size() && !defaultAction.empty()) {
        rbRule.addAction(defaultAction);
        defaultAction = "";
      }
    }
  }


  Result r;
  r.push_back(d);

  return r;
}
