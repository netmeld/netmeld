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

#include <netmeld/datalake/tools/AbstractDatalakeTool.hpp>

namespace nmdlt = netmeld::datalake::tools;


class Tool : public nmdlt::AbstractDatalakeTool
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
    Tool() : nmdlt::AbstractDatalakeTool
      (
       "list data in storage",  // printHelp() message
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
    addToolOptions() override
    {
      opts.addOptionalOption("by-tool", std::make_tuple(
            "by-tool",
            NULL_SEMANTIC,
            "Produce a list of data lake content"
            " ordered by ingest tool.")
          );
      opts.addOptionalOption("unbinned", std::make_tuple(
            "unbinned",
            NULL_SEMANTIC,
            "Produce a list of data lake content"
            " which has no defined ingest tool.")
          );
      opts.addOptionalOption("ingest-script", std::make_tuple(
            "ingest-script",
            NULL_SEMANTIC,
            "Produce an ingest script for data lake content.")
          );
      opts.addOptionalOption("before", std::make_tuple(
            "before",
            po::value<nmco::Time>()->default_value(nmco::Time()),
            "Use data timestamped before this date.  Default of now.")
          );
    }

    void
    displayByDevice(const std::vector<nmdlo::DataEntry>& _dataEntries) const
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
    displayByTool(const std::vector<nmdlo::DataEntry>& _dataEntries) const
    {
      LOG_INFO << "Tool Format Listing: 'tool:device_id->data_name'\n";
      for (const auto& de : _dataEntries) {
        if (de.getIngestTool().empty()) {
          continue;
        }
        LOG_INFO << de.getIngestTool()
                 << ":" << de.getDeviceId()
                 << "->" << de.getSaveName()
                 << '\n'
                 ;
      }
    }

    void
    displayUnbinned(const std::vector<nmdlo::DataEntry>& _dataEntries) const
    {
      LOG_INFO << "Unbinned Format Listing: 'device_id->data_name'\n";
      for (const auto& de : _dataEntries) {
        if (!de.getIngestTool().empty()) {
          continue;
        }
        LOG_INFO << de.getDeviceId()
                 << "->" << de.getSaveName()
                 << '\n'
                 ;
      }
    }

    void
    displayIngestScript( const std::vector<nmdlo::DataEntry>& _dataEntries
                       , const nmco::Time& _dts
                       ) const
    {
      LOG_INFO << "# Auto-generated by nmdl-list"
               << "\n# Data lake on or before DTS: " << _dts
               << "\nDB_NAME=\"site\";"
               << "\nDB_ARGS=\"\";"
               << "\n"
               ;
      for (const auto& de : _dataEntries) {
        if (de.getIngestTool().empty()) {
          continue;
        }
        LOG_INFO << de.getIngestCmd()
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
      const auto& dataLake     {getDatalakeHandler()};
      const auto& time         {opts.getValueAs<nmco::Time>("before")};
      const auto& dataEntries  {dataLake->getDataEntries(time,
        opts.exists("ingest-script") || opts.exists("by-tool") || opts.exists("unbinned"))};

      if (opts.exists("by-tool")) {
        displayByTool(dataEntries);
      } else if (opts.exists("unbinned")) {
        displayUnbinned(dataEntries);
      } else if (opts.exists("ingest-script")) {
        displayIngestScript(dataEntries, time);
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
