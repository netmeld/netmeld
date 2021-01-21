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
    *(pingLinux | pingWindows | ignoredLine)
      [qi::_val = pnx::bind(&Parser::getData, this)]
    ;


  // Linux
  pingLinux =
    linuxHeader > *linuxResponse > ignoredLine > linuxFooter
      [pnx::bind(&Parser::finalize, this)]
    ;

  linuxHeader =
    qi::lit("PING")
    >> (ipValue | hostname)
    >> qi::lit('(') >> -(hostname >> qi::lit('(')) >> ipValue >> +qi::lit(')')
    > +token > qi::eol
    ;

  linuxResponse =
    qi::uint_ >> qi::lit("bytes from")
    >> (  (ipValue)
        | (token >> qi::lit('(') >> ipValue >> qi::lit(')'))
       )
    >> qi::lit(": icmp") >> token >> qi::lit("ttl=") > +token > qi::eol
      [pnx::bind(&Parser::responsive, this) = true]
    ;

  linuxFooter =
    qi::lit("---") > //+token > qi::eol >
    +(+token > -qi::eol)
    ;


  // Windows
  pingWindows =
    windowsHeader > +windowsResponse > ignoredLine > windowsFooter
      [pnx::bind(&Parser::finalize, this)]
    ;

  windowsHeader =
    qi::lit("Pinging") > +token > qi::eol
    ;

  windowsResponse =
    qi::lit("Reply from")
    >> ipAddr
    >> (qi::lit(": bytes") | qi::lit(": time")) > +token > qi::eol
      [pnx::bind(&Parser::responsive, this) = true]
    ;

  windowsFooter =
    qi::lit("Ping statistics for") > //+token > qi::eol >
    +(+token > -qi::eol)
    ;


  // General
  ipValue =
    ipAddr
      [pnx::bind(&Parser::tgtIp, this) = qi::_1]
    >> -(qi::lit('%') >> ifaceName)
    ;

  hostname =
    domainName
      [pnx::bind([&](const std::string& val)
                   {tgtAliases.push_back(val);},
                 qi::_1)]
    ;

  ifaceName =
    +(  qi::ascii::alnum
      | qi::ascii::char_("-_.@")
     )
    ;

  token =
    +qi::ascii::graph
    ;

  ignoredLine =
    (+token > -qi::eol) | +qi::eol
    ;

  BOOST_SPIRIT_DEBUG_NODES(
      (start)
      (pingLinux)(linuxHeader)(linuxResponse)(linuxFooter)
      (pingWindows)(windowsHeader)(windowsResponse)(windowsFooter)
      (ignoredLine)
      (ipValue)(hostname)(ifaceName)
      //(token)
      );
}

// =============================================================================
// Parser helper methods
// =============================================================================
void
Parser::finalize()
{
  for (const auto& alias : tgtAliases) {
    tgtIp.addAlias(alias, REASON);
  }
  tgtIp.setResponding(responsive);
  data.push_back(tgtIp);
}

// Object return
Result
Parser::getData()
{
  Result r {data};
  return r;
}
