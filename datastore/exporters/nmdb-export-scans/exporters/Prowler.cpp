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

#include "Prowler.hpp"

namespace netmeld::datastore::exporters::scans {
  // ========================================================================
  // Constructors
  // ========================================================================
  Prowler::Prowler(const std::string& dbConnInfo) :
    ExportScan(dbConnInfo)
  {
    db.prepare("select_prowler_checks", R"(
        SELECT DISTINCT
            provider, account_id
          , service_name, sub_service_name
          , severity, check_id, description, risk, recommendation
            , recommendation_url
          , remediation_code
        FROM prowler_checks
        WHERE status = 'FAIL'
        ORDER BY provider, account_id
               , service_name, sub_service_name desc
               , severity asc, check_id asc
        )"
      );

    db.prepare("select_prowler_check_resources", R"(
        SELECT DISTINCT resource_id
        FROM prowler_checks
        WHERE status = 'FAIL'
          AND provider = $1
          AND account_id = $2
          AND check_id = $3
        ORDER BY resource_id
        )"
      );
  }

  // ========================================================================
  // Methods
  // ========================================================================
  void
  Prowler::exportTemplate(const std::unique_ptr<Writer>& writer) const
  {
    std::vector<std::vector<std::string>> data {
        // Everything filled in with extra resources
        { "PROVIDER_01", "ACCOUNT_ID_01"
        , "SERVICE_01", "SUB_SERVICE_01"
        , "SEVERITY_01", "CHECK_ID_01"
        , "DESCRIPTION", "RISK", "RECOMMENDATION", "URL", "CODE"
        , "RESOURCE_01", "RESOURCE_02", "RESOURCE_03"
        },
        // Everything that could be empty
        { "PROVIDER_01", "ACCOUNT_ID_01"
        , "SERVICE_02", ""
        , "SEVERITY_01", "CHECK_ID_01"
        , "DESCRIPTION", "", "RECOMMENDATION", "URL", "CODE"
        , //""
        },
        // Everything filled in no extras
        { "PROVIDER_02", "ACCOUNT_ID_01"
        , "SERVICE_01", "SUB_SERVICE_01"
        , "SEVERITY_01", "CHECK_ID_01"
        , "DESCRIPTION", "RISK", "RECOMMENDATION", "URL", "CODE"
        , "RESOURCE_01"
        },
      };

    for (const auto& entry : data) {
      writer->addRow(entry);
    }

    finalize(writer);
  }

  void
  Prowler::exportFromDb(const std::unique_ptr<Writer>& writer)
  {
    pqxx::read_transaction rt {db};
    for ( const auto& check
        : rt.exec_prepared("select_prowler_checks")
        )
    {
      std::vector<std::string> data;
      for (const auto& col : check) {
        data.emplace_back(col.c_str());
      }

      for ( const auto& resource
          : rt.exec_prepared("select_prowler_check_resources"
                            , check.at("provider").c_str()
                            , check.at("account_id").c_str()
                            , check.at("check_id").c_str()
                            )
          )
      {
        data.emplace_back(resource.at(0).c_str());
      }

      writer->addRow(data);
    }

    finalize(writer);
  }

  void
  Prowler::finalize(const std::unique_ptr<Writer>& writer) const
  {
    LOG_DEBUG << "Finalizing data output\n";

    std::string filename {"prowler-scan-results"};

    writer->writeData(filename, writer->getProwler());
    writer->clearData();
  }
}
