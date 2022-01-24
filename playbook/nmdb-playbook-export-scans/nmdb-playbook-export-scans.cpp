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

#include <ostream>
#include <regex>

#include <netmeld/core/utils/StringUtilities.hpp>
#include <netmeld/datastore/tools/AbstractExportTool.hpp>

#include "Writer.hpp"
#include "WriterContext.hpp"
#include "WriterCsv.hpp"

#include "ExportScanIntraNetwork.hpp"
#include "ExportScanInterNetwork.hpp"
#include "ExportScanNessus.hpp"
#include "ExportScanSshAlgorithms.hpp"

namespace nmcu = netmeld::core::utils;
namespace nmdt = netmeld::datastore::tools;
namespace nmdu = netmeld::datastore::utils;


// =============================================================================
// Export tool definition
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
      // nmco::ProgramOptions   opts;
  public: // Variables should rarely appear at this scope


  // ===========================================================================
  // Constructors
  // ===========================================================================
  private: // Constructors should rarely appear at this scope
  protected: // Constructors intended for internal/subclass API
  public: // Constructors should generally be public
    Tool() : nmdt::AbstractExportTool
      ("predefined ConTeXt formatted scan results",
       PROGRAM_NAME,
       PROGRAM_VERSION)
    {}


  // ===========================================================================
  // Methods
  // ===========================================================================
  private: // Methods part of internal API
    // Overriden from AbstractExportTool
    void
    addToolOptions() override
    {
      opts.addRequiredOption("out-format", std::make_tuple(
            "out-format",
            po::value<std::string>()->default_value("context"),
            "Export data to the specified format (context|csv)")
          );
      opts.addOptionalOption("intra-network", std::make_tuple(
            "intra-network",
            NULL_SEMANTIC,
            "Export intra-network scan information")
          );
      opts.addOptionalOption("inter-network", std::make_tuple(
            "inter-network",
            NULL_SEMANTIC,
            "Export inter-network scan information")
          );
      opts.addOptionalOption("nessus", std::make_tuple(
            "nessus",
            NULL_SEMANTIC,
            "Export nessus scan information")
          );
      opts.addOptionalOption("ssh", std::make_tuple(
            "ssh",
            NULL_SEMANTIC,
            "Export SSH algorithm scan information")
          );
      opts.addOptionalOption("to-file", std::make_tuple(
            "to-file",
            NULL_SEMANTIC,
            "Output to file (predefined naming) instead of stdout")
          );
    }

    // Overriden from AbstractExportTool
    int
    runTool() override
    {
      const auto& dbName    {getDbName()};
      const auto& dbArgs    {opts.getValue("db-args")};
      const auto& dbConInfo {"dbname=" + dbName + " " + dbArgs};

      const auto& toFile    {opts.exists("to-file")};
      const auto& outFormat {nmcu::toLower(opts.getValue("out-format"))};

      std::unique_ptr<Writer> writer;
      if ("context" == outFormat) {
        writer = std::make_unique<WriterContext>(toFile);
      } else if ("csv" == outFormat) {
        writer = std::make_unique<WriterCsv>(toFile);
      } else {
        LOG_ERROR << "Invalid output format, quitting." << std::endl;
        return nmcu::Exit::FAILURE;
      }

      if (opts.exists("intra-network")) {
        ExportScanIntraNetwork exporter {dbConInfo};
        exporter.exportScan(writer);
      }
      if (opts.exists("inter-network")) {
        ExportScanInterNetwork exporter {dbConInfo};
        exporter.exportScan(writer);
      }
      if (opts.exists("nessus")) {
        ExportScanNessus exporter {dbConInfo};
        exporter.exportScan(writer);
      }
      if (opts.exists("ssh")) {
        ExportScanSshAlgorithms exporter {dbConInfo};
        exporter.exportScan(writer);
      }

      return nmcu::Exit::SUCCESS;
    }

  protected: // Methods part of subclass API
  public: // Methods part of public API
};


// =============================================================================
// Program entry point
// =============================================================================
int main(int argc, char** argv) {
  Tool tool;
  return tool.start(argc, argv);
}
