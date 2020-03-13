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

#include <netmeld/core/objects/DeviceInformation.hpp>
#include <netmeld/core/objects/InterfaceNetwork.hpp>
#include <netmeld/core/objects/Route.hpp>
#include <netmeld/core/objects/Service.hpp>
#include <netmeld/core/objects/ToolObservations.hpp>
#include <netmeld/core/objects/Vlan.hpp>
#include <netmeld/core/parsers/ParserDomainName.hpp>
#include <netmeld/core/parsers/ParserIpAddress.hpp>
#include <netmeld/core/parsers/ParserMacAddress.hpp>
#include <netmeld/core/tools/AbstractImportTool.hpp>

namespace nmco = netmeld::core::objects;
namespace nmcp = netmeld::core::parsers;


// =============================================================================
// Data containers
// =============================================================================
struct Data
{
  std::string                          domainName;
  nmco::DeviceInformation              devInfo;

  std::vector<nmco::InterfaceNetwork>  ifaces;
  std::vector<nmco::Route>             routes;
  std::vector<nmco::Service>           services;
  std::vector<nmco::Vlan>              vlans;

  nmco::ToolObservations               observations;
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
    Data d;

    // Rules
    qi::rule<nmcp::IstreamIter, Result(), qi::ascii::blank_type>
      start;

    qi::rule<nmcp::IstreamIter, qi::ascii::blank_type>
      vlanDef,
      config;

    qi::rule<nmcp::IstreamIter, nmco::InterfaceNetwork(), qi::ascii::blank_type>
      nxIface;

    qi::rule<nmcp::IstreamIter, std::string()>
      tokens,
      token;

    nmcp::ParserDomainName  domainName;
    nmcp::ParserIpAddress   ipAddr;
    nmcp::ParserMacAddress  macAddr;

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
    // Device related
    void setVendor(const std::string&);
    void setDevId(const std::string&);
    void addNtpService(const nmco::IpAddress&);
    void addDhcpService(const nmco::IpAddress&);
    void addSnmpService(const nmco::IpAddress&);
    void addRadiusService(const nmco::IpAddress&);
    void addDnsService(const std::vector<nmco::IpAddress>&);
    void addLogService(const nmco::IpAddress&);
    void addRoute(const nmco::IpAddress&, const std::string&);
    void setGlobalCdp(const bool);
    void setGlobalBpduGuard(const bool);
    void setGlobalBpduFilter(const bool);
    void addSetIface(std::set<std::string>* const, const nmco::InterfaceNetwork&);

    // Interface related
    void addIface(nmco::InterfaceNetwork&);

    // Vlan related
    void addVlan(unsigned short, const std::string&);

    // Unsupported
    void unsup(const std::string&);
    void addObservation(const std::string&);

    // Object return
    Result getData();
};
#endif // PARSER_HPP
