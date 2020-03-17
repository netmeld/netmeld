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

#include "Parser.hpp"

// =============================================================================
// Parser logic
// =============================================================================
Parser::Parser() : Parser::base_type(start)
{
  start =
    (config) [qi::_val = pnx::bind(&Parser::getData, this)]
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
          [pnx::bind(&Parser::unsup, this, "host-name " + qi::_1)]
      | (qi::lit("domain-name") > fqdn)
          [pnx::bind(&Parser::unsup, this, "domain-name " + qi::_1)]
      | (qi::lit("name-server") > ipAddr)
          [pnx::bind(&Parser::serviceAddDns, this, qi::_1)]
//      | (login)
//      | (ntp)
      | ignoredBlock
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
      [pnx::bind(&Parser::ifaceInit, this, qi::_2),
       pnx::bind(&nmco::InterfaceNetwork::setMediaType,
                 pnx::bind(&Parser::tgtIface, this),
                 qi::_1)] >
    startBlock >
    *(  (qi::lit("address") >
         (  ipAddr
              [pnx::bind(&nmco::InterfaceNetwork::addIpAddress,
                         pnx::bind(&Parser::tgtIface, this),
                         qi::_1)]
          | token
              [pnx::bind(&Parser::unsup, this, "address " + qi::_1)]
         )
        )
      | (qi::lit("description") > token)
          [pnx::bind(&nmco::InterfaceNetwork::setDescription,
                     pnx::bind(&Parser::tgtIface, this),
                     qi::_1)]
      | ifaceFirewall
      | ignoredBlock
    ) > stopBlock
    ;

  ifaceFirewall =
    qi::lit("firewall") >> startBlock >
    *( (qi::lit("in") > startBlock >
        *((qi::lit("ipv6-name") | qi::lit("name")) > token > qi::eol)
          [pnx::bind(&Parser::ruleAddSrcIface, this, qi::_1)]
        > stopBlock)
     | ((qi::lit("out") | qi::lit("local")) > startBlock >
        *((qi::lit("ipv6-name") | qi::lit("name")) > token > qi::eol)
          [pnx::bind(&Parser::ruleAddDstIface, this, qi::_1)]
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
      [pnx::bind(&Parser::tgtBook, this) = qi::_1] >
    startBlock >
    *(  (qi::lit("address") > ipAddr)
          [pnx::bind(&Parser::netBookAddAddr, this, qi::_1)]
      | ignoredBlock
    ) > stopBlock
    ;

  ruleSets =
    (qi::lit("ipv6-name")|qi::lit("name")) > token
      [pnx::bind(&Parser::tgtZone, this) = qi::_1] >
    startBlock >
    *(  rule
      | (qi::lit("default-action") > token)
      | (qi::lit("enable-default-log"))
      | ignoredBlock
    ) > stopBlock
    ;

  rule =
    qi::lit("rule") > qi::ulong_
      [pnx::bind(&Parser::ruleInit, this, qi::_1)] >
    startBlock >
    *(  (qi::lit("action") > token)
          [pnx::bind(&nmco::AcRule::addAction,
                     pnx::bind(&Parser::tgtRule, this), qi::_1)]
      | (qi::lit("description") > token)
          [pnx::bind(&nmco::AcRule::setRuleDescription,
                     pnx::bind(&Parser::tgtRule, this), qi::_1)]
//      | (qi::lit("state") > startBlock > *() > stopBlock)
      | (qi::lit("protocol") > token)
          [pnx::bind(&Parser::proto, this) = qi::_1]
      | destination
      | source
      | (qi::lit("log") > token)
      | (qi::lit("disable"))
          [pnx::bind(&nmco::AcRule::disable,
                     pnx::bind(&Parser::tgtRule, this))]
      | ignoredBlock
    ) > stopBlock
    ;

  destination =
    qi::lit("destination") > startBlock >
    *(  (qi::lit("port") > token)
          [pnx::bind(&Parser::dstPort, this) = qi::_1]
      | (qi::lit("group") > startBlock >
         qi::lit("address-group") > token
          [pnx::bind(&nmco::AcRule::addDst,
                     pnx::bind(&Parser::tgtRule, this), qi::_1)] >
         qi::eol > stopBlock)
      | (qi::lit("address") > token)
          [pnx::bind(&nmco::AcRule::addDst,
                     pnx::bind(&Parser::tgtRule, this), qi::_1)]
      | ignoredBlock
    ) > stopBlock
    ;

  source =
    qi::lit("source") > startBlock >
    *(  (qi::lit("port") > token)
          [pnx::bind(&Parser::srcPort, this) = qi::_1]
      | (qi::lit("group") > startBlock >
         qi::lit("address-group") > token
          [pnx::bind(&nmco::AcRule::addSrc,
                     pnx::bind(&Parser::tgtRule, this), qi::_1)] >
         qi::eol > stopBlock)
      | (qi::lit("address") > token)
          [pnx::bind(&nmco::AcRule::addSrc,
                     pnx::bind(&Parser::tgtRule, this), qi::_1)]
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
      (system)
      (interfaces)(interface)
      (firewall)(group)(addressGroup)(ruleSets)(rule)(destination)
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
Parser::serviceAddDns(const nmco::IpAddress& _ipAddr)
{
  nmco::Service service {"DNS", _ipAddr};
  service.addDstPort("53");
  service.setProtocol("udp");
  service.setServiceReason("VyOS device config");
  d.services.push_back(service);
}


void
Parser::netBookAddAddr(const nmco::IpAddress& _ipAddr)
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
  for (auto& [ruleId, rule] : d.ruleBooks[_tgtZone]) {
    rule.addDstIface(tgtIface->getName());
  }
}

void
Parser::ruleAddSrcIface(const std::string& _tgtZone)
{
  for (auto& [ruleId, rule] : d.ruleBooks[_tgtZone]) {
    rule.addSrcIface(tgtIface->getName());
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
  Result r;
  r.push_back(d);

  return r;
}
