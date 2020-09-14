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

#ifndef NMAP_XML_PARSER_HPP
#define NMAP_XML_PARSER_HPP

#include <pugixml.hpp>

#include <netmeld/datastore/objects/IpAddress.hpp>
#include <netmeld/datastore/objects/MacAddress.hpp>
#include <netmeld/datastore/objects/OperatingSystem.hpp>
#include <netmeld/datastore/objects/Port.hpp>
#include <netmeld/datastore/objects/Service.hpp>
#include <netmeld/datastore/objects/ToolObservations.hpp>
#include <netmeld/datastore/parsers/ParserHelper.hpp>
#include <netmeld/datastore/tools/AbstractImportTool.hpp>

#include "NseResult.hpp"
#include "SshAlgorithm.hpp"
#include "SshPublicKey.hpp"
#include "TracerouteHop.hpp"

namespace nmdo = netmeld::datastore::objects;
namespace nmdp = netmeld::datastore::parsers;
namespace nmdt = netmeld::datastore::tools;
namespace nmdu = netmeld::datastore::utils;

// =============================================================================
// Data containers
// =============================================================================
struct Data
{
  std::vector<nmdo::MacAddress>       macAddrs;
  std::vector<nmdo::IpAddress>        ipAddrs;
  std::vector<nmdo::OperatingSystem>  oses;
  std::vector<TracerouteHop>          tracerouteHops;
  std::vector<nmdo::Port>             ports;
  std::vector<nmdo::Service>          services;
  std::vector<NseResult>              nseResults;
  std::vector<SshPublicKey>           sshKeys;
  std::vector<SshAlgorithm>           sshAlgorithms;

  nmdo::ToolObservations              observations;
};
typedef std::vector<Data>  Result;


// =============================================================================
// Parser definition
// =============================================================================
class ParserNmapXml
{
  // ===========================================================================
  // Variables
  // ===========================================================================
  private:
  protected:
  public:

  // ===========================================================================
  // Constructors
  // ===========================================================================
  private:
  protected:
  public:
    ParserNmapXml();

  // ===========================================================================
  // Methods
  // ===========================================================================
  private:
  protected:
    bool extractHostIsResponding(const pugi::xml_node&) const;
    nmdo::MacAddress extractHostMacAddr(const pugi::xml_node&) const;
    nmdo::IpAddress extractHostIpAddr(const pugi::xml_node&) const;

  public:
    std::tuple<std::string, std::string>
      extractExecutionTiming(const pugi::xml_node&);

    void extractMacAndIpAddrs(const pugi::xml_node&, Data&);
    void extractHostnames(const pugi::xml_node&, Data&);
    void extractOperatingSystems(const pugi::xml_node&, Data&);
    void extractTraceRoutes(const pugi::xml_node&, Data&);
    void extractPortsAndServices(const pugi::xml_node&, Data&);
    void extractNseAndSsh(const pugi::xml_node&, Data&);
};

#endif //NMAP_XML_PARSER_HPP
