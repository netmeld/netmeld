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

#include <netmeld/core/tools/AbstractInsertTool.hpp>

namespace nmcu = netmeld::core::utils;

namespace netmeld::core::tools {

  // ===========================================================================
  // Constructors
  // ===========================================================================
  AbstractInsertTool::AbstractInsertTool()
  {}

  AbstractInsertTool::AbstractInsertTool(
      const char* _helpBlurb,
      const char* _programName,
      const char* _version) :
    AbstractTool(_helpBlurb, _programName, _version)
  {}


  // ===========================================================================
  // Tool Entry Points (execution order)
  // ===========================================================================
  int
  AbstractInsertTool::runTool()
  {
    setToolRunId();

    const auto& dbName  {opts.getValue("db-name")};
    const auto& dbArgs  {opts.getValue("db-args")};
    pqxx::connection db{std::string("dbname=") + dbName + " " + dbArgs};
    nmcu::dbPrepareCommon(db);
    pqxx::work t{db};

    generalInserts(t);
    specificInserts(t);

    t.commit();

    if (!opts.exists("tool-run-id")) {
      LOG_INFO << "tool-run-id: " << toolRunId << '\n';
    }

    return nmcu::Exit::SUCCESS;
  }

  void
  AbstractInsertTool::printHelp() const
  {
    LOG_NOTICE << "Insert a manually specified " << helpBlurb
               << "\nUsage: " << programName << " [options]"
               << "\nOptions:\n"
               << opts
               << this->bugTeam
               << '\n';
  }

  void
  AbstractInsertTool::setToolRunId()
  {
    toolRunId
      = opts.exists("tool-run-id")
      ? (nmco::Uuid(opts.getValue("tool-run-id")))
      : (nmco::Uuid());
  }

  void
  AbstractInsertTool::specificInserts(pqxx::transaction_base&)
  {}

  void
  AbstractInsertTool::addToolBaseOptions()
  {
    opts.addInsertOptions();
  }

  void
  AbstractInsertTool::modifyToolOptions()
  {}


  // ===========================================================================
  // General Functions (alphabetical)
  // ===========================================================================
  void
  AbstractInsertTool::generalInserts(pqxx::transaction_base& t)
  {
    t.exec_prepared("insert_tool_run",
        toolRunId,
        programName,
        opts.getCommandLine(),
        "human",
        nullptr,
        nullptr);

    if (opts.exists("device-id")) {
      devInfo.setDeviceId(opts.getValue("device-id"));
    }
    if (opts.exists("device-type")) {
      devInfo.setDeviceType(opts.getValue("device-type"));
    }
    if (opts.exists("device-color")) {
      devInfo.setDeviceColor(opts.getValue("device-color"));
    }

    devInfo.save(t, toolRunId);
  }

  const nmco::Uuid
  AbstractInsertTool::getToolRunId() const
  {
    return toolRunId;
  }

  const std::string
  AbstractInsertTool::getDeviceId() const
  {
    return devInfo.getDeviceId();
  }
}
