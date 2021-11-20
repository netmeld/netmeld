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
#include <netmeld/datastore/objects/IpAddress.hpp>
#include <netmeld/core/objects/Uuid.hpp>

#include <netmeld/playbook/utils/QueriesPlaybook.hpp>

namespace nmdt = netmeld::datastore::tools;
namespace nmdo = netmeld::datastore::objects;
namespace nmcu = netmeld::core::utils;
namespace nmpbu = netmeld::playbook::utils;


class Tool : public nmdt::AbstractDatastoreTool
{
  // ===========================================================================
  // Variables
  // ===========================================================================
  private: // Variables should generally be private
    nmcu::FileManager& nmfm {nmcu::FileManager::getInstance()};

  protected: // Variables intended for internal/subclass API
  public: // Variables should rarely appear at this scope


  // ===========================================================================
  // Constructors
  // ===========================================================================
  private: // Constructors should rarely appear at this scope
  protected: // Constructors intended for internal/subclass API
  public: // Constructors should generally be public
    Tool() : nmdt::AbstractDatastoreTool
      (
       "Playbook tool",  // unused unless printHelp() is overridden
       PROGRAM_NAME,    // program name (set in CMakeLists.txt)
       PROGRAM_VERSION  // program version (set in CMakeLists.txt)
      )
    {}


  // ===========================================================================
  // Methods
  // ===========================================================================
  private: // Methods part of internal API
    void
    addToolOptions() override
    {
      opts.addRequiredOption("1", std::make_tuple(
            "[intra-network|inter-network]",
            NULL_SEMANTIC,
            "One required, but can provide both.  See Optional Options"
            " descriptions")
          );

      opts.addRequiredOption("stage", std::make_tuple(
            "stage",
            po::value<int>()->required(),
            "Numbered stage within the playbook")
          );
      opts.addRequiredOption("interface", std::make_tuple(
            "interface",
            po::value<std::string>()->required(),
            "Network interface to use")
          );
      opts.addRequiredOption("ip-addr", std::make_tuple(
            "ip-addr",
            po::value<std::string>()->required(),
            "IP address to assign to network interface")
          );

      opts.addOptionalOption("intra-network", std::make_tuple(
            "intra-network",
            NULL_SEMANTIC,
            "Add to LAN segment playbook")
          );
      opts.addOptionalOption("inter-network", std::make_tuple(
            "inter-network",
            NULL_SEMANTIC,
            "Add to across routers playbook")
          );

      opts.addOptionalOption("ptp-rtr-ip-addr", std::make_tuple(
            "ptp-rtr-ip-addr",
            po::value<std::string>()->required()->default_value(""),
            "Point-to-Point router IP address (Prefer using the"
            " nmdb-playbook-insert-router tool where possible)")
          );
      opts.addOptionalOption("description", std::make_tuple(
            "description",
            po::value<std::string>()->required()->default_value(""),
            "Description of stage")
          );
      opts.addOptionalOption("vlan", std::make_tuple(
            "vlan",
            po::value<uint16_t>()->required()->default_value(65535U),
            "802.1Q VLAN ID to use (if connected to a trunk port)")
          );
      opts.addOptionalOption("mac-addr", std::make_tuple(
            "mac-addr",
            po::value<std::string>()->required()->default_value(""),
            "MAC address to assign to network interface")
          );

      const auto& queryFileLoc {nmfm.getConfPath()/"playbook/queries.yaml"};
      opts.addAdvancedOption("query-file", std::make_tuple(
            "query-file",
            po::value<std::string>()->required()->default_value(queryFileLoc),
            "Location of queries file for playbook runs")
          );
    }

    int
    runTool() override
    {
      if (!opts.exists("intra-network") && !opts.exists("inter-network")) {
        LOG_ERROR << "Required option(s)"
                  << " --intra-network and/or --inter-network"
                  << " missing\n";
        std::exit(nmcu::Exit::FAILURE);
      }

      const auto& dbName  {getDbName()};
      const auto& dbArgs  {opts.getValue("db-args")};
      pqxx::connection db {"dbname=" + dbName + " " + dbArgs};
      nmpbu::PlaybookQueries pbq;
      pbq.init(opts.getValue("query-file"));
      pbq.dbPrepare(db);
      pqxx::work t{db};

      nmco::Uuid const pbSourceId;
      bool const isComplete = false;

      int const pbStage = opts.getValueAs<int>("stage");

      std::string const ifaceName = opts.getValue("interface");
      uint16_t const vlan = opts.getValueAs<uint16_t>("vlan");

      std::string const macAddr = opts.getValue("mac-addr");

      nmdo::IpAddress ipAddr   {opts.getValue("ip-addr")};
      if (!ipAddr.isValid()) {
        LOG_ERROR << "Aborting: Supplied IP address is invalid for usage"
              << std::endl;
        return nmcu::Exit::FAILURE;
      }

      std::string ptpRtrIpString = opts.getValue("ptp-rtr-ip-addr");
      if (!ptpRtrIpString.empty()) {
        nmdo::IpAddress ptpRtrIp {ptpRtrIpString};
        if (!ptpRtrIp.isValid()) {
          LOG_ERROR << "Aborting: Supplied point-to-point router IP"
                << " is invalid for usage"
                << std::endl;
          return nmcu::Exit::FAILURE;
        }
      }

      std::string const description = opts.getValue("description");

      if (opts.exists("intra-network")) {
        t.exec_prepared("insert_playbook_intra_network_source",
            pbSourceId,
            isComplete,
            pbStage,
            ifaceName,
            vlan,
            macAddr,
            ipAddr.toString(),
            description);
      }
      if (opts.exists("inter-network")) {
        t.exec_prepared("insert_playbook_inter_network_source",
            pbSourceId,
            isComplete,
            pbStage,
            ifaceName,
            vlan,
            macAddr,
            ipAddr.toString(),
            ptpRtrIpString,
            description);
      }

      t.commit();

      return nmcu::Exit::SUCCESS;
    }

  protected: // Methods part of subclass API
  public: // Methods part of public API
};


// =============================================================================
// Program entry point
// =============================================================================
int main(int argc, char** argv)
{
  Tool tool;
  return tool.start(argc, argv);
}
