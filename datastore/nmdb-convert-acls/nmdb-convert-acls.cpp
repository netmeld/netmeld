// =============================================================================
// Copyright 2020 National Technology & Engineering Solutions of Sandia, LLC
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

#include <boost/algorithm/string.hpp>
#include <pqxx/pqxx>
#include <cctype>
#include <regex>

#include <netmeld/datastore/objects/PortRange.hpp>
#include <netmeld/datastore/tools/AbstractDatastoreTool.hpp>

extern "C" {
#include <netdb.h>
}


namespace nmdo = netmeld::datastore::objects;
namespace nmdt = netmeld::datastore::tools;
namespace nmcu = netmeld::core::utils;


class Tool : public nmdt::AbstractDatastoreTool
{
  private:

  protected:
    // Inhertied from AbstractTool at this scope
      // std::string            helpBlurb;
      // std::string            programName;
      // std::string            version;
      // ProgramOptions         opts;
  public:


  // ===========================================================================
  // Constructors
  // ===========================================================================
  private: // Constructors should rarely appear at this scope
  protected: // Constructors intended for internal/subclass API
  public: // Constructors should generally be public
    Tool() : nmdt::AbstractDatastoreTool
      ("convert _ac_ to _acl_",
       PROGRAM_NAME, PROGRAM_VERSION)
    {}


  // ===========================================================================
  // Methods
  // ===========================================================================
  private:
    void addToolOptions() override
    {

    }

    int
    runTool() override
    {
      pqxx::connection db {getDbConnectString()};

      db.prepare(
        "select_raw_devices",
        "SELECT DISTINCT"
        "   tool_run_id,"
        "   device_id"
        " FROM raw_devices"
        " ORDER BY device_id, tool_run_id"
      );

      db.prepare(
        "select_raw_device_ac_services_flattened",
        "SELECT DISTINCT"
        "   tool_run_id,"
        "   device_id,"
        "   service_set,"
        "   service_set_data"
        " FROM raw_device_ac_services_flattened"
        " ORDER BY device_id, tool_run_id, service_set, service_set_data"
      );

      db.prepare(
        "select_raw_device_ac_nets_flattened",
        "SELECT DISTINCT"
        "   tool_run_id,"
        "   device_id,"
        "   net_set_id,"
        "   net_set,"
        "   net_set_data"
        " FROM raw_device_ac_nets_flattened"
        " ORDER BY device_id, tool_run_id, net_set_id, net_set, net_set_data"
      );

      // Only convert Juniper rules at this time.
      // The Cisco parser has enough issues that the ACLs are incorrect.
      db.prepare(
        "select_raw_device_ac_rules",
        "SELECT DISTINCT"
        "   rules.tool_run_id AS tool_run_id,"
        "   rules.device_id AS device_id,"
        "   rules.enabled AS enabled,"
        "   rules.ac_id AS ac_id,"
        "   rules.src_net_set_id AS src_net_set_id,"
        "   COALESCE(NULLIF(rules.src_net_set, ''), 'any') AS src_net_set,"
        "   rules.src_iface AS src_iface,"
        "   rules.dst_net_set_id AS dst_net_set_id,"
        "   COALESCE(NULLIF(rules.dst_net_set, ''), 'any') AS dst_net_set,"
        "   rules.dst_iface AS dst_iface,"
        "   COALESCE(NULLIF(rules.service_set, ''), 'any') AS service_set,"
        "   rules.action AS action,"
        "   rules.description AS description"
        " FROM raw_device_ac_rules AS rules"
        " JOIN device_hardware_information AS info"
        " ON (rules.device_id = info.device_id)"
        " WHERE (info.vendor = 'juniper')"
        " ORDER BY rules.device_id, rules.tool_run_id"
      );


      // INSERT queries against the new *_acl_* tables

      db.prepare(
        "insert_raw_device_acl_zone_base",
        "INSERT INTO raw_device_acl_zones_bases"
        " (tool_run_id, device_id, zone_id)"
        " VALUES ($1, $2, $3)"
        " ON CONFLICT"
        " (tool_run_id, device_id, zone_id)"
        " DO NOTHING"
      );
      db.prepare(
        "insert_raw_device_acl_zone_interface",
        "INSERT INTO raw_device_acl_zones_interfaces"
        " (tool_run_id, device_id, zone_id, interface_name)"
        " VALUES ($1, $2, $3, $4)"
        " ON CONFLICT"
        " (tool_run_id, device_id, zone_id, interface_name)"
        " DO NOTHING"
      );

      db.prepare(
        "insert_raw_device_acl_ip_net_base",
        "INSERT INTO raw_device_acl_ip_nets_bases"
        " (tool_run_id, device_id, ip_net_set_id)"
        " VALUES ($1, $2, $3)"
        " ON CONFLICT"
        " (tool_run_id, device_id, ip_net_set_id)"
        " DO NOTHING"
      );
      db.prepare(
        "insert_raw_device_acl_ip_net_ip_net",
        "INSERT INTO raw_device_acl_ip_nets_ip_nets"
        " (tool_run_id, device_id, ip_net_set_id, ip_net)"
        " VALUES ($1, $2, $3, network(($4)::INET))"
        " ON CONFLICT"
        " (tool_run_id, device_id, ip_net_set_id, ip_net)"
        " DO NOTHING"
      );

      db.prepare(
        "insert_raw_device_acl_port_base",
        "INSERT INTO raw_device_acl_ports_bases"
        " (tool_run_id, device_id, port_set_id)"
        " VALUES ($1, $2, $3)"
        " ON CONFLICT"
        " (tool_run_id, device_id, port_set_id)"
        " DO NOTHING"
      );
      db.prepare(
        "insert_raw_device_acl_port_port_range",
        "INSERT INTO raw_device_acl_ports_ports"
        " (tool_run_id, device_id, port_set_id, port_range)"
        " VALUES ($1, $2, $3, $4::PortRange)"
        " ON CONFLICT"
        " (tool_run_id, device_id, port_set_id, port_range)"
        " DO NOTHING"
      );

      db.prepare(
        "insert_raw_device_acl_service_base",
        "INSERT INTO raw_device_acl_services_bases"
        " (tool_run_id, device_id, service_id)"
        " VALUES ($1, $2, $3)"
        " ON CONFLICT"
        " (tool_run_id, device_id, service_id)"
        " DO NOTHING"
      );
      db.prepare(
        "insert_raw_device_acl_service_protocol",
        "INSERT INTO raw_device_acl_services_protocols"
        " (tool_run_id, device_id, service_id, protocol)"
        " VALUES ($1, $2, $3, $4)"
        " ON CONFLICT"
        " (tool_run_id, device_id, service_id, protocol)"
        " DO NOTHING"
      );
      db.prepare(
        "insert_raw_device_acl_service_port_ranges",
        "INSERT INTO raw_device_acl_services_ports"
        " (tool_run_id, device_id, service_id, protocol,"
        "  src_port_range, dst_port_range)"
        " VALUES ($1, $2, $3, $4,"
        "  $5::PortRange, $6::PortRange)"
        " ON CONFLICT"
        " (tool_run_id, device_id, service_id, protocol,"
        "  src_port_range, dst_port_range)"
        " DO NOTHING"
      );

      db.prepare(
        "insert_raw_device_acl_rule_service",
        "INSERT INTO raw_device_acl_rules_services"
        " (tool_run_id, device_id, priority, action,"
        "  incoming_zone_id, outgoing_zone_id,"
        "  src_ip_net_set_id, dst_ip_net_set_id,"
        "  service_id, description)"
        " VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10)"
        " ON CONFLICT"
        " (tool_run_id, device_id, priority, action,"
        "  incoming_zone_id, outgoing_zone_id,"
        "  src_ip_net_set_id, dst_ip_net_set_id,"
        "  service_id)"
        " DO NOTHING"
      );


      pqxx::work t{db};

      // Back-propagate guest device IDs into parent tool runs.
      t.exec(
        "INSERT INTO raw_devices ("
        " SELECT DISTINCT"
        "    srvcs.tool_run_id            AS tool_run_id,"
        "    virts.guest_device_id        AS device_id"
        " FROM raw_device_ac_services AS srvcs"
        " JOIN device_virtualizations AS virts"
        " ON (srvcs.device_id = virts.host_device_id)"
        " WHERE (srvcs.service_set IS NOT NULL)"
        " )"
        " ON CONFLICT DO NOTHING"
      );

      // Copy default/inheritable settings from host devices to guest devices.
      t.exec(
        "INSERT INTO raw_device_ac_services ("
        " SELECT DISTINCT"
        "    srvcs.tool_run_id            AS tool_run_id,"
        "    virts.guest_device_id        AS device_id,"
        "    srvcs.service_set            AS service_set,"
        "    srvcs.service_set_data       AS service_set_data"
        " FROM raw_device_ac_services AS srvcs"
        " JOIN device_hardware_information AS info"
        " ON (srvcs.device_id = info.device_id) AND"
        "    (info.vendor = 'juniper')"
        " JOIN device_virtualizations AS virts"
        " ON (srvcs.device_id = virts.host_device_id)"
        " WHERE (srvcs.service_set IS NOT NULL) AND"
        "       ((srvcs.service_set LIKE 'junos-%') OR"
        "        (srvcs.service_set LIKE 'any%'))"
        " )"
        " ON CONFLICT DO NOTHING"
      );


      // SELECT DISTINCT queries against existing *_ac_* tables

      pqxx::result deviceRows =
        t.exec_prepared("select_raw_devices");
      for (const auto& deviceRow : deviceRows) {
        std::string toolRunId;
        deviceRow.at("tool_run_id").to(toolRunId);
        std::string deviceId;
        deviceRow.at("device_id").to(deviceId);

        // Zone: any
        t.exec_prepared("insert_raw_device_acl_zone_base",
            toolRunId,
            deviceId,
            "any"
        );

        // IP net: any-ipv4
        t.exec_prepared("insert_raw_device_acl_ip_net_base",
            toolRunId,
            deviceId,
            "any-ipv4"
        );
        t.exec_prepared("insert_raw_device_acl_ip_net_ip_net",
            toolRunId,
            deviceId,
            "any-ipv4",
            "0.0.0.0/0"
        );

        // IP net: any-ipv6
        t.exec_prepared("insert_raw_device_acl_ip_net_base",
            toolRunId,
            deviceId,
            "any-ipv6"
        );
        t.exec_prepared("insert_raw_device_acl_ip_net_ip_net",
            toolRunId,
            deviceId,
            "any-ipv6",
            "::/0"
        );

        // IP net: any
        t.exec_prepared("insert_raw_device_acl_ip_net_base",
            toolRunId,
            deviceId,
            "any"
        );
        t.exec_prepared("insert_raw_device_acl_ip_net_ip_net",
            toolRunId,
            deviceId,
            "any",
            "0.0.0.0/0"
        );
        t.exec_prepared("insert_raw_device_acl_ip_net_ip_net",
            toolRunId,
            deviceId,
            "any",
            "::/0"
        );

        // service: any-tcp
        t.exec_prepared("insert_raw_device_acl_service_base",
            toolRunId,
            deviceId,
            "any-tcp"
        );
        t.exec_prepared("insert_raw_device_acl_service_protocol",
            toolRunId,
            deviceId,
            "any-tcp",
            "tcp"
        );
        t.exec_prepared("insert_raw_device_acl_service_port_ranges",
            toolRunId,
            deviceId,
            "any-tcp",
            "tcp",
            nmdo::PortRange{0, 65535},
            nmdo::PortRange{0, 65535}
        );

        // service: any-udp
        t.exec_prepared("insert_raw_device_acl_service_base",
            toolRunId,
            deviceId,
            "any-udp"
        );
        t.exec_prepared("insert_raw_device_acl_service_protocol",
            toolRunId,
            deviceId,
            "any-udp",
            "udp"
        );
        t.exec_prepared("insert_raw_device_acl_service_port_ranges",
            toolRunId,
            deviceId,
            "any-udp",
            "udp",
            nmdo::PortRange{0, 65535},
            nmdo::PortRange{0, 65535}
        );

        // service: any
        t.exec_prepared("insert_raw_device_acl_service_base",
            toolRunId,
            deviceId,
            "any"
        );
        t.exec_prepared("insert_raw_device_acl_service_protocol",
            toolRunId,
            deviceId,
            "any",
            "any"
        );
        t.exec_prepared("insert_raw_device_acl_service_port_ranges",
            toolRunId,
            deviceId,
            "any",
            "any",
            nmdo::PortRange{0, 65535},
            nmdo::PortRange{0, 65535}
        );
      }


      pqxx::result serviceRows =
        t.exec_prepared("select_raw_device_ac_services_flattened");
      for (const auto& serviceRow : serviceRows) {
        std::string toolRunId;
        serviceRow.at("tool_run_id").to(toolRunId);
        std::string deviceId;
        serviceRow.at("device_id").to(deviceId);
        std::string serviceId;
        serviceRow.at("service_set").to(serviceId);
        std::string serviceData;
        serviceRow.at("service_set_data").to(serviceData);

        t.exec_prepared("insert_raw_device_acl_service_base",
            toolRunId,
            deviceId,
            serviceId
        );

        if (std::string::npos != serviceData.find("--established")) {
          serviceData.erase(serviceData.find("--established"));
        }
        if (std::string::npos != serviceData.find("--tracked")) {
          serviceData.erase(serviceData.find("--tracked"));
        }

        std::vector<std::string> serviceParts;
        boost::split(serviceParts, serviceData, boost::is_any_of(":"));

        std::string serviceProtocol = serviceParts.at(0);
        std::string srcPortData;
        std::string dstPortData;
        if (3 <= serviceParts.size()) {
          srcPortData = serviceParts.at(1);
          dstPortData = serviceParts.at(2);
        }

        std::vector<std::string> protocols;
        if ("tcp-udp" == serviceProtocol) {
          protocols.push_back("tcp");
          protocols.push_back("udp");
        }
        else if (("tcp-ms-rpc" == serviceProtocol) || ("tcp-sun-rpc" == serviceProtocol)) {
          protocols.push_back("tcp");
        }
        else if (("udp-ms-rpc" == serviceProtocol) || ("udp-sun-rpc" == serviceProtocol)) {
          protocols.push_back("udp");
        }
        else if (!serviceProtocol.empty() && std::isdigit(serviceProtocol.at(0))) {
          try {
            // If the serviceProtocol is a numeric string, look up the corresponding protocol name.
            const int protoNumber{boost::lexical_cast<int>(serviceProtocol)};
            const protoent* protoEntity{getprotobynumber(protoNumber)};
            if (protoEntity) {
              protocols.push_back(protoEntity->p_name);
            }
          }
          catch (boost::bad_lexical_cast&) {
            // Consume the exception and continue with the non-numeric serviceProtocol string.
            protocols.push_back(serviceProtocol);
          }
        }
        else {
          protocols.push_back(serviceProtocol);
        }

        auto const srcPortRange = convertToPortRange(srcPortData);
        auto const dstPortRange = convertToPortRange(dstPortData);

        for (const auto& protocol : protocols) {
          t.exec_prepared("insert_raw_device_acl_service_protocol",
              toolRunId,
              deviceId,
              serviceId,
              protocol
          );
          t.exec_prepared("insert_raw_device_acl_service_port_ranges",
              toolRunId,
              deviceId,
              serviceId,
              protocol,
              srcPortRange,
              dstPortRange
          );
        }
      }


      pqxx::result acNetRows =
        t.exec_prepared("select_raw_device_ac_nets_flattened");
      for (const auto& acNetRow : acNetRows) {
        std::string toolRunId;
        acNetRow.at("tool_run_id").to(toolRunId);
        std::string deviceId;
        acNetRow.at("device_id").to(deviceId);
        std::string ipNetSetScope;
        acNetRow.at("net_set_id").to(ipNetSetScope);
        std::string ipNetSetId;
        acNetRow.at("net_set").to(ipNetSetId);
        std::string ipNet;
        acNetRow.at("net_set_data").to(ipNet);

        t.exec_prepared("insert_raw_device_acl_ip_net_base",
            toolRunId,
            deviceId,
            ipNetSetId
        );

        std::regex rex_ip{"^(([0-9.]+)|([0-9a-fA-F:]+))(/\\d{1,3})?$"};
        std::smatch m;
        if (!std::regex_match(ipNet, m, rex_ip)) {
          std::cerr
            << "Odd formatted ipNet: "
            << ipNet
            << std::endl;
          continue;
        }

        t.exec_prepared("insert_raw_device_acl_ip_net_ip_net",
            toolRunId,
            deviceId,
            ipNetSetId,
            ipNet
        );
      }


      pqxx::result acRuleRows =
        t.exec_prepared("select_raw_device_ac_rules");
      for (const auto& acRuleRow : acRuleRows) {
        std::string toolRunId;
        acRuleRow.at("tool_run_id").to(toolRunId);
        std::string deviceId;
        acRuleRow.at("device_id").to(deviceId);

        bool enabled;
        acRuleRow.at("enabled").to(enabled);
        size_t priority;
        acRuleRow.at("ac_id").to(priority);
        std::string incomingZoneId;
        acRuleRow.at("src_net_set_id").to(incomingZoneId);
        std::string srcIpNetSetId;
        acRuleRow.at("src_net_set").to(srcIpNetSetId);
        std::string incomingInterfaceName;
        acRuleRow.at("src_iface").to(incomingInterfaceName);
        std::string outgoingZoneId;
        acRuleRow.at("dst_net_set_id").to(outgoingZoneId);
        std::string dstIpNetSetId;
        acRuleRow.at("dst_net_set").to(dstIpNetSetId);
        std::string outgoingInterfaceName;
        acRuleRow.at("dst_iface").to(outgoingInterfaceName);
        std::string serviceId;
        acRuleRow.at("service_set").to(serviceId);
        std::string action;
        acRuleRow.at("action").to(action);
        std::string description;
        acRuleRow.at("description").to(description);

        t.exec_prepared("insert_raw_device_acl_zone_base",
            toolRunId,
            deviceId,
            incomingZoneId
        );
        if (!incomingInterfaceName.empty()) {
          t.exec_prepared("insert_raw_device_acl_zone_interface",
              toolRunId,
              deviceId,
              incomingZoneId,
              incomingInterfaceName
          );
        }

        t.exec_prepared("insert_raw_device_acl_zone_base",
            toolRunId,
            deviceId,
            outgoingZoneId
        );
        if (!outgoingInterfaceName.empty()) {
          t.exec_prepared("insert_raw_device_acl_zone_interface",
              toolRunId,
              deviceId,
              outgoingZoneId,
              outgoingInterfaceName
          );
        }

        std::regex rex_allow{"accept|allow|pass|permit"};
        std::regex rex_block{"block|deny|drop|reject"};
        std::smatch m;
        if (std::regex_search(action, m, rex_allow)) {
          action = "allow";
        }
        else if (std::regex_search(action, m, rex_block)) {
          action = "block";
        }

        t.exec_prepared("insert_raw_device_acl_rule_service",
            toolRunId,
            deviceId,
            priority,
            action,
            incomingZoneId,
            outgoingZoneId,
            srcIpNetSetId,
            dstIpNetSetId,
            serviceId,
            description
        );
      }


      // Add "default allow" rules to any devices without ACLs.
      t.exec(
        "INSERT INTO raw_device_acl_rules_services ("
        " SELECT DISTINCT"
        "   devices.tool_run_id         AS tool_run_id,"
        "   devices.device_id           AS device_id,"
        "   9000000                     AS priority,"
        "   'allow'                     AS action,"
        "   'any'                       AS incoming_zone_id,"
        "   'any'                       AS outgoing_zone_id,"
        "   'any'                       AS src_ip_net_set_id,"
        "   'any'                       AS dst_ip_net_set_id,"
        "   'any'                       AS service_id,"
        "   'implicit default allow'    AS description"
        " FROM raw_devices AS devices"
        " LEFT OUTER JOIN device_acl_rules_services AS rules"
        " ON (devices.device_id = rules.device_id)"
        " WHERE (rules.device_id IS NULL)"
        " )"
        " ON CONFLICT DO NOTHING"
      );


      t.commit();

      return nmcu::Exit::SUCCESS;
    }

    nmdo::PortRange
    convertToPortRange(const std::string& _portData)
    {
      std::string portData{_portData};

      if ("bgp" == portData) {
        portData = "179";
      }
      else if ("bootpc" == portData) {
        portData = "68";
      }
      else if ("bootps" == portData) {
        portData = "67";
      }
      else if ("cmd" == portData) {
        portData = "514";
      }
      else if ("domain" == portData) {
        portData = "53";
      }
      else if ("echo" == portData) {
        portData = "7";
      }
      else if ("ftp-data" == portData) {
        portData = "20";
      }
      else if ("ftp" == portData) {
        portData = "21";
      }
      else if ("https" == portData) {
        portData = "443";
      }
      else if ("http" == portData) {
        portData = "80";
      }
      else if ("isakmp" == portData) {
        portData = "500";
      }
      else if ("kerberos" == portData) {
        portData = "88";
      }
      else if ("lpd" == portData) {
        portData = "515";
      }
      else if ("ntp" == portData) {
        portData = "123";
      }
      else if ("pop3" == portData) {
        portData = "110";
      }
      else if ("smtp" == portData) {
        portData = "25";
      }
      else if ("snmptrap" == portData) {
        portData = "162";
      }
      else if ("snmp" == portData) {
        portData = "161";
      }
      else if ("ssh" == portData) {
        portData = "22";
      }
      else if ("syslog" == portData) {
        portData = "514";
      }
      else if ("tacacs" == portData) {
        portData = "49";
      }
      else if ("telnet" == portData) {
        portData = "23";
      }

      std::regex r0{"^$"};
      std::regex r1{"^(\\d{1,5})$"};
      std::regex r2{"^(\\d{1,5})-(\\d{1,5})$"};
      std::regex r3{"^<(\\d{1,5})$"};
      std::regex r4{"^>(\\d{1,5})$"};
      std::smatch m;
      // Port number range (">N")
      if (std::regex_match(portData, m, r4)) {
        return nmdo::PortRange{
            static_cast<uint16_t>(std::stoul(m[1]) + 1),
            65535
        };
      }
      // Port number range ("<N")
      if (std::regex_match(portData, m, r3)) {
        return nmdo::PortRange{
            0,
            static_cast<uint16_t>(std::stoul(m[1]) - 1)
        };
      }
      // Port number range ("N-M")
      if (std::regex_match(portData, m, r2)) {
        return nmdo::PortRange{
            static_cast<uint16_t>(std::stoul(m[1])),
            static_cast<uint16_t>(std::stoul(m[2]))
        };
      }
      // Single port number
      if (std::regex_match(portData, m, r1)) {
        return nmdo::PortRange{
            static_cast<uint16_t>(std::stoul(m[1]))
        };
      }
      // Unspecified ports (implicit "any")
      if (std::regex_match(portData, m, r0)) {
        return nmdo::PortRange{0, 65535};
      }

      std::cerr << "Didn't match portRange: " << portData << std::endl;
      return nmdo::PortRange{0, 0};
    }

  protected:
    // Inherited from AbstractTool at this scope
      // std::string const getDbName() const;
      // virtual void printVersion() const;
    // Inherited from AbstractGraphTool at this scope
      // virtual void printHelp() const;
  public:
    // Inherited from AbstractTool, don't override as primary tool entry point
      // int start(int, char**) noexcept;
};


// =============================================================================
// Program entry point
// =============================================================================
int
main(int argc, char** argv)
{
  Tool tool;
  return tool.start(argc, argv);
}
