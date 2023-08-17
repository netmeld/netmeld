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

#ifndef PARSER_HPP
#define PARSER_HPP

#include <netmeld/core/utils/StringUtilities.hpp>
#include <netmeld/datastore/objects/Route.hpp>
#include <netmeld/datastore/objects/ToolObservations.hpp>
#include <netmeld/datastore/parsers/ParserIpAddress.hpp>

namespace nmcu = netmeld::core::utils;
namespace nmdo = netmeld::datastore::objects;
namespace nmdp = netmeld::datastore::parsers;

// =============================================================================
// Data containers
// =============================================================================
struct Data {
  std::vector<nmdo::Route>  routes;
  nmdo::ToolObservations    observations;

  auto operator<=>(const Data&) const = default;
  bool operator==(const Data&) const = default;
};

typedef std::vector<Data> Result;


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
  protected:
    std::string currVrf;
    std::string currProtocol;

    nmdo::IpNetwork currDstIpNet;

    nmdo::ToolObservations currObservations;

    std::map<std::string, std::string> typeCodeLookups {
          {"%", ""} // next-hop override
        , {"*", ""} // candidate default
        , {"+", ""} // replicated route
        , {"A B", "bgp"}  // BGP Aggregate
        , {"A O", "ospf"} // OSPF Summary
        , {"B", "bgp"}
        , {"B E", "ebgp"}
        , {"B I", "ibgp"}
        , {"B L", "bgp"}  // BGP VRF leaked
        , {"C", "direct"}   // connected
        , {"D", "eigrp"}
        , {"D EX", "eigrp"} // EIGRP external
        , {"D*EX", "eigrp"} // EIGRP external
        , {"DH", "dhcp"}
        , {"DP", "dynamic"}   // dynamic policy route
        , {"E", "egp"}
        , {"E1", "ospf1"}
        , {"E2", "ospf2"}
        , {"EX", "eigrp"} // EIGRP external
        , {"G", "gribi"}
        , {"H", "nhrp"}
        , {"I", "igrp"}
        , {"IA", "ospf"}  // OSPF inter area
        , {"I L1", "is-is"}
        , {"I L2", "is-is"}
        , {"K", "direct"} // kernel
        , {"L", "local"}  // local; VRF leaked
        , {"L1", "is-is"} // see I L1
        , {"L2", "is-is"} // see I L2
        , {"M", "mobile-or-martian"}  // mobile; martian
        , {"N1", "ospf-nssa"} // OSPF NSSA external type 1
        , {"N2", "ospf-nssa"} // OSPF NSSA external type 2
        , {"NG", "static"}    // next-hop group static route
        , {"O3", "ospf3"}
        , {"O", "ospf"}
        , {"P", "static"} // periodic download static route
        , {"RC", "route-cache"} // route cache route
        , {"R", "rip"}
        , {"S", "static"}
        , {"S*", "static"}
        , {"U", "per-user-static"}
        , {"V", "vxlan"}  // VXLAN control service
        , {"a", "app"}    // application route
        , {"i", "is-is"}
        , {"ia", "is-is"} // IS-IS inter area
        , {"l", "lisp"}
        , {"o", "odr"}
        , {"p", ""} // overrides from PfR
        , {"su", "is-is"} // IS-IS summary
      };

    // Rules
    qi::rule<nmdp::IstreamIter, Result(), qi::ascii::blank_type>
        start
      ;

    qi::rule<nmdp::IstreamIter, Data(), qi::ascii::blank_type>
        vrf
      ;

    qi::rule<nmdp::IstreamIter, nmdo::Route(), qi::ascii::blank_type>
        ipv4Route
      , ipv6Route
      , ipv4RouteIos
      , ipv6RouteIos
      , ipv4RouteNxos
      , ipv6RouteNxos
      ;

    qi::rule<nmdp::IstreamIter, nmdo::IpNetwork(), qi::ascii::blank_type>
        ipv4Net
      , ipv6Net
      ;

    qi::rule<nmdp::IstreamIter>
        codesLegend
      , ignoredLine
      ;

    nmdp::ParserIpv4Address
        ipv4Addr
      ;

    nmdp::ParserIpv6Address
        ipv6Addr
      ;

    qi::rule<nmdp::IstreamIter, std::string(), qi::ascii::blank_type>
        vrfHeader
      , vrfHeaderIos
      , vrfHeaderNxos
      ;

    qi::rule<nmdp::IstreamIter, std::string()>
        csvToken
      , token
      ;

    qi::rule<nmdp::IstreamIter, qi::ascii::blank_type>
        uptime
      , ipv4RouteHeader
      ;

    qi::rule<nmdp::IstreamIter>
        vrfName
      ;

    qi::rule<nmdp::IstreamIter, void(nmdo::Route&), qi::ascii::blank_type>
        distanceMetric
      , ipv4TypeCodeDstIpNet
      , ipv6TypeCodeDstIpNet
      , ifaceName
      , rtrIpv4Addr
      , rtrIpv6Addr
      , dstIpv4Net
      , dstIpv6Net
      ;

    qi::rule<nmdp::IstreamIter, void(nmdo::Route&)>
        typeCode
      , type
      ;


  // ===========================================================================
  // Constructors
  // ===========================================================================
  public: // Constructor is only default and must be public
    Parser();

  // ===========================================================================
  // Methods
  // ===========================================================================
  private:
    void setCurrDstIpNet(const nmdo::IpNetwork&);
    void setCurrProtocol(const std::string&);
    void determineNullRoute(nmdo::Route&, const std::string&);
    void addRouteToData(Data&, nmdo::Route&);
    void addUnsupported(const std::string&);
    void finalizeVrfData(Data&);
    void updateProtocol(nmdo::Route&, const std::string&);
    void updateDstIpNet(nmdo::Route&, nmdo::IpNetwork&);

    nmdo::IpNetwork getCurrDstIpNet();
    std::string getCurrProtocol();
    std::string getTypeCodeValue(const std::string&);
};

#endif // PARSER_HPP
