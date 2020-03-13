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

#ifndef PARSER_HPP
#define PARSER_HPP

#include <netmeld/core/objects/AcRule.hpp>
#include <netmeld/core/objects/AcNetworkBook.hpp>
#include <netmeld/core/objects/AcServiceBook.hpp>
#include <netmeld/core/objects/InterfaceNetwork.hpp>
#include <netmeld/core/objects/Route.hpp>
#include <netmeld/core/parsers/ParserDomainName.hpp>
#include <netmeld/core/parsers/ParserIpAddress.hpp>
#include <netmeld/core/objects/ToolObservations.hpp>


namespace nmco = netmeld::core::objects;
namespace nmcp = netmeld::core::parsers;

typedef std::map<std::string, nmco::AcNetworkBook> NetworkBook;
typedef std::map<std::string, nmco::AcServiceBook> ServiceBook;
typedef std::map<size_t, nmco::AcRule> RuleBook;


// =============================================================================
// Data containers
// =============================================================================
struct Data
{
  std::map<std::string, nmco::Route>  routes;

  std::map<std::string, nmco::InterfaceNetwork>  ifaces;

  std::map<std::string, NetworkBook> networkBooks;
  std::map<std::string, ServiceBook> serviceBooks;
  std::map<std::string, RuleBook>    ruleBooks;

  nmco::ToolObservations observations;
};
typedef std::vector<Data>  Result;


// =============================================================================
// Parser definition
// =============================================================================
class Parser:
  public qi::grammar<nmcp::IstreamIter, Result(), qi::ascii::blank_type>
{
  // ===========================================================================
  // Variables
  // ===========================================================================
  private:
    // Rules
    qi::rule<nmcp::IstreamIter, Result(), qi::ascii::blank_type>
      start;

    qi::rule<nmcp::IstreamIter, qi::ascii::blank_type>
      config,
      interface, service, zone, address, group, policy,
      interfaces, routes;

    qi::rule<nmcp::IstreamIter, qi::ascii::blank_type, qi::locals<std::string>>
      route;

    qi::rule<nmcp::IstreamIter, std::vector<std::string>()>
      ifaceTypeName;

    qi::rule<nmcp::IstreamIter, std::string(), qi::ascii::blank_type>
      ipAddrOrFqdn,
      srvcSrcPort, srvcDstPort,
      vrouter;

    qi::rule<nmcp::IstreamIter, std::string()>
      token;

    nmcp::ParserIpAddress
      ipAddr;

    nmcp::ParserDomainName
      fqdn;

    Data d;

    std::string                         tgtIface;
    std::map<std::string, std::string>  zoneIfaceBook;

    const std::string DEFAULT_ZONE = "global";

    std::string bookName {DEFAULT_ZONE};
    std::string tgtZone  {DEFAULT_ZONE};

    size_t curRuleId;


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
    void setIfaceRoute(const std::string&, nmco::IpAddress&);
    void setIfaceGateway(const std::string&, nmco::IpAddress&);

    // Interface related
    void initIface(const std::string&, const std::string&);
    void disableIface();
    void updateIfaceIp(nmco::IpAddress&);
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
