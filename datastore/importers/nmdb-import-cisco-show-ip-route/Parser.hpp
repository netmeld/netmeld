// =============================================================================
// Copyright 2020 National Technology & Engineering Solutions of Sandia, LLC
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

#include <netmeld/datastore/objects/Route.hpp>
#include <netmeld/datastore/parsers/ParserIpAddress.hpp>

namespace nmdo = netmeld::datastore::objects;
namespace nmdp = netmeld::datastore::parsers;

// =============================================================================
// Data containers
// =============================================================================
typedef std::vector<nmdo::Route> Routes;

typedef std::pair<std::string, Routes> Vrf;
typedef std::vector<Vrf> Vrfs;

typedef Vrfs Result;


// =============================================================================
// Parser definition
// =============================================================================
class Parser:
  public qi::grammar<nmdp::IstreamIter, Result(), qi::ascii::blank_type>
{
  // ===========================================================================
  // Variables
  // ===========================================================================
  private:
    // Rules
    qi::rule<nmdp::IstreamIter, Result(), qi::ascii::blank_type>
      start;

    qi::rule<nmdp::IstreamIter, Vrf(), qi::ascii::blank_type>
      vrf;

    qi::rule<nmdp::IstreamIter, nmdo::Route(), qi::ascii::blank_type>
      ipv4Route,
      ipv6Route;

    qi::rule<nmdp::IstreamIter, nmdo::IpAddress(), qi::ascii::blank_type>
      dstIpv4Net, dstIpv4NetBlank,
      dstIpv6Net,
      rtrIpv4Addr,
      rtrIpv6Addr;

    qi::rule<nmdp::IstreamIter>
      codesLegend,
      ignoredLine;

    nmdp::ParserIpv4Address
      ipv4Addr;

    nmdp::ParserIpv6Address
      ipv6Addr;

    qi::rule<nmdp::IstreamIter, std::string(), qi::ascii::blank_type>
      vrfHeader,
      uptime,
      ifaceName;

    qi::rule<nmdp::IstreamIter, size_t()>
      distance,
      metric;

    qi::rule<nmdp::IstreamIter, std::string()>
      csvToken,
      typeCode, typeCodeBlank,
      token;

    std::string currProtocol;
    nmdo::IpAddress currDstIpNet;

  // ===========================================================================
  // Constructors
  // ===========================================================================
  public: // Constructor is only default and must be public
    Parser();

  // ===========================================================================
  // Methods
  // ===========================================================================
  private:
    void setCurrProtocol(const std::string&);
    std::string getCurrProtocol();
    void setCurrDstIpNet(const nmdo::IpAddress&);
    nmdo::IpAddress getCurrDstIpNet();
};

#endif // PARSER_HPP
