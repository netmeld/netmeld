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

#include "CiscoNamedBooks.hpp"

namespace nmdsic = netmeld::datastore::importers::cisco;

namespace netmeld::datastore::importers::cisco {
// =============================================================================
// Parser logic
// =============================================================================
CiscoNamedBooks::CiscoNamedBooks() : CiscoNamedBooks::base_type(start)
{
  using nmdsic::token;
  using nmdsic::tokens;
  using nmdsic::indent;

  start =
    config
    [pnx::bind(&CiscoNamedBooks::finalizeCurBook, this),
     qi::_val = pnx::bind(&CiscoNamedBooks::getData, this)]
    ;

  config =
    (  globalName
     | globalObjectGroup
     | globalObject
    )
    ;

  globalName =
    qi::lit("name")
    >> dataIp
    >> networkBookName
    > -description
    ;

  globalObjectGroup =
    (  objectGroupNetwork
     | objectGroupService
     | objectGroupProtocol
    )
    ;

	globalObject =
    (  objectNetwork
     | objectService
    )
    ;

  // object network val_name
  //  { host val_ip | subnet val_ip val_mask | range val_ip val_ip }
  objectNetwork =
    qi::lit("object network") > networkBookName > qi::eol
    >> *(indent
         >> (  objectNetworkHostLine
             | objectNetworkSubnetLine
             | objectNetworkRangeLine
             | objectNetworkNatLine
             | description
             | (&qi::eol) // space prefixed blank line
            )
       )
    ;
  objectNetworkHostLine =
    hostArgument > qi::eol
    ;
  objectNetworkSubnetLine =
    qi::lit("subnet") > (dataIpMask | dataIpPrefix) > qi::eol
    ;
  objectNetworkRangeLine =
    qi::lit("range") > dataIpRange > qi::eol
    ;
  objectNetworkNatLine =
    // nat (IN-IFACE,OUT-IFACE) (static|dynamic) OBJECT
    qi::lit("nat") > tokens > qi::eol
    ;


  // object-group *

  // object-group network val_name
  //  network-object { object val_name | host val_ip | val_ip val_mask}
  //  group-object val_name
  objectGroupNetwork =
    qi::lit("object-group network") > networkBookName > qi::eol
    >> *(indent
         > (  networkObjectLine
            | groupObjectLine
            | description
           )
        )
    ;
  networkObjectLine =
    qi::lit("network-object")
    > (  objectArgument
       | hostArgument
       | networkObjectMaskArgument
       | (dataIpMask | dataIpPrefix)
    ) > qi::eol
    ;
  objectArgument =
    qi::lit("object") > dataString
    ;
  networkObjectMaskArgument =
    (token >> ipAddr)
      [pnx::bind(&Parser::addPostMaskUpdate, this, qi::_1, qi::_2)]
    ;



  // object service val_name
  //  service { val_proto | icmp val_type | icmp6 val_type | { tcp | udp } [ source operator val_port ] [ destination operator val_port ]}
  // NOTE: operator = { eq | neq | lt | gt | range }
  objectService =
    (qi::lit("object service") >> serviceBookName > qi::eol)
    >> *(indent
         > (  objectServiceLine
            | description
           )
       )
    ;
//       // Ignore all other settings
//       | (qi::omit[+token])
//       | (&qi::eol) // space prefixed blank line
  objectServiceLine =
    qi::lit("service") > protocolArgument
    > -(sourceDestinationArgument | icmpTypeCode)
    > qi::eol [pnx::bind(&CiscoNamedBooks::finalizeService, this)]
    ;
  sourceDestinationArgument =
    qi::lit("source") > sourcePort > qi::lit("destination") > destinationPort
    ;

  // object-group service val_name { tcp | udp | tcp-udp }
  //  port-object { eq val_port | range val_start val_end }
  //  group-object val_name
  //  service-object { val_proto | icmp val_type | icmp6 val_type | { tcp | udp } [ source operator val_port ] [ destination operator val_port ]}
  // NOTE: operator = { eq | neq | lt | gt | range }
  objectGroupService =
    qi::lit("object-group service") >> serviceBookName > -protocolArgument > qi::eol
    >> *(indent
         > (  portObjectArgumentLine
            | groupObjectLine
            | serviceObjectLine
            | description
           )
    )
    ;
  portObjectArgumentLine =
    qi::lit("port-object") > unallocatedPort
    > qi::eol [pnx::bind(&CiscoNamedBooks::finalizeService, this)]
    ;
  serviceObjectLine =
    qi::lit("service-object")
    > (  (qi::lit("object") > token)
       | (protocolArgument > -(sourceDestinationArgument | icmpTypeCode)
      )
    > qi::eol [pnx::bind(&CiscoNamedBooks::finalizeService, this)]
    ;


  // object-group protocol val_name
  //  protocol-object val_proto
  //  group-object val_name
  objectGroupProtocol =
    qi::lit("object-group protocol") > serviceBookName > qi::eol
    >> *(indent
         > (  protocolObjectLine
            | groupObjectLine
            | description
           )
      )
    > qi::eol
    ;
  protocolObjectLine =
    qi::lit("protocol-object") > protocolArgument
    > qi::eol [pnx::bind(&CiscoNamedBooks::finalizeService, this)]
    ;





  //========
  // Helper piece-wise rules
  //========
  networkBookName =
    bookName [pnx::bind([&](const std::string& _name)
                        {curNetworkBook.setName(_name);}, qi::_1)]
    ;
  serviceBookName =
    bookName [pnx::bind([&](const std::string& _name)
                        {curServiceBook.setName(_name);}, qi::_1)]
    ;
  bookName =
    token
    ;

  description =
    qi::lit("description") > tokens > qi::eol
    ;

  hostArgument =
    qi::lit("host")
    > (dataIp | dataString)
    ;
  ipNoPrefix =
    (ipAddr.ipv4 | ipAddr.ipv6) >> !(qi::lit('/') >> ipAddr.prefix)
    ;
  dataIp =
    (&ipNoPrefix > ipAddr)
      [pnx::bind(&CiscoNamedBooks::addDataFromIp, this, qi::_1)]
    ;
  dataIpPrefix =
    (&((ipNoPrefix >> qi::eol) | !ipNoPrefix) >> ipAddr)
      [pnx::bind(&CiscoNamedBooks::addDataFromIp, this, qi::_1)]
    ;
  dataIpMask =
     (&ipNoPrefix >> ipAddr >> qi::omit[+qi::blank]
      >> !(&(qi::lit("0.0.0.0") | qi::lit("255.255.255.255")))
      >> &ipNoPrefix >> ipAddr)
       [pnx::bind(&CiscoNamedBooks::addDataFromIpMask, this, qi::_1, qi::_2)]
    ;
  dataIpRange =
    &(ipAddr >> ipAddr)
    > (token > token)
       [pnx::bind(&CiscoNamedBooks::addDataFromIpRange, this, qi::_1, qi::_2)]
    ;
  dataString =
    token [pnx::bind(&CiscoNamedBooks::addData, this, qi::_1)]
    ;

  protocolArgument =
    token [pnx::bind(&CiscoNamedBooks::curRuleProtocol, this) = qi::_1]
    ;

  icmpTypeCode = // TODO need to add this value
    qi::ushort_
      [qi::_pass = (0 <= qi::_1 && qi::_1 <= 255),
       pnx::bind(&CiscoNamedBooks::curRuleSrcPort, this) = qi::_1]
    >> -qi::short_
      [qi::_pass = (qi::_1 >= 0 && qi::_1 <= 255),
       pnx::bind(&CiscoNamedBooks::curRuleDstPort, this) = qi::_1]
    ;

  sourcePort =
    portArgument [pnx::bind(&CiscoNamedBooks::curRuleSrcPort, this) = qi::_1]
    ;
  destinationPort =
    portArgument [pnx::bind(&CiscoNamedBooks::curRuleDstPort, this) = qi::_1]
    ;
  unallocatedPort =
    portArgument [pnx::bind(&CiscoNamedBooks::curRuleUncPort, this) = qi::_1]
    ;
  portArgument =
    (  (qi::lit("eq") > token) [qi::_val = qi::_1]
     | (qi::lit("neq") > token) [qi::_val = "!" + qi::_1]
     | (qi::lit("lt") > token) [qi::_val = "<" + qi::_1]
     | (qi::lit("gt") > token) [qi::_val = ">" + qi::_1]
     | (qi::lit("range") > token > token) [qi::_val = (qi::_1 + "-" + qi::_2)]
    )
    ;

  groupObjectLine =
    qi::lit("group-object")
    > token [pnx::bind(&CiscoNamedBooks::addData, this, qi::_1)]
    ;


  BOOST_SPIRIT_DEBUG_NODES(
      //(start)
      (config)
      (sourcePort) (destinationPort)
      (icmpArgument)
        (icmpTypeCode) (icmpMessage)
      (bookName)
      (protocolArgument)
      (addressArgument) (addressArgumentIos) (mask)
      (portArgument)
      (dataIp) (dataIpMask) (dataIpPrefix)
        (ipNoPrefix)
      (ignoredRuleLine)
      //(token)(tokens)(indent)
      );
}

// =============================================================================
// Parser helper methods
// =============================================================================

void
CiscoNamedBooks::addData(const std::string& _data)
{
  curBook.addData(nmcu::trim(_data));
}

void
CiscoNamedBooks::addDataFromIp(const nmco::IpAddress& _ip)
{
  addData(_ip.toString());
}
void
CiscoNamedBooks::addDataFromIpMask(const nmco::IpAddress& _ip,
                                   const nmco::IpAddress& _mask)
{
  auto temp {_ip};
  temp.setMask(_mask);
  addData(temp.toString());
}
void
CiscoNamedBooks::addDataFromIpRange(const nmco::IpAddress& _start,
                                    const nmco::IpAddress& _end)
{
  std::ostringstream oss;
  oss << _start << "-" << _end;
  addData(oss.str());
}

void
CiscoNamedBooks::addPostMaskUpdate(const std::string& _otherBook,
                                   const nmco::IpAddress& _mask)
{
  if (0 == networkBooks.count(_otherBook)) {
    LOG_WARN << "CiscoNamedBooks:"
             << " Cannot apply mask (" << _mask << ")"
             << " to undefined book (" << _otherBook << ")"
             << '\n';
    return;
  }
  for (const auto& ip : networkBooks[_otherBook].getData()) {
    if (std::string::npos != ip.find('-')) {
      LOG_WARN << "CiscoNamedBooks:"
               << " Cannot apply mask (" << _mask << ")"
               << " to network range (" << ip << ")"
               << '\n';
      continue;
    }
    auto temp {ip};
    temp.setMask(_mask);
    addData(temp.toString());
  }
}

void
CiscoNameBooks::finalizeCurBook()
{
  networkBooks.emplace(tgtBook, curBook);
  curBook = nmco::AcNetworkBook();
}

// TODO finalizeService

std::string
CiscoNamedBooks::setMask(nmco::IpAddress& ipAddr, const nmco::IpAddress& mask)
{
  bool isContiguous {ipAddr.setMask(mask)};
  if (!isContiguous) {
    std::ostringstream oss;
    oss << "IpAddress (" << ipAddr
        << ") set with non-contiguous wildcard netmask (" << mask << ")";
  }

  return ipAddr.toString();
}

// Object return
Result
CiscoNamedBooks::getData()
{
  Result r(ruleBookName, ruleBook);
  return r;
}
}
