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

#include <regex>
#include <yaml-cpp/yaml.h>

#include <netmeld/core/objects/Uuid.hpp>
#include <netmeld/datastore/tools/AbstractDatastoreTool.hpp>
#include <netmeld/core/utils/Exit.hpp>
#include <netmeld/datastore/utils/QueriesCommon.hpp>

#include <netmeld/playbook/utils/QueriesPlaybook.hpp>


#include "CommandRunnerSingleton.hpp"
#include "RaiiIpAddr.hpp"
#include "RaiiIpRoute.hpp"
#include "RaiiIpLink.hpp"
#include "RaiiMacAddr.hpp"
#include "RaiiVlan.hpp"

namespace nmco = netmeld::core::objects;
namespace nmcu = netmeld::core::utils;
namespace nmdt = netmeld::datastore::tools;
namespace nmdu = netmeld::datastore::utils;
namespace nmpb = netmeld::playbook;
namespace nmpbu = netmeld::playbook::utils;

// Start of OLD PLAYBOOK DATA

#include <map>
#include <mutex>
#include <thread>

#include <set>
#include <vector>

#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>

namespace netmeld::playbook {
  std::mutex coutMutex; // this is used in a lot of places
}

struct SourceConfig
{
  nmco::Uuid             playbookSourceId;
  std::string            description;
  std::set<std::string>  ipRouters;
  int                    addrFamily;
};

using IpConfig            = std::map<std::string, SourceConfig>;
using VlanConfig          = std::map<std::string, IpConfig>;
using InterfaceConfig     = std::map<uint16_t, VlanConfig>;
using PlaybookStageConfig = std::map<std::string, InterfaceConfig>;
using Playbook            = std::map<size_t, PlaybookStageConfig>;


enum class PlaybookScope { UNKNOWN, INTRA_NETWORK, INTER_NETWORK };


// End of OLD PLAYBOOK DATA




class Tool : public nmdt::AbstractDatastoreTool
{
  // ===========================================================================
  // Variables
  // ===========================================================================
  private: // Variables should generally be private
    static size_t const MIN_CAPTURE_DURATION {90};

    nmcu::FileManager& nmfm {nmcu::FileManager::getInstance()};
    std::string pbDir;

    nmpbu::QueriesPlaybook queriesPb;
    std::string dbConnectString;

    std::string playsFile;

    bool execute  {false};
    bool headless {false};

    int family {4};
    std::string familyStr;
    std::set<size_t> enabledStages;
    std::set<size_t> enabledPhases;
    size_t maxPhases {3};

    sfs::path scriptPath;

    std::string roeExcludedPath;
    std::string roeNetworksPath;
    std::string respondingHostsPath;

    std::string commandTitlePrefix;
    std::string nmapPrefix;

    nmpb::CommandRunnerSingleton& cmdRunner =
      nmpb::CommandRunnerSingleton::getInstance();

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
      opts.removeOptionalOption("pipe");
      opts.removeAdvancedOption("tool-run-metadata");

      opts.addRequiredOption("1", std::make_tuple(
            "[intra-network|inter-network]",
            NULL_SEMANTIC,
            "One required for playbook generation.  See Optional Options"
            " descriptions")
          );
      opts.addRequiredOption("capture-duration", std::make_tuple(
            "capture-duration",
            po::value<size_t>()->required()->default_value(300),
            "Capture traffic for specified number of seconds when the physical"
            " interface is brought up")
          );
      const auto& savePathLoc {nmfm.getSavePath()/"playbook"};
      opts.addRequiredOption("save-path", std::make_tuple(
            "save-path",
            po::value<std::string>()->required()->default_value(savePathLoc),
            "Location to save output from playbook run")
          );

      opts.addOptionalOption("intra-network", std::make_tuple(
            "intra-network",
            NULL_SEMANTIC,
            "Generate intra-network (local-area network) playbook")
          );
      opts.addOptionalOption("inter-network", std::make_tuple(
            "inter-network",
            NULL_SEMANTIC,
            "Generate inter-network (across routers) playbook")
          );
      opts.addOptionalOption("stage", std::make_tuple(
            "stage",
            po::value<std::vector<size_t>>()->multitoken()->composing()->
              default_value(std::vector<size_t>{},"all"),
            "Only run the specified stages"
            "; Space separated list")
          );
      opts.addOptionalOption("phase", std::make_tuple(
            "phase",
            po::value<std::vector<size_t>>()->multitoken()->composing()->
              default_value(std::vector<size_t>{},"all"),
            "For all stages, only run the specified phases"
            "; Space separated list")
          );
      opts.addOptionalOption("execute", std::make_tuple(
            "execute",
            NULL_SEMANTIC,
            "Execute playbook (default: only display playbook)")
          );
      opts.addOptionalOption("headless", std::make_tuple(
            "headless",
            NULL_SEMANTIC,
            "Execute playbook in headless mode (default: run in GUI xterm)")
          );
      opts.addOptionalOption("no-prompt", std::make_tuple(
            "no-prompt",
            NULL_SEMANTIC,
            "Cycle stages without prompting user")
          );

      opts.addAdvancedOption("exclude-command", std::make_tuple(
            "exclude-command",
            po::value<std::vector<size_t>>()->multitoken()->composing()->
              default_value(std::vector<size_t>{},"none"),
            "Exclude specified command IDs"
            "; Space separated list"
            "; This can break expected logic in some cases")
          );
      opts.addAdvancedOption("queries-file", std::make_tuple(
            "queries-file",
            po::value<std::string>()->required()
              ->default_value(queriesPb.getDefaultQueryFilePath()),
            "Location of queries file for playbook runs")
          );
      const auto& playsFileLoc {nmfm.getConfPath()/"playbook/plays.yaml"};
      opts.addAdvancedOption("plays-file", std::make_tuple(
            "plays-file",
            po::value<std::string>()->required()->default_value(playsFileLoc),
            "Location of plays file for stages, phases, and commands")
          );
      }

    int
    runTool() override
    {
      const auto& playbookScope {validatePlaybookScope()};

      validateEnabledTests();

      // Execute, or not
      execute = opts.exists("execute");
      cmdRunner.setExecute(execute);

      // Headless, or not
      headless = opts.exists("headless");
      cmdRunner.setHeadless(headless);

      // Create directory for meta-data about and results from this tool run
      sfs::path const saveDir {opts.getValue("save-path")};
      sfs::create_directories(saveDir);
      pbDir = sfs::canonical(saveDir).string();

      sfs::path const playsPath = opts.getValue("plays-file");
      playsFile = sfs::canonical(playsPath).string();

      Playbook playbook;

      const auto& dbName  {getDbName()};
      const auto& dbArgs  {opts.getValue("db-args")};

      dbConnectString = {std::string("dbname=") + dbName + " " + dbArgs};
      pqxx::connection db {dbConnectString};
      queriesPb.init(opts.getValue("queries-file"));
      queriesPb.dbPrepare(db);

      if (true) {
        std::string query;
        switch (playbookScope) {
          case PlaybookScope::INTRA_NETWORK:
          {
            query = "select_playbook_intra_network";
            break;
          }
          case PlaybookScope::INTER_NETWORK:
          {
            query = "select_playbook_inter_network";
            break;
          }
          case PlaybookScope::UNKNOWN:  // intentional fallthrough
          default:
          {
            break;
          }
        }

        pqxx::read_transaction t {db};
        pqxx::result playbookIpSourceRows {t.exec_prepared(query)};
        for (const auto& playbookIpSourceRow : playbookIpSourceRows) {
          size_t playbookStage;
          playbookIpSourceRow.at("playbook_stage").to(playbookStage);
          std::string interfaceName;
          playbookIpSourceRow.at("interface_name").to(interfaceName);
          uint16_t vlan;
          playbookIpSourceRow.at("vlan").to(vlan, static_cast<uint16_t>(0));
          std::string macAddr;
          playbookIpSourceRow.at("mac_addr").to(macAddr);
          std::string ipAddr;
          playbookIpSourceRow.at("ip_addr").to(ipAddr);
          std::string rtrIpAddr;
          playbookIpSourceRow.at("next_hop_ip_addr").to(rtrIpAddr);

          nmco::Uuid playbookSourceId;
          playbookIpSourceRow.at("playbook_source_id").to(playbookSourceId);
          std::string description;
          playbookIpSourceRow.at("description").to(description);
          int addrFamily;
          playbookIpSourceRow.at("addr_family").to(addrFamily);

          // Create the source location for testing.
          playbook[playbookStage][interfaceName][vlan][macAddr][ipAddr]
            .playbookSourceId = playbookSourceId;
          playbook[playbookStage][interfaceName][vlan][macAddr][ipAddr]
            .description = description;
          playbook[playbookStage][interfaceName][vlan][macAddr][ipAddr]
            .addrFamily = addrFamily;

          if (opts.exists("inter-network") && !rtrIpAddr.empty()) {
            // Add any routers for inter-network tests from this location.
            playbook[playbookStage][interfaceName][vlan][macAddr][ipAddr]
              .ipRouters.insert(rtrIpAddr);
          }
        }
      }


      for (const auto& [playbookStage, stageConfigs] : playbook) {
        if (!enabledStages.empty() && !enabledStages.count(playbookStage)) {
          continue;
        }

        if (true) {
          std::lock_guard<std::mutex> coutLock {nmpb::coutMutex};
          LOG_INFO << "#### Playbook: Stage " << playbookStage << " ####"
                   << std::endl;
        }

        for (const auto& [ifaceName, ifaceConfigs] : stageConfigs) {
          LOG_INFO << "# Disconnect target(s), if necessary, and connect "
                   << ifaceName << "." << std::endl
                   << "# This interface will cycle through settings:"
                   << std::endl;

          for (const auto& [vlan, vlanConfigs] : ifaceConfigs) {
            for (const auto& [macAddr, ipConfigs] : vlanConfigs) {
              for (const auto& [ipAddr, srcConfig] : ipConfigs) {
                const auto& description {srcConfig.description};
                LOG_INFO << "#\tVLAN: " << vlan
                         << ", MAC: " << macAddr
                         << ", IP: " << ipAddr
                         << ", Desc: " << description
                         << std::endl;
                if (srcConfig.ipRouters.size()) {
                  LOG_INFO << "#\t\tRouters:";
                  for (const auto& rtrIpAddr : srcConfig.ipRouters) {
                    LOG_INFO << " " << rtrIpAddr;
                  }
                  LOG_INFO << std::endl;
                }
              }
            }
          }
          LOG_INFO << std::endl;
        }

        if (execute) {
          std::string userInput;

          // Sanity check, cycle all interfaces before aborting
          for (const auto& [ifaceName, ifaceConfigs] : stageConfigs) {

            // Ensure interface is down
            if (!ifaceIsDown(ifaceName)) {
              LOG_WARN << ifaceName << " link not down" << std::endl;
            }

            // Ensure interface has no ipv4/6 address assigned
            if (ifaceHasAddress(ifaceName)) {
              LOG_WARN << ifaceName << " has an address" << std::endl;
            }
          }

          while (!opts.exists("no-prompt")) {
            LOG_INFO << "Enter 'Y' to continue or 'Q' to quit: ";
            std::cin >> userInput;

            if (("Y" == userInput) || ("y" == userInput) ||
                ("Q" == userInput) || ("q" == userInput)) {
              break;
            }
          }

          if (("Q" == userInput) || ("q" == userInput)) {
            break;
          }
        }

        std::map<std::string, std::thread> physIfaceThreads;

        for (const auto& [ifaceName, ifaceConfigs] : stageConfigs) {

          // Start a thread to manage the configuration of this interface
          physIfaceThreads[ifaceName] =
            std::thread(&Tool::physIfaceThreadActions, this,
                   playbookScope, playbookStage,
                   ifaceName, ifaceConfigs);

          switch (playbookScope) {
            case PlaybookScope::INTRA_NETWORK:
              {
                // Run interface threads in parallel
                break;
              }
            case PlaybookScope::INTER_NETWORK:  // intentional fallthrough
            case PlaybookScope::UNKNOWN:  // intentional fallthrough
            default:
              {
                // Run interface threads serially (only one interface at a time)

                // Wait for thread to complete before starting next thread.
                physIfaceThreads[ifaceName].join();
                physIfaceThreads.erase(ifaceName);

                break;
              }
          }
        }

        // Wait for interface threads (if any) to finish
        for (auto& [_, ifaceThread] : physIfaceThreads) {
          ifaceThread.join();
        }
        physIfaceThreads.clear();

        if (true) {
          std::lock_guard<std::mutex> coutLock {nmpb::coutMutex};
          LOG_INFO << std::endl;
        }
      }

      return nmcu::Exit::SUCCESS;
    }

    PlaybookScope
    validatePlaybookScope()
    {
      if (    (opts.exists("intra-network") && opts.exists("inter-network"))
          || !(opts.exists("intra-network") || opts.exists("inter-network"))
         )
      {
        LOG_ERROR << "Required option --intra-network or --inter-network"
                  << std::endl;
        std::exit(nmcu::Exit::FAILURE);
      }

      // Ensure some test type specified
      PlaybookScope playbookScope {PlaybookScope::UNKNOWN};
      if (opts.exists("inter-network")) {
        playbookScope = PlaybookScope::INTER_NETWORK;
      }
      if (opts.exists("intra-network")) {
        playbookScope = PlaybookScope::INTRA_NETWORK;
      }
      assert(playbookScope != PlaybookScope::UNKNOWN);

      return playbookScope;
    }

    void
    validateEnabledTests()
    {
      // Handle inclusion/exclusion of tests
      auto stages {opts.getValueAs<std::vector<size_t>>("stage")};
      if (!stages.empty()) {
        enabledStages = std::set(stages.begin(), stages.end());
      }
      auto phases {opts.getValueAs<std::vector<size_t>>("phase")};
      if (!phases.empty()) {
        enabledPhases = std::set(phases.begin(), phases.end());
      }
      auto tests {opts.getValueAs<std::vector<size_t>>("exclude-command")};
      if (!tests.empty()) {
        cmdRunner.disableCommands(std::set(tests.begin(), tests.end()));
      }
    }

    // HELPERS
    bool
    ifaceIsDown(std::string const& ifaceName)
    {
      int socketId {socket(AF_INET, SOCK_DGRAM, 0)};
      if (socketId < 0) {
        LOG_WARN << "Socket creation for testing link status failed: "
                 << errno << std::endl;
        return false;
      }

      // Populate interface information for evaluation
      struct ifreq ifReq;
      strncpy(ifReq.ifr_name, ifaceName.c_str(), sizeof(ifReq.ifr_name));
      int rv {ioctl(socketId, SIOCGIFFLAGS, &ifReq)};
      close(socketId);

      if (rv == -1) {
        LOG_WARN << "ioctl query for testing link status failed: "
                 << errno << std::endl;
        return false;
      }

      return !((ifReq.ifr_flags & IFF_UP) && (ifReq.ifr_flags & IFF_RUNNING));
    }

    bool
    ifaceHasAddress(std::string const& ifaceName)
    {
      bool hasAddr {false};
      struct ifaddrs* ifAddrStruct {NULL};

      getifaddrs(&ifAddrStruct);

      for (auto ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        // Skip invalid interfaces
        if (ifaceName.compare(ifa->ifa_name) != 0 || !ifa->ifa_addr) {
          continue;
        }

        // Don't need to know the address, just that it's set
        auto addrFamily = ifa->ifa_addr->sa_family;
        if (addrFamily == AF_INET || addrFamily == AF_INET6) {
          hasAddr = true;
        }
      }

      if (ifAddrStruct != NULL) {
        freeifaddrs(ifAddrStruct);
      }

      return hasAddr;
    }

    bool
    ifaceHasIp(const std::string& ifaceName, const std::string& ipAddr)
    {
      bool hasIp {false};
      struct ifaddrs* ifAddrStruct {NULL};

      getifaddrs(&ifAddrStruct);

      auto ip  {ipAddr};
      auto pos {ip.find_first_of('/')};
      if (pos != std::string::npos) {
        ip = ip.substr(0, pos);
      }

      for (auto ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        // Skip invalid interfaces
        if (ifaceName.compare(ifa->ifa_name) != 0 || !ifa->ifa_addr) {
          continue;
        }

        auto addrFamily = ifa->ifa_addr->sa_family;
        char address[INET6_ADDRSTRLEN];
        if (addrFamily == AF_INET) {
          void* addrPtr {
            &reinterpret_cast<struct sockaddr_in *>(ifa->ifa_addr)->sin_addr};
          inet_ntop(addrFamily, addrPtr, address, INET_ADDRSTRLEN);
        }
        if (addrFamily == AF_INET6) {
          void* addrPtr {
            &reinterpret_cast<struct sockaddr_in6 *>(ifa->ifa_addr)->sin6_addr};
          inet_ntop(addrFamily, addrPtr, address, INET6_ADDRSTRLEN);
        }
        if (ip.compare(std::string(address)) == 0) {
          hasIp = true;
        }
      }

      if (ifAddrStruct != NULL) {
        freeifaddrs(ifAddrStruct);
      }

      return hasIp;
    }


    // COMMON
    void
    captureTraffic(std::string const& linkName, size_t const _duration)
    {
      // After bringing up the physical (non-VLAN) interface,
      // this software must wait at least 45 seconds for the
      // connected switch port to enter the forwarding state
      // (in case the connected switch port is not "portfast").
      //
      // We also want to capture any broadcast (or flooded unicast)
      // traffic on the switch port before we source any traffic.
      //
      // Sniffing traffic for at least 90 seconds (preferably longer)
      // accomplishes both of these goals at the same time.

      size_t duration {std::max(MIN_CAPTURE_DURATION, _duration)};
      std::ostringstream oss;
      oss << "clw dumpcap -i " << linkName << " -a duration:" << duration;

      {
        std::lock_guard<std::mutex> coutLock {nmpb::coutMutex};
        cmdRunner.systemExec(oss.str());
      }
    }

    void
    setCommandTitlePrefix(std::string const& type, size_t const playbookStage,
        std::string const& linkName, std::string const& srcIpAddr)
    {
      std::ostringstream oss;
      oss << type << ": S" << playbookStage
          << ", " << linkName
          << ", " << srcIpAddr
          << ","
          ;

      commandTitlePrefix = oss.str();
    }

    void
    updateNmapPrefix()
    {
      if (4 == family) {
        nmapPrefix = "clw nmap   ";
      } else {
        nmapPrefix = "clw nmap -6";
      }
    }

    void
    physIfaceThreadActions(PlaybookScope const playbookScope,
        size_t const playbookStage, std::string const& physIfaceName,
        InterfaceConfig const& ifaceConfigs)
    {
      // Bring up the physical interface only for:
      // - Passive network sniffing before sourcing any frames.
      // - Base interface for VLANs.
      // (Any direct testing with the physical interface is done later)

      { // Block scope needed for proper RAII
        // Change the MAC address if necessary.
        std::string const& srcMacAddr
          {(ifaceConfigs.begin()->second).begin()->first};
        nmpb::RaiiMacAddr raiiMacAddr {physIfaceName, srcMacAddr};

        // Bring up the physical interface.
        nmpb::RaiiIpLink raiiLink {physIfaceName};

        captureTraffic(physIfaceName,
            opts.getValueAs<size_t>("capture-duration"));

        // Proceed with testing on VLANs.
        std::map<uint16_t, std::thread> vlanThreads;

        for (const auto& [vlanId, vlanConfigs] : ifaceConfigs) {
          if (nmpb::VlanId::NONE == vlanId) {
            continue;
          }

          // Start a thread to manage the configuration of this VLAN.
          vlanThreads[vlanId] =
            std::thread(&Tool::vlanIfaceThreadActions, this,
                   playbookScope, playbookStage,
                   physIfaceName, vlanId, vlanConfigs);

          switch (playbookScope) {
            case PlaybookScope::INTRA_NETWORK:
              {
                // Run VLAN threads in parallel.
                break;
              }
            case PlaybookScope::INTER_NETWORK:  // intentional fallthrough
            case PlaybookScope::UNKNOWN:  // intentional fallthrough
            default:
              {
                // Run VLAN threads serially (only one VLAN active at a time).

                // Wait for thread to complete before starting next thread.
                vlanThreads[vlanId].join();
                vlanThreads.erase(vlanId);

                break;
              }
          }
        }

        // Wait for VLAN threads (if any) to finish.
        for (auto& [_, vlanThread] : vlanThreads) {
          vlanThread.join();
        }
        vlanThreads.clear();

        // Closing this block scope brings down the physical interface.
      }

      // Perform any direct testing with the physical interface (non-VLAN).

      for (const auto& [vlanId, vlanConfigs] : ifaceConfigs) {
        if (nmpb::VlanId::NONE != vlanId) {
          continue;
        }

        // Start a thread to manage the configuration of this VLAN.
        std::thread vlanThread =
          std::thread(&Tool::vlanIfaceThreadActions, this,
                 playbookScope, playbookStage,
                 physIfaceName, vlanId, vlanConfigs);

        vlanThread.join();
      }
    }


    void
    vlanIfaceThreadActions(PlaybookScope const playbookScope,
        size_t const playbookStage, std::string const& physIfaceName,
        uint16_t const vlan, VlanConfig const& vlanConfigs)
    {
      nmpb::RaiiVlan raiiVlan {physIfaceName, vlan};
      if (nmpb::VlanId::RESERVED <= vlan && nmpb::VlanId::NONE != vlan) {
        return; // vlan id is invalid, skip processing
      }
      std::string const linkName {raiiVlan.getLinkName()};

      // Each named Linux interface can only have one MAC address,
      // so MAC addresses can't be parallelized and must be assigned serially.

      for (const auto& [srcMacAddr, ipConfigs] : vlanConfigs) {
        // Change the VLAN MAC address if necessary.
        nmpb::RaiiMacAddr raiiMacAddr {linkName, srcMacAddr};

        // Bring up the VLAN link.
        nmpb::RaiiIpLink raiiLink {linkName};

        if (nmpb::VlanId::NONE == vlan) {
          // Ensure forwarding state (may not be "portfast") and snapshot
          captureTraffic(linkName, MIN_CAPTURE_DURATION);
        }

        for (const auto& [srcIpAddr, srcConf] : ipConfigs) {
          // do IP addr specific stuff
          family = srcConf.addrFamily;
          familyStr = std::to_string(family);
          updateNmapPrefix();

          nmpb::RaiiIpAddr raiiIpAddr {linkName, srcIpAddr};

          pqxx::connection db {dbConnectString};
          queriesPb.dbPrepare(db);

          switch (playbookScope) {
            case PlaybookScope::INTRA_NETWORK:
              {
                setCommandTitlePrefix("Intra",
                    playbookStage, linkName, srcIpAddr);
                intraNetworkActions(db, srcConf.playbookSourceId,
                    linkName, srcIpAddr);

                if (execute) {
                  pqxx::work t {db};
                  t.exec_prepared("playbook_intra_network_set_completed",
                      srcConf.playbookSourceId);
                  t.commit();
                }

                break;
              }
            case PlaybookScope::INTER_NETWORK:
              {
                for (const auto& rtrIpAddr : srcConf.ipRouters) {
                  setCommandTitlePrefix("Inter",
                      playbookStage, linkName, srcIpAddr);
                  interNetworkActions(db, srcConf.playbookSourceId,
                      linkName, rtrIpAddr, srcIpAddr);
                }

                if (execute) {
                  pqxx::work t {db};
                  t.exec_prepared("playbook_inter_network_set_completed",
                      srcConf.playbookSourceId);
                  t.commit();
                }

                break;
              }
            case PlaybookScope::UNKNOWN:  // intentional fallthrough
            default:
              {
                break;
              }
          }
        }
      }
    }

    void
    intraNetworkActions(pqxx::connection& db,
        const nmco::Uuid& playbookSourceId, std::string const& linkName,
        std::string const& srcIpAddr)
    {
      nmco::Uuid const sessionId;

      std::string ipNet;
      std::string ipNetBcast;

      if (true) {
        pqxx::read_transaction t {db};
        pqxx::result rows =
               t.exec_prepared("select_network_and_broadcast",
                   srcIpAddr);

        for (const auto& row : rows) {
          row.at("ip_net").to(ipNet);
          row.at("ip_net_bcast").to(ipNetBcast);
        }
      }

      //generateTargetFiles(sessionId.toString(), false);
      primeSessionData(sessionId.toString());
      networkPhases(db, sessionId, playbookSourceId, srcIpAddr,
                    linkName, ipNet, ipNetBcast, "intra-network",
                    "");

      cmdRunner.setExecute(execute); // update in case of alternate logic

      {
        std::lock_guard<std::mutex> coutLock {nmpb::coutMutex};
        LOG_INFO << std::endl;
      }
    }

    void
    interNetworkActions(pqxx::connection& db,
        const nmco::Uuid& playbookSourceId, std::string const& linkName,
        std::string const& rtrIpAddr, std::string const& srcIpAddr)
    {
      nmco::Uuid const sessionId;

      nmpb::RaiiIpRoute raiiIpRoute
      {linkName, rtrIpAddr};

      //// ARP/NDP the router: skip testing if unreachable
      //std::ostringstream oss;
      //oss << nmapPrefix << " -n -sn -e " << linkName
      //    << " " << rtrIpAddr << " 2>&1 | grep 'Host is up'"
      //    ;

      //{
      //  std::lock_guard<std::mutex> coutLock {nmpb::coutMutex};
      //  bool isRouterReachable = cmdRunner.systemExec(oss.str());
      //  if (!isRouterReachable) {
      //    LOG_INFO << std::endl;
      //    LOG_WARN << "Could not ARP/NDP router " << rtrIpAddr << std::endl
      //             << "Skipping tests through router " << rtrIpAddr << std::endl
      //             << std::endl;
      //    cmdRunner.setExecute(false); // disable, but maintain count
      //  }
      //}

      //generateTargetFiles(sessionId.toString(), true);
      primeSessionData(sessionId.toString());
      networkPhases(db, sessionId, playbookSourceId, srcIpAddr,
                    linkName, "", "", "inter-network",
                    rtrIpAddr);

      cmdRunner.setExecute(execute); // update in case of alternate logic

      {
        std::lock_guard<std::mutex> coutLock {nmpb::coutMutex};
        LOG_INFO << std::endl;
      }
    }

    void
    primeSessionData(std::string const& sessionId)
    {
      std::ostringstream oss;

      oss.str(std::string());
      oss << pbDir << "/ROE-Excluded-IPv" << family << "-" << sessionId;
      roeExcludedPath = oss.str();
      // TODO
      //nmfm.removeWrite(roeExcludedPath);

      oss.str(std::string());
      oss << pbDir << "/ROE-Networks-IPv" << family << "-" << sessionId;
      roeNetworksPath = oss.str();
      // TODO
      //nmfm.removeWrite(roeNetworksPath);

      oss.str(std::string());
      oss << pbDir << "/Responding-Hosts-IPv" << family << "-" << sessionId;
      respondingHostsPath = oss.str();
      // TODO
      //nmfm.removeWrite(respondingHostsPath);
    }

    template<typename T>
    bool
    yIs(const YAML::Node& _node, const std::string& _key, T _value)
    {
      return (_node[_key].IsDefined())
          && (_node[_key].as<T>() == _value)
          ;
    }

    void
    networkPhases(
      pqxx::connection& db,
      const nmco::Uuid& playbookSourceId,
      const nmco::Uuid& sessionId,
      const std::string& srcIpAddr,
      const std::string& linkName,
      const std::string& ipNet,
      const std::string& ipNetBcast,
      const std::string& target,
      const std::string& rtrIpAddr
    )
    {
      YAML::Node yConfig {YAML::LoadFile(playsFile)};

      const auto& yPhasesArray    {yConfig[target]["stage"]};
      const auto& yRunOptionsMap  {yConfig["runtime-options"]};

      size_t phaseId {1};

      // In stage; Per phase configuration
      for (const auto& yPhaseMap : yPhasesArray) {
        LOG_INFO << std::endl << "### Phase " << phaseId << std::endl;
        if (   yIs<bool>(yRunOptionsMap, "ignore-iface-state-change", true)
            && isPhaseRuntimeError(db, playbookSourceId, linkName, srcIpAddr))
        {
          LOG_WARN << "Disabling this phase execution" << std::endl;
          cmdRunner.setExecute(false);
        }

        for (const auto& yCmdSetMap : yPhaseMap["pre"]) {
          std::vector<std::tuple<std::string, std::string>> commands;

          if (!enabledPhases.empty() && !enabledPhases.count(phaseId)) {
            continue;
          }

          const auto& cmdSetName {yCmdSetMap["name"].as<std::string>()}; 
          std::string ipTarget {4 == family ? "ipv4" : "ipv6"};

          // In command set; Per command configuration
          for (const auto& yCmdMap : yCmdSetMap[ipTarget]) {
            // In command; Per command option configuration
            std::ostringstream ossOpts;
            for (const auto& yOpt : yCmdMap["opts"]) {
              std::vector<std::tuple<std::regex, std::string>> replacementMap {
                {std::regex(R"(\{\{srcIpAddr\}\})"), srcIpAddr},
                {std::regex(R"(\{\{linkName\}\})"), linkName},
                {std::regex(R"(\{\{ipNet\}\})"), ipNet},
                {std::regex(R"(\{\{ipNetBcast\}\})"), ipNetBcast},
                {std::regex(R"(\{\{roeExcludedPath\}\})"), roeExcludedPath},
                {std::regex(R"(\{\{roeNetworksPath\}\})"), roeNetworksPath},
                {std::regex(R"(\{\{respondingHostsPath\}\})"), respondingHostsPath},
                {std::regex(R"(\{\{rtrIpAddr\}\})"), rtrIpAddr},
                {std::regex(R"(\{\{family\}\})"), familyStr},
                {std::regex(R"(\{\{sessionId\}\})"), sessionId.toString()},
              };

              // Replace keywords with values
              auto opt {yOpt.as<std::string>()};
              for (const auto& [k,v] : replacementMap) {
                opt = std::regex_replace(opt, k, v);
              }
              
              ossOpts << " " << opt;
            }

            // Add command to command set
            std::ostringstream ossTitle, ossCmd;
            ossTitle << commandTitlePrefix << " "
                     << yCmdMap["title"].as<std::string>();
            ossCmd << yCmdMap["cmd"].as<std::string>()
                   << ossOpts.str();
            commands.emplace_back(ossTitle.str(), ossCmd.str());
          }

          for (const auto& yPsqlMap : yCmdSetMap["psql"]) {
            std::vector<std::tuple<std::regex, std::string>> replacementMap {
              {std::regex(R"(\{\{srcIpAddr\}\})"), srcIpAddr},
              {std::regex(R"(\{\{linkName\}\})"), linkName},
              {std::regex(R"(\{\{ipNet\}\})"), ipNet},
              {std::regex(R"(\{\{ipNetBcast\}\})"), ipNetBcast},
              {std::regex(R"(\{\{roeExcludedPath\}\})"), roeExcludedPath},
              {std::regex(R"(\{\{roeNetworksPath\}\})"), roeNetworksPath},
              {std::regex(R"(\{\{respondingHostsPath\}\})"), respondingHostsPath},
              {std::regex(R"(\{\{rtrIpAddr\}\})"), rtrIpAddr},
              {std::regex(R"(\{\{family\}\})"), familyStr},
              {std::regex(R"(\{\{sessionId\}\})"), sessionId.toString()},
            };

            // Replace keywords with values
            auto query   {yPsqlMap["query"].as<std::string>()};
            auto outfile {yPsqlMap["outfile"].as<std::string>()};
            for (const auto& [k,v] : replacementMap) {
              query = std::regex_replace(query, k, v);
              outfile = std::regex_replace(outfile, k, v);
            }

            // Add command to command set
            std::ostringstream ossTitle, ossCmd;
            ossTitle << commandTitlePrefix << " psql cmd";
            ossCmd << "psql " << '"' << dbConnectString << '"' << " -A -t -c "
                   << '"' << query << '"'
                   << " >> " << outfile;
            commands.emplace_back(ossTitle.str(), ossCmd.str());
          }

          { // Add command set to phase
            std::lock_guard<std::mutex> coutLock {nmpb::coutMutex};
            LOG_INFO << std::endl
                     << "## " << cmdSetName << std::endl;

            std::vector<std::tuple<std::string, std::string>> cmds;
            for (const auto& [_, cmd] : commands) {
              bool stillExec {cmdRunner.systemExec(cmd)};
              if (!stillExec) {
                LOG_WARN << std::endl
                         << yCmdSetMap["err-msg"].as<std::string>()
                         << std::endl;
                cmdRunner.setExecute(false); // disable to maintain count
              }
            }
          }
        }

        // In phase; Per command-set configuration
        for (const auto& yCmdSetMap : yPhaseMap["phase"]) {
          std::vector<std::tuple<std::string, std::string>> commands;

          if (!enabledPhases.empty() && !enabledPhases.count(phaseId)) {
            continue;
          }

          const auto& cmdSetName {yCmdSetMap["name"].as<std::string>()}; 
          std::string ipTarget {4 == family ? "ipv4" : "ipv6"};

          // In command set; Per command configuration
          for (const auto& yCmdMap : yCmdSetMap[ipTarget]) {
            // In command; Per command option configuration
            std::ostringstream ossOpts;
            for (const auto& yOpt : yCmdMap["opts"]) {
              auto opt {yOpt.as<std::string>()};

              std::vector<std::tuple<std::regex, std::string>> replacementMap {
                {std::regex(R"(\{\{srcIpAddr\}\})"), srcIpAddr},
                {std::regex(R"(\{\{linkName\}\})"), linkName},
                {std::regex(R"(\{\{ipNet\}\})"), ipNet},
                {std::regex(R"(\{\{ipNetBcast\}\})"), ipNetBcast},
                {std::regex(R"(\{\{roeExcludedPath\}\})"), roeExcludedPath},
                {std::regex(R"(\{\{roeNetworksPath\}\})"), roeNetworksPath},
                {std::regex(R"(\{\{respondingHostsPath\}\})"), respondingHostsPath},
                {std::regex(R"(\{\{rtrIpAddr\}\})"), rtrIpAddr},
                {std::regex(R"(\{\{family\}\})"), familyStr},
                {std::regex(R"(\{\{sessionId\}\})"), sessionId.toString()},
              };

              // Replace keywords with values
              for (const auto& [k,v] : replacementMap) {
                opt = std::regex_replace(opt, k, v);
              }
              
              ossOpts << " " << opt;
            }

            // Add command to command set
            std::ostringstream ossTitle, ossCmd;
            ossTitle << commandTitlePrefix << " "
                     << yCmdMap["title"].as<std::string>();
            ossCmd << yCmdMap["cmd"].as<std::string>()
                   << ossOpts.str();
            commands.emplace_back(ossTitle.str(), ossCmd.str());
          }
          
          { // Add command set to phase
            std::vector<std::tuple<std::string, std::string>> cmds;
            for (const auto& cmd : commands) {
              cmds.emplace_back(cmd);
            }

            std::lock_guard<std::mutex> coutLock {nmpb::coutMutex};
            LOG_INFO << std::endl
                     << "## " << cmdSetName << std::endl;
            cmdRunner.threadExec(cmds);
          }
        }

        cmdRunner.setExecute(execute); // update in case of alternate logic
        LOG_INFO << std::endl << "### End of phase " << phaseId << std::endl;

        ++phaseId;
      }
    }

    bool
    isPhaseRuntimeError(pqxx::connection& db,
        const nmco::Uuid& playbookSourceId, const std::string& linkName,
        const std::string& ipAddr)
    {
      // short-circuit, if not actually running
      if (!execute) {
        return false;
      }

      bool isError {false};
      pqxx::work t {db};

      if (ifaceIsDown(linkName)) {
        isError = true;
        LOG_ERROR << "Interface in down state" << std::endl;
        t.exec_prepared("insert_playbook_runtime_error",
            playbookSourceId, "Interface state changed (down) during execution"
            );
      }

      if (!ifaceHasAddress(linkName) || !ifaceHasIp(linkName, ipAddr)) {
        isError = true;
        LOG_ERROR << "Interface has no/incorrect IP address" << std::endl;
        t.exec_prepared("insert_playbook_runtime_error",
            playbookSourceId, "Interface lost IP address during execution"
            );
      }

      t.commit();

      return isError;
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
