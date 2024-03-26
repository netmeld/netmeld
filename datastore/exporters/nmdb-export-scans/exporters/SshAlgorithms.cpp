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

#include "SshAlgorithms.hpp"

#include <netmeld/core/utils/StringUtilities.hpp>

namespace nmcu = netmeld::core::utils;

namespace netmeld::datastore::exporters::scans {
  // ========================================================================
  // Constructors
  // ========================================================================
  SshAlgorithms::SshAlgorithms(const std::string& dbConnInfo) :
    ExportScan(dbConnInfo)
  {
    db.prepare("select_ssh_algorithms", R"(
        SELECT DISTINCT
          ip_addr, ssh_algo_type, ssh_algo_name
        FROM ssh_host_algorithms
        ORDER BY 1,2,3
      )");

    db.prepare("select_ssh_servers", R"(
        SELECT DISTINCT ip_addr
        FROM ssh_host_algorithms
        ORDER BY ip_addr
      )");

    db.prepare("select_ssh_algorithm_types", R"(
        SELECT DISTINCT ssh_algo_type
        FROM ssh_host_algorithms
        WHERE (ip_addr = $1)
        ORDER BY ssh_algo_type
      )");

    db.prepare("select_ssh_algorithm_names", R"(
        SELECT DISTINCT ssh_algo_name
        FROM ssh_host_algorithms
        WHERE (ip_addr = $1)
          AND (ssh_algo_type = $2)
        ORDER BY ssh_algo_name
      )");
  }

  // ========================================================================
  // Methods
  // ========================================================================
  void
  SshAlgorithms::exportTemplate(const std::unique_ptr<Writer>& writer) const
  {
    std::vector<std::vector<std::string>> data {
        { "IP", "HOSTNAME", "TYPE", "GOOD", good }
      , { "IP", "HOSTNAME", "TYPE", "NO_DETERMINATION", unk }
      , { "IP", "HOSTNAME", "TYPE", "BAD", bad }
      , { "IP_01", "HOSTNAME", "TYPE_01", "ALG_01", unk }
      , { "IP_02", "HOSTNAME", "TYPE_01", "ALG_01", unk }
      , { "IP_02", "HOSTNAME", "TYPE_02", "ALG_01", unk }
      , { "IP_02", "HOSTNAME", "TYPE_02", "ALG_02", unk }
      , { "IP_02", "HOSTNAME", "TYPE_03", "ALG_02", unk }
      , { "IP_03", "HOSTNAME", "TYPE_03", "ALG_02", unk }
      };

    for (const auto& entry : data) {
      writer->addRow(entry);
    }

    finalize(writer);
  }

  void
  SshAlgorithms::exportFromDb(const std::unique_ptr<Writer>& writer)
  {
    pqxx::read_transaction rt {db};
    for ( const auto& record
        : rt.exec_prepared("select_ssh_algorithms")
        )
    {
      std::string serverIp {record.at("ip_addr").c_str()};
      std::string algoType {record.at("ssh_algo_type").c_str()};
      std::string algoName {record.at("ssh_algo_name").c_str()};

      std::string hostname {getHostname(rt, serverIp)};

      std::string color {unk};
      auto searchType {algorithms.find(nmcu::toLower(algoType))};
      if (searchType != algorithms.end()) {
        auto types {searchType->second};
        auto searchName {types.find(algoName)};
        if (searchName != types.end()) {
          color = searchName->second;
        }
      }

      std::vector<std::string> entry
        {serverIp, hostname, algoType, algoName, color};

      writer->addRow(entry);
    }

    finalize(writer);
  }

  void
  SshAlgorithms::finalize(const std::unique_ptr<Writer>& writer) const
  {
    LOG_DEBUG << "Finalizing data output\n";

    std::string filename {"observed-ssh-algorithms"};

    writer->writeData(filename, writer->getSshAlgorithms());
    writer->clearData();
  }
}
