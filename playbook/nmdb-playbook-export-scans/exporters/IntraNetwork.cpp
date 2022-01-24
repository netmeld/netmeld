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

#include "IntraNetwork.hpp"

namespace netmeld::playbook::export_scans {
  // ========================================================================
  // Constructors
  // ========================================================================
  IntraNetwork::IntraNetwork(const std::string& dbConnInfo) :
    ExportScan(dbConnInfo)
  { 
    db.prepare(
      "select_intranetwork_scan_source",
      R"(
      SELECT DISTINCT src_ip_addr
      FROM intra_network_ports
      ORDER BY src_ip_addr
      )");

    db.prepare(
      "select_intranetwork_scan_destination",
      R"(
      SELECT DISTINCT dst_ip_addr
      FROM intra_network_ports
      WHERE ($1 = src_ip_addr)
      ORDER BY dst_ip_addr
      )");

    db.prepare(
      "select_intranetwork_scan_port",
      R"(
      SELECT DISTINCT
        protocol, port, port_state, port_reason
      FROM intra_network_ports
      WHERE ($1 = src_ip_addr)
        AND ($2 = dst_ip_addr)
        AND ( ( ( ('open' = port_state) OR ('closed' = port_state) )
                AND NOT ( ('ip' = protocol)
                          OR ('' = protocol)
                          OR ('-1' = port) ) )
              OR ( ('-1' = port) AND ('open' = port_state) ) )
      ORDER BY protocol, port, port_state, port_reason
      )");

    db.prepare(
      "select_intranetwork_scan_service",
      R"(
      SELECT DISTINCT
        service_name, service_description, service_reason
      FROM network_services
      WHERE ($1 = ip_addr)
        AND ($2 = protocol)
        AND ($3 = port)
      ORDER BY service_reason
      )");
  }

  // ========================================================================
  // Methods
  // ========================================================================
  void
  IntraNetwork::exportTemplate(auto& writer) const
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
      writer->addRow(entry);
    }
  }

  void
  IntraNetwork::exportFromDb(auto& writer, const std::string& srcIp)
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
        writer->addRow(data);
      }
    }
    t.abort();
  }

  void
  IntraNetwork::exportScan(std::unique_ptr<Writer>& writer)
  {
    pqxx::read_transaction t {db};
    pqxx::result sourceRows
      {t.exec_prepared("select_intranetwork_scan_source")};
    t.abort();

    if (0 == sourceRows.size()) {
      std::string srcIp {"IP/CIDR"};
      exportTemplate(writer);
      writer->writeData(
          "intra-network-from-" + srcIp,
          writer->getIntraNetwork(srcIp)
      );
      writer->clearData();
    } else {
      for (const auto& sourceRow : sourceRows) {
        std::string srcIp;
        sourceRow.at("src_ip_addr").to(srcIp);
        exportFromDb(writer, srcIp);
        writer->writeData(
            "intra-network-from-" + srcIp,
            writer->getIntraNetwork(srcIp)
        );
        writer->clearData();
      }
    }
  }
}
