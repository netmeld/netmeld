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


#include <netmeld/common/queries_common.hpp>

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/format.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <string>


using boost::format;


void
db_prepare_common(pqxx::connection& db)
{
  // ----------------------------------------------------------------------
  // TABLE: tool_runs
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_tool_run",
     "INSERT INTO tool_runs"
     " (id, tool_name, command_line, data_path, execute_time)"
     " SELECT $1, $2, $3, $4, TSRANGE($5, $6, '[]')"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM tool_runs"
     "   WHERE (id = $1)"
     " )");

  // ----------------------------------------------------------------------
  // TABLE: tool_run_interfaces
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_tool_run_interface",
     "INSERT INTO tool_run_interfaces"
     " (tool_run_id, interface_name, media_type, is_up)"
     " SELECT $1, $2, $3, $4"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM tool_run_interfaces"
     "   WHERE (tool_run_id = $1) AND"
     "         (interface_name = $2)"
     " )");

  // ----------------------------------------------------------------------
  // TABLE: tool_run_mac_addrs
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_tool_run_mac_addr",
     "INSERT INTO tool_run_mac_addrs"
     " (tool_run_id, interface_name, mac_addr)"
     " SELECT $1, $2, $3"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM tool_run_mac_addrs"
     "   WHERE (tool_run_id = $1) AND"
     "         (interface_name = $2) AND"
     "         (mac_addr = $3)"
     " )");

  // ----------------------------------------------------------------------
  // TABLE: tool_run_ip_addrs
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_tool_run_ip_addr",
     "INSERT INTO tool_run_ip_addrs"
     " (tool_run_id, interface_name, ip_addr)"
     " SELECT $1, $2, ($3)::INET"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM tool_run_ip_addrs"
     "   WHERE (tool_run_id = $1) AND"
     "         (interface_name = $2) AND"
     "         (ip_addr = ($3)::INET)"
     " )");

  // ----------------------------------------------------------------------
  // TABLE: tool_run_ip_routes
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_tool_run_ip_route",
     "INSERT INTO tool_run_ip_routes"
     " (tool_run_id, interface_name, dst_ip_net, rtr_ip_addr)"
     " SELECT $1, $2, network(($3)::INET), host(($4)::INET)::INET"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM tool_run_ip_routes"
     "   WHERE (tool_run_id = $1) AND"
     "         (interface_name = $2) AND"
     "         (dst_ip_net  = network(($3)::INET)) AND"
     "         (rtr_ip_addr = host(($4)::INET)::INET)"
     " )");


  // ----------------------------------------------------------------------
  // TABLE: raw_devices
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_raw_device",
     "INSERT INTO raw_devices"
     " (tool_run_id, device_id)"
     " SELECT $1, lower($2)"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM raw_devices"
     "   WHERE (tool_run_id = $1) AND (lower(device_id) = lower($2))"
     " )");


  // ----------------------------------------------------------------------
  // TABLE: device_extra_weights
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_device_extra_weight",
     "INSERT INTO device_extra_weights"
     " (device_id, extra_weight)"
     " SELECT lower($1), $2"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM device_extra_weights"
     "   WHERE (lower(device_id) = lower($1))"
     " )");


  // ----------------------------------------------------------------------
  // TABLE: raw_device_virtualizations
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_raw_device_virtualization",
     "INSERT INTO raw_device_virtualizations"
     " (tool_run_id, host_device_id, guest_device_id)"
     " SELECT $1, lower($2), lower($3)"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM raw_device_virtualizations"
     "   WHERE (tool_run_id = $1) AND"
     "         (lower(host_device_id) = lower($2)) AND (lower(guest_device_id) = lower($3))"
     " )");


  // ----------------------------------------------------------------------
  // TABLE: device_colors
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_device_color",
     "INSERT INTO device_colors"
     " (device_id, color)"
     " SELECT lower($1), $2"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM device_colors"
     "   WHERE (lower(device_id) = lower($1))"
     " )");

  // ----------------------------------------------------------------------
  // TABLE: raw_device_types
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_raw_device_type",
     "INSERT INTO raw_device_types"
     " (tool_run_id, device_id, device_type)"
     " SELECT $1, lower($2), lower($3)"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM raw_device_types"
     "   WHERE (tool_run_id = $1) AND"
     "         (lower(device_id) = lower($2)) AND"
     "         (lower(device_type) = lower($3))"
     " )");

  // ----------------------------------------------------------------------
  // TABLE: raw_device_hardware
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_raw_device_hardware",
     "INSERT INTO raw_device_hardware"
     " (tool_run_id, device_id, vendor, model, hardware_revision,"
     "  serial_number, description)"
     " SELECT $1, lower($2), lower($3), upper($4), upper($5), upper($6), $7"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM raw_device_hardware"
     "   WHERE (tool_run_id = $1) AND"
     "         (lower(device_id) = lower($2)) AND"
     "         (lower(vendor) = lower($3)) AND"
     "         (upper(model) = upper($4)) AND"
     "         (upper(hardware_revision) = upper($5)) AND"
     "         (upper(serial_number) = upper($6)) AND"
     "         (lower(description) = lower($7))"
     " )");

  // ----------------------------------------------------------------------
  // TABLE: raw_device_interfaces
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_raw_device_interface",
     "INSERT INTO raw_device_interfaces"
     " (tool_run_id, device_id, interface_name, media_type, is_up)"
     " SELECT $1, lower($2), $3, $4, $5"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM raw_device_interfaces"
     "   WHERE (tool_run_id = $1) AND (lower(device_id) = lower($2)) AND"
     "         (interface_name = $3)"
     " )");

  // ----------------------------------------------------------------------
  // TABLE: raw_device_mac_addrs
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_raw_device_mac_addr",
     "INSERT INTO raw_device_mac_addrs"
     " (tool_run_id, device_id, interface_name, mac_addr)"
     " SELECT $1, lower($2), $3, $4"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM raw_device_mac_addrs"
     "   WHERE (tool_run_id = $1) AND (lower(device_id) = lower($2)) AND"
     "         (interface_name = $3) AND"
     "         (mac_addr = $4)"
     " )");

  // ----------------------------------------------------------------------
  // TABLE: raw_device_ip_addrs
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_raw_device_ip_addr",
     "INSERT INTO raw_device_ip_addrs"
     " (tool_run_id, device_id, interface_name, ip_addr)"
     " SELECT $1, lower($2), $3, host(($4)::INET)::INET"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM raw_device_ip_addrs"
     "   WHERE (tool_run_id = $1) AND (lower(device_id) = lower($2)) AND"
     "         (interface_name = $3) AND"
     "         (ip_addr = host(($4)::INET)::INET)"
     " )");

  // ----------------------------------------------------------------------
  // TABLE: raw_device_ip_routes
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_raw_device_ip_route",
     "INSERT INTO raw_device_ip_routes"
     " (tool_run_id, device_id, interface_name, dst_ip_net, rtr_ip_addr)"
     " SELECT $1, lower($2), $3, network(($4)::INET), host(($5)::INET)::INET"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM raw_device_ip_routes"
     "   WHERE (tool_run_id = $1) AND (lower(device_id) = lower($2)) AND"
     "         (dst_ip_net  = network(($4)::INET)) AND"
     "         (rtr_ip_addr = host(($5)::INET)::INET)"
     " )");

  // ----------------------------------------------------------------------
  // TABLE: raw_device_link_connections
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_raw_device_link_connections",
     "INSERT INTO raw_device_link_connections"
     " (tool_run_id, self_device_id, self_interface_name, peer_mac_addr)"
     " SELECT $1, lower($2), $3, $4"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM raw_device_link_connections"
     "   WHERE (tool_run_id = $1) AND (lower(self_device_id) = lower($2)) AND"
     "         (peer_mac_addr = $4)"
     " )");

  // ----------------------------------------------------------------------
  // TABLE: raw_mac_addrs
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_raw_mac_addr",
     "INSERT INTO raw_mac_addrs"
     " (tool_run_id, mac_addr, is_responding)"
     " SELECT $1, $2, $3"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM raw_mac_addrs"
     "   WHERE (tool_run_id = $1) AND"
     "         (mac_addr = $2)"
     " )");

  // ----------------------------------------------------------------------
  // TABLE: raw_ip_addrs
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_raw_ip_addr",
     "INSERT INTO raw_ip_addrs"
     " (tool_run_id, ip_addr, is_responding)"
     " SELECT $1, host(($2)::INET)::INET, $3"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM raw_ip_addrs"
     "   WHERE (tool_run_id = $1) AND"
     "         (ip_addr = host(($2)::INET)::INET)"
     " )");

  // ----------------------------------------------------------------------
  // TABLE: raw_mac_addrs_ip_addrs
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_raw_mac_addr_ip_addr",
     "INSERT INTO raw_mac_addrs_ip_addrs"
     " (tool_run_id, mac_addr, ip_addr)"
     " SELECT $1, $2, host(($3)::INET)::INET"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM raw_mac_addrs_ip_addrs"
     "   WHERE (tool_run_id = $1) AND"
     "         (mac_addr = $2) AND"
     "         (ip_addr = host(($3)::INET)::INET)"
     " )");

  // ----------------------------------------------------------------------
  // TABLE: raw_hostnames
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_raw_hostname",
     "INSERT INTO raw_hostnames"
     " (tool_run_id, ip_addr, hostname, reason)"
     " SELECT $1, host(($2)::INET)::INET, lower($3), $4"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM raw_hostnames"
     "   WHERE (tool_run_id = $1) AND"
     "         (ip_addr = host(($2)::INET)::INET) AND"
     "         (lower(hostname) = lower($3)) AND"
     "         (reason = $4)"
     " )");

  // ----------------------------------------------------------------------
  // TABLE: raw_operating_systems
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_raw_operating_system",
     "INSERT INTO raw_operating_systems"
     " (tool_run_id, ip_addr,"
     "  vendor_name, product_name, product_version, cpe, accuracy)"
     " SELECT $1, host(($2)::INET)::INET, $3, $4, $5, $6, $7"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM raw_operating_systems"
     "   WHERE (tool_run_id = $1) AND"
     "         (ip_addr = host(($2)::INET)::INET) AND"
     "         (vendor_name = $3) AND"
     "         (product_name = $4) AND (product_version = $5)"
     " )");

  // ----------------------------------------------------------------------
  // TABLE: raw_vlans
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_raw_vlan",
     "INSERT INTO raw_vlans"
     " (tool_run_id, vlan, description)"
     " SELECT $1, $2, $3"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM raw_vlans"
     "   WHERE (tool_run_id = $1) AND"
     "         (vlan = $2)"
     " )");

  db.prepare
    ("update_raw_vlan_description",
     "UPDATE raw_vlans"
     " SET description = $3"
     " WHERE (tool_run_id = $1) AND"
     "       (vlan = $2)");

  // ----------------------------------------------------------------------
  // TABLE: raw_ip_nets
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_raw_ip_net",
     "INSERT INTO raw_ip_nets"
     " (tool_run_id, ip_net, description)"
     " SELECT $1, network(($2)::INET), $3"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM raw_ip_nets"
     "   WHERE (tool_run_id = $1) AND"
     "         (ip_net = network(($2)::INET))"
     " )");

  db.prepare
    ("update_raw_ip_net_description",
     "UPDATE raw_ip_nets"
     " SET description = $3"
     " WHERE (tool_run_id = $1) AND"
     "       (ip_net = network(($2)::INET))");

  // ----------------------------------------------------------------------
  // TABLE: ip_nets_extra_weights
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_ip_net_extra_weight",
     "INSERT INTO ip_nets_extra_weights"
     " (ip_net, extra_weight)"
     " SELECT network(($1)::INET), $2"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM ip_nets_extra_weights"
     "   WHERE (ip_net = network(($1)::INET))"
     " )");

  // ----------------------------------------------------------------------
  // TABLE: raw_vlans_ip_nets
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_raw_vlan_ip_net",
     "INSERT INTO raw_vlans_ip_nets"
     " (tool_run_id, vlan, ip_net)"
     " SELECT $1, $2, network(($3)::INET)"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM raw_vlans_ip_nets"
     "   WHERE (tool_run_id = $1) AND"
     "         (vlan = $2) AND"
     "         (ip_net = network(($3)::INET))"
     " )");


  // ----------------------------------------------------------------------
  // TABLE: raw_ip_traceroutes
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_raw_ip_traceroute",
     "INSERT INTO raw_ip_traceroutes"
     " (tool_run_id, hop_count, rtr_ip_addr, dst_ip_addr)"
     " SELECT $1, $2, host(($3)::INET)::INET, host(($4)::INET)::INET"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM raw_ip_traceroutes"
     "   WHERE (tool_run_id = $1) AND"
     "         (hop_count = $2) AND"
     "         (rtr_ip_addr = host(($3)::INET)::INET) AND"
     "         (dst_ip_addr = host(($4)::INET)::INET)"
     " )");


  // ----------------------------------------------------------------------
  // TABLE: raw_ports
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_raw_port",
     "INSERT INTO raw_ports"
     " (tool_run_id, ip_addr, protocol, port, port_state, port_reason)"
     " SELECT $1, host(($2)::INET)::INET, $3, $4, $5, $6"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM raw_ports"
     "   WHERE (tool_run_id = $1) AND"
     "         (ip_addr = host(($2)::INET)::INET) AND"
     "         (protocol = $3) AND (port = $4)"
     " )");

  // ----------------------------------------------------------------------
  // TABLE: raw_services
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_raw_service",
     "INSERT INTO raw_services"
     " (tool_run_id, ip_addr, protocol, port,"
     "  service_name, service_description, service_reason)"
     " SELECT $1, host(($2)::INET)::INET, $3, $4, $5, $6, $7"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM raw_services"
     "   WHERE (tool_run_id = $1) AND"
     "         (ip_addr = host(($2)::INET)::INET) AND"
     "         (protocol = $3) AND (port = $4)"
     " )");

  // ----------------------------------------------------------------------
  // TABLE: raw_nessus_results
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_raw_nessus_result",
     "INSERT INTO raw_nessus_results"
     " (tool_run_id, ip_addr, protocol, port,"
     "  plugin_id, plugin_name, plugin_family, plugin_type, plugin_output,"
     "  severity, description, solution)"
     " SELECT $1, host(($2)::INET)::INET, $3, $4,"
     "        $5, $6, $7, $8, $9, $10, $11, $12"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM raw_nessus_results"
     "   WHERE (tool_run_id = $1) AND"
     "         (ip_addr = host(($2)::INET)::INET) AND"
     "         (protocol = $3) AND (port = $4) AND"
     "         (plugin_id = $5)"
     " )");

  // ----------------------------------------------------------------------
  // TABLE: raw_nessus_results_cves
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_raw_nessus_result_cve",
     "INSERT INTO raw_nessus_results_cves"
     " (tool_run_id, ip_addr, protocol, port, plugin_id, cve_id)"
     " SELECT $1, host(($2)::INET)::INET, $3, $4, $5, ($6)::CVE"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM raw_nessus_results_cves"
     "   WHERE (tool_run_id = $1) AND"
     "         (ip_addr = host(($2)::INET)::INET) AND"
     "         (protocol = $3) AND (port = $4) AND"
     "         (plugin_id = $5) AND (cve_id = ($6)::CVE)"
     " )");

  // ----------------------------------------------------------------------
  // TABLE: raw_nessus_results_metasploit_modules
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_raw_nessus_result_metasploit_module",
     "INSERT INTO raw_nessus_results_metasploit_modules"
     " (tool_run_id, ip_addr, protocol, port, plugin_id, metasploit_name)"
     " SELECT $1, host(($2)::INET)::INET, $3, $4, $5, $6"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM raw_nessus_results_metasploit_modules"
     "   WHERE (tool_run_id = $1) AND"
     "         (ip_addr = host(($2)::INET)::INET) AND"
     "         (protocol = $3) AND (port = $4) AND"
     "         (plugin_id = $5) AND (metasploit_name = $6)"
     " )");

  // ----------------------------------------------------------------------
  // TABLE: raw_nse_results
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_raw_nse_result",
     "INSERT INTO raw_nse_results"
     " (tool_run_id, ip_addr, protocol, port, script_id, script_output)"
     " SELECT $1, host(($2)::INET)::INET, $3, $4, $5, $6"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM raw_nse_results"
     "   WHERE (tool_run_id = $1) AND"
     "         (ip_addr = host(($2)::INET)::INET) AND"
     "         (protocol = $3) AND (port = $4) AND"
     "         (script_id = $5)"
     " )");

  // ----------------------------------------------------------------------
  // TABLE: raw_ssh_host_public_keys
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_raw_ssh_host_public_key",
     "INSERT INTO raw_ssh_host_public_keys"
     " (tool_run_id, ip_addr, protocol, port,"
     "  ssh_key_type, ssh_key_bits, ssh_key_fingerprint, ssh_key_public)"
     " SELECT $1, host(($2)::INET)::INET, $3, $4, $5, $6, $7, $8"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM raw_ssh_host_public_keys"
     "   WHERE (tool_run_id = $1) AND"
     "         (ip_addr = host(($2)::INET)::INET) AND"
     "         (protocol = $3) AND (port = $4) AND"
     "         (ssh_key_type = $5) AND (ssh_key_bits = $6) AND"
     "         (ssh_key_fingerprint = $7)"
     " )");

  // ----------------------------------------------------------------------
  // TABLE: raw_ssh_host_algorithms
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_raw_ssh_host_algorithm",
     "INSERT INTO raw_ssh_host_algorithms"
     " (tool_run_id, ip_addr, protocol, port,"
     "  ssh_algo_type, ssh_algo_name)"
     " SELECT $1, host(($2)::INET)::INET, $3, $4, $5, $6"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM raw_ssh_host_algorithms"
     "   WHERE (tool_run_id = $1) AND"
     "         (ip_addr = host(($2)::INET)::INET) AND"
     "         (protocol = $3) AND (port = $4) AND"
     "         (ssh_algo_type = $5) AND (ssh_algo_name = $6)"
     " )");

  // ----------------------------------------------------------------------
  // TABLE: raw_device_acl_action_sets
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_raw_device_acl_action_set",
     "INSERT INTO raw_device_acl_action_sets"
     " (tool_run_id, device_id, action_set)"
     " SELECT $1, lower($2), $3"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM raw_device_acl_action_sets"
     "   WHERE (tool_run_id = $1) AND"
     "         (lower(device_id) = lower($2)) AND"
     "         (action_set = $3)"
     " )");

  // ----------------------------------------------------------------------
  // TABLE: raw_device_acl_actions
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_raw_device_acl_action",
     "INSERT INTO raw_device_acl_actions"
     " (tool_run_id, device_id, action_set, action)"
     " SELECT $1, lower($2), $3, $4"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM raw_device_acl_actions"
     "   WHERE (tool_run_id = $1) AND"
     "         (lower(device_id) = lower($2)) AND"
     "         (action_set = $3) AND"
     "         (action = $4)"
     " )");

  // ----------------------------------------------------------------------
  // TABLE: raw_device_acl_ip_net_sets
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_raw_device_acl_ip_net_set",
     "INSERT INTO raw_device_acl_ip_net_sets"
     " (tool_run_id, device_id, ip_net_set)"
     " SELECT $1, lower($2), $3"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM raw_device_acl_ip_net_sets"
     "   WHERE (tool_run_id = $1) AND"
     "         (lower(device_id) = lower($2)) AND"
     "         (ip_net_set = $3)"
     " )");

  // ----------------------------------------------------------------------
  // TABLE: raw_device_acl_ip_nets
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_raw_device_acl_ip_net",
     "INSERT INTO raw_device_acl_ip_nets"
     " (tool_run_id, device_id, ip_net_set, ip_net)"
     " SELECT $1, lower($2), $3, $4"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM raw_device_acl_ip_nets"
     "   WHERE (tool_run_id = $1) AND"
     "         (lower(device_id) = lower($2)) AND"
     "         (ip_net_set = $3) AND"
     "         (ip_net = ($4)::CIDR)"
     " )");

  // ----------------------------------------------------------------------
  // TABLE: raw_device_acl_port_range_sets
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_raw_device_acl_port_range_set",
     "INSERT INTO raw_device_acl_port_range_sets"
     " (tool_run_id, device_id, port_range_set)"
     " SELECT $1, lower($2), $3"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM raw_device_acl_port_range_sets"
     "   WHERE (tool_run_id = $1) AND"
     "         (lower(device_id) = lower($2)) AND"
     "         (port_range_set = $3)"
     " )");

  // ----------------------------------------------------------------------
  // TABLE: raw_device_acl_port_ranges
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_raw_device_acl_port_range",
     "INSERT INTO raw_device_acl_port_ranges"
     " (tool_run_id, device_id, port_range_set, protocol, port_range)"
     " SELECT $1, lower($2), $3, $4, $5"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM raw_device_acl_port_ranges"
     "   WHERE (tool_run_id = $1) AND"
     "         (lower(device_id) = lower($2)) AND"
     "         (port_range_set = $3) AND"
     "         (protocol = $4) AND"
     "         (port_range = $5)"
     " )");

  // ----------------------------------------------------------------------
  // TABLE: raw_device_acls
  // ----------------------------------------------------------------------

  db.prepare
    ("insert_raw_device_acl",
     "INSERT INTO raw_device_acls"
     " (tool_run_id, device_id, acl_set, acl_number, action_set,"
     "  src_ip_net_set, dst_ip_net_set,"
     "  src_port_range_set, dst_port_range_set)"
     " SELECT $1, lower($2), $3, $4, $5, $6, $7, $8, $9"
     " WHERE NOT EXISTS ("
     "   SELECT 1 FROM raw_device_acls"
     "   WHERE (tool_run_id = $1) AND"
     "         (lower(device_id) = lower($2)) AND"
     "         (acl_set = $3) AND"
     "         (acl_number = $4) AND"
     "         (action_set = $5) AND"
     "         (src_ip_net_set = $6) AND"
     "         (dst_ip_net_set = $7) AND"
     "         (src_port_range_set = $8) AND"
     "         (dst_port_range_set = $9)"
     " )");


  // ----------------------------------------------------------------------
  // Ensure that certain table entries are present (create them if absent)
  // ----------------------------------------------------------------------

  if (true) {
    pqxx::transaction<> t{db};

    t.prepared("insert_tool_run")
      ("32b2fd62-08ff-4d44-8da7-6fbd581a90c6")  // id
      ("human")   // tool_name
      ("human")   // command_line
      ("human")   // data_path
      ()          // execute_time_lower (NULL)
      ()          // execute_time_upper (NULL)
      .exec();

    t.commit();
  }

  // ----------------------------------------------------------------------
}


namespace pqxx {


// ----------------------------------------------------------------------
// CVE <--> Postgresql CVE
// ----------------------------------------------------------------------

const char*
string_traits<CVE>::
name()
{
  return "CVE";
}


bool
string_traits<CVE>::
has_null()
{
  return false;
}


bool
string_traits<CVE>::
is_null(CVE const&)
{
  return false;
}


CVE
string_traits<CVE>::
null()
{
  internal::throw_null_conversion(name());
  return CVE(0, 0);
}


void
string_traits<CVE>::
from_string(const char str[], CVE& obj)
{
  obj = CVE(str);
}


PGSTD::string
string_traits<CVE>::
to_string(CVE const& obj)
{
  return (format("(%|u|, %|u|)") % obj.year() % obj.number()).str();
}


// ----------------------------------------------------------------------
// boost::posix_time::ptime <--> Postgresql TIMESTAMP
// ----------------------------------------------------------------------

const char*
string_traits<boost::posix_time::ptime>::
name()
{
  return "boost::posix_time::ptime";
}


bool
string_traits<boost::posix_time::ptime>::
has_null()
{
  return true;
}


bool
string_traits<boost::posix_time::ptime>::
is_null(boost::posix_time::ptime const& obj)
{
  return obj.is_not_a_date_time();
}


boost::posix_time::ptime
string_traits<boost::posix_time::ptime>::
null()
{
  return boost::posix_time::ptime(boost::posix_time::not_a_date_time);
}


void
string_traits<boost::posix_time::ptime>::
from_string(const char str[], boost::posix_time::ptime& obj)
{
  if (std::string("-infinity") == str) {
    obj = boost::posix_time::ptime(boost::posix_time::neg_infin);
    return;
  }

  if (std::string("infinity") == str) {
    obj = boost::posix_time::ptime(boost::posix_time::pos_infin);
    return;
  }

  obj = boost::posix_time::time_from_string(str);
}


PGSTD::string
string_traits<boost::posix_time::ptime>::
to_string(boost::posix_time::ptime const& obj)
{
  if (obj.is_neg_infinity()) {
    return "-infinity";
  }

  if (obj.is_pos_infinity()) {
    return "infinity";
  }

  return boost::posix_time::to_iso_extended_string(obj);
}


// ----------------------------------------------------------------------
// boost::uuids::uuid <--> Postgresql UUID
// ----------------------------------------------------------------------

const char*
string_traits<boost::uuids::uuid>::
name()
{
  return "boost::uuids::uuid";
}


bool
string_traits<boost::uuids::uuid>::
has_null()
{
  return true;
}


bool
string_traits<boost::uuids::uuid>::
is_null(boost::uuids::uuid const& obj)
{
  return obj.is_nil();
}


boost::uuids::uuid
string_traits<boost::uuids::uuid>::
null()
{
  return boost::uuids::nil_uuid();
}


void
string_traits<boost::uuids::uuid>::
from_string(const char str[], boost::uuids::uuid& obj)
{
  obj = boost::uuids::string_generator()(str);
}


PGSTD::string
string_traits<boost::uuids::uuid>::
to_string(boost::uuids::uuid const& obj)
{
  return boost::uuids::to_string(obj);
}


// ----------------------------------------------------------------------
// IPv4_Addr <--> Postgresql INET
// ----------------------------------------------------------------------

const char*
string_traits<IPv4_Addr>::
name()
{
  return "IPv4_Addr";
}


bool
string_traits<IPv4_Addr>::
has_null()
{
  return false;
}


bool
string_traits<IPv4_Addr>::
is_null(IPv4_Addr const&)
{
  return false;
}


IPv4_Addr
string_traits<IPv4_Addr>::
null()
{
  internal::throw_null_conversion(name());
  return IPv4_Addr();
}


void
string_traits<IPv4_Addr>::
from_string(const char str[], IPv4_Addr& obj)
{
  obj = IPv4_Addr::from_string(str);
}


PGSTD::string
string_traits<IPv4_Addr>::
to_string(IPv4_Addr const& obj)
{
  return obj.to_string();
}


// ----------------------------------------------------------------------
// IPv6_Addr <--> Postgresql INET
// ----------------------------------------------------------------------

const char*
string_traits<IPv6_Addr>::
name()
{
  return "IPv6_Addr";
}


bool
string_traits<IPv6_Addr>::
has_null()
{
  return false;
}


bool
string_traits<IPv6_Addr>::
is_null(IPv6_Addr const&)
{
  return false;
}


IPv6_Addr
string_traits<IPv6_Addr>::
null()
{
  internal::throw_null_conversion(name());
  return IPv6_Addr();
}


void
string_traits<IPv6_Addr>::
from_string(const char str[], IPv6_Addr& obj)
{
  obj = IPv6_Addr::from_string(str);
}


PGSTD::string
string_traits<IPv6_Addr>::
to_string(IPv6_Addr const& obj)
{
  return obj.to_string();
}


// ----------------------------------------------------------------------
// IP_Addr <--> Postgresql INET
// ----------------------------------------------------------------------

const char*
string_traits<IP_Addr>::
name()
{
  return "IP_Addr";
}


bool
string_traits<IP_Addr>::
has_null()
{
  return false;
}


bool
string_traits<IP_Addr>::
is_null(IP_Addr const&)
{
  return false;
}


IP_Addr
string_traits<IP_Addr>::
null()
{
  internal::throw_null_conversion(name());
  return IP_Addr();
}


void
string_traits<IP_Addr>::
from_string(const char str[], IP_Addr& obj)
{
  obj = IP_Addr::from_string(str);
}


PGSTD::string
string_traits<IP_Addr>::
to_string(IP_Addr const& obj)
{
  return obj.to_string();
}


// ----------------------------------------------------------------------
// IPv4_Addr_with_Prefix <--> Postgresql INET
// ----------------------------------------------------------------------

const char*
string_traits<IPv4_Addr_with_Prefix>::
name()
{
  return "IPv4_Addr_with_Prefix";
}


bool
string_traits<IPv4_Addr_with_Prefix>::
has_null()
{
  return false;
}


bool
string_traits<IPv4_Addr_with_Prefix>::
is_null(IPv4_Addr_with_Prefix const&)
{
  return false;
}


IPv4_Addr_with_Prefix
string_traits<IPv4_Addr_with_Prefix>::
null()
{
  internal::throw_null_conversion(name());
  return IPv4_Addr_with_Prefix();
}


void
string_traits<IPv4_Addr_with_Prefix>::
from_string(const char str[], IPv4_Addr_with_Prefix& obj)
{
  obj = IPv4_Addr_with_Prefix::from_string(str);
}


PGSTD::string
string_traits<IPv4_Addr_with_Prefix>::
to_string(IPv4_Addr_with_Prefix const& obj)
{
  return obj.to_string();
}


// ----------------------------------------------------------------------
// IPv6_Addr_with_Prefix <--> Postgresql INET
// ----------------------------------------------------------------------

const char*
string_traits<IPv6_Addr_with_Prefix>::
name()
{
  return "IPv6_Addr_with_Prefix";
}


bool
string_traits<IPv6_Addr_with_Prefix>::
has_null()
{
  return false;
}


bool
string_traits<IPv6_Addr_with_Prefix>::
is_null(IPv6_Addr_with_Prefix const&)
{
  return false;
}


IPv6_Addr_with_Prefix
string_traits<IPv6_Addr_with_Prefix>::
null()
{
  internal::throw_null_conversion(name());
  return IPv6_Addr_with_Prefix();
}


void
string_traits<IPv6_Addr_with_Prefix>::
from_string(const char str[], IPv6_Addr_with_Prefix& obj)
{
  obj = IPv6_Addr_with_Prefix::from_string(str);
}


PGSTD::string
string_traits<IPv6_Addr_with_Prefix>::
to_string(IPv6_Addr_with_Prefix const& obj)
{
  return obj.to_string();
}


// ----------------------------------------------------------------------
// IP_Addr_with_Prefix <--> Postgresql INET
// ----------------------------------------------------------------------

const char*
string_traits<IP_Addr_with_Prefix>::
name()
{
  return "IP_Addr_with_Prefix";
}


bool
string_traits<IP_Addr_with_Prefix>::
has_null()
{
  return false;
}


bool
string_traits<IP_Addr_with_Prefix>::
is_null(IP_Addr_with_Prefix const&)
{
  return false;
}


IP_Addr_with_Prefix
string_traits<IP_Addr_with_Prefix>::
null()
{
  internal::throw_null_conversion(name());
  return IP_Addr_with_Prefix();
}


void
string_traits<IP_Addr_with_Prefix>::
from_string(const char str[], IP_Addr_with_Prefix& obj)
{
  obj = IP_Addr_with_Prefix::from_string(str);
}


PGSTD::string
string_traits<IP_Addr_with_Prefix>::
to_string(IP_Addr_with_Prefix const& obj)
{
  return obj.to_string();
}


}  // namespace pqxx
