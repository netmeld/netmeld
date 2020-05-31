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

#include <netmeld/datastore/objects/DeviceInformation.hpp>
#include <netmeld/datastore/objects/InterfaceNetwork.hpp>
#include <netmeld/datastore/objects/Route.hpp>
#include <netmeld/datastore/objects/Service.hpp>
#include <netmeld/datastore/objects/ToolObservations.hpp>
#include <netmeld/datastore/objects/Vlan.hpp>
#include <netmeld/datastore/parsers/ParserDomainName.hpp>
#include <netmeld/datastore/parsers/ParserIpAddress.hpp>
#include <netmeld/datastore/parsers/ParserMacAddress.hpp>
#include <netmeld/datastore/tools/AbstractImportTool.hpp>

namespace nmdo = netmeld::datastore::objects;
namespace nmdp = netmeld::datastore::parsers;


// =============================================================================
// Data containers
// =============================================================================
struct Data
{
  std::string                          domainName;
  nmdo::DeviceInformation              devInfo;

  std::map<std::string, nmdo::InterfaceNetwork>  ifaces;
  std::vector<nmdo::Route>             routes;
  std::vector<nmdo::Service>           services;
  std::vector<nmdo::Vlan>              vlans;

  nmdo::ToolObservations               observations;
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
  private:
    // Rules
    qi::rule<nmdp::IstreamIter, Result(), qi::ascii::blank_type>
      start;

    qi::rule<nmdp::IstreamIter, qi::ascii::blank_type>
      vlanDef,
      config,
      interface, switchport, spanningTree
      ;

    qi::rule<nmdp::IstreamIter, std::string()>
      tokens,
      token;

    nmdp::ParserDomainName  domainName;
    nmdp::ParserIpAddress   ipAddr;
    nmdp::ParserMacAddress  macAddr;

    // Supporting data structures
    Data d;

    bool isNo {false};

    nmdo::InterfaceNetwork* tgtIface;

    bool globalCdpEnabled         {true};
    bool globalBpduGuardEnabled   {false};
    bool globalBpduFilterEnabled  {false};
    std::set<std::string>  ifacesCdpManuallySet;
    std::set<std::string>  ifacesBpduGuardManuallySet;
    std::set<std::string>  ifacesBpduFilterManuallySet;

  // ===========================================================================
  // Constructors
  // ===========================================================================
  public: // Constructor is only default and must be public
    Parser();

  // ===========================================================================
  // Methods
  // ===========================================================================
  private: // Methods which should be hidden from API users
    void ifaceInit(const std::string&);
    void ifaceSetUpdate(std::set<std::string>* const);

    void routeAdd(const nmdo::IpAddress&, const std::string&);

    void serviceAddNtp(const nmdo::IpAddress&);
    void serviceAddDhcp(const nmdo::IpAddress&);
    void serviceAddSnmp(const nmdo::IpAddress&);
    void serviceAddRadius(const nmdo::IpAddress&);
    void serviceAddDns(const std::vector<nmdo::IpAddress>&);
    void serviceAddSyslog(const nmdo::IpAddress&);

    void vlanAdd(unsigned short, const std::string&);
    void vlanAddIfaceData();

    // Unsupported
    void unsup(const std::string&);
    void addObservation(const std::string&);

    // Object return
    Result getData();
};
#endif // PARSER_HPP
