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

/* Notes:
   - This unit is part of the complilation process to help ensure consistency
     between templates and the actual data
   - Various data is included and most is commented solely for educational
     purposes
     - In non-template, remove data as makes sense

   Guidelines:
   - Base classes contain some implementation (even if NOOP) for every method
     - Method overriding is intentional to alter behaviour, not scope hiding
     - Final has not been used to facilitate new concepts
       - This may change as code base matures
   - Import tools are never "base" classes
   - Data order
     - 1st tier: Variables, Constructors, Methods
     - 2nd tier: private, protected, public
   - Section headers should generally be left to help code organization
   - Parser logic should be separate
   - Most functions or data should be scoped to protected to facilitate
     unit testing.
*/

#include <netmeld/datastore/tools/AbstractImportSpiritTool.hpp>
#include <netmeld/datastore/parsers/ParserHelper.hpp> // if parser not needed

#include "Parser.hpp"

namespace nmdo = netmeld::datastore::objects;
namespace nmdp = netmeld::datastore::parsers;
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
  private: // Variables should rarely appear at this scope
  protected: // Variables intended for internal/subclass API
    // Inhertied from AbstractTool at this scope
      // std::string            helpBlurb;
      // std::string            programName;
      // std::string            version;
      // ProgramOptions         opts;
    // Inhertied from AbstractImportSpiritTool at this scope
      // TResults                 tResults;
      // nmco::Uuid               toolRunId;
      // nmco::Time               executionStart;
      // nmco::Time               executionStop;
      // nmco::DeviceInformation  devInfo;
  public: // Variables should rarely appear at this scope


  // ===========================================================================
  // Constructors
  // ===========================================================================
  private: // Constructors should rarely appear at this scope
  protected: // Constructors intended for internal/subclass API
  public: // Constructors should generally be public
    Tool() : nmdt::AbstractImportSpiritTool<P,R>
      ( "some-command --flag"   // command line tool imports data from
      , PROGRAM_NAME            // program name (set in CMakeLists.txt)
      , PROGRAM_VERSION         // program version (set in CMakeLists.txt)
      )
    {}


  // ===========================================================================
  // Methods
  // ===========================================================================
  private: // Methods part of internal API
    // Overriden from AbstractImportSpiritTool
    void
    addToolOptions() override
    {
      this->opts.addRequiredOption(
          "tool-option"
        , std::make_tuple( "tool-option"
                         , po::value<std::string>()->required()
                         , "Some tool option"
                         )
        );

      this->opts.removeOptionalOption("pipe");
      this->opts.removeAdvancedOption("tool-run-metadata");
    }

    // Overriden from AbstractImportSpiritTool
    void
    specificInserts(pqxx::transaction_base& t) override
    {
      const auto& toolRunId {this->getToolRunId()};
      const auto& deviceId  {this->getDeviceId()};

      for (auto& result : this->tResults) {
        // muck

        // save
        LOG_DEBUG << result.toDebugString() << std::endl;
        result.save(t, toolRunId, deviceId);

        // link
      }
    }

  protected: // Methods part of subclass API
    // Inherited from AbstractTool at this scope
      // std::string const getDbName() const;
      // virtual void printVersion() const;
    // Inherited from AbstractImportSpiritTool at this scope
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
int main(int argc, char** argv)
{
  Tool<nmdp::DummyParser, Result> tool; // if parser not needed
  //Tool<Parser, Result> tool; // if parser needed
  return tool.start(argc, argv);
}
