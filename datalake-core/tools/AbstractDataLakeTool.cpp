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

#include <netmeld/datalake/core/objects/HandlerGit.hpp>

#include "AbstractDataLakeTool.hpp"


namespace netmeld::datalake::core::tools {

  // ===========================================================================
  // Constructors
  // ===========================================================================
  AbstractDataLakeTool::AbstractDataLakeTool()
  {}

  AbstractDataLakeTool::AbstractDataLakeTool(
      const char* _helpBlurb,
      const char* _programName,
      const char* _version) :
    nmct::AbstractTool(_helpBlurb, _programName, _version)
  {}


  // ===========================================================================
  // Tool Entry Points (execution order)
  // ===========================================================================
  void
  AbstractDataLakeTool::addToolBaseOptions()
  {
    opts.addRequiredOption("lake-type", std::make_tuple(
          "lake-type",
          po::value<std::string>()->required(),
          "Data lake type.")
        );

    opts.removeRequiredOption("db-name");
    opts.removeAdvancedOption("db-args");
  }

  void
  AbstractDataLakeTool::modifyToolOptions()
  {}

  void
  AbstractDataLakeTool::printHelp() const
  {
    LOG_NOTICE << "Generate " << helpBlurb
               << "\nUsage: " << programName << " [options]"
               << "\nOptions:\n"
               << opts
               << this->bugTeam
               << '\n';
  }

  int
  AbstractDataLakeTool::runTool()
  {
    LOG_WARN << "Data lake tool did not implement execution logic\n";
    return nmcu::Exit::SUCCESS;
  }


  // ===========================================================================
  // General Functions (alphabetical)
  // ===========================================================================
  std::unique_ptr<nmdlco::DataLake>
  AbstractDataLakeTool::getDataLakeHandler()
  {
    const auto& lakeType {opts.getValue("lake-type")};

    std::unique_ptr<nmdlco::DataLake> dataLake = nullptr;
    if ("git" == lakeType) {
      dataLake = std::make_unique<nmdlco::HandlerGit>();
    }

    if (nullptr == dataLake) {
      LOG_ERROR << "Unsupported data lake type: " << lakeType << '\n';
      std::exit(nmcu::Exit::FAILURE);
    } else {
      return dataLake;
    }
  }
}
