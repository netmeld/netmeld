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

#include <netmeld/core/objects/IpAddress.hpp>
#include <netmeld/core/tools/AbstractTool.hpp>

#include <netmeld/playbook/core/utils/QueriesPlaybook.hpp>

namespace nmco = netmeld::core::objects;
namespace nmct = netmeld::core::tools;
namespace nmcu = netmeld::core::utils;
namespace nmpbcu = netmeld::playbook::core::utils;


class Tool : public nmct::AbstractTool
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
      // nmco::ProgramOptions   opts;

  public: // Variables should rarely appear at this scope


  // ===========================================================================
  // Constructors
  // ===========================================================================
  private: // Constructors should rarely appear at this scope

  protected: // Constructors intended for internal/subclass API

  public: // Constructors should generally be public
    Tool() : nmct::AbstractTool
      (
       "playbook tool", // unused unless printHelp() is overridden
       PROGRAM_NAME,    // program name (set in CMakeLists.txt)
       PROGRAM_VERSION  // program version (set in CMakeLists.txt)
      )
    {}


  // ===========================================================================
  // Methods
  // ===========================================================================
  private: // Methods part of internal API
    // Overriden from AbstractTool
    void
    addToolBaseOptions() override // Pre-subclass operations
    {
      opts.addRequiredOption("ip-addr", std::make_tuple(
            "ip-addr",
            po::value<std::string>()->required(),
            "IP address of router to use")
          );

      opts.removeOptionalOption("pipe");
      opts.removeAdvancedOption("tool-run-metadata");
    }

    // Overriden from AbstractTool
    void
    modifyToolOptions() override {}

    int
    runTool() override
    {
      const auto& dbName  {getDbName()};
      const auto& dbArgs  {opts.getValue("db-args")};
      pqxx::connection db {"dbname=" + dbName + " " + dbArgs};
      nmpbcu::dbPreparePlaybook(db);

      nmco::IpAddress ipAddr {opts.getValue("ip-addr")};

      if (ipAddr.isValid()) {
        pqxx::work t{db};

        t.exec_prepared("insert_playbook_ip_router",
            ipAddr.toString());

        t.commit();
      } else {
        LOG_WARN << "Invalid router IP address provided, nothing done" << std::endl;
      }

      return nmcu::Exit::SUCCESS;
    }

  protected: // Methods part of subclass API
    // Inherited from AbstractTool at this scope
      // std::string const getDbName() const;
      // virtual void printHelp() const;
      // virtual void printVersion() const;
      // virtual void runTool();

  public: // Methods part of public API
    // Inherited from AbstractTool, don't override as primary tool entry point
      // int start(int, char**) noexcept;
};


// =============================================================================
// Program entry point
// =============================================================================
int main(int argc, char** argv)
{
  Tool tool;
  return tool.start(argc, argv);
}
