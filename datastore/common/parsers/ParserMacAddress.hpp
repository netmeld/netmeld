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

#ifndef PARSER_MAC_ADDRESS_HPP
#define PARSER_MAC_ADDRESS_HPP

#include <netmeld/datastore/objects/MacAddress.hpp>
#include <netmeld/datastore/parsers/ParserHelper.hpp>

namespace nmdo = netmeld::datastore::objects;


namespace netmeld::datastore::parsers {

  class ParserMacAddress :
    public qi::grammar<IstreamIter, nmdo::MacAddress()>
  {
    public:
      ParserMacAddress() : ParserMacAddress::base_type(start)
      {
        start =
            (macAddr8 | macAddr6)
              [qi::_val = pnx::construct<nmdo::MacAddress>(qi::_1)]
          ;

        macAddr6 =
            qi::hold[hexByte >> qi::repeat(5)[qi::lit(':') >> hexByte]]
          | qi::hold[hexByte >> qi::repeat(5)[qi::lit('-') >> hexByte]]
          | qi::hold[hexByte >> hexByte >>
                     qi::repeat(2)[qi::lit('.') >> hexByte >> hexByte]]
          ;

        macAddr8 =
            qi::hold[hexByte >> qi::repeat(7)[qi::lit(':') >> hexByte]]
          | qi::hold[hexByte >> qi::repeat(7)[qi::lit('-') >> hexByte]]
          | qi::hold[hexByte >> hexByte >>
                     qi::repeat(3)[qi::lit('.') >> hexByte >> hexByte]]
          ;

        BOOST_SPIRIT_DEBUG_NODES((start)(macAddr6)(macAddr8));
      }

      qi::rule<IstreamIter, nmdo::MacAddress()>
        start;

      qi::rule<IstreamIter, std::vector<uint8_t>>
        macAddr6, macAddr8;

      qi::uint_parser<uint8_t, 16, 2, 2>
        hexByte;
  };
}
#endif // PARSER_MAC_ADDRESS_HPP
