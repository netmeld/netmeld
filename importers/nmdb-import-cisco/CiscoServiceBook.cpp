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

#include "CiscoServiceBook.hpp"

namespace nmdsic = netmeld::datastore::importers::cisco;

namespace netmeld::datastore::importers::cisco {
// =============================================================================
// Parser logic
// =============================================================================
CiscoServiceBook::CiscoServiceBook() : CiscoServiceBook::base_type(start)
{
  using nmdsic::token;
  using nmdsic::tokens;
  using nmdsic::indent;

  start =
    config
      [pnx::bind(&CiscoServiceBook::finalizeCurBook, this),
       qi::_val = pnx::bind(&CiscoServiceBook::getData, this)]
    ;

  config =
    (  objectService 
     | objectGroupService
     | objectGroupProtocol
    )
    ;

  objectService =
    (qi::lit("object service ") >> bookName > qi::eol)
    >> *(indent
         > (  objectServiceLine
            | description
            | (qi::eol) // space prefixed blank line
           )
       )
    ;
  objectServiceLine =
    qi::lit("service ") > protocolArgument
    > -sourcePort
    > -destinationPort
    > -icmpArgument
    > qi::eol [pnx::bind(&CiscoServiceBook::addCurData, this)]
    ;

  objectGroupService =
    qi::lit("object-group service ") >> bookName > -protocolArgument > qi::eol
    >> *(indent
         > (  portObjectArgumentLine
            | serviceObjectLine
            | groupObjectLine
            | description
            | (qi::eol) // space prefixed blank line
           )
       )
    ;
  portObjectArgumentLine =
    qi::lit("port-object ") > unallocatedPort
    > qi::eol [pnx::bind(&CiscoServiceBook::addCurData, this)]
    ;
  serviceObjectLine =
    qi::lit("service-object ")
    > (  (objectArgument)
       | (protocolArgument > -sourcePort > -destinationPort > -icmpArgument)
      )
    > qi::eol [pnx::bind(&CiscoServiceBook::addCurData, this)]
    ;
  groupObjectLine =
    qi::lit("group-object ") > dataString > qi::eol
    ;


  objectGroupProtocol =
    qi::lit("object-group protocol ") > bookName > qi::eol
    >> *(indent
         > (  protocolObjectLine
            | groupObjectLine
            | description
            | (qi::eol) // space prefixed blank line
           )
       )
    ;
  protocolObjectLine =
    qi::lit("protocol-object ") > protocolArgument > qi::eol
      [pnx::bind(&CiscoServiceBook::addCurData, this)]
    ;


  //========
  // Helper piece-wise rules
  //========
  bookName =
    token [pnx::bind([&](const std::string& _name)
                     {curBook.setName(_name);}, qi::_1)]
    ;

  description =
    qi::lit("description ") > tokens > qi::eol
    ;


  protocolArgument =
    token [pnx::bind(&CiscoServiceBook::curProtocol, this) = qi::_1]
    ;

  sourcePort =
    (qi::lit("source ") > portArgument)
      [pnx::bind(&CiscoServiceBook::curSrcPort, this) = qi::_1]
    ;

  destinationPort =
    (qi::lit("destination ") > portArgument)
      [pnx::bind(&CiscoServiceBook::curDstPort, this) = qi::_1]
    ;

  icmpArgument =
    (icmpTypeCode | icmpMessage)
    ;
  icmpTypeCode =
    qi::as_string[+qi::digit]
      [pnx::bind(&CiscoServiceBook::curSrcPort, this) = qi::_1]
    > -(+qi::blank > -qi::as_string[+qi::digit]
      [pnx::bind(&CiscoServiceBook::curDstPort, this) = qi::_1])
    ;
  icmpMessage = // A token is too greedy, so...
    (  qi::string("administratively-prohibited")
     | qi::string("alternate-address")
     | qi::string("conversion-error")
     | qi::string("dod-host-prohibited")
     | qi::string("dod-net-prohibited")
     | qi::string("echo-reply")
     | qi::string("echo")
     | qi::string("general-parameter-problem")
     | qi::string("host-isolated")
     | qi::string("host-precedence-unreachable")
     | qi::string("host-redirect")
     | qi::string("host-tos-redirect")
     | qi::string("host-tos-unreachable")
     | qi::string("host-unknown")
     | qi::string("host-unreachable")
     | qi::string("information-reply")
     | qi::string("information-request")
     | qi::string("mask-reply")
     | qi::string("mask-request")
     | qi::string("mobile-redirect")
     | qi::string("net-redirect")
     | qi::string("net-tos-redirect")
     | qi::string("net-tos-unreachable")
     | qi::string("net-unreachable")
     | qi::string("network-unknown")
     | qi::string("no-room-for-option")
     | qi::string("option-missing")
     | qi::string("packet-too-big")
     | qi::string("parameter-problem")
     | qi::string("port-unreachable")
     | qi::string("precedence-unreachable")
     | qi::string("protocol-unreachable")
     | qi::string("reassembly-timeout")
     | qi::string("redirect")
     | qi::string("router-advertisement")
     | qi::string("router-solicitation")
     | qi::string("source-quench")
     | qi::string("source-route-failed")
     | qi::string("time-exceeded")
     | qi::string("timestamp-reply")
     | qi::string("timestamp-request")
     | qi::string("traceroute")
     | qi::string("ttl-exceeded")
     | qi::string("unreachable")
    ) [pnx::bind(&CiscoServiceBook::curDstPort, this) = qi::_1]
    ;

  unallocatedPort =
    portArgument
      [pnx::bind(&CiscoServiceBook::curSrcPort, this) = qi::_1,
       pnx::bind(&CiscoServiceBook::curDstPort, this) = qi::_1]
    ;
  portArgument =
    (  (qi::lit("eq ") > token) [qi::_val = qi::_1]
     | (qi::lit("neq ") > token) [qi::_val = "!" + qi::_1]
     | (qi::lit("lt ") > token) [qi::_val = "<" + qi::_1]
     | (qi::lit("gt ") > token) [qi::_val = ">" + qi::_1]
     | (qi::lit("range ") > token > token) [qi::_val = (qi::_1 + "-" + qi::_2)]
    )
    ;

  objectArgument =
    qi::lit("object ") > dataString
    ;

  dataString =
    token [pnx::bind(&CiscoServiceBook::addData, this, qi::_1)]
    ;




  BOOST_SPIRIT_DEBUG_NODES(
      //(start)
      (config)
      (objectService)
        (objectServiceLine)
      (objectGroupService)
        (portObjectArgumentLine)
        (serviceObjectLine)
      (objectGroupProtocol)
        (protocolObjectLine)
        (groupObjectLine)
      (bookName)
      (description)
      (protocolArgument)
      (icmpArgument)
        (icmpTypeCode)
        (icmpMessage)
      (sourcePort)
      (destinationPort)
      (unallocatedPort)
      (portArgument)
      (objectArgument)
      (dataString)
      //(token)(tokens)(indent)
      );
}

// =============================================================================
// Parser helper methods
// =============================================================================

void
CiscoServiceBook::addData(const std::string& _data)
{
  const auto& data {nmcu::trim(_data)};
  curBook.addData(data);
}

void
CiscoServiceBook::addCurData()
{
  const auto& serviceString
    {nmcu::getSrvcString(curProtocol, curSrcPort, curDstPort)};
  addData(serviceString);
  curSrcPort.clear();
  curDstPort.clear();
}

void
CiscoServiceBook::finalizeCurBook()
{
  const auto& tgtBook {curBook.getName()};
  if (0 == serviceBooks.count(tgtBook)) {
    serviceBooks.emplace(curBook.getName(), curBook);
  } else {
    auto& existingBook {serviceBooks.at(tgtBook)};
    for (auto& data : curBook.getData()) {
      existingBook.addData(data);
    }
  }
  curBook = nmco::AcServiceBook();
}


// Object return
ServiceBooks
CiscoServiceBook::getFinalVersion()
{
  ServiceBooks zoneBooks;
  zoneBooks.emplace(ZONE, serviceBooks);
  for (const auto& [zone, books] : zoneBooks) {
    for (const auto& [name, book] : books) {
      nmcu::expanded(zoneBooks, zone, name, ZONE);
    }
  }

  return zoneBooks;
}

ServiceBooks
CiscoServiceBook::getData()
{

  return ServiceBooks();
}

} // end of namespace
