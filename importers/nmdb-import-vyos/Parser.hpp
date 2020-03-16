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
#include <netmeld/core/objects/ToolObservations.hpp>
#include <netmeld/core/objects/Service.hpp>

#include <netmeld/core/parsers/ParserDomainName.hpp>
#include <netmeld/core/parsers/ParserIpAddress.hpp>

namespace nmco = netmeld::core::objects;
namespace nmcp = netmeld::core::parsers;

// =============================================================================
// Data containers
// =============================================================================
struct Data {
  nmco::DeviceInformation devInfo {"VyOS"};

  std::map<std::string, nmco::InterfaceNetwork> ifaces;

  std::vector<nmco::Service> services;

  nmco::ToolObservations observations;
};
typedef std::vector<Data>    Result;


// =============================================================================
// Parser definition
// =============================================================================
class Parser :
  public qi::grammar<nmcp::IstreamIter, Result(), qi::ascii::blank_type>
{
  // ===========================================================================
  // Variables
  // ===========================================================================
  private: // Variables are always private
    // Rules
    qi::rule<nmcp::IstreamIter, Result(), qi::ascii::blank_type>
      start;

    qi::rule<nmcp::IstreamIter, qi::ascii::blank_type>
      config,
      system,
      interfaces, interface,
      startBlock, stopBlock, ignoredBlock;

    qi::rule<nmcp::IstreamIter, std::string()>
      token;

    qi::rule<nmcp::IstreamIter>
      comment;

    nmcp::ParserIpAddress  ipAddr;
    nmcp::ParserDomainName fqdn;

    // Supporting data structures
    Data d;

    std::string tgtIfaceName {""};

  // ===========================================================================
  // Constructors
  // ===========================================================================
  public: // Constructor is only default and must be public
    Parser();

  // ===========================================================================
  // Methods
  // ===========================================================================
  private:
    void initIface(const std::string&);
    void addIfaceIpAddr(nmco::IpAddress&);
    void updateIfaceType(const std::string&);
    void updateIfaceDesc(const std::string&);

    void addServiceDns(const nmco::IpAddress&);

    void unsup(const std::string&);

    Result getData();
};
#endif // PARSER_HPP
