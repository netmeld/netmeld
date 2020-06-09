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

#include <netmeld/core/tools/AbstractTool.hpp>


namespace netmeld::core::tools {

  // ===========================================================================
  // Constructors
  // ===========================================================================
  AbstractTool::AbstractTool() :
    helpBlurb("Netmeld tool"),
    programName("TODO"),
    version("TODO")
  {}

  AbstractTool::AbstractTool(
      const char* _helpBlurb,
      const char* _programName,
      const char* _version) :
    helpBlurb(_helpBlurb),
    programName(_programName),
    version(_version)
  {}


  // ===========================================================================
  // Tool Entry Points (execution order)
  // ===========================================================================
  int
  AbstractTool::start(int argc, char** argv) noexcept
  {
    try {
      opts.addBaseOptions();
      addToolBaseOptions();
      modifyToolOptions();
      switch(opts.validateOptions(argc, argv)) {
        case nmcu::ProgramOptions::HELP :
          printHelp();
          std::exit(nmcu::Exit::SUCCESS);
          break;
        case nmcu::ProgramOptions::VERSION :
          printVersion();
          std::exit(nmcu::Exit::SUCCESS);
          break;
        default:
          break;
      }

      if (opts.exists("verbosity")) {
        nmcu::LoggerSingleton::getInstance().setLevel(
            opts.getValueAs<nmcu::Severity>("verbosity"));
      }

      return runTool();
    } catch (std::exception& e) {
      LOG_ERROR << e.what() << std::endl;
      std::exit(nmcu::Exit::FAILURE);
    }
  }

  void
  AbstractTool::addToolBaseOptions()
  {}

  void
  AbstractTool::modifyToolOptions()
  {}

  void
  AbstractTool::printHelp() const
  {
    LOG_NOTICE << helpBlurb << '\n'
               << "\nUsage: " << programName << " [options]"
               << "\nOptions:\n"
               << opts
               << bugTeam
               << '\n';
  }

  void
  AbstractTool::printVersion() const
  {
    LOG_NOTICE << programName << " " << version
               << "\n\n"
               << copyright
               << "\n\n"
               << author
               << '\n';
  }

  int
  AbstractTool::runTool()
  {
    return nmcu::Exit::SUCCESS;
  }


  // ===========================================================================
  // General Functions (alphabetical)
  // ===========================================================================
}
