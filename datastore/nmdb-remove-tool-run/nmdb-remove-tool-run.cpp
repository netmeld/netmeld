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

#include <pqxx/pqxx>

#include <netmeld/core/tools/AbstractTool.hpp>

namespace nmct = netmeld::core::tools;
namespace nmcu = netmeld::core::utils;


class Tool : public nmct::AbstractTool
{
  public:
    Tool() : nmct::AbstractTool
      ("Remove tool run and all accosiated data", PROGRAM_NAME, PROGRAM_VERSION)
    {}

    void
    addToolBaseOptions() override
    {
      opts.addRequiredOption("tool-run-id", std::make_tuple(
          "tool-run-id",
          po::value<std::string>(),
          "Tool run UUID to remove from the database.")
        );
    }

    int
    runTool() override
    {
      const auto& toolRunId  {opts.getValue("tool-run-id")};
      const auto& dbName     {opts.getValue("db-name")};
      const auto& dbArgs     {opts.getValue("db-args")};

      pqxx::connection db{std::string("dbname=") + dbName + " " + dbArgs};
      pqxx::work t{db};

      db.prepare
      ("delete_tool_run",
       "DELETE FROM tool_runs"
       " WHERE (id = $1)");

      t.exec_prepared("delete_tool_run",
          toolRunId);

      t.commit();

      return nmcu::Exit::SUCCESS;
    }
};


int
main(int argc, char** argv)
{
  Tool tool;
  return tool.start(argc, argv);
}
