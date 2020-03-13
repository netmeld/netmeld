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
#include <netmeld/core/objects/DeviceInformation.hpp>
#include <netmeld/core/objects/InterfaceNetwork.hpp>
#include <netmeld/core/objects/Route.hpp>
#include <netmeld/core/parsers/ParserDomainName.hpp>
#include <netmeld/core/parsers/ParserIpAddress.hpp>
#include <netmeld/core/parsers/ParserMacAddress.hpp>
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
  nmco::DeviceInformation              devInfo;
  std::vector<nmco::InterfaceNetwork>  ifaces;

  std::map<std::string, NetworkBook>  networkBooks;
  std::map<std::string, ServiceBook>  serviceBooks;
  std::map<std::string, RuleBook>     ruleBooks;

  nmco::ToolObservations observations;
};
typedef std::vector<Data> Result;


// =============================================================================
// Parser definition
// =============================================================================
class Parser :
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
      ignoredLine,
      config;

    qi::rule<nmcp::IstreamIter, nmco::InterfaceNetwork(), qi::ascii::blank_type,
             qi::locals<nmco::IpAddress>>
      asaIface;

    qi::rule<nmcp::IstreamIter, qi::ascii::blank_type,
             qi::locals<std::string>>
      objNet,
      objGrpNet;

    qi::rule<nmcp::IstreamIter, qi::ascii::blank_type,
             qi::locals<std::string, std::string>>
      objSrv,
      objGrpSrv,
      objGrpProto;

    qi::rule<nmcp::IstreamIter, std::string(), qi::ascii::blank_type>
      ipAddrStr;

    qi::rule<nmcp::IstreamIter, qi::ascii::blank_type,
             qi::locals<std::string>>
      policy;

    qi::rule<nmcp::IstreamIter, std::string(), qi::ascii::blank_type>
      addressArgument,
      portArgument,
      srvRngVal,
      objSrvProto,
      objSrvSrc,
      objSrvDst;

    qi::rule<nmcp::IstreamIter, std::string()>
      logVal,
      tokens,
      token;

    nmcp::ParserDomainName  fqdn;
    nmcp::ParserIpAddress   ipAddr;
    nmcp::ParserMacAddress  macAddr;

    Data d;

    std::map<std::string, nmco::InterfaceNetwork>  ifaceAliases;

    const std::string ZONE {"global"};

    std::pair<bool, std::string>  tgtProto;
    std::pair<bool, std::string>  tgtSrcPort;
    std::pair<bool, std::string>  tgtDstPort;

    size_t                         curRuleId;
    std::map<std::string, size_t>  ruleIds;
    std::string                    curRuleDescription;

  // ===========================================================================
  // Constructors
  // ===========================================================================
  public: // Constructor is only default and must be public
    Parser();

  // ===========================================================================
  // Methods
  // ===========================================================================
  private:
    // Device related
    void setDevId(const std::string&);
    void setVendor(const std::string&);

    // Interface related
    void addIface(nmco::InterfaceNetwork&);
    void addIfaceAlias(const nmco::InterfaceNetwork&, const std::string&);
    nmco::IpAddress getIpFromAlias(const std::string&);

    // AC related
    void updateNetBookIp(const std::string&, const nmco::IpAddress&);
    void updateNetBookRange(const std::string&,
                            const std::string&, const std::string&);
    void updateNetBookGroup(const std::string&, const std::string&);
    void updateNetBookGroupMask(const std::string&, const std::string&,
                                const nmco::IpAddress&);

    void updateSrvcBook(const std::string&, const std::string&);
    void updateSrvcBookGroup(const std::string&, const std::string&);

    void updateCurRuleId(const std::string&);
    void updateRuleAction(const std::string&, const std::string&);
    void updateTgtProto(const std::string&, const bool);
    void updateTgtSrcPort(const std::string&, const bool);
    void updateTgtDstPort(const std::string&, const bool);
    void updateRuleService(const std::string&);
    void updateRuleSrc(const std::string&, const std::string&);
    void updateRuleDst(const std::string&, const std::string&);
    void disableRule(const std::string&);
    void assignRules(const std::string&, const std::string&,
                     const std::string&);

    // Unsupported
    void unsup(const std::string&);

    // Object return
    Result getData();
};
#endif // PARSER_HPP
