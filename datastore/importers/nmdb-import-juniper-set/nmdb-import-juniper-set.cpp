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

#include <netmeld/datastore/tools/AbstractImportSpiritTool.hpp>

#include "Parser.hpp"

namespace nmdo = netmeld::datastore::objects;
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
  private: // Variables should generally be private
  protected: // Variables intended for internal/subclass API
  public: // Variables should rarely appear at this scope


  // ===========================================================================
  // Constructors
  // ===========================================================================
  private: // Constructors should rarely appear at this scope
  protected: // Constructors intended for internal/subclass API
  public: // Constructors should generally be public
    Tool() : nmdt::AbstractImportSpiritTool<P,R>
      (
       "get config",    // command line tool imports data from
       PROGRAM_NAME,    // program name (set in CMakeLists.txt)
       PROGRAM_VERSION  // program version (set in CMakeLists.txt)
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

      auto& devInfo {this->devInfo};
      devInfo.setVendor("Juniper");
      if (this->opts.exists("device-type")) {
        devInfo.setDeviceType(this->opts.getValue("device-type"));
      }
      devInfo.save(t, toolRunId);
      LOG_DEBUG << devInfo.toDebugString() << '\n';

      LOG_DEBUG << "Iterating over results\n";
      for (auto& results : this->tResults) {
        LOG_DEBUG << "Iterating over interfaces\n";
        for (auto& [_, result] : results.ifaces) {
          result.save(t, toolRunId, deviceId);
          LOG_DEBUG << result.toDebugString() << '\n';
        }

        LOG_DEBUG << "Iterating over routes\n";
        //for (auto& [_, result] : results.routes) {
        for (auto& result : results.routes) {
          result.save(t, toolRunId, deviceId);
          LOG_DEBUG << result.toDebugString() << '\n';
        }

        LOG_DEBUG << "Iterating over networkBooks\n";
        for (auto& [zone, nets] : results.networkBooks) {
          for (auto& [name, book] : nets) {
            book.setId(zone);
            book.setName(name);
            book.save(t, toolRunId, deviceId);
            LOG_DEBUG << book.toDebugString() << '\n';
          }
        }
        LOG_DEBUG << "Iterating over serviceBooks\n";
        for (auto& [zone, apps] : results.serviceBooks) {
          for (auto& [name, book] : apps) {
            book.setId(zone);
            book.setName(name);
            book.save(t, toolRunId, deviceId);
            LOG_DEBUG << book.toDebugString() << '\n';
          }
        }
        LOG_DEBUG << "Iterating over ruleBooks\n";
        for (auto& [_, book] : results.ruleBooks) {
          for (auto& [_, rule] : book) {
            rule.save(t, toolRunId, deviceId);
            LOG_DEBUG << rule.toDebugString() << '\n';
          }
        }
        LOG_DEBUG << "Iterating over Observations\n";
        results.observations.save(t, toolRunId, deviceId);
        LOG_DEBUG << results.observations.toDebugString() << "\n";
      }
    }

  protected: // Methods part of subclass API
  public: // Methods part of public API
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
