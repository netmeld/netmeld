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
//        ((token > token)
//           [pnx::bind(&Parser::ifaceInit, this, qi::_2),
//            pnx::bind(&Parser::ifaceUpdateType, this, qi::_1)] >
//         startBlock > interface > stopBlock)
      | ignoredBlock
     ) > stopBlock
    ;

  interface =
    (token > token)
      [pnx::bind(&Parser::ifaceInit, this, qi::_2),
       pnx::bind(&Parser::ifaceUpdateType, this, qi::_1)] >
    startBlock >
    *(  (qi::lit("address") >
         (  ipAddr [pnx::bind(&Parser::ifaceAddIpAddr, this, qi::_1)]
          | token  [pnx::bind(&Parser::unsup, this, "address " + qi::_1)]
         )
        )
      | (qi::lit("description") > token)
          [pnx::bind(&Parser::ifaceUpdateDesc, this, qi::_1)]
      | ignoredBlock
    ) > stopBlock
    ;

  firewall =
    qi::lit("firewall") >> startBlock >
    *(  group
//      | ipv6rules
//      | ipv4rules
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
  tgtIfaceName = _name;
  auto& iface {d.ifaces[tgtIfaceName]};
  iface.setName(tgtIfaceName);
}

void
Parser::ifaceUpdateType(const std::string& _type)
{
  auto& iface {d.ifaces[tgtIfaceName]};
  iface.setMediaType(_type);
}

void
Parser::ifaceUpdateDesc(const std::string& _desc)
{
  auto& iface {d.ifaces[tgtIfaceName]};
  iface.setDescription(_desc);
}

void
Parser::ifaceAddIpAddr(nmco::IpAddress& _ipAddr)
{
  auto& iface {d.ifaces[tgtIfaceName]};
  iface.addIpAddress(_ipAddr);
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
