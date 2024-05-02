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

#include <netmeld/datastore/objects/AcRule.hpp>
#include <netmeld/datastore/objects/AcNetworkBook.hpp>
#include <netmeld/datastore/objects/AcServiceBook.hpp>
#include <netmeld/datastore/objects/InterfaceNetwork.hpp>
#include <netmeld/datastore/objects/Route.hpp>
#include <netmeld/datastore/parsers/ParserDomainName.hpp>
#include <netmeld/datastore/parsers/ParserIpAddress.hpp>
#include <netmeld/datastore/objects/ToolObservations.hpp>


namespace nmdo = netmeld::datastore::objects;
namespace nmdp = netmeld::datastore::parsers;

typedef std::map<std::string, nmdo::AcNetworkBook> NetworkBook;
typedef std::map<std::string, nmdo::AcServiceBook> ServiceBook;
typedef std::map<size_t, nmdo::AcRule> RuleBook;


// =============================================================================
// Data containers
// =============================================================================
struct Data
{
  std::map<std::string, nmdo::Route>  routes;

  std::map<std::string, nmdo::InterfaceNetwork>  ifaces;

  std::map<std::string, NetworkBook> networkBooks;
  std::map<std::string, ServiceBook> serviceBooks;
  std::map<std::string, RuleBook>    ruleBooks;

  nmdo::ToolObservations observations;

  auto operator<=>(const Data&) const = default;
  bool operator==(const Data&) const = default;
};
typedef std::vector<Data>  Result;


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

    qi::rule<nmdp::IstreamIter, qi::ascii::blank_type>
      config,
      interface, service, zone, address, group, policy,
      interfaces, routes;

    qi::rule<nmdp::IstreamIter, qi::ascii::blank_type, qi::locals<std::string>>
      route;

    qi::rule<nmdp::IstreamIter, std::vector<std::string>()>
      ifaceTypeName;

    qi::rule<nmdp::IstreamIter, std::string(), qi::ascii::blank_type>
      ipAddrOrFqdn,
      srvcSrcPort, srvcDstPort,
      vrouter;

    qi::rule<nmdp::IstreamIter, std::string()>
      token;

    nmdp::ParserIpAddress
      ipAddr;

    nmdp::ParserDomainName
      fqdn;

    Data d;

    std::string                         tgtIface;
    std::map<std::string, std::string>  zoneIfaceBook;

    const std::string DEFAULT_ZONE {"global"};

    std::string bookName {DEFAULT_ZONE};
    std::string tgtZone  {DEFAULT_ZONE};

    size_t curRuleId;

    const std::string DEFAULT_VRF_ID {""};//{"master"};


  // ===========================================================================
  // Constructors
  // ===========================================================================
  public: // Constructor is only default and must be public
    Parser();


  // ===========================================================================
  // Methods
  // ===========================================================================
  private:
    // Route related
    void setIfaceRoute(const std::string&, const nmdo::IpAddress&);
    void setIfaceGateway(const std::string&, const nmdo::IpAddress&);

    // Interface related
    void initIface(const std::string&, const std::string&);
    void disableIface();
    void updateIfaceIp(const nmdo::IpAddress&);
    void updateZoneIfaceBook(const std::string&);

    // Access control related
    void updateNetBook(const std::string&, const std::string&,
                       const std::string&);
    void updateNetBookGroup(const std::string&, const std::string&,
                            const std::string&);
    void updateSrvcBook(const std::string&, const std::string&);
    void updateSrvcBookGroup(const std::string&, const std::string&);
    void updateCurRuleId(const size_t);
    void disableRule();
    void updateRule(const std::string&, const std::string&);
    void addRule( const std::string&, const std::string&, const std::string&,
                  const std::string&, const std::string&,
                  const std::vector<std::string>&);

    // Unsupported
    void unsup(const std::string&);

    Result getData();
};
#endif // PARSER_HPP
