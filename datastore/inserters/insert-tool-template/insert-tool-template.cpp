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
   - Insert tools are never "base" classes
   - Data order
     - 1st tier: Variables, Constructors, Methods
     - 2nd tier: private, protected, public
   - Section headers should generally be left to help code organization
   - A custom parser isn't typically needed, but if so follow standard parser
     rules (see other templates)
*/

#include <netmeld/datastore/objects/AbstractDatastoreObject.hpp>
#include <netmeld/datastore/tools/AbstractInsertTool.hpp>

namespace nmdo = netmeld::datastore::objects;
namespace nmdt = netmeld::datastore::tools;


// =============================================================================
// Insert tool definition
// =============================================================================
class Tool : public nmdt::AbstractInsertTool
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
    // Inhertied from AbstractInsertTool at this scope
      // nmco::Uuid              toolRunId;
  public: // Variables should rarely appear at this scope


  // ===========================================================================
  // Constructors
  // ===========================================================================
  private: // Constructors should rarely appear at this scope
  protected: // Constructors intended for internal/subclass API
  public: // Constructors should generally be public
    Tool() : nmdt::AbstractInsertTool
      (
       "help blurb",    // help message, prefixed with:
                        //   "Insert a manually specified "
       PROGRAM_NAME,    // program name (set in CMakeLists.txt)
       PROGRAM_VERSION  // program version (set in CMakeLists.txt)
      )
    {}


  // ===========================================================================
  // Methods
  // ===========================================================================
  private: // Methods part of internal API
    // Overriden from AbstractInsertTool
    void
    addToolOptions() override
    {
      // This method is typically heavily customized/populated

      opts.addRequiredOption("tool-option", std::make_tuple(
            "tool-option",
            po::value<std::string>()->required(),
            "Some tool option")
          );

      // Option order is sorted, alter for grouping by adding prefix to 1st arg
      opts.addOptionalOption("sort-order-01-option-b", std::make_tuple(
            "option-b",
            po::value<std::string>()->default_value("default value"),
            "Some tool option")
          );
      opts.addOptionalOption("sort-order-02-option-a", std::make_tuple(
            "option-a",
            po::value<std::string>(),
            "Some tool option")
          );
    }

    // Overriden from AbstractInsertTool
    void
    specificInserts(pqxx::transaction_base& t) override
    {
      // Contains tool's primary logic
      // const auto& deviceId {getDeviceId(}); // if device-id defined
      const auto& toolRunId {getToolRunId()};

      // Object construction from command line args
      if (opts.exists("option-a")) {
        nmdo::AbstractDatastoreObject ao;

        // Alter object state based on command line args

        // Save finalized object
        ao.save(t, toolRunId, ""); // if device-id is not defined
        //ao.save(t, toolRunId, deviceId) //if device-id defined
        LOG_DEBUG << ao.toDebugString() << std::endl;
      }
    }

  protected: // Methods part of subclass API
    // Inherited from AbstractTool at this scope
      // std::string const getDbName() const;
      // virtual void printVersion() const;
    // Inherited from AbstractInsertTool at this scope
      // std::string const getDeviceId() const;
      // nmco::Uuid   const getToolRunId() const;
      // virtual void printHelp() const;
      // virtual int  runTool();
      // virtual void setToolRunId();
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
