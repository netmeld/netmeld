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

#ifndef PARSER_HPP
#define PARSER_HPP

#include <netmeld/datastore/objects/PortRange.hpp>
#include <netmeld/datastore/parsers/ParserHelper.hpp>

namespace nmdo  = netmeld::datastore::objects;
namespace nmdp  = netmeld::datastore::parsers;

// =============================================================================
// Data containers
// =============================================================================
//typedef std::tuple<std::string, size_t, size_t>  Data;
struct Data {
  std::string     protocols;
  nmdo::PortRange portRange;

  auto operator<=>(const Data&) const = default;
  bool operator==(const Data&) const = default;
};
typedef std::vector<Data> Result;

// =============================================================================
// Parser definition
// =============================================================================
class Parser :
  public qi::grammar<nmdp::IstreamIter, Result(), qi::ascii::blank_type>
{
  // ===========================================================================
  // Variables
  // ===========================================================================
  protected:
    // Rules
    qi::rule<nmdp::IstreamIter, Result(), qi::ascii::blank_type>
        start
      ;

    qi::rule<nmdp::IstreamIter, Data(), qi::ascii::blank_type>
        line
      , portRange
      ;

    qi::rule<nmdp::IstreamIter, std::string()>
        protocol
      ;

    qi::rule<nmdp::IstreamIter, qi::ascii::blank_type>
        comment
      ;

  // ===========================================================================
  // Constructors
  // ===========================================================================
  public: // Constructor is only default and must be public
    Parser();
};
#endif // PARSER_HPP
