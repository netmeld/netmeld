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
   - Export tools are never "base" classes
   - Data order
     - 1st tier: Variables, Constructors, Methods
     - 2nd tier: private, protected, public
   - Section headers should generally be left to help code organization
   - A custom parser isn't typically needed, but if so follow standard parser
     rules (see other templates)
*/

#include <netmeld/datastore/tools/AbstractExportTool.hpp>
#include <netmeld/datastore/objects/AbstractDatastoreObject.hpp>

namespace nmco = netmeld::core::objects;
namespace nmcu = netmeld::core::utils;
namespace nmdo = netmeld::datastore::objects;
namespace nmdt = netmeld::datastore::tools;


// =============================================================================
// Import tool definition
// =============================================================================
class Tool : public nmdt::AbstractExportTool
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
  public: // Variables should rarely appear at this scope


  // ===========================================================================
  // Constructors
  // ===========================================================================
  private: // Constructors should rarely appear at this scope
  protected: // Constructors intended for internal/subclass API
  public: // Constructors should generally be public
    Tool() : nmdt::AbstractExportTool
      (
       "help blurb",    // help blurb, prefixed with:
                        //   "Generate "
       PROGRAM_NAME,    // program name (set in CMakeLists.txt)
       PROGRAM_VERSION  // program version (set in CMakeLists.txt)
      )
    {}


  // ===========================================================================
  // Methods
  // ===========================================================================
  private: // Methods part of internal API
    // Overriden from AbstractExportTool
    void
    modifyToolOptions() override
    {
      // This method probably needs customization

      opts.addOptionalOption("tool-option", std::make_tuple(
            "tool-option",
            po::value<std::string>(),
            "Some tool option")
          );
    }

    // Overriden from AbstractExportTool
    int
    runTool() override
    {
      // Contains tool's primary logic

      const auto& dbName  {getDbName()};
      const auto& dbArgs  {opts.getValue("db-args")};
      pqxx::connection db {"dbname=" + dbName + " " + dbArgs};

      db.prepare
        ("select_tool_runs",
         "SELECT *"
         " FROM tool_runs");

      db.prepare
        ("select_tool_runs_with_args",
         "SELECT DISTINCT"
         "   tool_name, data_path"
         " FROM tool_runs"
         " WHERE ($1 = tool_name)"
         "   AND ($2 = data_path)");

      // Use a read_transaction and do not commit()
      pqxx::read_transaction t {db};

      pqxx::result toolRuns =
        t.exec_prepared("select_tool_runs");
          // any arguments separated by commas after table name

      for (const auto& toolRun : toolRuns) {
        nmco::Uuid uuid;
        toolRun.at("id").to(uuid);
        std::string toolName;
        toolRun.at("tool_name").to(toolName);
        std::string commandLine;
        toolRun.at("command_line").to(commandLine);

        LOG_INFO << uuid << std::endl
                 << "  " <<toolName << "::" << commandLine << std::endl;
      }

      return nmcu::Exit::SUCCESS;
    }

  protected: // Methods part of subclass API
    // Inherited from AbstractTool at this scope
      // virtual void printVersion() const;
    // Inherited from AbstractDatastoreTool at this scope
      // std::string const getDbName() const;
    // Inherited from AbstractExportTool at this scope
      // virtual void printHelp() const;
  public: // Methods part of public API
    // Inherited from AbstractTool, don't override as primary tool entry point
      // int start(int, char**) noexcept;
};


// =============================================================================
// Program entry point
// =============================================================================
int main(int argc, char** argv) {
  Tool tool;
  return tool.start(argc, argv);
}
