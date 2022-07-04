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

#include "Prowler.hpp"

namespace netmeld::export_scans {
  // ========================================================================
  // Constructors
  // ========================================================================
  Prowler::Prowler(const std::string& dbConnInfo) :
    ExportScan(dbConnInfo)
  {
    db.prepare(
      "select_prowler_checks",
      R"(
      SELECT DISTINCT
          service, severity
        , control_id, level, control, risk, remediation
        , documentation_link
      FROM prowler_checks
      WHERE status = 'FAIL'
      ORDER BY service, severity asc, control_id asc
      )");

    db.prepare(
      "select_prowler_check_resources",
      R"(
      SELECT DISTINCT resource_id
      FROM prowler_checks
      WHERE status = 'FAIL'
        AND service = $1
        AND severity = $2
        AND control_id = $3
      ORDER BY resource_id
      )");
  }

  // ========================================================================
  // Methods
  // ========================================================================
  void
  Prowler::exportTemplate(const auto& writer) const
  {
    std::vector<std::vector<std::string>> data {
        { "SERVICE_01", "SEVERITY_01",
          "CTRL_ID_01", "LEVEL", "CONTROL", "RISK", "REMEDIATION", "LINK",
          "RESOURCE_01",
        },
        { "SERVICE_01", "SEVERITY_01",
          "CTRL_ID_02", "LEVEL", "CONTROL", "RISK", "REMEDIATION", "LINK",
          "RESOURCE_01", "RESOURCE_02", "RESOURCE_03",
        },
        { "SERVICE_01", "SEVERITY_02",
          "CTRL_ID_01", "LEVEL", "CONTROL", "RISK", "REMEDIATION", "LINK",
          "RESOURCE_01",
        },
        { "SERVICE_02", "SEVERITY_01",
          "CTRL_ID_01", "LEVEL", "CONTROL", "RISK", "REMEDIATION", "LINK",
          "RESOURCE_01",
        },
      };

    for (const auto& entry : data) {
      writer->addRow(entry);
    }
  }

  void
  Prowler::exportFromDb(const auto& writer, const pqxx::result& checks)
  {
    pqxx::read_transaction t {db};

    for (const auto& check : checks) {
      std::string service;
      check.at("service").to(service);
      std::string severity;
      check.at("severity").to(severity);
      std::string controlId;
      check.at("control_id").to(controlId);
      std::string level;
      check.at("level").to(level);
      std::string control;
      check.at("control").to(control);
      std::string risk;
      check.at("risk").to(risk);
      std::string remediation;
      check.at("remediation").to(remediation);
      std::string docLink;
      check.at("documentation_link").to(docLink);

      std::vector<std::string> data {
          service, severity,
          controlId, level, control, risk, remediation, docLink
        };

      pqxx::result resourceRows {
          t.exec_prepared("select_prowler_check_resources",
                          service, severity, controlId)
        };

      for (const auto& resourceRow : resourceRows) {
        std::string resourceId;
        resourceRow.at("resource_id").to(resourceId);

        data.push_back(resourceId);
      }
      writer->addRow(data);
    }
    t.abort();
  }

  void
  Prowler::exportScan(const std::unique_ptr<Writer>& writer)
  {
    pqxx::read_transaction t {db};
    pqxx::result sourceRows
      {t.exec_prepared("select_prowler_checks")};
    t.abort();

    if (0 == sourceRows.size()) {
      exportTemplate(writer);
    } else {
      exportFromDb(writer, sourceRows);
    }

    LOG_DEBUG << "Got data, writing to file" << std::endl;

    std::string filename {"prowler-scan-results"};

    writer->writeData(filename, writer->getProwler());
    writer->clearData();
  }
}
