// =============================================================================
// Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC
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

#include <pugixml.hpp>

#include <netmeld/datastore/objects/Cve.hpp>
#include <netmeld/datastore/objects/DeviceInformation.hpp>
#include <netmeld/datastore/objects/Interface.hpp>
#include <netmeld/datastore/objects/IpAddress.hpp>
#include <netmeld/datastore/objects/MacAddress.hpp>
#include <netmeld/datastore/objects/OperatingSystem.hpp>
#include <netmeld/datastore/objects/Port.hpp>
#include <netmeld/datastore/objects/TracerouteHop.hpp>
#include <netmeld/datastore/parsers/ParserHelper.hpp>
#include <netmeld/datastore/parsers/ParserIpAddress.hpp>
#include <netmeld/datastore/tools/AbstractImportSpiritTool.hpp>
#include <netmeld/core/utils/ContainerUtilities.hpp>

#include "InterfaceHelper.hpp"
#include "MetasploitModule.hpp"
#include "NessusResult.hpp"
#include "ParserNessusInterface.hpp"

namespace nmdo = netmeld::datastore::objects;
namespace nmdp = netmeld::datastore::parsers;
namespace nmdt = netmeld::datastore::tools;
namespace nmdu = netmeld::datastore::utils;

struct Data
{
  std::vector<nmdo::MacAddress>        macAddrs;
  std::vector<nmdo::IpAddress>         ipAddrs;
  std::vector<nmdo::OperatingSystem>   oses;
  std::vector<nmdo::Port>              ports;
  std::vector<nmdo::TracerouteHop>     tracerouteHops;
  std::vector<NessusResult>            nessusResults;
  //std::set<NessusResult>            nessusResults;
  std::vector<nmdo::Cve>               cves;
  std::vector<MetasploitModule>        metasploitModules;
  std::map<nmdo::IpAddress, InterfaceHelper>     interfaces;

  auto operator<=>(const Data&) const = default;
  bool operator==(const Data&) const = default;
};
typedef std::vector<Data>             Results;


template<typename P, typename R>
class Tool : public nmdt::AbstractImportSpiritTool<P,R>
{
  public:
    Tool() : nmdt::AbstractImportSpiritTool<P,R>
      ("Nessus's XML output (.nessus file)", PROGRAM_NAME, PROGRAM_VERSION)
    {}

    void
    addToolOptions() override
    {
      this->opts.removeRequiredOption("device-id");
      this->opts.addOptionalOption("device-id", std::make_tuple(
            "device-id",
            po::value<std::string>(),
            "(Not used) Name of device.")
          );

      this->opts.removeOptionalOption("device-type");
    }

    void
    parseData() override
    {
      pugi::xml_document doc;
      if (!doc.load_file(this->getDataPath().string().c_str())) {
        LOG_ERROR << "Could not open XML: "
                  << this->getDataPath().string()
                  << std::endl;
        std::exit(nmcu::Exit::FAILURE);
      }

      pugi::xml_node reportNode =
        doc.select_node("/NessusClientData_v2/Report").node();
      if (!reportNode) {
        LOG_ERROR << "Could not find XML element: /NessusClientData_v2/Report"
                  << std::endl;
        std::exit(nmcu::Exit::FAILURE);
      }

      extractExecutionTiming(reportNode);

      Data data;

      for (const auto& reportHost : reportNode.select_nodes("ReportHost")) {
        pugi::xml_node reportHostNode = reportHost.node();
        parseReportHost(reportHostNode, data);
      }

      this->tResults.push_back(data);
    }

    void
    specificInserts(pqxx::transaction_base& t) override
    {
      const auto& toolRunId {this->getToolRunId()};

      for (auto& results : this->tResults) {
        LOG_DEBUG << "Iterating over MacAddrs\n";
        for (auto& result : results.macAddrs) {
          result.save(t, toolRunId, "");
          LOG_DEBUG << result.toDebugString() << std::endl;
        }

        LOG_DEBUG << "Iterating over IpAddrs\n";
        for (auto& result : results.ipAddrs) {
          result.save(t, toolRunId, "");
          LOG_DEBUG << result.toDebugString() << std::endl;
        }

        LOG_DEBUG << "Iterating over Oses\n";
        for (auto& result : results.oses) {
          result.save(t, toolRunId, "");
          LOG_DEBUG << result.toDebugString() << std::endl;
        }

        LOG_DEBUG << "Iterating over Ports\n";
        for (auto& result : results.ports) {
          result.save(t, toolRunId, "");
          LOG_DEBUG << result.toDebugString() << std::endl;
        }

        LOG_DEBUG << "Iterating over TracerouteHops\n";
        for (auto& result : results.tracerouteHops) {
          result.save(t, toolRunId, "");
          LOG_DEBUG << result.toDebugString() << std::endl;
        }

        LOG_DEBUG << "Iterating over NessusResults\n";
        for (auto& result : results.nessusResults) {
          result.save(t, toolRunId, "");
          LOG_DEBUG << result.toDebugString() << std::endl;
        }

        LOG_DEBUG << "Iterating over Cves\n";
        for (auto& result : results.cves) {
          LOG_DEBUG << result.toDebugString() << std::endl;
          result.save(t, toolRunId, "");
        }

        LOG_DEBUG << "Iterating over MetasploitModules\n";
        for (auto& result : results.metasploitModules) {
          result.save(t, toolRunId, "");
          LOG_DEBUG << result.toDebugString() << std::endl;
        }

        LOG_DEBUG << "Iterating over Interfaces\n";
        for (auto& helper : results.interfaces) {
          for (auto& wrapMap : std::get<1>(helper).interfaces) {
            nmdo::DeviceInformation devInfo;
            devInfo.setDeviceId(std::get<1>(wrapMap).deviceId);
            if (this->opts.exists("device-color")) {
              devInfo.setDeviceColor(this->opts.getValue("device-color"));
            }
            auto& result = std::get<1>(wrapMap).interface;

            // save the raw_device here so an Interface can be saved.
            if (devInfo.isValid()) {
              devInfo.save(t, toolRunId);
              const auto& deviceId {devInfo.getDeviceId()};
              // check validity to prevent verbose warnings
              if (result.getMacAddress().isValid()) {
                result.save(t, toolRunId, deviceId);
              }
              LOG_DEBUG << result.toDebugString() << std::endl;
            } else {
              // Save just MacAddress and associated IpAddress in the case that
              // there is no deviceId and cannot save a full Interface
              auto mac = result.getMacAddress();
              mac.save(t, toolRunId, "");
              LOG_DEBUG << mac.toDebugString() << std::endl;
            }
          }
        }
      }
    }

  private:
    // =========================================================================
    // Supporting Functions
    // =========================================================================
    void
    extractExecutionTiming(pugi::xml_node const& reportNode)
    {
      std::string const formatString = "%a %b %d %H:%M:%S %Y";

      // Find earliest start time

      auto hostStarts = reportNode
        .select_nodes("ReportHost/HostProperties/tag[@name='HOST_START']");

      nmco::Time executeTimeLower("infinity");
      for (const auto& timeLower : hostStarts) {
        pugi::xml_node timeLowerNode = timeLower.node();

        LOG_DEBUG << "[str] Start: " << timeLowerNode.text().as_string() << std::endl;

        nmco::Time tempLower;
        tempLower.readFormatted(timeLowerNode.text().as_string(), formatString);

        LOG_DEBUG << "[tmp] Start: " << tempLower << std::endl;

        if ((executeTimeLower.isNull()) || (tempLower < executeTimeLower)) {
          executeTimeLower = tempLower;
        }
      }
      this->executionStart = executeTimeLower;

      // Find latest end time

      auto hostEnds = reportNode
        .select_nodes("ReportHost/HostProperties/tag[@name='HOST_END']");

      nmco::Time executeTimeUpper("-infinity");
      for (const auto& timeUpper : hostEnds) {
        pugi::xml_node timeUpperNode = timeUpper.node();

        LOG_DEBUG << "[str] Stop : " << timeUpperNode.text().as_string() << std::endl;

        nmco::Time tempUpper;
        tempUpper.readFormatted(timeUpperNode.text().as_string(), formatString);

        LOG_DEBUG << "[tmp] Stop : " << tempUpper << std::endl;

        if ((executeTimeUpper.isNull()) || (executeTimeUpper < tempUpper)) {
          executeTimeUpper = tempUpper;
        }
      }
      this->executionStop = executeTimeUpper;

      LOG_DEBUG << "[nmdo] Start: " << this->executionStart << std::endl;
      LOG_DEBUG << "[nmdo] Stop : " << this->executionStop << std::endl;
    }


    // =========================================================================
    // XML Parsing Functions
    // =========================================================================
    void
    parseReportItem(pugi::xml_node const& reportItemNode, nmdo::IpAddress ipAddr,
                    std::string hostname, Data& data)
    {
      // Extact Port and Nessus results
      std::string   protocol =
        reportItemNode.attribute("protocol").as_string();
      int           portNum =
        reportItemNode.attribute("port").as_int();
      std::string   portState =
        (portNum ? "open" : "none");
      std::string   portReason =
        "nessus scan";
      //std::string   serviceName =
      //  reportItemNode.attribute("svc_name").as_string();
      unsigned int  severity =
        reportItemNode.attribute("severity").as_uint();
      unsigned int  pluginId =
        reportItemNode.attribute("pluginID").as_uint();
      std::string   pluginName =
        reportItemNode.attribute("pluginName").as_string();
      std::string   pluginFamily =
        reportItemNode.attribute("pluginFamily").as_string();
      std::string   pluginType =
        reportItemNode.select_node("plugin_type").node().text().as_string();
      std::string   pluginOutput =
        reportItemNode.select_node("plugin_output").node().text().as_string();
      std::string   description =
        reportItemNode.select_node("description").node().text().as_string();
      std::string   solution =
        reportItemNode.select_node("solution").node().text().as_string();

      nmdo::Port port(ipAddr);
      port.setProtocol(protocol);
      port.setPort(portNum);
      port.setState(portState);
      port.setReason(portReason);

      //data.ports.push_back(port);
      nmcu::pushBackIfUnique(&data.ports, port);

      NessusResult nr;
      nr.port = port;
      nr.pluginId = pluginId;
      nr.pluginName = pluginName;
      nr.pluginFamily = pluginFamily;
      nr.pluginType = pluginType;
      nr.pluginOutput = pluginOutput;
      nr.severity = severity;
      nr.description = description;
      nr.solution = solution;

      nmcu::pushBackIfUnique(&data.nessusResults, nr);
      //data.nessusResults.push_back(nr);
      //data.nessusResults.emplace(nr);

      // Extract CVE identifiers
      for (const auto& cveItem : reportItemNode.select_nodes("cve")) {
        pugi::xml_node cveNode = cveItem.node();

        nmdo::Cve cve(cveNode.text().as_string());
        cve.setPort(port);
        cve.setPluginId(pluginId);
        //data.cves.push_back(cve);
        nmcu::pushBackIfUnique(&data.cves, cve);
      }

      // Extract Metasploit modules
      for (const auto& msfn : reportItemNode.select_nodes("metasploit_name")) {
        pugi::xml_node msfNameNode = msfn.node();

        std::string name = msfNameNode.text().as_string();

        MetasploitModule mm;
        mm.port = port;
        mm.pluginId = pluginId;
        mm.name = name;

        //data.metasploitModules.push_back(mm);
        nmcu::pushBackIfUnique(&data.metasploitModules, mm);
      }

      // Extract Interface mappings from 3 different plugins
      enum pluginIds { IPv6 = 25202, IPv4 = 25203, Mac = 33276 };
      if (pluginId == IPv6 || pluginId == IPv4 || pluginId == Mac) {
        auto parsedInterfaces = nmdp::fromString<nmdp::ParserNessusInterface,
                                                std::vector<nmdo::Interface>>
                                               (pluginOutput);

        auto it = data.interfaces.find(ipAddr);
        if (it == data.interfaces.end()) {
          // Create new
          data.interfaces.emplace(ipAddr, InterfaceHelper{});
          it = data.interfaces.find(ipAddr);
        }

        // Update existing
        for (auto& iface : parsedInterfaces) {
          it->second.add(iface, hostname);
        }
      }

    }

    void
    parseReportHost(pugi::xml_node const& reportHostNode, Data& data)
    {
      bool const isResponding = true;

      // Extract Ip Address
      nmdo::IpAddress ipAddr;
      auto tags = reportHostNode.select_nodes
        ("HostProperties/tag[@name='host-ip' or @name='container-host']");

      for (const auto& tag : tags) {
        pugi::xml_node tagNode = tag.node();

        std::string ipAddrStr = tagNode.text().as_string();
        if (ipAddrStr.empty()) {
          continue;
        }
        ipAddr.setAddress(ipAddrStr);
        ipAddr.setResponding(isResponding);

        // Extract host FQND
        auto fqdnTags = reportHostNode.select_nodes
          ("HostProperties/tag[@name='host-fqdn']");
        for (const auto& fqdnTag : fqdnTags) {
          ipAddr.addAlias(fqdnTag.node().text().as_string(), "nessus scan");
        }

        // Extract hostname
        auto hostTags = reportHostNode.select_nodes
          ("HostProperties/tag[@name='hostname']");
        for (const auto& hostTag : hostTags) {
          ipAddr.addAlias(hostTag.node().text().as_string(), "nessus scan");
        }

        //data.ipAddrs.push_back(ipAddr);
        nmcu::pushBackIfUnique(&data.ipAddrs, ipAddr);
      }

      // Extract Mac Address
      tags = reportHostNode.select_nodes
        ("HostProperties/tag[@name='mac-address']");

      for (const auto& tag : tags) {
        pugi::xml_node tagNode = tag.node();

        std::string macAddrs {tagNode.text().as_string()};
        if (!macAddrs.empty()) {
          // Nessus puts multiple MACs under one tag, so split and iterate
          boost::char_separator<char> sep("\n");
          boost::tokenizer<boost::char_separator<char>> tokens(macAddrs, sep);

          size_t macCount {0};
          for (auto it = tokens.begin(); it != tokens.end(); ++it, ++macCount) {
            nmdo::MacAddress macAddr(*it);
            macAddr.setResponding(isResponding);
            //data.macAddrs.push_back(macAddr);
            nmcu::pushBackIfUnique(&data.macAddrs, macAddr);
          }

          if (1 == macCount) { // If only one found, associate MAC-to-IP
            data.macAddrs.back().addIpAddress(ipAddr);
          }
        }
      }

      // Extract Operating System
      tags = reportHostNode.select_nodes
        ("HostProperties/tag[starts-with(@name, 'cpe')]");

      for (const auto& tag : tags) {
        pugi::xml_node tagNode = tag.node();
        std::string cpe {tagNode.text().as_string()};

        std::string prefix {"cpe:/o:"};
        if (0 == cpe.compare(0, prefix.size(), prefix)) {
          nmdo::OperatingSystem os(ipAddr);
          os.setCpe(cpe);
          os.setAccuracy(1.0);

          //data.oses.push_back(os);
          nmcu::pushBackIfUnique(&data.oses, os);
        }
      }

      // Extract traceroute hops
      auto tracerouteHopTags = reportHostNode.select_nodes
        ("HostProperties/tag[starts-with(@name, 'traceroute-hop-')]");
      for (const auto& tracerouteHopTag : tracerouteHopTags) {
        const pugi::xml_node tagNode{tracerouteHopTag.node()};
        const std::string hopIpAddrStr{tagNode.text().as_string()};

        if (nmdp::matchString<nmdp::ParserIpAddress, nmdo::IpAddress>(hopIpAddrStr)) {
          nmdo::TracerouteHop tracerouteHop;
          tracerouteHop.setDstIp(ipAddr);
          tracerouteHop.setHopIp(nmdo::IpAddress{hopIpAddrStr});
          tracerouteHop.setHopCount(static_cast<uint32_t>(std::stoul(
              std::string{tagNode.attribute("name").as_string()}.substr(15)
            )));

          //data.tracerouteHops.emplace_back(tracerouteHop);
          nmcu::pushBackIfUnique(&data.tracerouteHops, tracerouteHop);
        }
      }

      // Get first hostname to pass down for creating Interfaces
      auto hostTag = reportHostNode.select_node
        ("HostProperties/tag[@name='hostname']");
      std::string hostname = hostTag.node().text().as_string();

      // Extract ReportItem elements
      for (const auto& reportItem : reportHostNode.select_nodes("ReportItem")) {
        pugi::xml_node reportItemNode = reportItem.node();
        parseReportItem(reportItemNode, ipAddr, hostname, data);
      }
    }
};

int
main(int argc, char** argv)
{
  Tool<nmdp::DummyParser, Results> tool;
  return tool.start(argc, argv);
}

