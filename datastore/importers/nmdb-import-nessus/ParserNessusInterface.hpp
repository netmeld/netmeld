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

#ifndef PARSER_NESSUS_INTERFACE_HPP
#define PARSER_NESSUS_INTERFACE_HPP


#include <netmeld/core/objects/Interface.hpp>
#include <netmeld/core/parsers/ParserHelper.hpp>
#include <netmeld/core/parsers/ParserIpAddress.hpp>
#include <netmeld/core/parsers/ParserMacAddress.hpp>

namespace nmco = netmeld::core::objects;
namespace nmcp = netmeld::core::parsers;

typedef std::vector<nmco::Interface>       ResultPni;


namespace netmeld::core::parsers {

  class ParserNessusInterface :
    public qi::grammar<IstreamIter, ResultPni()>
  {
    public:
      ParserNessusInterface() : ParserNessusInterface::base_type(start)
      {
        start =
          *(qi::eol) >>
          *qi::omit[(token >> -qi::lit(' '))] >>
          *(qi::eol) >>
          *(macAddrLine | ipAddrLine)
          ;

        // TODO Why do none of these use the space skipper?
        macAddrLine =
          qi::lit("  - ") >> // two spaces before -
          macAddr [pnx::bind(&nmco::Interface::setMacAddress, &qi::_val, qi::_1)] >>
          (  (qi::lit(" (interface ") >> ifaceName
                [pnx::bind(&nmco::Interface::setName, &qi::_val, qi::_1)])
           | (qi::lit(" (interfaces ") >> token
                [pnx::bind(&nmco::Interface::setName, &qi::_val, qi::_1)] >>
              -qi::omit[+(qi::blank >> token)] >> qi::eol
             )
          )
          ;

        ipAddrLine =
          qi::lit(" - ") >> // one space before -
          ipAddr [pnx::bind(&nmco::Interface::addIpAddress, &qi::_val, qi::_1)] >>
          qi::lit(" (on interface ") >>
          ifaceName [pnx::bind(&nmco::Interface::setName, &qi::_val, qi::_1)]
          ;

        ifaceName =
          +(qi::char_ - qi::lit(")")) >>
          qi::lit(")") >>
          qi::eol
          ;

        token =
          +qi::ascii::graph
          ;

        BOOST_SPIRIT_DEBUG_NODES(
            (start)
            (ifaceName)
            (macAddrLine) (ipAddrLine)
            (token)
            );
      }

      qi::rule<IstreamIter, ResultPni()>
        start;

      qi::rule<IstreamIter, nmco::Interface()>
        macAddrLine,
        ipAddrLine;

      qi::rule<IstreamIter, std::string()>
        ifaceName,
        token;

      nmcp::ParserIpAddress
        ipAddr;

      nmcp::ParserMacAddress
        macAddr;
  };
}
#endif //PARSER_NESSUS_INTERFACE_HPP
