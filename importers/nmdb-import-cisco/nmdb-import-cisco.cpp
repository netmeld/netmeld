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

namespace nmo  = netmeld::core::objects;
namespace tools = netmeld::core::tools;
namespace utils = netmeld::core::utils;


// =============================================================================
// Import tool definition
// =============================================================================
template<typename P, typename R>
class Tool : public tools::AbstractImportTool<P,R>
{
  public:
    Tool() : tools::AbstractImportTool<P,R>
      (
       "show running-config",
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
      auto results   = this->tResults;
      auto toolRunId = this->getToolRunId();

      for (auto& objects : results) {
        LOG_DEBUG << "HARDWARE INFO:\n";
        auto deviceId = objects.dhi.getDeviceId();

        auto dhi = objects.dhi;
        if (this->opts.exists("device-type")) {
          dhi.setDeviceType(this->opts.getValue("device-type"));
        }
        if (this->opts.exists("device-color")) {
          dhi.setDeviceColor(this->opts.getValue("device-color"));
        }
        dhi.save(t, toolRunId, deviceId);
        LOG_DEBUG << dhi.toDebugString() << std::endl;

        for (auto& result : objects.aaas) {
          // 04-03-2019 NOTE: Manually saving here because we do not have nor
          // do we want a netmeld core object for AAA entries at this time.
          t.exec_prepared("insert_raw_device_aaa",
              toolRunId,
              deviceId,
              result);

          LOG_DEBUG << result << std::endl;
        }

        LOG_DEBUG << "OBSERVATIONS:\n";
        LOG_DEBUG << objects.observations.toDebugString() << std::endl;
        objects.observations.save(t, toolRunId, deviceId);

        LOG_DEBUG << "SERVICE INFO:\n";
        for (auto& result : objects.services) {
          LOG_DEBUG << result.toDebugString() << std::endl;
          result.save(t, toolRunId, "");
        }

        LOG_DEBUG << "ROUTE INFO:\n";
        for (auto& result : objects.routes) {
          LOG_DEBUG << result.toDebugString() << std::endl;
          result.save(t, toolRunId, deviceId);
        }

        LOG_DEBUG << "VLAN INFO:\n";
        for (auto& result : objects.vlans) {
          LOG_DEBUG << result.toDebugString() << std::endl;
          result.save(t, toolRunId, deviceId);
        }

        LOG_DEBUG << "INTERFACE INFO:\n";
        for (auto& result : objects.ifaces) {
          LOG_DEBUG << result.toDebugString() << std::endl;
          result.save(t, toolRunId, deviceId);
        }
      }
    }
};

int
main(int argc, char** argv)
{
  Tool<Parser, Result> tool;
  return tool.start(argc, argv);
}
