// =============================================================================
// Copyright 2022 National Technology & Engineering Solutions of Sandia, LLC
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

#include "Nessus.hpp"

namespace netmeld::playbook::export_scans {
  // ========================================================================
  // Constructors
  // ========================================================================
  Nessus::Nessus(const std::string& dbConnInfo) :
    ExportScan(dbConnInfo)
  { 
    db.prepare(
      "select_nessus_plugins",
      R"(
      SELECT DISTINCT
        plugin_id, plugin_name, severity, description, solution
      FROM nessus_results
      ORDER BY severity desc, plugin_id asc
      )");

    db.prepare(
      "select_nessus_destinations",
      R"(
      SELECT DISTINCT ip_addr
      FROM nessus_results
      WHERE (plugin_id = $1)
      ORDER BY ip_addr
      )");
  }

  // ========================================================================
  // Methods
  // ========================================================================
  void
  Nessus::exportTemplate(auto& writer) const
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
      writer->addRow(entry);
    }
  }

  void
  Nessus::exportFromDb(auto& writer, pqxx::result& plugins)
  {
    pqxx::read_transaction t {db};

    for (const auto& plugin : plugins) {
      std::string id;
      plugin.at("plugin_id").to(id);
      std::string name;
      plugin.at("plugin_name").to(name);
      std::string severity;
      plugin.at("severity").to(severity);
      std::string description;
      plugin.at("description").to(description);

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
      writer->addRow(data);
    }
    t.abort();
  }

  void
  Nessus::exportScan(std::unique_ptr<Writer>& writer)
  {
    pqxx::read_transaction t {db};
    pqxx::result sourceRows
      {t.exec_prepared("select_nessus_plugins")};
    t.abort();

    if (0 == sourceRows.size()) {
      exportTemplate(writer);
    } else {
      exportFromDb(writer, sourceRows);
    }

    std::string filename {"nessus-scan-results"};

    writer->writeData(filename, writer->getNessus());
    writer->clearData();
  }
}
