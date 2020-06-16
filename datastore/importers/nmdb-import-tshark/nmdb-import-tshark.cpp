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

#include <csignal>
#include <future>

#include <netmeld/datastore/objects/DeviceInformation.hpp>
#include <netmeld/datastore/tools/AbstractImportTool.hpp>
#include <netmeld/datastore/parsers/ParserHelper.hpp>

#include "Parser.hpp"
#include "DataContainerSingleton.hpp"

namespace nmdo = netmeld::datastore::objects;
namespace nmdt = netmeld::datastore::tools;
namespace nmdp = netmeld::datastore::parsers;


// Signal handler, just restore default handler after called once
void sigIntHandler(int);
void sigIntHandler(int) { std::signal(SIGINT, SIG_DFL); }

// =============================================================================
// Import tool definition
// =============================================================================
template<typename P, typename R>
class Tool : public nmdt::AbstractImportTool<P,R>
{
  // ===========================================================================
  // Variables
  // ===========================================================================
  private: // Variables should generally be private
    std::future<void> parser;

  protected: // Variables intended for internal/subclass API
  public: // Variables should rarely appear at this scope


  // ===========================================================================
  // Constructors
  // ===========================================================================
  private: // Constructors should rarely appear at this scope
  protected: // Constructors intended for internal/subclass API
  public: // Constructors should generally be public
    Tool() : nmdt::AbstractImportTool<P,R>
      (
       "tshark -V -T json",  // command line tool imports data from
       PROGRAM_NAME,         // program name (set in CMakeLists.txt)
       PROGRAM_VERSION       // program version (set in CMakeLists.txt)
      )
    {}


  // ===========================================================================
  // Methods
  // ===========================================================================
  private: // Methods part of internal API
    // Overriden from AbstractImportTool
    void
    addToolOptions() override
    {
      this->opts.removeRequiredOption("device-id");
      this->opts.addOptionalOption("device-id", std::make_tuple(
            "device-id",
            po::value<std::string>(),
            "Name of device.")
          );

      // remove and add as optional so can take pipe (without save) or file
      this->opts.removeRequiredOption("data-path");
      this->opts.addOptionalOption("data-path", std::make_tuple(
            "data-path",
            po::value<std::string>(),
            "Data to parse. Either --data-path param"
            " or implicit last argument.")
          );

      this->opts.addOptionalOption("quiet", std::make_tuple(
            "quiet",
            NULL_SEMANTIC,
            "Suppress observational output."
            )
          );
    }

    // Overriden from AbstractImportTool
    void
    parseData() override
    {
      this->executionStart = nmco::Time();
      if (this->opts.exists("data-path")) { // file given, normal parse
        const auto dataPath {this->getDataPath()};
        parser = std::async(
            std::launch::async,
            [&dataPath]()
            {
              nmdp::fromFilePathMM<Parser<nmdp::ConstIter>,R>(dataPath);
            }
            );
        parser.get();
      } else { // no file, parse std::cin
        std::signal(SIGINT, sigIntHandler); // temp ignore sigint
        parser = std::async(
            std::launch::async,
            []()
            {
              try {
                nmdp::fromStdIn<Parser<nmdp::IstreamIter>,R>();
              } catch (...) {
                LOG_WARN << "Partial save, data stream ended abruptly\n";
              }
            }
            );
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      this->executionStop = nmco::Time();
    }

    // Overriden from AbstractImportTool
    void
    specificInserts(pqxx::transaction_base& t) override
    {
      const auto& toolRunId {this->getToolRunId()};
      const auto& deviceId  {this->getDeviceId()};

      auto& dcs {DataContainerSingleton::getInstance()};

      auto quiet {this->opts.exists("quiet")};

      // Commit transaction, use tool run entry per data set transaction
      t.commit();
      const auto& dbName  {this->opts.getValue("db-name")};
      const auto& dbArgs  {this->opts.getValue("db-args")};
      pqxx::connection db {std::string("dbname=") + dbName + " " + dbArgs };
      nmdu::dbPrepareCommon(db);

      LOG_DEBUG << "Iterating over results\n";
      size_t toolPacketCount {0};
      bool parserDone {false};
      while (!parserDone) {
        if (!dcs.hasData()) {
          std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        } else {
          auto resultsv {dcs.getData()};
          toolPacketCount += resultsv.size();
          for (auto& results : resultsv) {
            pqxx::work pt {db};
            LOG_DEBUG << "Iterating over VLANs\n";
            for (auto& [id, result] : results.vlans) {
              result.save(pt, toolRunId, deviceId);
              LOG_DEBUG << id << "--" << result.toDebugString() << "\n";
            }

            LOG_DEBUG << "Iterating over MACs\n";
            for (auto& [id, result] : results.macAddrs) {
              result.save(pt, toolRunId, deviceId);
              LOG_DEBUG << id << "--" << result.toDebugString() << "\n";
            }

            LOG_DEBUG << "Iterating over IPs\n";
            for (auto& [id, result] : results.ipAddrs) {
              result.save(pt, toolRunId, deviceId);
              LOG_DEBUG << id << "--" << result.toDebugString() << "\n";
            }

            LOG_DEBUG << "Iterating over Interfaces\n";
            for (auto& [id, result] : results.ifaces) {
              nmdo::DeviceInformation devInfo;
              devInfo.setDeviceId(id);
              devInfo.save(pt,toolRunId);
              const auto& deviceInfoId {devInfo.getDeviceId()};
              result.save(pt, toolRunId, deviceInfoId);
              LOG_DEBUG << id << "--" << result.toDebugString() << "\n";
            }

            LOG_DEBUG << "Iterating over Services\n";
            for (auto result : results.services) {
              result.save(pt, toolRunId, "");
              LOG_DEBUG << result.toDebugString() << "\n";
            }

            LOG_DEBUG << "Iterating over Observations\n";
            if (quiet) {
              results.observations.saveQuiet(pt, toolRunId, deviceId);
            } else {
              results.observations.save(pt, toolRunId, deviceId);
            }
            LOG_DEBUG << results.observations.toDebugString() << "\n";

            pt.commit();
          }
        }
        LOG_INFO << std::flush;

        if (parser.valid()) {
          auto status {parser.wait_for(std::chrono::milliseconds(100))};
          if (std::future_status::ready == status) {
            this->executionStop = nmco::Time();
            pqxx::work pt {db};
            pt.exec_prepared("update_tool_run",
                toolRunId,
                this->executionStart,
                this->executionStop);
            pt.commit();
            parser.get(); // invalidate future
          }
        }
        if (!parser.valid() && !dcs.hasData()) {
          parserDone = true;
        }
      }
      LOG_INFO << "Tool packets processed: " << toolPacketCount << "\n";
    }

  protected: // Methods part of subclass API
  public: // Methods part of public API
};


// =============================================================================
// Program entry point
// =============================================================================
int main(int argc, char** argv) {
  Tool<nmdp::DummyParser, Result> tool;  // We'll specify parsing directly
  return tool.start(argc, argv);
}
