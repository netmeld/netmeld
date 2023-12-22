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

#include "InterNetwork.hpp"

namespace netmeld::datastore::exporters::scans {
  // ========================================================================
  // Constructors
  // ========================================================================
  InterNetwork::InterNetwork(const std::string& dbConnInfo) :
    ExportScan(dbConnInfo)
  {
    db.prepare("select_internetwork_scan_source", R"(
        SELECT DISTINCT src_ip_addr
        FROM inter_network_ports
        ORDER BY src_ip_addr
      )");

    db.prepare("select_internetwork_scan_router", R"(
        SELECT DISTINCT next_hop_ip_addr
        FROM inter_network_ports
        WHERE ($1 = src_ip_addr)
        ORDER BY next_hop_ip_addr
      )");

    db.prepare("select_internetwork_scan_destination", R"(
        SELECT DISTINCT dst_ip_addr
        FROM inter_network_ports
        WHERE ($1 = src_ip_addr)
          AND ($2 = next_hop_ip_addr)
        ORDER BY dst_ip_addr
      )");

    db.prepare("select_internetwork_scan_port", R"(
        SELECT DISTINCT
          protocol, port, port_state, port_reason
        FROM inter_network_ports
        WHERE ($1 = src_ip_addr)
          AND ($2 = next_hop_ip_addr)
          AND ($3 = dst_ip_addr)
          AND ( ( ( ('open' = port_state) OR ('closed' = port_state) )
                  AND NOT ( ('ip' = protocol) OR ('' = protocol)
                            OR ('-1' = port) ) )
                OR ( ('-1' = port) AND ('open' = port_state) ) )
        ORDER BY protocol, port, port_state, port_reason
      )");
  }

  // ========================================================================
  // Methods
  // ========================================================================
  void
  InterNetwork::exportTemplate(const std::unique_ptr<Writer>& writer) const
  {
    std::vector<std::tuple<std::string, std::string>> tuples {
        { "RTR_IP_01", "DST_IP_01" }
      , { "RTR_IP_02", "DST_IP_02" }
      , { "RTR_IP_02", "DST_IP_03" }
      , { "RTR_IP_03", "DST_IP_04" }
      , { "RTR_IP_03", "DST_IP_04" }
      , { "RTR_IP_04", "DST_IP_04" }
      };

    for (const auto& [rtrIp, dstIp] : tuples) {
      std::vector<std::string> entry {
          rtrIp, "HOSTNAME"
        , dstIp, "HOSTNAME"
        , "NUM", "PROTO", "STATE", "REASON"
        };

      writer->addRow(entry);
    }

    finalize(writer);
  }

  void
  InterNetwork::exportFromDb(const std::unique_ptr<Writer>& writer)
  {
    bool empty {true};
    pqxx::read_transaction rt {db};
    for ( const auto& sourceRow
        : rt.exec_prepared("select_internetwork_scan_source")
        )
    {
      empty = false;
      srcIp = sourceRow.at(0).c_str();

      for ( const auto& routerRow
          : rt.exec_prepared("select_internetwork_scan_router", srcIp)
          )
      {
        std::string rtrIp   {routerRow.at("next_hop_ip_addr").c_str()};
        std::string rtrName {getHostname(rt, rtrIp)};

        for ( const auto& destinationRow
            : rt.exec_prepared("select_internetwork_scan_destination"
                              , srcIp, rtrIp
                              )
            )
        {
          std::string dstIp     {destinationRow.at("dst_ip_addr").c_str()};
          std::string dstIpName {getHostname(rt, dstIp)};

          for ( const auto& portRow
              : rt.exec_prepared("select_internetwork_scan_port"
                                , srcIp, rtrIp, dstIp
                                )
              )
          {
            std::string protocol    {portRow.at("protocol").c_str()};
            std::string port        {portRow.at("port").c_str()};
            std::string portState   {portRow.at("port_state").c_str()};
            std::string portReason  {portRow.at("port_reason").c_str()};

            if ("-1" == port) {
              port = "other";
            }

            std::vector<std::string> entry {
                rtrIp, rtrName
              , dstIp, dstIpName
              , port, protocol, portState, portReason
            };
            writer->addRow(entry);
          }
        }
      }

      finalize(writer);
    }

    if (empty) {
      finalize(writer);
    }
  }

  void
  InterNetwork::finalize(const std::unique_ptr<Writer>& writer) const
  {
    LOG_DEBUG << "Finalizing data output\n";

    std::string filename {"inter-network-from-" + srcIp};

    writer->writeData(filename, writer->getInterNetwork(srcIp));
    writer->clearData();
  }
}
