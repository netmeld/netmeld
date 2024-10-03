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

#include <netmeld/datastore/tools/AbstractImportJsonTool.hpp>

#include "Parser.hpp"

namespace nmdo = netmeld::datastore::objects;
namespace nmdt = netmeld::datastore::tools;


template<typename P, typename R>
class Tool : public nmdt::AbstractImportJsonTool<P,R>
{
  public:
    Tool() : nmdt::AbstractImportJsonTool<P,R>
      ("F5 REST API JSON output (.json files)"
      , PROGRAM_NAME
      , PROGRAM_VERSION
      )
    {
      this->devInfo.setVendor("F5");
    }

    void
    specificInserts(pqxx::transaction_base& t) override
    {
      const auto& toolRunId{this->getToolRunId()};

      LOG_DEBUG << "Iterating over results:" << std::endl;
      for (auto& result : this->tResults) {
        nmdo::DeviceInformation deviceInfo{this->devInfo};
        const std::string baseDeviceId{deviceInfo.getDeviceId()};
        result.observations.save(t, toolRunId, baseDeviceId);

        LOG_DEBUG << "Iterating over logical-systems:" << std::endl;
        for (auto& [logicalSystemName, logicalSystem] : result.logicalSystems) {
          LOG_DEBUG << "logicalSystemName: " << logicalSystemName << std::endl;
          std::string deviceId{baseDeviceId};
          if (!logicalSystemName.empty()) {
            deviceId += (":" + logicalSystemName);
          }
          deviceInfo.setDeviceId(deviceId);
          LOG_DEBUG << deviceInfo.toDebugString() << std::endl;
          deviceInfo.save(t, toolRunId);

          LOG_DEBUG << "Iterating over interfaces:" << std::endl;
          for (auto& [_, iface] : logicalSystem.ifaces) {
            LOG_DEBUG << iface.toDebugString() << std::endl;
            iface.save(t, toolRunId, deviceId);
          }

          LOG_DEBUG << "Iterating over VRFs:" << std::endl;
          for (auto& [_, vrf] : logicalSystem.vrfs) {
            LOG_DEBUG << vrf.toDebugString() << std::endl;
            vrf.save(t, toolRunId, deviceId);
          }
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

