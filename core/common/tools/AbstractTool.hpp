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

#ifndef ABSTRACT_TOOL_HPP
#define ABSTRACT_TOOL_HPP

#include <netmeld/core/utils/LoggerSingleton.hpp>
#include <netmeld/core/utils/ProgramOptions.hpp>
#include <netmeld/core/utils/StringUtilities.hpp>

namespace nmcu = netmeld::core::utils;


namespace netmeld::core::tools {

  class AbstractTool
  {
    // =========================================================================
    // Variables
    // =========================================================================
    protected:
      std::string            helpBlurb;
      std::string            programName;
      std::string            version;

      nmcu::ProgramOptions  opts;


      std::string bugTeam {
        "Report bugs to <Netmeld@sandia.gov>."
      };
      std::string author {
        "Written by the Netmeld team at Sandia National Laboratories.\n"
        "Netmeld (pre v1.0) originally written by Michael Berg (2013-2015)."
      };
      std::string copyright {
        "Copyright 2017 National Technology & Engineering Solutions of\n"
        "Sandia, LLC (NTESS). Under the terms of Contract DE-NA0003525\n"
        "with NTESS, the U.S. Government retains certain rights in this\n"
        "software."
        "\n\n"
        "Permission is hereby granted, free of charge, to any person\n"
        "obtaining a copy of this software and associated documentation\n"
        "files (the \"Software\"), to deal in the Software without\n"
        "restriction, including without limitation the rights to use,\n"
        "copy, modify, merge, publish, distribute, sublicense, and/or\n"
        "sell copies of the Software, and to permit persons to whom the\n"
        "Software is furnished to do so, subject to the following\n"
        "conditions:"
        "\n\n"
        "The above copyright notice and this permission notice shall be\n"
        "included in all copies or substantial portions of the Software."
        "\n\n"
        "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY\n"
        "KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE\n"
        "WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE\n"
        "AND NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR\n"
        "COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n"
        "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,\n"
        "ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE\n"
        "USE OR OTHER DEALINGS IN THE SOFTWARE."
      };

    // =========================================================================
    // Constructors
    // =========================================================================
    protected:
      // Default constructor, provided only for convienence
      AbstractTool();
      // Standard constructor, should be primary
      AbstractTool(const char*, const char*, const char*);


    // =========================================================================
    // Methods
    // =========================================================================
    protected:
      /* The following two functions work in tandem.  Both allow modification of
         a tool's program options.
         - addToolBaseOptions() is for a subclass of AbstractTool which is going
            to be subclassed itself (1st teir)
         - modifyToolOptions() is for a subclass of the subclass of AbstractTool
            (2nd teir)
         The addToolBaseOptions() is called before modifyToolOptions().  Scope
         appropriately to intended usage of tool as they are early entry points
         and allow potential modification of overall tool behaviour.
      */
      virtual void addToolBaseOptions();
      virtual void modifyToolOptions();

      virtual void printHelp() const;
      virtual void printVersion() const;

      // Tool specific behavior entry point
      virtual int  runTool();

    public:
      // Tool generic behavior entry point
      int start(int, char**) noexcept;
  };
}
#endif // ABSTRACT_TOOL_HPP
