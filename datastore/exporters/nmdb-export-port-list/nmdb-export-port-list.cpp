// =============================================================================
// Copyright 2017 National Technology & Engineering Solutions of Sandia, LLC
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

#include <netmeld/datastore/tools/AbstractExportTool.hpp>

#include "Parser.hpp"

namespace nmdt = netmeld::datastore::tools;
namespace nmcu = netmeld::core::utils;


class Tool : public nmdt::AbstractExportTool
{
  protected:
    std::string portsStringFromBitset(std::bitset<65536> const& ports)
    {
      std::string portsString;

      uint16_t portRangeFirst {0};
      uint16_t portRangeLast  {0};
      bool     inPortRange    {false};

      for (size_t i {0}; i <= ports.size(); ++i) {
        if ((i < ports.size()) && ports.test(i)) {
          // Port's bit is set: start a port range if not already in a range.
          if (!inPortRange) {
            portRangeFirst = boost::numeric_cast<uint16_t>(i);
            inPortRange = true;
          }
        }
        else {
          // Port's bit is not set or max port: end port range if in a range.
          if (inPortRange) {
            portRangeLast = boost::numeric_cast<uint16_t>(i-1);
            inPortRange = false;

            // Append the completed range onto the ports string.
            portsString += std::to_string(static_cast<uint32_t>(portRangeFirst));
            if (portRangeFirst < portRangeLast) {
              portsString += "-";
              portsString += std::to_string(static_cast<uint32_t>(portRangeLast));
            }
            portsString += ",";
          }
        }
      }

      return portsString;
    }

  public:
    Tool() : nmdt::AbstractExportTool
      (
        "list of ports suitable for use with Nmap.",
        PROGRAM_NAME,
        PROGRAM_VERSION
      )
    {}

  private:
    void addToolOptions() override
    {
      opts.addOptionalOption("tcp", std::make_tuple(
            "tcp,t",
            NULL_SEMANTIC,
            "Enable output of TCP ports from resource file or DB.")
          );
      opts.addOptionalOption("tcp-all", std::make_tuple(
            "tcp-all,T",
            NULL_SEMANTIC,
            "Enable output of TCP ports 0-65535.")
          );
      opts.addOptionalOption("udp", std::make_tuple(
            "udp,u",
            NULL_SEMANTIC,
            "Enable output of UDP ports from resource file or DB.")
          );
      opts.addOptionalOption("udp-all", std::make_tuple(
            "udp-all,U",
            NULL_SEMANTIC,
            "Enable output of UDP ports 0-65535.")
          );
      opts.addOptionalOption("sctp", std::make_tuple(
            "sctp,y",
            NULL_SEMANTIC,
            "Enable output of SCTP ports from resource file or DB.")
          );
      opts.addOptionalOption("stcp-all", std::make_tuple(
            "sctp-all,Y",
            NULL_SEMANTIC,
            "Enable output of SCTP ports 0-65535.")
          );
      opts.addOptionalOption("from-db", std::make_tuple(
            "from-db,D",
            NULL_SEMANTIC,
            "Use ports from database instead of from resource file.")
          );
          
      auto& nmfm {nmcu::FileManager::getInstance()};
      opts.addOptionalOption("config-path", std::make_tuple(
            "config-path,c",
            po::value<std::string>()->default_value(nmfm.getConfPath()/"port-list.conf"),
            "Use specified port configuration file instead of the default.")
          );

    }

    int
    runTool() override
    {
      std::bitset<65536> tcpPorts;
      std::bitset<65536> udpPorts;
      std::bitset<65536> sctpPorts;

      if (opts.exists("from-db")) {
        const auto& dbName  {opts.getValue("db-name")};
        const auto& dbArgs  {opts.getValue("db-args")};
        pqxx::connection db{std::string("dbname=") + dbName + " " + dbArgs};

        pqxx::read_transaction t {db};

        pqxx::result portRows =
          t.exec("SELECT DISTINCT protocol, port"
                 " FROM ports"
                 " WHERE ((port_state = 'open') OR"
                 "        (port_state = 'closed')) AND"
                 "       (0 <= port)");
        for (const auto& portRow : portRows) {
          std::string protocol;
          portRow.at("protocol").to(protocol);

          uint16_t port;
          portRow.at("port").to(port);

          if ("tcp" == protocol) {
            tcpPorts.set(port);
          }
          else if ("udp" == protocol) {
            udpPorts.set(port);
          }
          else if ("sctp" == protocol) {
            sctpPorts.set(port);
          }
        }
      }
      else {
        // Open and parse the port list file.
        sfs::path const portListPath {sfs::path(opts.getValue("config-path"))};

        Results portRanges {
          nmdp::fromFilePath<Parser, Results>(portListPath.string())
        };

        for (const auto& x : portRanges) {
          std::string const& protocol {std::get<0>(x)};

          size_t const portFirst {std::get<1>(x)};
          size_t const portLast  {std::get<2>(x) ? std::get<2>(x) : portFirst};

          if (opts.exists("tcp") && (std::string::npos != protocol.find('T'))) {
            for (size_t p {portFirst}; p <= portLast; ++p) {
              tcpPorts.set(p);
            }
          }

          if (opts.exists("udp") && (std::string::npos != protocol.find('U'))) {
            for (size_t p {portFirst}; p <= portLast; ++p) {
              udpPorts.set(p);
            }
          }

          if (opts.exists("sctp") && (std::string::npos != protocol.find('Y'))) {
            for (size_t p {portFirst}; p <= portLast; ++p) {
              sctpPorts.set(p);
            }
          }
        }
      }

      // If this program is being used, ports are being specified to nmap.
      // Nmap is unhappy if there are no ports, so ensure at least one.
      // Focus on services that are very common inside enterprise networks.
      tcpPorts.set(80);    // HTTP
      tcpPorts.set(443);   // HTTPS
      tcpPorts.set(22);    // SSH
      tcpPorts.set(25);    // SMTP
      tcpPorts.set(53);    // DNS
      tcpPorts.set(445);   // Microsoft CIFS
      tcpPorts.set(88);    // Kerberos v5
      tcpPorts.set(389);   // LDAP
      tcpPorts.set(636);   // LDAPS

      udpPorts.set(53);    // DNS
      udpPorts.set(123);   // NTP
      udpPorts.set(161);   // SNMP
      udpPorts.set(88);    // Kerberos v5
      udpPorts.set(389);   // LDAP

      sctpPorts.set(80);   // HTTP
      sctpPorts.set(443);  // HTTPS
      sctpPorts.set(2905); // M3UA
      sctpPorts.set(3868); // Diameter

      if (opts.exists("tcp-all")) {
        tcpPorts.set();
      }

      if (opts.exists("udp-all")) {
        udpPorts.set();
      }

      if (opts.exists("sctp-all")) {
        sctpPorts.set();
      }

      std::string nmapPorts;
      if (opts.exists("tcp") || opts.exists("tcp-all")) {
        nmapPorts += "T:";
        nmapPorts += portsStringFromBitset(tcpPorts);
      }
      if (opts.exists("udp") || opts.exists("udp-all")) {
        nmapPorts += "U:";
        nmapPorts += portsStringFromBitset(udpPorts);
      }
      if (opts.exists("sctp") || opts.exists("sctp-all")) {
        nmapPorts += "Y:";
        nmapPorts += portsStringFromBitset(sctpPorts);
      }
      if (nmapPorts.size()) {
        nmapPorts.pop_back();  // Remove trailing ","
      }

      LOG_NOTICE << nmapPorts << std::endl;

      return nmcu::Exit::SUCCESS;
    }

};


int main(int argc, char** argv) {
  Tool tool;
  return tool.start(argc, argv);
}
