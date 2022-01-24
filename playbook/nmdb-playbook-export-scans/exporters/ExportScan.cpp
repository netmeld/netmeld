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

#include "ExportScan.hpp"

namespace netmeld::playbook::export_scans {
  // ==========================================================================
  // Constructors
  // ==========================================================================
  ExportScan::ExportScan(const std::string& dbConnInfo) :
    db(dbConnInfo)
  {
    db.prepare
      ("select_network_scan_name",
       "SELECT DISTINCT string_agg(device_id, ', ') as device_id"
       " FROM device_ip_addrs"
       " WHERE ($1 = ip_addr)");
  }


  // ==========================================================================
  // Methods
  // ==========================================================================
  std::string
  ExportScan::getHostname(
      pqxx::read_transaction& t, const std::string& targetIp
    ) const
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
}
