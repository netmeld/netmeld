// =============================================================================
// Copyright 2022 National Technology & Engineering Solutions of Sandia, LLC
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

using json = nlohmann::json;

// =============================================================================
// Import tool definition
// =============================================================================
template<typename P, typename R>
class Tool : public nmdt::AbstractImportJsonTool<P,R>
{
  // ===========================================================================
  // Variables
  // ===========================================================================
  private: // Variables should generally be private
  protected: // Variables intended for internal/subclass API
  public: // Variables should rarely appear at this scope


  // ===========================================================================
  // Constructors
  // ===========================================================================
  private: // Constructors should rarely appear at this scope
  protected: // Constructors intended for internal/subclass API
  public: // Constructors should generally be public
    Tool() : nmdt::AbstractImportJsonTool<P,R>
      (
       "aws ec2 describe-instances",  // command line tool imports data from
       PROGRAM_NAME,           // program name (set in CMakeLists.txt)
       PROGRAM_VERSION         // program version (set in CMakeLists.txt)
      )
    {}


  // ===========================================================================
  // Methods
  // ===========================================================================
  private: // Methods part of internal API
    // Overriden from AbstractImportJsonTool
    void
    addToolOptions() override
    {
      this->opts.removeRequiredOption("device-id");
      this->opts.addAdvancedOption("device-id", std::make_tuple(
            "device-id",
            po::value<std::string>(),
            "(Not used) Name of device.")
          );

      this->opts.removeOptionalOption("device-type");
      this->opts.removeOptionalOption("device-color");
    }

    // Overriden from AbstractImportJsonTool
    void
    specificInserts(pqxx::transaction_base& t) override
    {
      const auto& toolRunId {this->getToolRunId()};
      const auto& deviceId  {this->getDeviceId()};

      for (auto& results : this->tResults) {
        LOG_DEBUG << "Iterating over AWS Instances\n";
        for (auto& result : results.instances) {
          result.save(t, toolRunId, deviceId);
          LOG_DEBUG << result.toDebugString() << std::endl;
        }

        LOG_DEBUG << "Iterating over Observations\n";
        results.observations.save(t, toolRunId, deviceId);
        LOG_DEBUG << results.observations.toDebugString() << '\n';
      }
    }

  protected: // Methods part of subclass API
  public: // Methods part of public API
};


// =============================================================================
// Program entry point
// =============================================================================
int main(int argc, char** argv) {
  Tool<Parser, Result> tool;
  return tool.start(argc, argv);
}
