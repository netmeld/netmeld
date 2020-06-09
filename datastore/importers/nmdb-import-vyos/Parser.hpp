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

#include <netmeld/datastore/objects/AcNetworkBook.hpp>
#include <netmeld/datastore/objects/AcRule.hpp>
#include <netmeld/datastore/objects/AcServiceBook.hpp>
#include <netmeld/datastore/objects/DeviceInformation.hpp>
#include <netmeld/datastore/objects/InterfaceNetwork.hpp>
#include <netmeld/datastore/objects/Service.hpp>
#include <netmeld/datastore/objects/ToolObservations.hpp>
#include <netmeld/datastore/parsers/ParserDomainName.hpp>
#include <netmeld/datastore/parsers/ParserIpAddress.hpp>

namespace nmdo = netmeld::datastore::objects;
namespace nmdp = netmeld::datastore::parsers;

typedef std::map<std::string, nmdo::AcNetworkBook>  NetworkBook;
//typedef std::map<std::string, nmdo::AcServiceBook>  ServiceBook;
typedef std::map<size_t, nmdo::AcRule>              RuleBook;

// =============================================================================
// Data containers
// =============================================================================
struct Data {
  nmdo::DeviceInformation devInfo {"VyOS"};

  std::map<std::string, nmdo::InterfaceNetwork> ifaces;

  std::vector<nmdo::Service> services;

  std::map<std::string, NetworkBook>  networkBooks;
//  std::map<std::string, ServiceBook>  serviceBooks;
  std::map<std::string, RuleBook>     ruleBooks;

  nmdo::ToolObservations observations;
};
typedef std::vector<Data>    Result;


// =============================================================================
// Parser definition
// =============================================================================
class Parser :
  public qi::grammar<nmdp::IstreamIter, Result(), qi::ascii::blank_type>
{
  // ===========================================================================
  // Variables
  // ===========================================================================
  private: // Variables are always private
    // Rules
    qi::rule<nmdp::IstreamIter, Result(), qi::ascii::blank_type>
      start;

    qi::rule<nmdp::IstreamIter, qi::ascii::blank_type>
      config,
      system,login,user,
      interfaces, interface, ifaceFirewall,
      firewall, group, addressGroup, ruleSets, rule, destination, source,
      startBlock, stopBlock, ignoredBlock;

    qi::rule<nmdp::IstreamIter, std::string()>
      token;

    qi::rule<nmdp::IstreamIter>
      comment;

    nmdp::ParserIpAddress  ipAddr;
    nmdp::ParserDomainName fqdn;

    // Supporting data structures
    Data d;

    nmdo::InterfaceNetwork*  tgtIface;
    std::string              tgtIfaceName;

    const std::string DEFAULT_ZONE {"global"};

    std::string tgtZone  {DEFAULT_ZONE};
    std::string tgtBook;

    size_t         curRuleId  {0};
    nmdo::AcRule*  tgtRule;
    std::string    proto;
    std::string    srcPort;
    std::string    dstPort;
    std::string    defaultAction;

    std::vector<std::string> states;

    std::string  creds[4];

  // ===========================================================================
  // Constructors
  // ===========================================================================
  public: // Constructor is only default and must be public
    Parser();

  // ===========================================================================
  // Methods
  // ===========================================================================
  private:
    void ifaceInit(const std::string&);

    void serviceAddDns(const nmdo::IpAddress&);

    void netBookAddAddr(const nmdo::IpAddress&);

    void ruleInit(size_t);
    void ruleAddDstIface(const std::string&);
    void ruleAddSrcIface(const std::string&);
    void ruleAddService();

    void noteCredentials();

    void unsup(const std::string&);

    Result getData();
};
#endif // PARSER_HPP
