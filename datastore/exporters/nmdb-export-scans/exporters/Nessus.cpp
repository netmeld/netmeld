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

#include "Nessus.hpp"

namespace netmeld::datastore::exporters::scans {
  // ========================================================================
  // Constructors
  // ========================================================================
  Nessus::Nessus(const std::string& dbConnInfo) :
    ExportScan(dbConnInfo)
  {
    db.prepare("select_nessus_plugins", R"(
        SELECT DISTINCT
          plugin_id, plugin_name, severity, description, solution
        FROM nessus_results
        ORDER BY severity desc, plugin_id asc
      )");

    db.prepare("select_nessus_destinations", R"(
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
  Nessus::exportTemplate(const std::unique_ptr<Writer>& writer) const
  {
    std::vector<std::vector<std::string>> data {
        { "ID_01", "HIGH", "PLUGIN_NAME", "DESCRIPTION",
          "IP_01", "HOSTNAME",
        }
      , { "ID_02", "HIGH", "PLUGIN_NAME", "DESCRIPTION",
          "IP_01", "HOSTNAME", "IP_02", "HOSTNAME",
          "IP_03", "HOSTNAME", "IP_04", "HOSTNAME",
        }
      , { "ID_03", "MODERATE", "PLUGIN_NAME", "DESCRIPTION",
          "IP_01", "HOSTNAME", "IP_02", "HOSTNAME",
        }
      , { "ID_04", "LOW", "PLUGIN_NAME", "DESCRIPTION",
          "IP_01", "HOSTNAME",
        }
      };
    for (const auto& entry : data) {
      writer->addRow(entry);
    }

    finalize(writer);
  }

  void
  Nessus::exportFromDb(const std::unique_ptr<Writer>& writer)
  {
    pqxx::read_transaction rt {db};
    for ( const auto& plugin
        : rt.exec_prepared("select_nessus_plugins")
        )
    {
      std::string id          {plugin.at("plugin_id").c_str()};
      std::string name        {plugin.at("plugin_name").c_str()};
      std::string severity    {plugin.at("severity").c_str()};
      std::string description {plugin.at("description").c_str()};

      std::vector<std::string> entry {id, severity, name, description};

      for ( const auto& destinationRow
          : rt.exec_prepared("select_nessus_destinations", id)
          )
      {
        std::string ipAddr      {destinationRow.at("ip_addr").c_str()};
        std::string ipAddrName  {getHostname(rt, ipAddr)};

        entry.push_back(ipAddr);
        entry.push_back(ipAddrName);
      }

      writer->addRow(entry);
    }

    finalize(writer);
  }

  void
  Nessus::finalize(const std::unique_ptr<Writer>& writer) const
  {
    LOG_DEBUG << "Finalizing data output\n";

    std::string filename {"nessus-scan-results"};

    writer->writeData(filename, writer->getProwler());
    writer->clearData();
  }
}
