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

#include <netmeld/datastore/tools/AbstractImportTool.hpp>
#include <netmeld/core/utils/ForkExec.hpp>

typedef std::vector<std::string>  Results;

namespace nmdt = netmeld::datastore::tools;
namespace nmcu = netmeld::core::utils;


template<typename P, typename R>
class Tool : public nmdt::AbstractImportTool<P,R>
{
  private:
    std::string
    readCommandLine(sfs::path const& p) const
    {
      std::ifstream f{p.string()};
      std::string s;
      getline(f, s);
      f.close();

      return s;
    }

  public:
    Tool() : nmdt::AbstractImportTool<P,R>
      ("clw", PROGRAM_NAME, PROGRAM_VERSION)
    {}

    void
    addToolOptions() override
    {
      this->opts.removeRequiredOption("device-id");
      this->opts.addOptionalOption("device-id", std::make_tuple(
            "device-id",
            po::value<std::string>(),
            "(Not used) Name of device.")
          );

      this->opts.removeOptionalOption("device-type");
      this->opts.removeOptionalOption("device-color");
      this->opts.removeOptionalOption("pipe");

      this->opts.removeAdvancedOption("tool-run-id");
      this->opts.removeAdvancedOption("tool-run-metadata");
    }

    void
    setToolRunId() override
    {
      this->toolRunId.readUuid(this->getDataPath()/"tool_run_id.txt");
    }

    void
    parseData() override
    {
      this->executionStart.readTime(this->getDataPath()/"timestamp_start.txt");
      this->executionStop.readTime(this->getDataPath()/"timestamp_end.txt");

      this->helpBlurb =
        readCommandLine(this->getDataPath()/"command_line_modified.txt");

      this->programName =
        this->helpBlurb.substr(0, this->helpBlurb.find(' '));
    }

    void
    specificInserts(pqxx::transaction_base& t) override
    {
      t.commit(); // commit transaction so called tool processing works

      const auto& dbName    {this->getDbName()};
      const auto& severity  {this->opts.template
                               getValueAs<nmcu::Severity>("verbosity")};
      const auto& verbosity {nmcu::toString(severity)};
      const auto& toolRunId {this->getToolRunId()};
      const auto& toolName  {this->programName};
      const auto& results   {this->getDataPath()};

      // =======================================================================
      // Local device data collection processing
      // =======================================================================
      if (true) {
        std::vector<std::string> args = {
          "nmdb-import-ip-addr-show",
          "--db-name", dbName,
          "--verbosity", verbosity,
          "--tool-run-id", toolRunId.toString(),
          "--tool-run-metadata",
          "--device-id", "__tool-run-metadata__",
          (results/"ip_addr_show.txt").string()
        };

        nmcu::forkExecWait(args);
      }

      if (true) {
        std::vector<std::string> args = {
          "nmdb-import-ip-route-show",
          "--db-name", dbName,
          "--verbosity", verbosity,
          "--tool-run-id", toolRunId.toString(),
          "--tool-run-metadata",
          "--device-id", "__tool-run-metadata__"
        };

        // IPv4 routes
        args.push_back((results/"ip4_route_show.txt").string());
        nmcu::forkExecWait(args);
        args.pop_back();

        // IPv6 routes
        args.push_back((results/"ip6_route_show.txt").string());
        nmcu::forkExecWait(args);
        args.pop_back();
      }

      // =======================================================================
      // Remote device data collection processing
      // =======================================================================
      if (toolName == "nmap") {
        std::vector<std::string> args = {
          "nmdb-import-nmap",
          "--db-name", dbName,
          "--verbosity", verbosity,
          "--tool-run-id", toolRunId.toString(),
          (results/"results.xml").string()
        };

        nmcu::forkExecWait(args);
      }
      else if ((toolName == "ping") || (toolName == "ping6")) {
        std::vector<std::string> args = {
          "nmdb-import-ping",
          "--db-name", dbName,
          "--verbosity", verbosity,
          "--tool-run-id", toolRunId.toString(),
          (results/"stdout.txt").string()
        };

        nmcu::forkExecWait(args);
      }
    }
};

int
main(int argc, char** argv)
{
  Tool<std::nullptr_t, std::nullptr_t> tool;
  return tool.start(argc, argv);
}
