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

#include <netmeld/datastore/tools/AbstractImportSpiritTool.hpp>

#include "Parser.hpp"

namespace nmdt = netmeld::datastore::tools;


template<typename P, typename R>
class Tool : public nmdt::AbstractImportSpiritTool<P,R>
{
  public:
    Tool() : nmdt::AbstractImportSpiritTool<P,R>
      ("ip addr show", PROGRAM_NAME, PROGRAM_VERSION)
    {}

    void
    toolRunMetadataInserts(pqxx::transaction_base& t) override
    {
      const auto& toolRunId {this->getToolRunId()};

      for (auto& results : this->tResults) {
        for (auto& result: results.ifaces) {
          result.saveAsMetadata(t, toolRunId);
          LOG_DEBUG << "[TRM] " << result.toDebugString() << std::endl;
        }
      }
    }

    void
    specificInserts(pqxx::transaction_base& t) override
    {
      const auto& toolRunId {this->getToolRunId()};
      const auto& deviceId  {this->getDeviceId()};

      for (auto& results : this->tResults) {
        LOG_DEBUG << "Iterating over Interfaces\n";
        for (auto& result: results.ifaces) {
          result.save(t, toolRunId, deviceId);
          LOG_DEBUG << result.toDebugString() << '\n';
        }

        LOG_DEBUG << "Iterating over Observations\n";
        results.observations.save(t, toolRunId, deviceId);
        LOG_DEBUG << results.observations.toDebugString() << "\n";
      }
    }
};

int
main(int argc, char** argv)
{
  Tool<Parser, Result> tool;
  return tool.start(argc, argv);
}
