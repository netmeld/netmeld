// =============================================================================
// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
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

// =============================================================================
// Import tool definition
// =============================================================================
template<typename P, typename R>
class Tool : public nmdt::AbstractImportSpiritTool<P,R>
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
    Tool() : nmdt::AbstractImportSpiritTool<P,R>
      (
       "apk list -I -v",       // command line tool imports data from
       PROGRAM_NAME,           // program name (set in CMakeLists.txt)
       PROGRAM_VERSION         // program version (set in CMakeLists.txt)
      )
    {}

  // ===========================================================================
  // Methods
  // ===========================================================================
  private: // Methods part of internal API
    void
    specificInserts(pqxx::transaction_base& t) override
    {
      const auto& toolRunId {this->getToolRunId()};
      const auto& deviceId  {this->getDeviceId()};
      for (auto& results : this->tResults) {
        LOG_DEBUG << "Iterating over Packages";
        for(auto& result : results.packages){
          result.save(t, toolRunId, deviceId);
          LOG_DEBUG << result.toDebugString() << std::endl;
        }
        LOG_DEBUG << "Iterating over Observations\n";
        results.observations.save(t, toolRunId, deviceId);
        LOG_DEBUG << results.observations.toDebugString() << "\n";
      }
    }

  protected:
  public:
};

// =============================================================================
// Program entry point
// =============================================================================
int
main(int argc, char** argv) {
  Tool<Parser, Result> tool;
  return tool.start(argc, argv);
}
