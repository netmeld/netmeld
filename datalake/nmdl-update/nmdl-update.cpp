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

#include <netmeld/datalake/core/objects/DataEntry.hpp>
#include <netmeld/datalake/core/tools/AbstractDataLakeTool.hpp>

namespace nmdlco = netmeld::datalake::core::objects;
namespace nmdlct = netmeld::datalake::core::tools;


class Tool : public nmdlct::AbstractDataLakeTool
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
    Tool() : nmdlct::AbstractDataLakeTool
      (
       "add or update data in storage",  // printHelp() message
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
    modifyToolOptions() override
    {
      opts.addRequiredOption("device-id", std::make_tuple(
            "device-id",
            po::value<std::string>()->required(),
            "Device for which to associate data.")
          );

      opts.addRequiredOption("data-path", std::make_tuple(
            "data-path",
            po::value<std::string>()->required(),
            "Data of file system to store; a path."
            " Either --data-path param or implicit last argument.")
          );

      opts.addOptionalOption("tool", std::make_tuple(
            "tool",
            po::value<std::string>(),
            "Tool to associate for import operations"
            " external to the data lake.")
          );
      opts.addOptionalOption("tool-args", std::make_tuple(
            "tool-args",
            po::value<std::string>(),
            "Tool arguments to use during an import operation.")
          );
      opts.addOptionalOption("rename", std::make_tuple(
            "rename",
            po::value<std::string>(),
            "Data file name to use instead when data path file is stored;"
            " not a path.")
          );
    }

  protected: // Methods part of subclass API
    // Inherited from AbstractTool at this scope
      // std::string const getDbName() const;
      // virtual void printHelp() const;
      // virtual void printVersion() const;
    int
    runTool() override
    {
      auto const& dataLake {getDataLakeHandler()};
      nmdlco::DataEntry de;

      const auto& deviceId {opts.getValue("device-id")};
      de.setDeviceId(deviceId);

      const auto& dataPath {opts.getValue("data-path")};
      de.setDataPath(dataPath);

      if (opts.exists("tool")) {
        const auto& importTool {opts.getValue("tool")};
        de.setImportTool(importTool);
      }

      if (opts.exists("tool-args")) {
        const auto& toolArgs {opts.getValue("tool-args")};
        de.setToolArgs(toolArgs);
      }

      if (opts.exists("rename")){
        const auto& newName {opts.getValue("rename")};
        de.setNewName(newName);
      }

      dataLake->commit(de);

      return nmcu::Exit::SUCCESS;
    }

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
