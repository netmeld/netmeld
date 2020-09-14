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

#include <netmeld/core/utils/FileManager.hpp>
#include <netmeld/datalake/handlers/Git.hpp>

#include <netmeld/datalake/tools/AbstractDatalakeTool.hpp>


namespace netmeld::datalake::tools {

  // ===========================================================================
  // Constructors
  // ===========================================================================
  AbstractDatalakeTool::AbstractDatalakeTool()
  {}

  AbstractDatalakeTool::AbstractDatalakeTool(
      const char* _helpBlurb,
      const char* _programName,
      const char* _version) :
    nmct::AbstractTool(_helpBlurb, _programName, _version)
  {}


  // ===========================================================================
  // Tool Entry Points (execution order)
  // ===========================================================================
  void
  AbstractDatalakeTool::addModuleOptions()
  {
    opts.addRequiredOption("lake-type", std::make_tuple(
          "lake-type",
          po::value<std::string>()->required()->default_value("git"),
          "Data lake type.")
        );

    const nmcu::FileManager& nmfm {nmcu::FileManager::getInstance()};
    dataLakePath = {nmfm.getSavePath()/"datalake"};
    opts.addRequiredOption("lake-path", std::make_tuple(
          "lake-path",
          po::value<std::string>()->required()->default_value(dataLakePath),
          "Data lake target path.")
        );
  }

  void
  AbstractDatalakeTool::printHelp() const
  {
    LOG_NOTICE << "Data lake tool to " << helpBlurb
               << "\nUsage: " << programName << " [options]"
               << "\nOptions:\n"
               << opts
               << this->bugTeam
               << '\n';
  }

  int
  AbstractDatalakeTool::runTool()
  {
    LOG_WARN << "Data lake tool did not implement execution logic\n";
    return nmcu::Exit::SUCCESS;
  }


  // ===========================================================================
  // General Functions (alphabetical)
  // ===========================================================================
  std::unique_ptr<nmdlh::AbstractHandler>
  AbstractDatalakeTool::getDatalakeHandler()
  {
    const auto& lakeType {opts.getValue("lake-type")};
    dataLakePath = {opts.getValue("lake-path")};


    if ("git" == lakeType) {
      return std::make_unique<nmdlh::Git>(dataLakePath);
    } else {
      LOG_ERROR << "Unsupported data lake type: " << lakeType << '\n';
      std::exit(nmcu::Exit::FAILURE);
    }
  }
}
