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

#include <netmeld/datalake/core/tools/AbstractDataLakeTool.hpp>


namespace nmcu = netmeld::core::utils;
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
       "general-tool",  // unused unless printHelp() is overridden
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
      //opts.addOptionalOption("by-device-id", std::make_tuple(
      //      "by-device-id",
      //      NULL_SEMANTIC,
      //      "Produce a list of data lake content"
      //      " ordered by device id.")
      //    );
      opts.addOptionalOption("by-tool", std::make_tuple(
            "by-tool",
            NULL_SEMANTIC,
            "Produce a list of data lake content"
            " ordered by import tool.")
          );
      opts.addOptionalOption("unbinned", std::make_tuple(
            "unbinned",
            NULL_SEMANTIC,
            "Produce a list of data lake content"
            " which has no defined import tool.")
          );
      opts.addOptionalOption("import-script", std::make_tuple(
            "import-script",
            NULL_SEMANTIC,
            "Produce an import script for data lake content.")
          );
      opts.addOptionalOption("from-dts", std::make_tuple(
            "from-dts",
            po::value<std::string>(),
            "Use data from this date prior.  Default of now.")
          );
    }

    void
    displayByDevice(std::vector<nmdlco::DataEntry> _dataEntries)
    {
      LOG_INFO << "Device Format Listing: 'device_id->data_name'\n";
      for (const auto& de : _dataEntries) {
        LOG_INFO << de.getDeviceId()
                 << "->" << de.getSaveName()
                 << '\n'
                 ;
      }
    }

    void
    displayByTool(std::vector<nmdlco::DataEntry> _dataEntries)
    {
      LOG_INFO << "Tool Format Listing: 'tool:device_id->data_name'\n";
      for (const auto& de : _dataEntries) {
        if (de.getImportTool().empty()) {
          continue;
        }
        LOG_INFO << de.getImportTool()
                 << ":" << de.getDeviceId()
                 << "->" << de.getSaveName()
                 << '\n'
                 ;
      }
    }

    void
    displayUnbinned(std::vector<nmdlco::DataEntry> _dataEntries)
    {
      LOG_INFO << "Unbinned Format Listing: 'device_id->data_name'\n";
      for (const auto& de : _dataEntries) {
        if (!de.getImportTool().empty()) {
          continue;
        }
        LOG_INFO << de.getDeviceId()
                 << "->" << de.getSaveName()
                 << '\n'
                 ;
      }
    }

    void
    displayImportScript(std::vector<nmdlco::DataEntry> _dataEntries)
    {
      LOG_INFO << "# Auto-generated by nmdl-list"
               << "\nDB_NAME=\"site\";"
               << "\nDB_ARGS=\"\";"
               << "\n"
               ;
      for (const auto& de : _dataEntries) {
        if (de.getImportTool().empty()) {
          continue;
        }
        // TODO split Netmeld vs other
        LOG_INFO << de.getImportCmd()
                 << ";\n"
                 ;
      }
    }

  protected: // Methods part of subclass API
    // Inherited from AbstractTool at this scope
      // std::string const getDbName() const;
      // virtual void printHelp() const;
      // virtual void printVersion() const;
    int
    runTool() override
    {
      const auto& dataLake {getDataLakeHandler()};

      // TODO handle DTS alignment in Handler

      const auto& dataEntries {dataLake->getDataEntries()};
      if (opts.exists("by-tool")) {
        displayByTool(dataEntries);
      } else if (opts.exists("unbinned")) {
        displayUnbinned(dataEntries);
      } else if (opts.exists("import-script")) {
        displayImportScript(dataEntries);
      } else {
        displayByDevice(dataEntries);
      }

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
