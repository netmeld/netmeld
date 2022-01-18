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

#include <ostream>
#include <regex>

#include <netmeld/datastore/tools/AbstractExportTool.hpp>

#include "WriterContext.hpp"

namespace nmdt = netmeld::datastore::tools;
namespace nmdu = netmeld::datastore::utils;


// =============================================================================
// Export tool definition
// =============================================================================
class Tool : public nmdt::AbstractExportTool
{
  // ===========================================================================
  // Variables
  // ===========================================================================
  private: // Variables should generally be private
    std::string good {"darkgreen"};
    std::string bad  {"red"};
    std::map<std::string, std::string> algoComp
    {
    };
    std::map<std::string, std::string> algoEnc
    {
      {"aes128-ctr", good},
      {"aes256-ctr", good},
      {"aes128-gcm@openssh.com", good},
      {"aes256-gcm@openssh.com", good},

      {"none", bad},
      {"3des-cbc", bad},
      {"blowfish-cbc", bad},
      {"twofish-cbc", bad},
      {"twofish128-cbc", bad},
      {"twofish256-cbc", bad},
      {"cast128-cbc", bad},
      {"arcfour", bad},
      {"arcfour128", bad},
      {"arcfour256", bad},
      {"aes128-cbc", bad},
      {"aes192-cbc", bad},
      {"aes256-cbc", bad},
      {"rijndael128-cbc", bad},
      {"rijndael192-cbc", bad},
      {"rijndael256-cbc", bad},
      {"rijndael-cbc@lysator.liu.se", bad}
    };
    std::map<std::string, std::string> algoKex
    {
      {"curve25519-sha256", good},
      {"curve25519-sha256@libssh.org", good},
      {"diffie-hellman-group14-sha256", good},
      {"diffie-hellman-group16-sha512", good},
      {"diffie-hellman-group18-sha512", good},
      {"diffie-hellman-group-exchange-sha256", good},

      {"diffie-hellman-group1-sha1", bad},
      {"diffie-hellman-group-exchange-sha1", bad}
    };
    std::map<std::string, std::string> algoMac
    {
      {"hmac-sha2-256-etm@openssh.com", good},
      {"hmac-sha2-512-etm@openssh.com", good},
      {"umac-128-etm@openssh.com", good},

      {"none", bad},
      {"hmac-sha1-96", bad},
      {"hmac-sha2-256-96", bad},
      {"hmac-sha2-512-96", bad},
      {"hmac-md5", bad},
      {"hmac-md5-96", bad},
      {"hmac-ripemd160", bad},
      {"hmac-ripemd160@openssh.com", bad},
      {"hmac-sha1-96-etm@openssh.com", bad},
      {"hmac-md5-etm@openssh.com", bad},
      {"hmac-md5-96-etm@openssh.com", bad},
      {"hmac-ripemd160-etm@openssh.com", bad}
    };
    std::map<std::string, std::string> algoKey
    {
      {"ssh-ed25519-cert-v01@openssh.com", good},
      {"ssh-rsa-cert-v01@openssh.com", good},
      {"ssh-ed25519", good},
      {"ssh-rsa", good},

      {"ssh-dss", bad},
      {"ssh-rsa-cert-v00@openssh.com", bad},
      {"ssh-dss-cert-v00@openssh.com", bad},
      {"ssh-dss-cert-v01@openssh.com", bad}
    };

  protected: // Variables intended for internal/subclass API
    // Inhertied from AbstractTool at this scope
      // std::string            helpBlurb;
      // std::string            programName;
      // std::string            version;
      // nmco::ProgramOptions   opts;
  public: // Variables should rarely appear at this scope


  // ===========================================================================
  // Constructors
  // ===========================================================================
  private: // Constructors should rarely appear at this scope
  protected: // Constructors intended for internal/subclass API
  public: // Constructors should generally be public
    Tool() : nmdt::AbstractExportTool
      ("predefined ConTeXt formatted scan results",
       PROGRAM_NAME,
       PROGRAM_VERSION)
    {}


  // ===========================================================================
  // Methods
  // ===========================================================================
  private: // Methods part of internal API
    // Overriden from AbstractExportTool
    void
    addToolOptions() override
    {
      opts.addOptionalOption("intra-network", std::make_tuple(
            "intra-network",
            NULL_SEMANTIC,
            "Export intra-network scan information")
          );
      opts.addOptionalOption("inter-network", std::make_tuple(
            "inter-network",
            NULL_SEMANTIC,
            "Export inter-network scan information")
          );
      opts.addOptionalOption("nessus", std::make_tuple(
            "nessus",
            NULL_SEMANTIC,
            "Export nessus scan information")
          );
      opts.addOptionalOption("ssh", std::make_tuple(
            "ssh",
            NULL_SEMANTIC,
            "Export SSH algorithm scan information")
          );
      opts.addOptionalOption("toFile", std::make_tuple(
            "toFile",
            NULL_SEMANTIC,
            "Output to file (predefined naming) instead of stdout")
          );
    }

    // Overriden from AbstractExportTool
    int
    runTool() override
    {
      const auto& dbName  {getDbName()};
      const auto& dbArgs  {opts.getValue("db-args")};
      pqxx::connection db {"dbname=" + dbName + " " + dbArgs};
      loadPreparedStmts(db);

      if (opts.exists("intra-network")) {
        exportIntraNetworkScan(db);
      }
      if (opts.exists("inter-network")) {
        exportInterNetworkScan(db);
      }
      if (opts.exists("nessus")) {
        exportNessusScan(db);
      }
      if (opts.exists("ssh")) {
        exportSshAlgorithms(db);
      }

      return nmcu::Exit::SUCCESS;
    }

    // Helpers
    void
    writeData(std::string filename, std::string data)
    {
      if (!opts.exists("toFile")) {
        LOG_INFO << "---START OF " << filename << "---" << std::endl
                 << data << std::endl;
      } else {
        std::regex bs {"/"};
        filename = std::regex_replace(filename, bs, "_");

        LOG_DEBUG << "Writing to file: " << filename << std::endl;
        std::ofstream ofs
          {filename, std::ios_base::binary | std::ios_base::trunc};

        ofs << data << std::endl;
        ofs.close();
      }
    }

    std::string
    getHostname(pqxx::read_transaction& t, std::string targetIp)
    {
      std::string name {""};

      pqxx::result nameRows
        {t.exec_prepared("select_network_scan_name", targetIp)};

      size_t count {nameRows.size()};
      switch (count) {
        case 0:
        {
          // do nothing
          break;
        }
        case 1:
        {
          nameRows.begin().at("device_id").to(name);
          break;
        }
        default:
        {
          LOG_WARN << "Query `select_network_scan_name`"
                   << " returned more than one row"
                   << " (" << count << ")."
                   << std::endl;
          nameRows.begin().at("device_id").to(name);
          break;
        }
      }

      return name;
    }

    // Intra-Network
    void
    exportIntraNetworkScan(pqxx::connection& db)
    {
      WriterContext wc;
 
      pqxx::read_transaction t {db};
      pqxx::result sourceRows
        {t.exec_prepared("select_intranetwork_scan_source")};

      if (0 == sourceRows.size()) {
        std::string srcIp {"IP/CIDR"};
        exportIntraNetworkScanTemplate(wc);
        std::string filename {"intra-network-from-" + srcIp + ".tex"};
        writeData(filename, wc.writeIntraNetwork(srcIp));
        return;
      }

      for (const auto& sourceRow : sourceRows) {
        std::string srcIp;
        sourceRow.at("src_ip_addr").to(srcIp);
        exportIntraNetworkScanResults(wc, db, srcIp);
        std::string filename {"intra-network-from-" + srcIp + ".tex"};
        writeData(filename, wc.writeIntraNetwork(srcIp));
      }
    }

    void
    exportIntraNetworkScanTemplate(WriterContext& wc)
    {
      std::vector<std::vector<std::string>> data {
        { "IP_01","HOSTNAME", "NUM","PROTO", "STATE","REASON",
          "NAME","DESCRIPTION"
        },
        { "IP_02","HOSTNAME", "NUM","PROTO", "STATE","REASON",
          "NAME","DESCRIPTION"
        },
        { "IP_02","HOSTNAME", "NUM","PROTO", "STATE","REASON",
          "NAME","DESCRIPTION"
        },
        { "IP_03","HOSTNAME", "NUM","PROTO", "STATE","REASON",
          "NAME","DESCRIPTION"
        },
      };
      for (const auto& entry : data) {
        wc.addRow(entry);
      }
    }

    void
    exportIntraNetworkScanResults(WriterContext& wc, pqxx::connection& db, std::string srcIp)
    {
      pqxx::read_transaction t {db};
      pqxx::result destinationRows
        {t.exec_prepared("select_intranetwork_scan_destination", srcIp)};

      for (const auto& destinationRow : destinationRows) {
        std::string dstIp;
        destinationRow.at("dst_ip_addr").to(dstIp);

        std::string dstIpName {getHostname(t, dstIp)};

        pqxx::result portRows
          {t.exec_prepared("select_intranetwork_scan_port", srcIp, dstIp)};

        for (const auto& portRow : portRows) {
          std::string protocol;
          portRow.at("protocol").to(protocol);
          std::string port;
          portRow.at("port").to(port);
          std::string portState;
          portRow.at("port_state").to(portState);
          std::string portReason;
          portRow.at("port_reason").to(portReason);

          pqxx::result serviceRows {
            t.exec_prepared("select_intranetwork_scan_service",
                            dstIp, protocol, port)
          };

          std::string serviceName;
          std::string serviceDesc;
          for (const auto& serviceRow : serviceRows) {
            std::string tmpSrvcName;
            serviceRow.at("service_name").to(tmpSrvcName);
            std::string tmpSrvcDesc;
            serviceRow.at("service_description").to(tmpSrvcDesc);
            std::string tmpSrvcReason;
            serviceRow.at("service_reason").to(tmpSrvcReason);

            if (   ("probed" == tmpSrvcReason)
                || (serviceName.empty() && "unknown" != tmpSrvcName))
            {
              serviceName = tmpSrvcName;
              serviceDesc = tmpSrvcDesc;
            }
          }

          if ("-1" == port) {
            continue;
          }

          std::vector<std::string> data {
            dstIp, dstIpName, port, protocol, portState, portReason,
            serviceName, serviceDesc
          };
          wc.addRow(data);
        }
      }
    }

    // Inter-Network
    void
    exportInterNetworkScan(pqxx::connection& db)
    {
      WriterContext wc;

      pqxx::read_transaction t {db};
      pqxx::result sourceRows
        {t.exec_prepared("select_internetwork_scan_source")};

      if (0 == sourceRows.size()) {
        std::string srcIp {"IP/CIDR"};
        exportInterNetworkScanTemplate(wc);
        std::string filename {"inter-network-from-" + srcIp + ".tex"};
        writeData(filename, wc.writeInterNetwork(srcIp));
        return;
      }

      for (const auto& sourceRow : sourceRows) {
        std::string srcIp;
        sourceRow.at("src_ip_addr").to(srcIp);
        exportInterNetworkScanResults(wc, db, srcIp);
        std::string filename {"inter-network-from-" + srcIp + ".tex"};
        writeData(filename, wc.writeInterNetwork(srcIp));
      }
    }

    void
    exportInterNetworkScanTemplate(WriterContext& wc)
    {
      std::vector<std::vector<std::string>> data {
        { "RTR_IP_01","HOSTNAME", "DST_IP_01","HOSTNAME",
          "NUM","PROTO", "STATE","REASON"
        },
        { "RTR_IP_02","HOSTNAME", "DST_IP_02","HOSTNAME",
          "NUM","PROTO", "STATE","REASON"
        },
        { "RTR_IP_02","HOSTNAME", "DST_IP_03","HOSTNAME",
          "NUM","PROTO", "STATE","REASON"
        },
        { "RTR_IP_03","HOSTNAME", "DST_IP_04","HOSTNAME",
          "NUM","PROTO", "STATE","REASON"
        },
        { "RTR_IP_03","HOSTNAME", "DST_IP_04","HOSTNAME",
          "NUM","PROTO", "STATE","REASON"
        },
        { "RTR_IP_04","HOSTNAME", "DST_IP_04","HOSTNAME",
          "NUM","PROTO", "STATE","REASON"
        },
      };
      for (const auto& entry : data) {
        wc.addRow(entry);
      }
    }

    void
    exportInterNetworkScanResults(WriterContext wc, pqxx::connection& db, std::string srcIp)
    {
      pqxx::read_transaction t {db};
      pqxx::result routerRows
        {t.exec_prepared("select_internetwork_scan_router", srcIp)};
      for (const auto& routerRow : routerRows) {
        std::string rtrIp;
        routerRow.at("next_hop_ip_addr").to(rtrIp);

        std::string rtrName {getHostname(t, rtrIp)};

        pqxx::result destinationRows {
          t.exec_prepared("select_internetwork_scan_destination", srcIp, rtrIp)
        };
        for (const auto& destinationRow : destinationRows) {
          std::string dstIp;
          destinationRow.at("dst_ip_addr").to(dstIp);

          std::string dstIpName {getHostname(t, dstIp)};

          pqxx::result portRows {
            t.exec_prepared("select_internetwork_scan_port",
                srcIp, rtrIp, dstIp)
          };
          for (const auto& portRow : portRows) {
            std::string protocol;
            portRow.at("protocol").to(protocol);
            std::string port;
            portRow.at("port").to(port);
            std::string portState;
            portRow.at("port_state").to(portState);
            std::string portReason;
            portRow.at("port_reason").to(portReason);

            if ("-1" == port) {
              port = "other";
            }

            std::vector<std::string> data {
              rtrIp, rtrName, dstIp, dstIpName,
              port, protocol, portState, portReason
            };
            wc.addRow(data);
          }
        }
      }
    }

    // Nessus
    void
    exportNessusScan(pqxx::connection& db)
    {
      WriterContext wc;
      std::string filename {"nessus-scan-results.tex"};

      pqxx::read_transaction t {db};
      pqxx::result pluginRows
        {t.exec_prepared("select_nessus_plugins")};

      if (0 == pluginRows.size()) {
        exportNessusScanTemplate(wc);
        writeData(filename, wc.writeNessus());
      } else {
        exportNessusScanResults(wc, db, pluginRows);
        writeData(filename, wc.writeNessus());
      }
    }

    void
    exportNessusScanResults(WriterContext& wc, pqxx::connection& db, pqxx::result pluginRows)
    {
      pqxx::read_transaction t {db};

      for (const auto& pluginRow : pluginRows) {
        std::string id;
        pluginRow.at("plugin_id").to(id);
        std::string name;
        pluginRow.at("plugin_name").to(name);
        std::string severity;
        pluginRow.at("severity").to(severity);
        std::string description;
        pluginRow.at("description").to(description);

        std::vector<std::string> data
          {id, severity, name, description};

        pqxx::result destinationRows
          {t.exec_prepared("select_nessus_destinations", id)};

        for (const auto& destinationRow : destinationRows) {
          std::string ipAddr;
          destinationRow.at("ip_addr").to(ipAddr);

          std::string ipAddrName {getHostname(t, ipAddr)};

          data.push_back(ipAddr);
          data.push_back(ipAddrName);
        }
        wc.addRow(data);
      }
    }

    void
    exportNessusScanTemplate(WriterContext& wc)
    {
      std::vector<std::vector<std::string>> data {
        { "ID_01","HIGH", "PLUGIN_NAME","DESCRIPTION",
          "IP_01","HOSTNAME",
        },
        { "ID_02","HIGH", "PLUGIN_NAME","DESCRIPTION",
          "IP_01","HOSTNAME", "IP_02","HOSTNAME",
          "IP_03","HOSTNAME", "IP_04","HOSTNAME",
        },
        { "ID_03","MODERATE", "PLUGIN_NAME","DESCRIPTION",
          "IP_01","HOSTNAME", "IP_02","HOSTNAME",
        },
        { "ID_04","LOW", "PLUGIN_NAME","DESCRIPTION",
          "IP_01","HOSTNAME",
        },
      };
      for (const auto& entry : data) {
        wc.addRow(entry);
      }
    }

    // SSH Algorithms
    void
    exportSshAlgorithms(pqxx::connection& db)
    {
      WriterContext wc;
      std::string filename {"observed-ssh-algorithms.tex"};

      pqxx::read_transaction t {db};
      pqxx::result records
        {t.exec_prepared("select_ssh_algorithms")};

      if (0 == records.size()) {
        exportSshAlgorithmsTemplate(wc);
        writeData(filename, wc.writeSshAlgorithms());
      } else {
        exportSshAlgorithmsResults(wc, db, records);
        writeData(filename, wc.writeSshAlgorithms());
      }

    }

    void
    exportSshAlgorithmsTemplate(WriterContext& wc)
    {
      std::vector<std::vector<std::string>> data {
        { "IP","HOSTNAME", "TYPE","GOOD", good },
        { "IP","HOSTNAME", "TYPE","NO_DETERMINATION", "black" },
        { "IP","HOSTNAME", "TYPE","BAD", bad },
        { "IP_01","HOSTNAME", "TYPE_01","ALG_01", "black" },
        { "IP_02","HOSTNAME", "TYPE_01","ALG_01", "black" },
        { "IP_02","HOSTNAME", "TYPE_02","ALG_01", "black" },
        { "IP_02","HOSTNAME", "TYPE_02","ALG_02", "black" },
        { "IP_02","HOSTNAME", "TYPE_03","ALG_02", "black" },
        { "IP_03","HOSTNAME", "TYPE_03","ALG_02", "black" },
      };
      for (const auto& entry : data) {
        wc.addRow(entry);
      }
    }

    void
    exportSshAlgorithmsResults(WriterContext& wc, pqxx::connection& db, pqxx::result records)
    {
      pqxx::read_transaction t {db};

      for (const auto& record : records) {
        std::string serverIp;
        record.at("ip_addr").to(serverIp);
        std::string algoType;
        record.at("ssh_algo_type").to(algoType);
        std::string algoName;
        record.at("ssh_algo_name").to(algoName);

        std::string hostname {getHostname(t, serverIp)};

        std::string color {"black"};
        if ("compression_algorithms" == algoType) {
          auto search {algoComp.find(algoName)};
          if (search != algoComp.end()) {
            color = search->second;
          }
        } else if ("encryption_algorithms" == algoType) {
          auto search {algoEnc.find(algoName)};
          if (search != algoEnc.end()) {
            color = search->second;
          }
        } else if ("kex_algorithms" == algoType) {
          auto search {algoKex.find(algoName)};
          if (search != algoKex.end()) {
            color = search->second;
          }
        } else if ("mac_algorithms" == algoType) {
          auto search {algoMac.find(algoName)};
          if (search != algoMac.end()) {
            color = search->second;
          }
        } else if ("server_host_key_algorithms" == algoType) {
          auto search {algoKey.find(algoName)};
          if (search != algoKey.end()) {
            color = search->second;
          }
        }

        std::vector<std::string> data
          {serverIp, hostname, algoType, algoName, color};

        wc.addRow(data);
      }
    }

    void
    loadPreparedStmts(pqxx::connection& db)
    {
      // Common
      db.prepare
        ("select_network_scan_name",
         "SELECT DISTINCT string_agg(device_id, ', ') as device_id"
         " FROM device_ip_addrs"
         " WHERE ($1 = ip_addr)");


      // Intranetwork
      db.prepare
        ("select_intranetwork_scan_source",
         "SELECT DISTINCT src_ip_addr"
         " FROM intra_network_ports"
         " ORDER BY src_ip_addr");

      db.prepare
        ("select_intranetwork_scan_destination",
         "SELECT DISTINCT dst_ip_addr"
         " FROM intra_network_ports"
         " WHERE ($1 = src_ip_addr)"
         " ORDER BY dst_ip_addr");

      db.prepare
        ("select_intranetwork_scan_port",
         "SELECT DISTINCT"
         "   protocol, port, port_state, port_reason"
         " FROM intra_network_ports"
         " WHERE ($1 = src_ip_addr)"
         "   AND ($2 = dst_ip_addr)"
         "   AND ( ((('open' = port_state) OR ('closed' = port_state))"
         "          AND NOT (('ip' = protocol) OR ('' = protocol) OR ('-1' = port)))"
         "         OR (('-1' = port) and ('open' = port_state)))"
         " ORDER BY protocol, port, port_state, port_reason");

      db.prepare
        ("select_intranetwork_scan_service",
         "SELECT DISTINCT"
         "   service_name, service_description, service_reason"
         " FROM network_services"
         " WHERE ($1 = ip_addr)"
         "   AND ($2 = protocol)"
         "   AND ($3 = port)"
         " ORDER BY service_reason");


      // Internetwork
      db.prepare
        ("select_internetwork_scan_source",
         "SELECT DISTINCT src_ip_addr"
         " FROM inter_network_ports"
         " ORDER BY src_ip_addr");

      db.prepare
        ("select_internetwork_scan_router",
         "SELECT DISTINCT next_hop_ip_addr"
         " FROM inter_network_ports"
         " WHERE ($1 = src_ip_addr)"
         " ORDER BY next_hop_ip_addr");

      db.prepare
        ("select_internetwork_scan_destination",
         "SELECT DISTINCT dst_ip_addr"
         " FROM inter_network_ports"
         " WHERE ($1 = src_ip_addr)"
         "   AND ($2 = next_hop_ip_addr)"
         " ORDER BY dst_ip_addr");

      db.prepare
        ("select_internetwork_scan_port",
         "SELECT DISTINCT"
         "   protocol, port, port_state, port_reason"
         " FROM inter_network_ports"
         " WHERE ($1 = src_ip_addr)"
         "   AND ($2 = next_hop_ip_addr)"
         "   AND ($3 = dst_ip_addr)"
         "   AND ( ((('open' = port_state) OR ('closed' = port_state))"
         "          AND NOT (('ip' = protocol) OR ('' = protocol) OR ('-1' = port)))"
         "         OR (('-1' = port) and ('open' = port_state)))"
         " ORDER BY protocol, port, port_state, port_reason");


      // Nessus
      db.prepare
        ("select_nessus_plugins",
         "SELECT DISTINCT"
         "   plugin_id, plugin_name, plugin_family, plugin_type,"
         "   severity, description, solution"
         " FROM nessus_results"
         " ORDER BY severity desc, plugin_id asc");

      db.prepare
        ("select_nessus_destinations",
         "SELECT DISTINCT ip_addr"
         " FROM nessus_results"
         " WHERE (plugin_id = $1)"
         " ORDER BY ip_addr");


      // SSH Algorithms
      db.prepare
        ("select_ssh_algorithms",
         "SELECT DISTINCT"
         "   ip_addr, ssh_algo_type, ssh_algo_name"
         " FROM ssh_host_algorithms"
         " ORDER BY 1,2,3");

      db.prepare
        ("select_ssh_servers",
         "SELECT DISTINCT ip_addr"
         " FROM ssh_host_algorithms"
         " ORDER BY ip_addr");

      db.prepare
        ("select_ssh_algorithm_types",
         "SELECT DISTINCT ssh_algo_type"
         " FROM ssh_host_algorithms"
         " WHERE (ip_addr = ?)"
         " ORDER BY ssh_algo_type");

      db.prepare
        ("select_ssh_algorithm_names",
         "SELECT DISTINCT ssh_algo_name"
         " FROM ssh_host_algorithms"
         " WHERE (ip_addr = ?)"
         "   AND (ssh_algo_type = ?)"
         " ORDER BY ssh_algo_name");
    }

  protected: // Methods part of subclass API
  public: // Methods part of public API
};


// =============================================================================
// Program entry point
// =============================================================================
int main(int argc, char** argv) {
  Tool tool;
  return tool.start(argc, argv);
}
