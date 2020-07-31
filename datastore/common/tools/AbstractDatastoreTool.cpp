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

#include <netmeld/datastore/tools/AbstractDatastoreTool.hpp>


namespace netmeld::datastore::tools {

  // ===========================================================================
  // Constructors
  // ===========================================================================
  AbstractDatastoreTool::AbstractDatastoreTool() :
    nmct::AbstractTool()
  {}

  AbstractDatastoreTool::AbstractDatastoreTool(
      const char* _helpBlurb,
      const char* _programName,
      const char* _version) :
    nmct::AbstractTool(_helpBlurb, _programName, _version)
  {}


  // ===========================================================================
  // Tool Entry Points (execution order)
  // ===========================================================================
  void
  AbstractDatastoreTool::addModuleOptions()
  {
    opts.addRequiredOption("db-name", std::make_tuple(
          "db-name",
          po::value<std::string>()->required()->default_value(NETMELD_DB_NAME),
          "Database to connect to.")
        );
    opts.addAdvancedOption("db-args", std::make_tuple(
          "db-args",
          po::value<std::string>()->default_value(""),
          "Additional database connection args."
          " Space separated `key=value` libpqxx connection string parameters.")
        );
  }


  // ===========================================================================
  // General Functions (alphabetical)
  // ===========================================================================
  void
  AbstractDatastoreTool::addRequiredDeviceId()
  {
    opts.addRequiredOption("device-id", std::make_tuple(
          "device-id",
          po::value<std::string>()->required(),
          "Name of device.")
        );
    opts.addOptionalOption("device-type", std::make_tuple(
          "device-type",
          po::value<std::string>(),
          "Type of device, determines graph icon.")
        );
    opts.addOptionalOption("device-color", std::make_tuple(
          "device-color",
          po::value<std::string>(),
          "Graph color of device(s).")
        );
  }

  std::string const
  AbstractDatastoreTool::getDbName() const
  {
    return opts.getValue("db-name");
  }
}
