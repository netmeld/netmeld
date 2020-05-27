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

#ifndef PARSER_IP_ADDRESS_HPP
#define PARSER_IP_ADDRESS_HPP

#include <netmeld/core/objects/IpAddress.hpp>
#include <netmeld/core/parsers/ParserHelper.hpp>

namespace nmco = netmeld::core::objects;


namespace netmeld::core::parsers {

  class ParserIpAddress :
    public qi::grammar<IstreamIter, nmco::IpAddress()>
  {
    public:
      ParserIpAddress() : ParserIpAddress::base_type(start)
      {
        start =
          qi::eps [qi::_val = pnx::construct<nmco::IpAddress>()] >>
          ( (qi::as_string[ipv4 | ipv6])
            [pnx::bind(&nmco::IpAddress::setAddress, &qi::_val, qi::_1)]
            >>
            -(qi::lit('/') >> prefix)
            [pnx::bind(&nmco::IpAddress::setPrefix, &qi::_val, qi::_1)]
          )
          ;

        ipv4 = // currently this expects chars, so octet can't return uints
          //    ____
          // 255.255
          qi::hold[octet >> qi::repeat(3)[qi::char_('.') >> octet]]
          ;

        octet = // unless changes occur in Boost, time waster for bounds checks
          (qi::repeat(1,3)[qi::ascii::digit])
          ;

        ipv6 =
          // ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
            qi::hold[h16 >> qi::repeat(7)[qi::hold[qi::char_(':') >> h16]]]
          // _____
          // ffff::ffff
          | qi::hold[qi::repeat(1,7)[qi::hold[h16 >> qi::char_(':')]] >>
                     qi::repeat(1,7)[qi::hold[qi::char_(':') >> h16]]]
          // _____
          // ffff::
          | qi::hold[qi::repeat(1,7)[qi::hold[h16 >> qi::char_(':')]] >>
                     qi::char_(':')]
          //  _____
          // ::ffff
          | qi::hold[qi::char_(':') >>
                     qi::repeat(1,7)[qi::hold[qi::char_(':') >> h16]]]
          // ::
          | qi::hold[qi::string("::")]
          ;

        h16 =
          (qi::repeat(1,4)[qi::ascii::xdigit])
          ;

        prefix %=
          qi::uint_ >> qi::eps[qi::_pass = (qi::_val >= 0 && qi::_val <= 128)]
          ;

        BOOST_SPIRIT_DEBUG_NODES((start)(ipv4)(ipv6)(prefix)(octet)(h16));
      }

      qi::rule<IstreamIter, nmco::IpAddress()>
        start;

      qi::rule<IstreamIter, std::string()>
        ipv4, ipv6,
        octet, h16;

      qi::rule<IstreamIter, unsigned int>
        prefix;
  };
}
#endif // PARSER_IP_ADDRESS_HPP
