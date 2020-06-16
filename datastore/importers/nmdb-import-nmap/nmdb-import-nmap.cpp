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

#include <pugixml.hpp>

#include <netmeld/datastore/objects/IpAddress.hpp>
#include <netmeld/datastore/objects/MacAddress.hpp>
#include <netmeld/datastore/objects/OperatingSystem.hpp>
#include <netmeld/datastore/objects/Port.hpp>
#include <netmeld/datastore/objects/Service.hpp>
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
};
typedef std::vector<Data>  Results;


template<typename P, typename R>
class Tool : public nmdt::AbstractImportTool<P,R>
{
  public:
    Tool() : nmdt::AbstractImportTool<P,R>
      ("Nmap's XML output (.xml files)", PROGRAM_NAME, PROGRAM_VERSION)
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
      this->opts.removeOptionalOption("device-color");

      this->opts.addOptionalOption("scan-origin-ip", std::make_tuple(
          "scan-origin-ip",
          po::value<std::string>(),
          "Ip address of device where nmap scan originated")
        );
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

      pugi::xml_node const nmapNode = doc.select_node("/nmaprun").node();
      if (!nmapNode) {
        LOG_ERROR << "Could not find XML element: /nmaprun"
                  << std::endl;
        std::exit(nmcu::Exit::FAILURE);
      }

      extractExecutionTiming(nmapNode);

      Data data;

      extractMacAndIpAddrs(nmapNode, data);
      extractHostnames(nmapNode, data);
      extractOperatingSystems(nmapNode, data);
      extractTraceRoutes(nmapNode, data);
      extractPortsAndServices(nmapNode, data);
      extractNseAndSsh(nmapNode, data);

      this->tResults.push_back(data);
    }

    void
    specificInserts(pqxx::transaction_base& t) override
    {
      const auto& toolRunId {this->getToolRunId()};

      for (auto& results : this->tResults) {
        LOG_DEBUG << "Iterating over macAddrs\n";
        for (auto& result : results.macAddrs) {
          result.save(t, toolRunId, "");
          LOG_DEBUG << result.toDebugString() << std::endl;
        }

        LOG_DEBUG << "Iterating over ipAddrs\n";
        for (auto& result : results.ipAddrs) {
          result.save(t, toolRunId, "");
          LOG_DEBUG << result.toDebugString() << std::endl;
        }

        LOG_DEBUG << "Iterating over oses\n";
        for (auto& result : results.oses) {
          result.save(t, toolRunId, "");
          LOG_DEBUG << result.toDebugString() << std::endl;
        }

        LOG_DEBUG << "Iterating over tracerouteHops\n";
        for (auto& result : results.tracerouteHops) {
          result.save(t, toolRunId, "");
          LOG_DEBUG << result.toDebugString() << std::endl;
        }

        LOG_DEBUG << "Iterating over ports\n";
        for (auto& result : results.ports) {
          result.save(t, toolRunId, "");
          LOG_DEBUG << result.toDebugString() << std::endl;
        }

        LOG_DEBUG << "Iterating over services\n";
        for (auto& result : results.services) {
          if (this->opts.exists("scan-origin-ip")) {
            result.setSrcAddress(
              this->opts.template getValueAs<nmdo::IpAddress>("scan-origin-ip")
            );
          }
          LOG_DEBUG << result.toDebugString() << std::endl;
          result.save(t, toolRunId, "");
        }

        LOG_DEBUG << "Iterating over nseResults\n";
        for (auto& result : results.nseResults) {
          result.save(t, toolRunId, "");
          LOG_DEBUG << result.toDebugString() << std::endl;
        }

        LOG_DEBUG << "Iterating over sshKeys\n";
        for (auto& result : results.sshKeys) {
          result.save(t, toolRunId, "");
          LOG_DEBUG << result.toDebugString() << std::endl;
        }

        LOG_DEBUG << "Iterating over sshAlgorithms\n";
        for (auto& result : results.sshAlgorithms) {
          result.save(t, toolRunId, "");
          LOG_DEBUG << result.toDebugString() << std::endl;
        }
      }
    }

  private:
    // =========================================================================
    // Supporting Functions
    // =========================================================================
    void
    extractExecutionTiming(pugi::xml_node const& nmapNode)
    {
      std::string start = nmapNode.attribute("start").as_string();
      std::string stop  = nmapNode.select_node("runstats/finished")
                          .node().attribute("time").as_string();

      LOG_DEBUG << "[str] Start: " << start << std::endl;
      LOG_DEBUG << "[str] Stop : " << stop << std::endl;

      this->executionStart.readUnixTimestamp(start);
      this->executionStop.readUnixTimestamp(stop);

      LOG_DEBUG << "[nmco] Start: " << this->executionStart << std::endl;
      LOG_DEBUG << "[nmco] Stop : " << this->executionStop << std::endl;
    }

    bool
    extractHostIsResponding(pugi::xml_node const& nodeHost) const
    {
      bool isResponding = false;

      pugi::xml_node const nodeStatus = nodeHost.child("status");

      if (std::string("up") == nodeStatus.attribute("state").as_string()) {
        if (std::string("user-set") !=
            nodeStatus.attribute("reason").as_string()) {
          // Any "up" reason other than "user-set" indicates
          // that the host (or something else) responded.
          isResponding = true;
        }
        else {  // "user-set" hosts:
          if (0 < (nodeHost.select_nodes("ports/port/state"
                               "[@state='open' or @state='closed']").size())) {
            // The presence of "open" or "closed" ports indicates
            // that the host (or something else) responded.
            isResponding = true;
          }
        }
      }

      return isResponding;
    }

    nmdo::MacAddress
    extractHostMacAddr(pugi::xml_node const& nodeHost) const
    {
      pugi::xml_node const nodeMacAddr = nodeHost
        .select_node("address[@addrtype='mac']")
        .node();

      std::string macString = nodeMacAddr.attribute("addr").as_string();

      if (macString.empty()) {
        return nmdo::MacAddress();
      }

      return nmdo::MacAddress(macString);
    }

    nmdo::IpAddress
    extractHostIpAddr(pugi::xml_node const& nodeHost) const
    {
      pugi::xml_node const nodeIpAddr = nodeHost
        .select_node("address[@addrtype='ipv4' or @addrtype='ipv6']")
        .node();

      nmdo::IpAddress ipAddr;

      std::string ipString = nodeIpAddr.attribute("addr").as_string();
      if (!ipString.empty()) {
        ipAddr.setAddress(ipString);
      }

      return ipAddr;
    }


    // =========================================================================
    // XML Parsing Functions
    // =========================================================================
    void
    extractMacAndIpAddrs(pugi::xml_node nmapNode, Data& data)
    { // This code block ensures that all of the ip_addrs and mac_addrs
      // from the scan are present for the other table's foreign keys.
      for (const auto& xHost : nmapNode.select_nodes("host")) {
        pugi::xml_node nodeHost = xHost.node();

        bool const isResponding = extractHostIsResponding(nodeHost);

        nmdo::MacAddress macAddr = extractHostMacAddr(nodeHost);
        macAddr.setResponding(isResponding);
        auto ipAddr {extractHostIpAddr(nodeHost)};
        macAddr.addIp(ipAddr);

        data.macAddrs.push_back(macAddr);
      }
    }

    void
    extractHostnames(pugi::xml_node nmapNode, Data& data)
    {
      for (const auto& xHostname :
            nmapNode.select_nodes("host/hostnames/hostname")) {
        pugi::xml_node nodeHostname = xHostname.node();
        pugi::xml_node nodeHost = nodeHostname.parent().parent();

        nmdo::IpAddress ipAddr = extractHostIpAddr(nodeHost);

        ipAddr.addAlias(nodeHostname.attribute("name").as_string(),
                        nodeHostname.attribute("type").as_string());

        data.ipAddrs.push_back(ipAddr);
      }
    }

    void
    extractOperatingSystems(pugi::xml_node nmapNode, Data& data)
    {
      for (const auto& xOsclass :
             nmapNode.select_nodes("host/os/osmatch/osclass")) {
        pugi::xml_node nodeOs = xOsclass.node();
        pugi::xml_node nodeHost = nodeOs.parent().parent().parent();

        nmdo::IpAddress ipAddr(extractHostIpAddr(nodeHost));
        nmdo::OperatingSystem os(ipAddr);
        os.setVendorName(nodeOs.attribute("vendor").as_string());
        os.setProductName(nodeOs.attribute("osfamily").as_string());
        os.setProductVersion(nodeOs.attribute("osgen").as_string());
        os.setAccuracy(nodeOs.attribute("accuracy").as_double() / 100.0);
        os.setCpe(nodeOs.select_node("cpe").node().text().as_string());

        data.oses.push_back(os);
      }
    }

    void
    extractTraceRoutes(pugi::xml_node nmapNode, Data& data)
    { // This code block identifies ip_addrs of routers along a route.
      // The routers may or may not be in the target address space,
      // so might need to be inserted into the ip_addrs table.
      for (const auto& xHop : nmapNode.select_nodes("host/trace/hop")) {
        pugi::xml_node nodeHop = xHop.node();

        TracerouteHop hop;
        hop.hopCount = nodeHop.attribute("ttl").as_int();

        hop.rtrIpAddr = nmdo::IpAddress(nodeHop.attribute("ipaddr").as_string());
        hop.rtrIpAddr.setResponding(true);

        hop.dstIpAddr = extractHostIpAddr(nodeHop.parent().parent());

        data.tracerouteHops.push_back(hop);
      }
    }

    void
    extractPortsAndServices(pugi::xml_node nmapNode, Data& data)
    {
      for (const auto& xExtrareasons :
             nmapNode.select_nodes("host/ports/extraports/extrareasons")) {
        pugi::xml_node nodeExtrareasons = xExtrareasons.node();
        pugi::xml_node nodeExtraports = nodeExtrareasons.parent();
        pugi::xml_node nodeHost = nodeExtraports.parent().parent();

        nmdo::IpAddress ipAddr = extractHostIpAddr(nodeHost);
        nmdo::Port port(ipAddr);
        port.setPort(-1);
        port.setState(nodeExtraports.attribute("state").as_string());

        std::string portReason =
          nodeExtrareasons.attribute("reason").as_string();
        port.setReason(portReason);

        std::string protocol;
        if (1 == nmapNode.select_nodes("scaninfo").size()) {
          // If there is only a single scaninfo element,
          // all extraports protocols must be the scaninfo's protocol.
          protocol = nmapNode.select_node("scaninfo")
                      .node().attribute("protocol").as_string();
        }
        else if (  (portReason == "tcp-response")
                || (portReason == "tcp-responses")
                || (portReason == "syn-ack")
                || (portReason == "syn-acks")
                || (portReason == "reset")
                || (portReason == "resets")) {
          // If there are multiple scaninfo elements
          // (meaning the scan was a multi-protocol scan),
          // certain port_reason values indicate or imply TCP.
          protocol = "tcp";
        }
        else if (  (portReason == "udp-response")
                || (portReason == "udp-responses")
                || (portReason == "port-unreach")
                || (portReason == "port-unreaches")) {
          // If there are multiple scaninfo elements
          // (meaning the scan was a multi-protocol scan),
          // certain port_reason values indicate or imply UDP.
          protocol = "udp";
        }

        port.setProtocol(protocol);

        data.ports.push_back(port);
      }

      for (const auto& xPort : nmapNode.select_nodes("host/ports/port")) {
        pugi::xml_node nodePort = xPort.node();
        pugi::xml_node nodePortState = nodePort.child("state");
        pugi::xml_node nodeHost = nodePort.parent().parent();

        nmdo::IpAddress ipAddr = extractHostIpAddr(nodeHost);
        nmdo::Port port(ipAddr);

        std::string protocol = nodePort.attribute("protocol").as_string();
        port.setProtocol(protocol);

        int portNum = nodePort.attribute("portid").as_int();
        port.setPort(portNum);

        port.setState(nodePortState.attribute("state").as_string());
        port.setReason(nodePortState.attribute("reason").as_string());

        data.ports.push_back(port);

        pugi::xml_node nodeService = nodePort.child("service");
        if (nodeService) {
          nmdo::Service service(nodeService.attribute("name").as_string(),
                               ipAddr);
          service.setProtocol(protocol);

          std::string portStr = std::to_string(portNum);
          service.addDstPort(portStr);

          service.setServiceDescription(
              nodeService.attribute("product").as_string());
          service.setServiceReason(nodeService.attribute("method").as_string());

          data.services.push_back(service);
        }
      }
    }

    void
    extractNseAndSsh(pugi::xml_node nmapNode, Data& data)
    {
      for (const auto& xScript :
             nmapNode.select_nodes("host/ports/port/script")) {
        pugi::xml_node nodeScript = xScript.node();
        pugi::xml_node nodePort = nodeScript.parent();
        pugi::xml_node nodeHost = nodePort.parent().parent();

        nmdo::IpAddress ipAddr(extractHostIpAddr(nodeHost));

        NseResult nse;
        nse.port = nmdo::Port(ipAddr);
        nse.port.setProtocol(nodePort.attribute("protocol").as_string());
        nse.port.setPort(nodePort.attribute("portid").as_int());
        nse.scriptId = nodeScript.attribute("id").as_string();
        nse.scriptOutput = nodeScript.attribute("output").as_string();

        data.nseResults.push_back(nse);

        if (nse.scriptId == "ssh-hostkey") {
          for (const auto& xTable : nodeScript.select_nodes("table")) {
            pugi::xml_node nodeTable = xTable.node();

            SshPublicKey key;
            key.port = nse.port;

            key.type = nodeTable.select_node("elem[@key='type']")
              .node().text().as_string();

            key.bits = nodeTable.select_node("elem[@key='bits']")
              .node().text().as_int();

            key.fingerprint =
              nodeTable.select_node("elem[@key='fingerprint']")
                .node().text().as_string();

            key.key = nodeTable.select_node("elem[@key='key']")
              .node().text().as_string();

            data.sshKeys.push_back(key);
          }
        }
        else if (nse.scriptId == "ssh2-enum-algos") {
          for (const auto& xTable : nodeScript.select_nodes("table")) {
            pugi::xml_node nodeTable = xTable.node();

            for (const auto& xElem : nodeTable.select_nodes("elem")) {
              pugi::xml_node nodeElem = xElem.node();

              SshAlgorithm algo;
              algo.port = nse.port;
              algo.type = nodeTable.attribute("key").as_string();
              algo.name = nodeElem.text().as_string();

              data.sshAlgorithms.push_back(algo);
            }
          }
        }
      }
    }
};


int
main(int argc, char** argv)
{
  Tool<nmdp::DummyParser, Results> tool;
  return tool.start(argc, argv);
}

