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

#include <regex>

#include <pugixml.hpp>

#include <netmeld/datastore/objects/IpAddress.hpp>
#include <netmeld/datastore/parsers/ParserHelper.hpp>
#include <netmeld/datastore/tools/AbstractImportTool.hpp>

#include "ParserNmapXml.hpp"

namespace nmdo = netmeld::datastore::objects;
namespace nmdp = netmeld::datastore::parsers;
namespace nmdt = netmeld::datastore::tools;
namespace nmdu = netmeld::datastore::utils;


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

      ParserNmapXml nxp;

      auto t {nxp.extractExecutionTiming(nmapNode)};

      this->executionStart.readUnixTimestamp(std::get<0>(t));
      this->executionStop.readUnixTimestamp(std::get<1>(t));

      LOG_DEBUG << "[nmco] Start: " << this->executionStart << std::endl;
      LOG_DEBUG << "[nmco] Stop : " << this->executionStop << std::endl;

      Data data;

      nxp.extractMacAndIpAddrs(nmapNode, data);
      nxp.extractHostnames(nmapNode, data);
      nxp.extractOperatingSystems(nmapNode, data);
      nxp.extractTraceRoutes(nmapNode, data);
      nxp.extractPortsAndServices(nmapNode, data);
      nxp.extractNseAndSsh(nmapNode, data);

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
          LOG_DEBUG << result.toString() << std::endl;
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
          LOG_DEBUG << result.toString() << std::endl;
        }

        LOG_DEBUG << "Iterating over sshKeys\n";
        for (auto& result : results.sshKeys) {
          result.save(t, toolRunId, "");
          LOG_DEBUG << result.toString() << std::endl;
        }

        LOG_DEBUG << "Iterating over sshAlgorithms\n";
        for (auto& result : results.sshAlgorithms) {
          result.save(t, toolRunId, "");
          LOG_DEBUG << result.toString() << std::endl;
        }
      }
    }

  private:
};


int
main(int argc, char** argv)
{
  Tool<nmdp::DummyParser, Result> tool;
  return tool.start(argc, argv);
}
