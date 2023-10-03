// =============================================================================
// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
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

#include <netmeld/core/objects/Uuid.hpp>
#include <netmeld/datastore/tools/AbstractDatastoreTool.hpp>
#include <netmeld/datastore/utils/NetmeldPostgresConversions.hpp>


namespace nmco = netmeld::core::objects;
namespace nmcu = netmeld::core::utils;
namespace nmdt = netmeld::datastore::tools;


class Tool : public nmdt::AbstractDatastoreTool
{
  public:
    Tool() : nmdt::AbstractDatastoreTool
      ("Remove tool run and all associated data",
       PROGRAM_NAME, PROGRAM_VERSION)
    {}

    void
    addToolOptions() override
    {
      opts.addRequiredOption("tool-run-id", std::make_tuple(
          "tool-run-id",
          po::value<std::vector<std::string>>()->multitoken(),
          "Tool run UUID(s) to remove from the database."
          " Either --tool-run-id param or implicit last argument.")
        );

      opts.addPositionalOption("tool-run-id", -1);
    }

    int
    runTool() override
    {
      if (!opts.exists("tool-run-id")) {
        LOG_WARN << "UUID not given; not running"
                 << std::endl;
      } else {
        pqxx::connection db {getDbConnectString()};

        db.prepare("delete_tool_run", R"(
              DELETE FROM tool_runs
              WHERE (id = $1)
              )"
            );

        pqxx::work t {db};
        for (const auto& uuidStr : opts.getValues("tool-run-id")) {
          try {
          const nmco::Uuid toolRunId {uuidStr};
          const auto& results {t.exec_prepared("delete_tool_run", toolRunId)};

          LOG_INFO << "Removal count for " << uuidStr
                   << ": " << std::size(results) << '\n'
                   ;
          } catch (std::exception& e) {
            LOG_WARN << "Skipping " << uuidStr
                     << " as error encountered -- " << e.what()
                     << std::endl;
          }
        }

        t.commit();
      }

      return nmcu::Exit::SUCCESS;
    }
};


int
main(int argc, char** argv)
{
  Tool tool;
  return tool.start(argc, argv);
}
