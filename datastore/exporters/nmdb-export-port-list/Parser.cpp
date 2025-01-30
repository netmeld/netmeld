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

#include "Parser.hpp"

// =============================================================================
// Parser logic
// =============================================================================
Parser::Parser() : Parser::base_type(start)
{
  start =
    *(line)
    ;

  line =
    ( (portRange >> -(comment) >> qi::eol)
    | (comment >> qi::eol)
    | (qi::eol)
    )
    ;

  const auto makeData =
      pnx::bind([](const std::string& a, const size_t b, const size_t c) {
                  Data d;
                  d.protocols = a;
                  if (0 == c) { d.portRange = nmdo::PortRange(b);     }
                  else        { d.portRange = nmdo::PortRange(b, c);  }
                  return d;
               }
               , qi::_1, qi::_2, qi::_3
               )
    ;
  portRange =
    ( (protocol >> qi::lit(':') >> qi::uint_ >> qi::lit('-') >> qi::uint_)
        [(qi::_val = makeData
        , qi::_pass = qi::_2 < qi::_3
        )]
    | (protocol >> qi::lit(':') >> qi::uint_ >> qi::attr(0))
        [qi::_val = makeData]
    )
    ;

  protocol =
    +(qi::char_("TUY"))
    ;

  comment =
    (qi::lit('#') >> *(qi::ascii::graph | qi::ascii::blank))
    ;

  BOOST_SPIRIT_DEBUG_NODES(
      (start)
      (line)
      (portRange)
      (protocol)
      (comment)
    );
}
