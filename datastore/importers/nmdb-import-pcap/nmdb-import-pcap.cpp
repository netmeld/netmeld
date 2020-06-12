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

#include <netmeld/datastore/tools/AbstractImportTool.hpp>
#include <netmeld/datastore/parsers/ParserHelper.hpp>

#include "Parser.hpp"

namespace nmco = netmeld::core::objects;
namespace nmdp = netmeld::datastore::parsers;
namespace nmdt = netmeld::datastore::tools;


template<typename P, typename R>
class Tool : public nmdt::AbstractImportTool<P,R>
{
  public:
    Tool() : nmdt::AbstractImportTool<P,R>
      ("pcap file", PROGRAM_NAME, PROGRAM_VERSION)
    {}

    void
    addToolOptions() override
    {
      this->opts.removeRequiredOption("device-id");
      this->opts.addOptionalOption("device-id", std::make_tuple(
            "device-id",
            po::value<std::string>(),
            "Name of device.")
          );

      this->opts.removeOptionalOption("device-type");
    }

    void
    parseData() override
    {
      const auto& dataFile {this->getDataPath().string()};
      Parser p;

      this->executionStart = nmco::Time();
      this->tResults = p.processFile(dataFile);
      this->executionStop = nmco::Time();
    }

    void
    specificInserts(pqxx::transaction_base& t) override
    {
      const auto& toolRunId {this->getToolRunId()};
      const auto& deviceId  {this->getDeviceId()};

      for (auto& results : this->tResults) {
        for (auto& [id, result]: results.macAddrs) {
          result.save(t, toolRunId, deviceId);
          LOG_DEBUG << result.toDebugString() << std::endl;
        }
        for (auto& [id, result] : results.ipAddrs) {
          result.save(t, toolRunId, deviceId);
          LOG_DEBUG << result.toDebugString() << std::endl;
        }
        for (auto& [id, result] : results.vlans) {
          result.save(t, toolRunId, deviceId);
          LOG_DEBUG << result.toDebugString() << std::endl;
        }

        results.observations.save(t, toolRunId, deviceId);
      }
    }
};


int
main(int argc, char** argv)
{
  Tool<nmdp::DummyParser, Result> tool;
  return tool.start(argc, argv);
}
