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

namespace nmco = netmeld::core::objects;
namespace nmct = netmeld::core::tools;


// =============================================================================
// Import tool definition
// =============================================================================
template<typename P, typename R>
class Tool : public nmct::AbstractImportTool<P,R>
{
  // ===========================================================================
  // Variables
  // ===========================================================================


  // ===========================================================================
  // Constructors
  // ===========================================================================
  public:
    Tool() : nmct::AbstractImportTool<P,R>
      (
       "show running-config",
       PROGRAM_NAME,
       PROGRAM_VERSION
      )
    {}

  // ===========================================================================
  // Methods
  // ===========================================================================
  private: // Methods part of internal API
    void
    specificInserts(pqxx::transaction_base& t) override
    {
      const auto& toolRunId        {this->getToolRunId()};
      const auto& defaultDeviceId  {this->getDeviceId()};

      bool first {true};

      LOG_DEBUG << "Iterating over results\n";
      for (auto& results : this->tResults) {
        auto deviceId {defaultDeviceId};

        LOG_DEBUG << "Adding device information\n";
        auto& devInfo {results.devInfo};
        if (!first && !devInfo.getDeviceId().empty()) {
          deviceId = defaultDeviceId + ":" + devInfo.getDeviceId();
        }
        devInfo.setDeviceId(deviceId);
        if (this->opts.exists("device-type") &&
            devInfo.getDeviceType().empty())
        {
          devInfo.setDeviceType(this->opts.getValue("device-type"));
        }
        if (this->opts.exists("device-color")) {
          devInfo.setDeviceColor(this->opts.getValue("device-color"));
        }
        devInfo.save(t, toolRunId);
        LOG_DEBUG << devInfo.toDebugString() << '\n';

        LOG_DEBUG << "Iteration over AAAs\n";
        for (auto& result : results.aaas) {
          // 04-03-2019 NOTE: Manually saving here because we do not have nor
          // do we want a netmeld core object for AAA entries at this time.
          t.exec_prepared("insert_raw_device_aaa",
              toolRunId,
              deviceId,
              result);

          LOG_DEBUG << result << std::endl;
        }

        LOG_DEBUG << "Iterating over Observations\n";
        results.observations.save(t, toolRunId, deviceId);
        LOG_DEBUG << results.observations.toDebugString() << std::endl;

        LOG_DEBUG << "Iterating over Services\n";
        for (auto& result : results.services) {
          result.save(t, toolRunId, "");
          LOG_DEBUG << result.toDebugString() << std::endl;
        }

        LOG_DEBUG << "Iterating over routes\n";
        for (auto& result : results.routes) {
          result.save(t, toolRunId, deviceId);
          LOG_DEBUG << result.toDebugString() << std::endl;
        }

        LOG_DEBUG << "Iterating over vlans\n";
        for (auto& result : results.vlans) {
          result.save(t, toolRunId, deviceId);
          LOG_DEBUG << result.toDebugString() << std::endl;
        }

        LOG_DEBUG << "Iterating over interfaces\n";
        for (auto& [name, result] : results.ifaces) {
          result.save(t, toolRunId, deviceId);
          LOG_DEBUG << result.toDebugString() << std::endl;
        }

        first = false;
      }
    }
};


// =============================================================================
// Program entry point
// =============================================================================
int
main(int argc, char** argv)
{
  Tool<Parser, Result> tool;
  return tool.start(argc, argv);
}
