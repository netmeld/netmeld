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

#include "IntraNetwork.hpp"

namespace netmeld::datastore::exporters::scans {
  // ========================================================================
  // Constructors
  // ========================================================================
  IntraNetwork::IntraNetwork(const std::string& dbConnInfo) :
    ExportScan(dbConnInfo)
  {
    db.prepare("select_intranetwork_scan_source", R"(
        SELECT DISTINCT src_ip_addr
        FROM intra_network_ports
        ORDER BY src_ip_addr
      )");

    db.prepare("select_intranetwork_scan_destination", R"(
        SELECT DISTINCT dst_ip_addr
        FROM intra_network_ports
        WHERE ($1 = src_ip_addr)
        ORDER BY dst_ip_addr
      )");

    db.prepare("select_intranetwork_scan_port", R"(
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

    db.prepare("select_intranetwork_scan_service", R"(
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
  IntraNetwork::exportTemplate(const std::unique_ptr<Writer>& writer) const
  {
    std::vector<std::string> ips {
        "IP_01"
      , "IP_02"
      , "IP_02"
      , "IP_03"
      };
    for (const auto& dstIp : ips) {
      std::vector<std::string> entry {
          dstIp, "HOSTNAME"
        , "PORT", "PROTOCOL", "STATE", "REASON"
        , "SERVICE_NAME", "SERVICE_REASON"
        };

      writer->addRow(entry);
    }

    finalize(writer);
  }

  void
  IntraNetwork::exportFromDb(const std::unique_ptr<Writer>& writer)
  {
    bool empty {true};
    pqxx::read_transaction rt {db};
    for ( const auto& sourceRow
        : rt.exec_prepared("select_intranetwork_scan_source")
        )
    {
      empty = false;
      srcIp = sourceRow.at(0).c_str();

      for ( const auto& destinationRow
          : rt.exec_prepared("select_intranetwork_scan_destination", srcIp)
          )
      {
        std::string dstIp     {destinationRow.at("dst_ip_addr").c_str()};
        std::string dstIpName {getHostname(rt, dstIp)};

        for ( const auto& portRow
            : rt.exec_prepared("select_intranetwork_scan_port", srcIp, dstIp)
            )
        {
          std::string protocol    {portRow.at("protocol").c_str()};
          std::string port        {portRow.at("port").c_str()};
          std::string portState   {portRow.at("port_state").c_str()};
          std::string portReason  {portRow.at("port_reason").c_str()};

          if ("-1" == port) { // short circuit, protocol scan result
            continue;
          }

          std::string serviceName;
          std::string serviceDesc;
          for ( const auto& serviceRow
              : rt.exec_prepared("select_intranetwork_scan_service"
                                , dstIp, protocol, port
                                )
              )
          {
            std::string srvcName   {serviceRow.at("service_name").c_str()};
            std::string srvcDesc   {serviceRow.at("service_description").c_str()};
            std::string srvcReason {serviceRow.at("service_reason").c_str()};

            if (  ("probed" == srvcReason)
               || (serviceName.empty() && "unknown" != srvcName)
               )
            {
              serviceName = srvcName;
              serviceDesc = srvcDesc;
            }
          }

          std::vector<std::string> entry {
              dstIp, dstIpName
            , port, protocol, portState, portReason
            , serviceName, serviceDesc
          };
          writer->addRow(entry);
        }
      }

      finalize(writer);
    }

    if (empty) {
      finalize(writer);
    }
  }

  void
  IntraNetwork::finalize(const std::unique_ptr<Writer>& writer) const
  {
    LOG_DEBUG << "Finalizing data output\n";

    std::string filename {"intra-network-from-" + srcIp};

    writer->writeData(filename, writer->getIntraNetwork(srcIp));
    writer->clearData();
  }
}
