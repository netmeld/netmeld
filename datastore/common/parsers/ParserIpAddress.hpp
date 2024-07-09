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

#ifndef PARSER_IP_ADDRESS_HPP
#define PARSER_IP_ADDRESS_HPP

#include <netmeld/datastore/objects/IpAddress.hpp>
#include <netmeld/datastore/parsers/ParserHelper.hpp>

namespace nmdo = netmeld::datastore::objects;


namespace netmeld::datastore::parsers {

  class ParserIpv4Address :
    public qi::grammar<IstreamIter, nmdo::IpAddress()>
  {
    public:
      ParserIpv4Address() : ParserIpv4Address::base_type(start)
      {
        start =
          qi::eps [qi::_val = pnx::construct<nmdo::IpAddress>()]
          >> (  (qi::as_string[ipv4])
                [pnx::bind(&nmdo::IpAddress::setAddress, &qi::_val, qi::_1)]
             >> -(qi::lit('/') >> prefix)
                [pnx::bind(&nmdo::IpAddress::setPrefix, &qi::_val, qi::_1)]
             )
          ;

        ipv4 = // currently needs chars, so octet can't return uints
          qi::hold[octet >> qi::repeat(3)[qi::char_('.') >> octet]]
          ;

        octet %=
          qi::raw[qi::uint_ [qi::_pass = (qi::_1 <= 255)]]
          ;

        prefix %=
          qi::uint_ [qi::_pass = (qi::_1 <= 32)]
          ;

        BOOST_SPIRIT_DEBUG_NODES((start)(ipv4)(prefix));
      }

      qi::rule<IstreamIter, nmdo::IpAddress()>
        start;

      qi::rule<IstreamIter, std::string()>
          ipv4
        , octet
        ;

      qi::rule<IstreamIter, unsigned int()>
        prefix;
  };


  class ParserIpv6Address :
    public qi::grammar<IstreamIter, nmdo::IpAddress()>
  {
    protected:
      unsigned char h16Count  {0};
      bool          altForm   {false};

      ParserIpv4Address ipv4Addr;

    public:
      ParserIpv6Address() : ParserIpv6Address::base_type(start)
      {
        using namespace qi;

        start =
          qi::eps [qi::_val = pnx::construct<nmdo::IpAddress>()] >>
          ( (qi::as_string[ipv6])
            [pnx::bind(&nmdo::IpAddress::setAddress, &qi::_val, qi::_1)]
            >>
            -(qi::lit('/') >> prefix)
            [pnx::bind(&nmdo::IpAddress::setPrefix, &qi::_val, qi::_1)]
          )
          ;

        /*
        */
        //ms96
        ls32 = hold[(h16 >> char_(':') >> h16)] | ipv4Addr.ipv4;
        ipv6 =
          // 1:2:3:4:5:6:(7:8|1.2.3.4)
            hold[(repeat(6)[h16 >> char_(':')] >> ls32)]
          // ::1:2:3:4:(5:6|1.2.3.4)
          | hold[(string("::") >> repeat(4)[h16 >> char_(':')] >> ls32)]
          // ::1:2:3:(4:5|1.2.3.4)
          | hold[(string("::") >> repeat(3)[h16 >> char_(':')] >> ls32)]
          // ::1:2:(3:4|1.2.3.4)
          | hold[(string("::") >> repeat(2)[h16 >> char_(':')] >> ls32)]
          // ::1:(2:3|1.2.3.4)
          | hold[(string("::") >> h16 >> char_(':') >> ls32)]
          // ::(1:2|1.2.3.4)
          | hold[(string("::") >> ls32)]
          // ::1
          | hold[(string("::") >> h16)]
          // ::
          | hold[string("::")]
          // 1::2:3:4:(5:6|1.2.3.4)
          | hold[(h16 >> string("::") >> repeat(3)[h16 >> char_(':')] >> ls32)]
          // (1|1:2)::3:4:(5:6|1.2.3.4)
          | hold[(repeat(1,2)[h16 >> char_(':')] >> char_(':') >> repeat(2)[h16 >> char_(':')] >> ls32)]
          // (1|1:2|1:2:3)::4:(5:6|1.2.3.4)
          | hold[(repeat(1,3)[h16 >> char_(':')] >> char_(':') >> h16 >> char_(':') >> ls32)]
          // (1|1:2|1:2:3|1:2:3:4)::(5:6|1.2.3.4)
          | hold[(repeat(1,4)[h16 >> char_(':')] >> char_(':') >> ls32)]
          // (1|1:2|1:2:3|1:2:3:4|1:2:3:4:5)::6
          | hold[(repeat(1,5)[h16 >> char_(':')] >> char_(':') >> h16)]
          // (1|1:2|1:2:3|1:2:3:4|1:2:3:4:5|1:2:3:4:5:6)::
          | hold[(repeat(1,6)[h16 >> char_(':')] >> char_(':'))]
          ;

        h16 %=
          (qi::repeat(1,4)[qi::ascii::xdigit]) [pnx::ref(h16Count)++]
          ;

        prefix %=
          qi::uint_ [qi::_pass = (qi::_val <= 128)]
          ;

        BOOST_SPIRIT_DEBUG_NODES(
            (start)
            (ipv6)
            (prefix)
            //(h16)
          );
      }

      qi::rule<IstreamIter, nmdo::IpAddress()>
        start;

      qi::rule<IstreamIter, std::string()>
          ipv6
        , h16
        , ls32
        ;

      qi::rule<IstreamIter, unsigned int>
        prefix;

      qi::rule<IstreamIter>
        resetConstraints;
  };


  class ParserIpAddress :
    public qi::grammar<IstreamIter, nmdo::IpAddress()>
  {
    public:
      ParserIpAddress() : ParserIpAddress::base_type(start)
      {
        start = (ipv4Addr | ipv6Addr)
          ;

        BOOST_SPIRIT_DEBUG_NODES((start));
      }

      qi::rule<IstreamIter, nmdo::IpAddress()>
        start;

      ParserIpv4Address
        ipv4Addr;

      ParserIpv6Address
        ipv6Addr;
  };
}
#endif // PARSER_IP_ADDRESS_HPP
