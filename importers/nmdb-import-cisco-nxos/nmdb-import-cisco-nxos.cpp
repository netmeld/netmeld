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

#include <netmeld/core/tools/AbstractImportTool.hpp>

#include "Parser.hpp"

namespace nmct = netmeld::core::tools;
namespace nmcu = netmeld::core::utils;


// =============================================================================
// Import tool definition
// =============================================================================
template<typename P, typename R>
class Tool : public nmct::AbstractImportTool<P,R>
{
  public:
    Tool() : nmct::AbstractImportTool<P,R>
      (
       "Cisco NX-OS show running-config",
       PROGRAM_NAME,
       PROGRAM_VERSION
      )
    {}

    void
    modifyToolOptions() override
    {
      this->opts.removeRequiredOption("device-id");
      this->opts.removeAdvancedOption("tool-run-metadata");
    }

    void
    specificInserts(pqxx::transaction_base& t) override
    {
      const auto& toolRunId {this->getToolRunId()};
      auto& results         {this->tResults};

      for (auto& objects : results) {
        auto& devInfo {objects.devInfo};
        if (this->opts.exists("device-color")) {
          devInfo.setDeviceColor(this->opts.getValue("device-color"));
        }
        if (this->opts.exists("device-type")) {
          devInfo.setDeviceType(this->opts.getValue("device-type"));
        }
        LOG_DEBUG << devInfo.toDebugString() << '\n';
        devInfo.save(t, toolRunId);

        const auto& deviceId {devInfo.getDeviceId()};
        // Process the rest of the results
        LOG_DEBUG << "Iterating over vlans\n";
        for (auto& result : objects.vlans) {
          LOG_DEBUG << result.toDebugString() << std::endl;
          result.save(t, toolRunId, deviceId);
        }

        LOG_DEBUG << "Iterating over ifaces\n";
        for (auto& result : objects.ifaces) {
          LOG_DEBUG << result.toDebugString() << std::endl;
          result.save(t, toolRunId, deviceId);
        }

        LOG_DEBUG << "Iterating over routes\n";
        for (auto& result : objects.routes) {
          LOG_DEBUG << result.toDebugString() << std::endl;
          result.save(t, toolRunId, deviceId);
        }

        LOG_DEBUG << "Iterating over services\n";
        for (auto& result : objects.services) {
          LOG_DEBUG << result.toDebugString() << std::endl;
          result.save(t, toolRunId, "");
        }

        LOG_DEBUG << "Iterating over observations:\n";
        LOG_DEBUG << objects.observations.toDebugString() << std::endl;
        objects.observations.save(t, toolRunId, deviceId);
      }
    }
};

int
main(int argc, char** argv)
{
  Tool<Parser, Result> tool;
  return tool.start(argc, argv);
}
