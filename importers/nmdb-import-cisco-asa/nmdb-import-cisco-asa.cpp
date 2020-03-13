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
namespace nmcu = netmeld::core::utils;


// =============================================================================
// Import tool definition
// =============================================================================
template<typename P, typename R>
class Tool : public nmct::AbstractImportTool<P,R>
{
  // ===========================================================================
  // Variables
  // ===========================================================================
  private: // Variables should generally be private
  protected: // Variables intended for internal/subclass API
    // Inhertied from AbstractTool at this scope
      // std::string            helpBlurb;
      // std::string            programName;
      // std::string            version;
      // ProgramOptions         opts;
    // Inhertied from AbstractImportTool at this scope
      // TResults               tResults;
      // nmco::Uuid              toolRunId;
      // nmco::Time              executionStart;
      // nmco::Time              executionStop;
  public: // Variables should rarely appear at this scope


  // ===========================================================================
  // Constructors
  // ===========================================================================
  private: // Constructors should rarely appear at this scope
  protected: // Constructors intended for internal/subclass API
  public: // Constructors should generally be public
    Tool() : nmct::AbstractImportTool<P,R>
      (
       "show running-config all",  // command line tool imports data from
       PROGRAM_NAME,               // program name (set in CMakeLists.txt)
       PROGRAM_VERSION             // program version (set in CMakeLists.txt)
      )
    {}


  // ===========================================================================
  // Methods
  // ===========================================================================
  private: // Methods part of internal API
    // Overriden from AbstractImportTool
    void
    modifyToolOptions() override
    {
      this->opts.removeRequiredOption("device-id");
      this->opts.removeAdvancedOption("tool-run-metadata");
    }

    // Overriden from AbstractImportTool
    void
    specificInserts(pqxx::transaction_base& t) override
    {
      const auto& toolRunId {this->getToolRunId()};
      auto& results         {this->tResults};

      LOG_DEBUG << "Iterating over results\n";
      for (auto& objects : results) {
        auto& devInfo {objects.devInfo};
        if (this->opts.exists("device-color")) {
          devInfo.setDeviceColor(this->opts.getValue("device-color"));
        }
        if (this->opts.exists("device-type")) {
          devInfo.setDeviceType(this->opts.getValue("device-type"));
        }
        devInfo.save(t, toolRunId);
        LOG_DEBUG << devInfo.toDebugString() << '\n';

        const auto& deviceId {devInfo.getDeviceId()};

        // Process the rest of the results
        LOG_DEBUG << "Iterating over ifaces\n";
        for (auto& result : objects.ifaces) {
          result.save(t, toolRunId, deviceId);
          LOG_DEBUG << result.toDebugString() << '\n';
        }

        LOG_DEBUG << "Iterating over networkBooks\n";
        for (auto& [zone, nets] : objects.networkBooks) {
          for (auto& [name, book] : nets) {
            book.setId(zone);
            book.setName(name);
            book.save(t, toolRunId, deviceId);
            LOG_DEBUG << book.toDebugString() << '\n';
          }
        }
        LOG_DEBUG << "Iterating over serviceBooks\n";
        for (auto& [zone, apps] : objects.serviceBooks) {
          for (auto& [name, book] : apps) {
            book.setId(zone);
            book.setName(name);
            book.save(t, toolRunId, deviceId);
            LOG_DEBUG << book.toDebugString() << '\n';
          }
        }
        LOG_DEBUG << "Iterating over ruleBooks\n";
        for (auto& [name, book] : objects.ruleBooks) {
          LOG_DEBUG << name << '\n';
          for (auto& [id, rule] : book) {
            rule.save(t, toolRunId, deviceId);
            LOG_DEBUG << rule.toDebugString() << '\n';
          }
        }

        LOG_DEBUG << "Iterating over Observations\n";
        objects.observations.save(t, toolRunId, deviceId);
        LOG_DEBUG << objects.observations.toDebugString() << "\n";
      }
    }

  protected: // Methods part of subclass API
    // Inherited from AbstractTool at this scope
      // std::string const getDbName() const;
      // virtual void printVersion() const;
    // Inherited from AbstractImportTool at this scope
      // fs::path    const getDataPath() const;
      // std::string const getDeviceId() const;
      // nmco::Uuid   const getToolRunId() const;
      // virtual void parseData();
      // virtual void printHelp() const;
      // virtual int  runTool();
      // virtual void setToolRunId();
      // virtual void toolRunMetadataInserts(pqxx::transaction_base&) const;
  public: // Methods part of public API
    // Inherited from AbstractTool, don't override as primary tool entry point
      // int start(int, char**) noexcept;
};


// =============================================================================
// Program entry point
// =============================================================================
int main(int argc, char** argv) {
  Tool<Parser, Result> tool; // if parser needed
  return tool.start(argc, argv);
}
