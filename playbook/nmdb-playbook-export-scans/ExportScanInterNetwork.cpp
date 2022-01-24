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

#include "ExportScanInterNetwork.hpp"

// ========================================================================
// Constructors
// ========================================================================
ExportScanInterNetwork::ExportScanInterNetwork(const std::string& dbConnInfo) :
  ExportScan(dbConnInfo)
{ 
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
}

// ========================================================================
// Methods
// ========================================================================
void
ExportScanInterNetwork::exportTemplate(auto& writer)
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
    writer->addRow(entry);
  }
}

void
ExportScanInterNetwork::exportFromDb(auto& writer, std::string& srcIp)
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
        writer->addRow(data);
      }
    }
  }
}

void
ExportScanInterNetwork::exportScan(std::unique_ptr<Writer>& writer)
{
  pqxx::read_transaction t {db};
  pqxx::result sourceRows
    {t.exec_prepared("select_internetwork_scan_source")};

  if (0 == sourceRows.size()) {
    std::string srcIp {"IP/CIDR"};
    exportTemplate(writer);
    writer->writeData(
        "inter-network-from-" + srcIp,
        writer->getInterNetwork(srcIp)
    );
    writer->clearData();
  } else {
    for (const auto& sourceRow : sourceRows) {
      std::string srcIp;
      sourceRow.at("src_ip_addr").to(srcIp);
      exportFromDb(writer, srcIp);
      writer->writeData(
          "inter-network-from-" + srcIp,
          writer->getInterNetwork(srcIp)
      );
      writer->clearData();
    }
  }
}
