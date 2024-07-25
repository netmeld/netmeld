// =============================================================================
// Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC
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
#include <boost/algorithm/string.hpp>

#include "Parser.hpp"

namespace nmcu = netmeld::core::utils;


// =============================================================================
// Parser logic:
// See RFC 1035, 5395, and others for information relevant to this grammar.
// =============================================================================
Parser::Parser() : Parser::base_type(start)
{
  start =
    +dnsLookup
    ;

  // Helpers for object DnsLookup
  auto dnsLookupSetStatus =
      pnx::bind(&nmdo::DnsLookup::setStatus, &qi::_val, qi::_1);
  auto dnsLookupSetQuestion =
      pnx::bind(&nmdo::DnsLookup::setQuestion, &qi::_val, qi::_1);
  auto dnsLookupAddResponseSection =
      pnx::bind(&nmdo::DnsLookup::addResponseSection, &qi::_val, qi::_1);
  auto dnsLookupSetResolver =
      pnx::bind(&nmdo::DnsLookup::setResolver, &qi::_val, qi::_1);

  dnsLookup =
    *qi::eol
    >> -tryingHeader
    >> *qi::eol
    >> +(statusHeader [dnsLookupSetStatus]
       | comment
       )
    >> qi::eol
    >> -optPseudoSection
    >> questionSection [dnsLookupSetQuestion]
    >> *(responseSection [dnsLookupAddResponseSection])
    >> +(serverFooter [dnsLookupSetResolver]
       | receivedFooter [dnsLookupSetResolver]
       | comment
       )
    > *qi::eol
    ;

  optPseudoSection =
    qi::lit(";; OPT PSEUDOSECTION:")
    >> qi::eol
    >> // Need to consume single-semicolon lines,
    // but not the double-colon section header that
    // can follow without a separating blank line.
    *( qi::lit(";")
    >> (qi::char_ - qi::char_(";") - qi::eol)
    >> *(qi::char_ - qi::eol)
    >> qi::eol
    )
    > *qi::eol
    ;

  questionSection =
    qi::omit[questionSectionHeader]
    >> questionRecord
    > +qi::eol
    ;

  responseSection =
    (  responseSectionHeader
    >> responseRecords
    > +qi::eol
    ) [(qi::_val = pnx::construct<DnsResponseSection>(qi::_1, qi::_2))]
    ;

  responseRecords =
    *responseRecord
    ;

  tryingHeader =
    // Consume everything (including newlines) until a comment is reached
    qi::lit("Trying")
    >> +(qi::char_ - qi::char_(";"))
    ;

  statusHeader =
    qi::lit(";; ->>HEADER<<- opcode:")
    >> qi::omit[token]
    >> qi::lit(", status:")
    >> token
    >> qi::lit(", id:")
    >> qi::omit[token]
    > qi::eol
    ;

  questionSectionHeader =
    qi::lit(";;")
    >> qi::string("QUESTION")
    >> qi::lit("SECTION:")
    > qi::eol
    ;

  responseSectionHeader =
    qi::lit(";;")
    >> ( qi::string("ANSWER")
       | qi::string("AUTHORITY")
       | qi::string("ADDITIONAL")
       )
    >> qi::lit("SECTION:")
    > qi::eol
    ;

  // Helpers for object DnsQuestion
  auto dnsQuestionSetFqdn =
      pnx::bind(&nmco::DnsQuestion::setFqdn, &qi::_val, qi::_1);
  auto dnsQuestionSetClass =
      pnx::bind(&nmco::DnsQuestion::setClass, &qi::_val, qi::_1);
  auto dnsQuestionSetType =
      pnx::bind(&nmco::DnsQuestion::setType, &qi::_val, qi::_1);

  questionRecord =
    qi::lit(";")
    >> resourceFqdn [dnsQuestionSetFqdn]
    >> qi::omit[qi::lit(".")]
    >> resourceClass [dnsQuestionSetClass]
    >> resourceType [dnsQuestionSetType]
    > qi::eol
    ;

  // Helpers for object DnsResponse
  auto dnsResponseSetFqdn =
      pnx::bind(&nmco::DnsResponse::setFqdn, &qi::_val, qi::_1);
  auto dnsResponseSetTtl =
      pnx::bind(&nmco::DnsResponse::setTtl, &qi::_val, qi::_1);
  auto dnsResponseSetClass =
      pnx::bind(&nmco::DnsResponse::setClass, &qi::_val, qi::_1);
  auto dnsResponseSetType =
      pnx::bind(&nmco::DnsResponse::setType, &qi::_val, qi::_1);
  auto dnsResponseSetData =
      pnx::bind(&nmco::DnsResponse::setData, &qi::_val, qi::_1);

  responseRecord =
    ( (resourceFqdn >> qi::omit[qi::lit(".")]) [dnsResponseSetFqdn]
    | (qi::omit[qi::lit(".")])
    )
    >> resourceTtl [dnsResponseSetTtl]
    >> resourceClass [dnsResponseSetClass]
    >> resourceType [dnsResponseSetType]
    >> resourceData [dnsResponseSetData]
    > qi::eol
    ;

  // Helpers for object Port
  auto portSetIpAddr =
      pnx::bind(&nmdo::Port::setIpAddr, &qi::_val, qi::_1);
  auto portSetPort =
      pnx::bind(&nmdo::Port::setPort, &qi::_val, qi::_1);

  serverFooter =
    qi::lit(";; SERVER:")
    >> ipAddr [portSetIpAddr]
    >> qi::lit("#")
    >> qi::uint_ [portSetPort]
    >> qi::omit[+(qi::char_ - qi::eol)]
    > qi::eol
    ;

  receivedFooter =
    qi::lit(";; Received")
    >> qi::omit[qi::uint_]
    >> qi::lit("bytes from")
    >> ipAddr [portSetIpAddr]
    >> qi::lit("#")
    >> qi::uint_ [portSetPort]
    >> qi::omit[+(qi::char_ - qi::eol)]
    > qi::eol
    ;

  // CLASS or QCLASS
  resourceClass =
    ( qi::string("IN")  // Internet
    | qi::string("CH")  // Chaos
    | qi::string("CS")  // CSNET (obsolete)
    | qi::string("HS")  // Hesiod
    | qi::string("*")   // ANY class
    )
    ;

  // TYPE or QTYPE are upper case, digit, or dash.
  // https://en.wikipedia.org/wiki/List_of_DNS_record_types
  resourceType =
    ( (qi::char_("A-Z") >> *qi::char_("A-Z0-9-"))
    | qi::string("*")   // ANY type
    )
    ;

  // TTL
  resourceTtl =
    qi::uint_
    ;

  // RDATA
  resourceData =
    +(qi::char_ - qi::eol)
    ;

  comment =
    qi::lit(";")
    >> *(qi::char_ - qi::eol)
    > qi::eol
    ;

  token =
    +(qi::ascii::alnum)
    ;


  BOOST_SPIRIT_DEBUG_NODES(
    (start)
    (tryingHeader)
    (questionSectionHeader)(questionRecord)
    (responseSectionHeader)(responseRecord)
    (resourceClass)
    (resourceType)
    (resourceTtl)
    (resourceData)
    (statusHeader)
    (serverFooter)(receivedFooter)
    (comment)
    );
}


// =============================================================================
// Parser helper methods
// =============================================================================
