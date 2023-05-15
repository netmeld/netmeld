// =============================================================================
// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
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
  start = (
    systeminfo
    ) [(qi::_val = pnx::bind(&Parser::getData, this))]
  ;
  systeminfo =
      qi::lit("\r\nHost Name:") >> +(qi::char_ - qi::eol) [(qi::_val.host_name = qi::_1)]
      // qi::lit("OS Name:") >> +qi::blank >> +(qi::char_ - qi::eol) >>
      // qi::lit("OS Version:") >> +qi::blank >> +(qi::char_ - qi::eol) >>
      // qi::lit("OS Manufacturer:" >> +qi::blank >> +(qi::char_ - qi::eol) >> qi::eol) >>
      // qi::lit("OS Configuration:" >> +qi::blank >> +(qi::char_ - qi::eol) >> qi::eol) >>
      // qi::lit("OS Build Type:" >> +qi::blank >> +(qi::char_ - qi::eol) >> qi::eol) >>
      // qi::lit("Registered Owner:" >> +qi::blank >> +(qi::char_ - qi::eol) >> qi::eol) >>
      // qi::lit("Registered Organization:" >> +qi::blank >> +(qi::char_ - qi::eol) >> qi::eol) >>
      // qi::lit("Product ID:" >> +qi::blank >> +(qi::char_ - qi::eol) >> qi::eol) >>
      // qi::lit("Original Install Date:" >> +qi::blank >> +(qi::char_ - qi::eol) >> qi::eol) >>
      // qi::lit("System Boot Time:" >> +qi::blank >> +(qi::char_ - qi::eol) >> qi::eol) >>
      // qi::lit("System Manufacturer:" >> +qi::blank >> +(qi::char_ - qi::eol) >> qi::eol) >>
      // qi::lit("System Model:" >> +qi::blank >> +(qi::char_ - qi::eol) >> qi::eol) >>
      // qi::lit("System Type:" >> +qi::blank >> +(qi::char_ - qi::eol) >> qi::eol) >>
      // qi::lit("Processor(s):" >> +qi::blank >> +(qi::char_ - qi::eol) >> qi::eol) >>
      // qi::lit("Windows Directory:" >> +qi::blank >> +(qi::char_ - qi::eol) >> qi::eol) >>
      // qi::lit("System Directory:" >> +qi::blank >> +(qi::char_ - qi::eol) >> qi::eol) >>
      // qi::lit("Boot Device:" >> +qi::blank >> +(qi::char_ - qi::eol) >> qi::eol) >>
      // qi::lit("System Locale:" >> +qi::blank >> +(qi::char_ - qi::eol) >> qi::eol) >>
      // qi::lit("Input Locale:" >> +qi::blank >> +(qi::char_ - qi::eol) >> qi::eol) >>
      // qi::lit("Time Zone:" >> +qi::blank >> +(qi::char_ - qi::eol) >> qi::eol) >>
      // qi::lit("Total Physical Memory:" >> +qi::blank >> +(qi::char_ - qi::eol) >> qi::eol) >>
      // qi::lit("Available Physical Memory:" >> +qi::blank >> +(qi::char_ - qi::eol) >> qi::eol) >>
      // qi::lit("Virtual Memory: Max Size:" >> +qi::blank >> +(qi::char_ - qi::eol) >> qi::eol) >>
      // qi::lit("Virtual Memory: Available:" >> +qi::blank >> +(qi::char_ - qi::eol) >> qi::eol) >>
      // qi::lit("Virtual Memory: In Use:" >> +qi::blank >> +(qi::char_ - qi::eol) >> qi::eol) >>
      // qi::lit("Page File Location(s):" >> +qi::blank >> +(qi::char_ - qi::eol) >> qi::eol);
  ;

  ignoredLine =
    (qi::ascii::print > -qi::eol) | +qi::eol
  ;

  // Allows for error handling and debugging of qi.
  BOOST_SPIRIT_DEBUG_NODES(
      (start)
      (systeminfo)
      );
}

// =============================================================================
// Parser helper methods
// =============================================================================
// void
// Parser::addPackage(const nmdo::Package& packg)
// {
//   data.packages.push_back(packg);
// }

Result
Parser::getData()
{
  Result r;
  r.push_back(data);
  return r;
}
