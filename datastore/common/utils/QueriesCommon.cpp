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

#include <netmeld/datastore/utils/QueriesCommon.hpp>


namespace netmeld::datastore::utils {

  void
  dbPrepareCommon(pqxx::connection& db)
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
       "   WHERE ($1 = id)"
       " )");

    db.prepare
      ("update_tool_run",
       "UPDATE tool_runs"
       " SET execute_time = TSRANGE($2, $3, '[]')"
       " WHERE ($1 = id)");

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
       "   WHERE ($1 = tool_run_id)"
       "     AND ($2 = interface_name)"
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
       "   WHERE ($1 = tool_run_id)"
       "     AND ($2 = interface_name)"
       "     AND ($3 = mac_addr)"
       " )");

    // ----------------------------------------------------------------------
    // TABLE: tool_run_ip_addrs
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_tool_run_ip_addr",
       "INSERT INTO tool_run_ip_addrs"
       " (tool_run_id, interface_name, ip_addr)"
       " SELECT $1, $2, $3"
       " WHERE NOT EXISTS ("
       "   SELECT 1 FROM tool_run_ip_addrs"
       "   WHERE ($1 = tool_run_id)"
       "     AND ($2 = interface_name)"
       "     AND ($3 = ip_addr)"
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
       "   WHERE ($1 = tool_run_id)"
       "     AND ($2 = interface_name)"
       "     AND (network(($3)::INET) = dst_ip_net)"
       "     AND (host(($4)::INET)::INET = rtr_ip_addr)"
       " )");


    // ----------------------------------------------------------------------
    // TABLE: raw_devices
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_device",
       "INSERT INTO raw_devices"
       " (tool_run_id, device_id)"
       " SELECT $1, $2"
       " WHERE NOT EXISTS ("
       "   SELECT 1 FROM raw_devices"
       "   WHERE ($1 = tool_run_id)"
       "     AND ($2 = device_id)"
       " )");


    // ----------------------------------------------------------------------
    // TABLE: raw_devices_aaa
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_device_aaa",
       "INSERT INTO raw_devices_aaa"
       " (tool_run_id, device_id, aaa_command)"
       " SELECT $1, $2, $3"
       " WHERE NOT EXISTS ("
       "   SELECT 1 FROM raw_devices_aaa"
       "   WHERE ($1 = tool_run_id)"
       "     AND ($2 = device_id)"
       "     AND ($3 = aaa_command)"
       " )");

    // ----------------------------------------------------------------------
    // TABLE: device_extra_weights
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_device_extra_weight",
       "INSERT INTO device_extra_weights"
       " (device_id, extra_weight)"
       " SELECT $1, $2"
       " WHERE NOT EXISTS ("
       "   SELECT 1 FROM device_extra_weights"
       "   WHERE ($1 = device_id)"
       " )");


    // ----------------------------------------------------------------------
    // TABLE: raw_device_virtualizations
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_device_virtualization",
       "INSERT INTO raw_device_virtualizations"
       " (tool_run_id, host_device_id, guest_device_id)"
       " SELECT $1, $2, $3"
       " WHERE NOT EXISTS ("
       "   SELECT 1 FROM raw_device_virtualizations"
       "   WHERE ($1 = tool_run_id)"
       "     AND ($2 = host_device_id)"
       "     AND ($3 = guest_device_id)"
       " )");


    // ----------------------------------------------------------------------
    // TABLE: device_colors
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_device_color",
       "INSERT INTO device_colors"
       " (device_id, color)"
       " SELECT $1, $2"
       " WHERE NOT EXISTS ("
       "   SELECT 1 FROM device_colors"
       "   WHERE ($1 = device_id)"
       " )");


    // ----------------------------------------------------------------------
    // TABLE: raw_device_hardware_information
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_device_hardware_information",
       "INSERT INTO raw_device_hardware_information"
       " (tool_run_id, device_id, device_type, vendor,"
       "  model, hardware_revision, serial_number, description)"
       " SELECT $1, $2, nullif($3, ''), nullif($4, ''),"
       "        nullif($5, ''), nullif($6, ''),"
       "        nullif($7, ''), nullif($8, '')"
       " WHERE NOT EXISTS ("
       "   SELECT 1 FROM raw_device_hardware_information"
       "   WHERE ($1 = tool_run_id)"
       "     AND ($2 = device_id)"
       "     AND (nullif($3, '') = device_type)"
       "     AND (nullif($4, '') = vendor)"
       "     AND (nullif($5, '') = model)"
       "     AND (nullif($6, '') = hardware_revision)"
       "     AND (nullif($7, '') = serial_number)"
       "     AND (nullif($8, '') = description)"
       " )");

    // ----------------------------------------------------------------------
    // TABLE: raw_device_interfaces
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_device_interface",
       "INSERT INTO raw_device_interfaces"
       " (tool_run_id, device_id, interface_name, media_type, is_up)"
       " SELECT $1, $2, $3, $4, $5"
       " WHERE NOT EXISTS ("
       "   SELECT 1 FROM raw_device_interfaces"
       "   WHERE ($1 = tool_run_id)"
       "     AND ($2 = device_id)"
       "     AND ($3 = interface_name)"
       " )");

    // ----------------------------------------------------------------------
    // TABLE: raw_device_mac_addrs
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_device_mac_addr",
       "INSERT INTO raw_device_mac_addrs"
       " (tool_run_id, device_id, interface_name, mac_addr)"
       " SELECT $1, $2, $3, $4"
       " WHERE NOT EXISTS ("
       "   SELECT 1 FROM raw_device_mac_addrs"
       "   WHERE ($1 = tool_run_id)"
       "     AND ($2 = device_id)"
       "     AND ($3 = interface_name)"
       "     AND ($4 = mac_addr)"
       " )");

    // ----------------------------------------------------------------------
    // TABLE: raw_device_ip_addrs
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_device_ip_addr",
       "INSERT INTO raw_device_ip_addrs"
       " (tool_run_id, device_id, interface_name, ip_addr)"
       " SELECT $1, $2, $3, host(($4)::INET)::INET"
       " WHERE NOT EXISTS ("
       "   SELECT 1 FROM raw_device_ip_addrs"
       "   WHERE ($1 = tool_run_id)"
       "     AND ($2 = device_id)"
       "     AND ($3 = interface_name)"
       "     AND (host(($4)::INET)::INET = ip_addr)"
       " )");

    // ----------------------------------------------------------------------
    // TABLE: raw_device_ip_routes
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_device_ip_route",
       "INSERT INTO raw_device_ip_routes"
       " (tool_run_id, device_id, interface_name, dst_ip_net, rtr_ip_addr)"
       " SELECT $1, $2, nullif($3, ''),"
       "        network(($4)::INET), host((nullif($5, '0.0.0.0/0'))::INET)::INET"
       " WHERE NOT EXISTS ("
       "   SELECT 1 FROM raw_device_ip_routes"
       "   WHERE ($1 = tool_run_id)"
       "     AND ($2 = device_id)"
       "     AND (nullif($3, '') = interface_name)"
       "     AND (network(($4)::INET) = dst_ip_net)"
       "     AND (host((nullif($5, '0.0.0.0/0'))::INET)::INET = rtr_ip_addr)"
       " )"
       );

    // ----------------------------------------------------------------------
    // TABLE: raw_device_link_connections
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_device_link_connection",
       "INSERT INTO raw_device_link_connections"
       " (tool_run_id, self_device_id, self_interface_name, peer_mac_addr)"
       " SELECT $1, $2, $3, $4"
       " WHERE NOT EXISTS ("
       "   SELECT 1 FROM raw_device_link_connections"
       "   WHERE ($1 = tool_run_id)"
       "     AND ($2 = self_device_id)"
       "     AND ($4 = peer_mac_addr)"
       " )");

    // ----------------------------------------------------------------------
    // TABLE: raw_device_ip_servers
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_device_ip_server",
       "INSERT INTO raw_device_ip_servers"
       " (tool_run_id, device_id, interface_name, service_name,"
       "  server_ip_addr, port, local_service, description)"
       " SELECT $1, $2, $3, $4, host(($5)::INET)::INET,"
       "        $6, $7, nullif($8, '')"
       " WHERE NOT EXISTS ("
       "   SELECT 1 FROM raw_device_ip_servers"
       "   WHERE ($1 = tool_run_id)"
       "     AND ($2 = device_id)"
       "     AND ($3 = interface_name)"
       "     AND ($4 = service_name)"
       "     AND (host(($5)::INET)::INET = server_ip_addr)"
       //"     AND ($6 = port)"
       //"     AND ($7 = local_service)"
       //"     AND (nullif($8, '') = description)"
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
       "   WHERE ($1 = tool_run_id)"
       "     AND ($2 = mac_addr)"
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
       "   WHERE ($1 = tool_run_id)"
       "     AND (host(($2)::INET)::INET = ip_addr)"
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
       "   WHERE ($1 = tool_run_id)"
       "     AND ($2 = mac_addr)"
       "     AND (host(($3)::INET)::INET = ip_addr)"
       " )");

    // ----------------------------------------------------------------------
    // TABLE: raw_hostnames
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_hostname",
       "INSERT INTO raw_hostnames"
       " (tool_run_id, ip_addr, hostname, reason)"
       " SELECT $1, host(($2)::INET)::INET, $3, $4"
       " WHERE NOT EXISTS ("
       "   SELECT 1 FROM raw_hostnames"
       "   WHERE ($1 = tool_run_id)"
       "     AND (host(($2)::INET)::INET = ip_addr)"
       "     AND ($3 = hostname)"
       "     AND ($4 = reason)"
       " )");

    // ----------------------------------------------------------------------
    // TABLE: raw_operating_systems
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_operating_system",
       "INSERT INTO raw_operating_systems"
       " (tool_run_id, ip_addr,"
       "  vendor_name, product_name, product_version, cpe, accuracy)"
       " SELECT $1, host(($2)::INET)::INET,"
       "        nullif($3, ''), nullif($4, ''), nullif($5, ''),"
       "        nullif($6, ''), $7"
       " WHERE NOT EXISTS ("
       "   SELECT 1 FROM raw_operating_systems"
       "   WHERE ($1 = tool_run_id)"
       "     AND (host(($2)::INET)::INET = ip_addr)"
       "     AND (nullif($3, '') = vendor_name)"
       "     AND (nullif($4, '') = product_name)"
       "     AND (nullif($5, '') = product_version)"
       " )");

    // ----------------------------------------------------------------------
    // VLAN Related
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_vlan",
       "INSERT INTO raw_vlans"
       " (tool_run_id, vlan, description)"
       " SELECT $1, $2, nullif($3, '')"
       " WHERE NOT EXISTS ("
       "   SELECT 1 FROM raw_vlans"
       "   WHERE ($1 = tool_run_id)"
       "     AND ($2 = vlan)"
       " )");

    db.prepare
      ("insert_raw_device_vlan",
       "INSERT INTO raw_device_vlans"
       " (tool_run_id, device_id, vlan, description)"
       " SELECT $1, $2, $3, nullif($4, '')"
       " WHERE NOT EXISTS ("
       "   SELECT 1 FROM raw_device_vlans"
       "   WHERE ($1 = tool_run_id)"
       "     AND ($2 = device_id)"
       "     AND ($3 = vlan)"
       " )");

    db.prepare
      ("insert_raw_vlan_ip_net",
       "INSERT INTO raw_vlans_ip_nets"
       " (tool_run_id, vlan, ip_net)"
       " SELECT $1, $2, network(($3)::INET)"
       " WHERE NOT EXISTS ("
       "   SELECT 1 FROM raw_vlans_ip_nets"
       "   WHERE ($1 = tool_run_id)"
       "     AND ($2 = vlan)"
       "     AND (network(($3)::INET) = ip_net)"
       " )");

    db.prepare
      ("insert_raw_device_vlan_ip_net",
       "INSERT INTO raw_device_vlans_ip_nets"
       " (tool_run_id, device_id, vlan, ip_net)"
       " SELECT $1, $2, $3, network(($4)::INET)"
       " WHERE NOT EXISTS ("
       "   SELECT 1 FROM raw_device_vlans_ip_nets"
       "   WHERE ($1 = tool_run_id)"
       "     AND ($2 = device_id)"
       "     AND ($3 = vlan)"
       "     AND (network(($4)::INET) = ip_net)"
       " )");

    // ----------------------------------------------------------------------
    // TABLE: raw_ip_nets
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_ip_net",
       "INSERT INTO raw_ip_nets"
       " (tool_run_id, ip_net, description)"
       " SELECT $1, network(($2)::INET), nullif($3, '')"
       " WHERE NOT EXISTS ("
       "   SELECT 1 FROM raw_ip_nets"
       "   WHERE ($1 = tool_run_id)"
       "     AND (network(($2)::INET) = ip_net)"
       " )");

    db.prepare
      ("update_raw_ip_net_description",
       "UPDATE raw_ip_nets"
       " SET description = nullif($3, '')"
       " WHERE ($1 = tool_run_id)"
       "       (network(($2)::INET) = ip_net)");

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
       "   WHERE (network(($1)::INET) = ip_net)"
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
       "   WHERE ($1 = tool_run_id)"
       "     AND ($2 = hop_count)"
       "     AND (host(($3)::INET)::INET = rtr_ip_addr)"
       "     AND (host(($4)::INET)::INET = dst_ip_addr)"
       " )");


    // ----------------------------------------------------------------------
    // TABLE: raw_ports
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_port",
       "INSERT INTO raw_ports"
       " (tool_run_id, ip_addr, protocol, port, port_state, port_reason)"
       " SELECT $1, host(($2)::INET)::INET, $3, $4,"
       "        nullif($5, ''), nullif($6, '')"
       " WHERE NOT EXISTS ("
       "   SELECT 1 FROM raw_ports"
       "   WHERE ($1 = tool_run_id)"
       "     AND (host(($2)::INET)::INET = ip_addr)"
       "     AND ($3 = protocol)"
       "     AND ($4 = port)"
       " )");

    // ----------------------------------------------------------------------
    // TABLE: raw_network_services
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_network_service",
       "INSERT INTO raw_network_services"
       " (tool_run_id, ip_addr, protocol, port,"
       "  service_name, service_description, service_reason, observer_ip_addr)"
       " SELECT $1, host(($2)::INET)::INET, $3, $4,"
       "        nullif($5, ''), nullif($6, ''), nullif($7, ''),"
       "        (nullif($8, '0.0.0.0/0'))::INET"
       " WHERE NOT EXISTS ("
       "   SELECT 1 FROM raw_network_services"
       "   WHERE ($1 = tool_run_id)"
       "     AND (host(($2)::INET)::INET = ip_addr)"
       "     AND ($3 = protocol)"
       "     AND ($4 = port)"
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
       " SELECT $1, host(($2)::INET)::INET, $3, $4, $5,"
       "        nullif($6, ''), nullif($7, ''), nullif($8, ''), nullif($9, ''),"
       "        $10, nullif($11, ''), nullif($12, '')"
       " WHERE NOT EXISTS ("
       "   SELECT 1 FROM raw_nessus_results"
       "   WHERE ($1 = tool_run_id)"
       "     AND (host(($2)::INET)::INET = ip_addr)"
       "     AND ($3 = protocol)"
       "     AND ($4 = port)"
       "     AND ($5 = plugin_id)"
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
       "   WHERE ($1 = tool_run_id)"
       "     AND (host(($2)::INET)::INET = ip_addr)"
       "     AND ($3 = protocol)"
       "     AND ($4 = port)"
       "     AND ($5 = plugin_id)"
       "     AND (($6)::CVE = cve_id)"
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
       "   WHERE ($1 = tool_run_id)"
       "     AND (host(($2)::INET)::INET = ip_addr)"
       "     AND ($3 = protocol)"
       "     AND ($4 = port)"
       "     AND ($5 = plugin_id)"
       "     AND ($6 = metasploit_name)"
       " )");

    // ----------------------------------------------------------------------
    // TABLE: raw_nse_results
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_nse_result",
       "INSERT INTO raw_nse_results"
       " (tool_run_id, ip_addr, protocol, port, script_id, script_output)"
       " SELECT $1, host(($2)::INET)::INET, $3, $4, $5,"
       "        nullif($6, '')"
       " WHERE NOT EXISTS ("
       "   SELECT 1 FROM raw_nse_results"
       "   WHERE ($1 = tool_run_id)"
       "     AND (host(($2)::INET)::INET = ip_addr)"
       "     AND ($3 = protocol)"
       "     AND ($4 = port)"
       "     AND ($5 = script_id)"
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
       "   WHERE ($1 = tool_run_id)"
       "     AND (host(($2)::INET)::INET = ip_addr)"
       "     AND ($3 = protocol)"
       "     AND ($4 = port)"
       "     AND ($5 = ssh_key_type)"
       "     AND ($6 = ssh_key_bits)"
       "     AND ($7 = ssh_key_fingerprint)"
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
       "   WHERE ($1 = tool_run_id)"
       "     AND (host(($2)::INET)::INET = ip_addr)"
       "     AND ($3 = protocol)"
       "     AND ($4 = port)"
       "     AND ($5 = ssh_algo_type)"
       "     AND ($6 = ssh_algo_name)"
       " )");

    // ----------------------------------------------------------------------
    // TABLE: InterfaceNetwork
    // ----------------------------------------------------------------------
    db.prepare
      ("insert_raw_device_interfaces_cdp",
       "INSERT INTO raw_device_interfaces_cdp"
       "  (tool_run_id, device_id, interface_name, is_cdp_enabled)"
       " SELECT $1, $2, $3, $4"
       " WHERE NOT EXISTS ("
       "   SELECT 1 FROM raw_device_interfaces_cdp"
       "   WHERE ($1 = tool_run_id)"
       "     AND ($2 = device_id)"
       "     AND ($3 = interface_name)"
       " )");

    db.prepare
      ("insert_raw_device_interfaces_bpdu",
       "INSERT INTO raw_device_interfaces_bpdu"
       "  (tool_run_id, device_id, interface_name, "
       "   is_bpduguard_enabled, is_bpdufilter_enabled)"
       " SELECT $1, $2, $3, $4, $5"
       " WHERE NOT EXISTS ("
       "   SELECT 1 FROM raw_device_interfaces_bpdu"
       "   WHERE ($1 = tool_run_id)"
       "     AND ($2 = device_id)"
       "     AND ($3 = interface_name)"
       " )");

    db.prepare
      ("insert_raw_device_interfaces_portfast",
       "INSERT INTO raw_device_interfaces_portfast"
       "  (tool_run_id, device_id, interface_name, is_portfast_enabled)"
       " SELECT $1, $2, $3, $4"
       " WHERE NOT EXISTS ("
       "   SELECT 1 FROM raw_device_interfaces_portfast"
       "   WHERE ($1 = tool_run_id)"
       "     AND ($2 = device_id)"
       "     AND ($3 = interface_name)"
       " )");

    db.prepare
      ("insert_raw_device_interfaces_mode",
       "INSERT INTO raw_device_interfaces_mode"
       "  (tool_run_id, device_id, interface_name, interface_mode)"
       " SELECT $1, $2, $3, $4"
       " WHERE NOT EXISTS ("
       "   SELECT 1 FROM raw_device_interfaces_mode"
       "   WHERE ($1 = tool_run_id)"
       "     AND ($2 = device_id)"
       "     AND ($3 = interface_name)"
       " )");

    db.prepare
      ("insert_raw_device_interfaces_port_security",
       "INSERT INTO raw_device_interfaces_port_security"
       "  (tool_run_id, device_id, interface_name, "
       "   is_port_security_enabled, is_mac_addr_sticky, max_mac_addrs, "
       "   violation_action)"
       " SELECT $1, $2, $3, $4, $5, $6, $7"
       " WHERE NOT EXISTS ("
       "   SELECT 1 FROM raw_device_interfaces_port_security"
       "   WHERE ($1 = tool_run_id)"
       "     AND ($2 = device_id)"
       "     AND ($3 = interface_name)"
       " )");

    db.prepare
      ("insert_raw_device_interfaces_port_security_mac_addr",
       "INSERT INTO raw_device_interfaces_port_security_mac_addrs"
       "  (tool_run_id, device_id, interface_name, mac_addr)"
       " SELECT $1, $2, $3, $4"
       " WHERE NOT EXISTS ("
       "   SELECT 1 FROM raw_device_interfaces_port_security_mac_addrs"
       "   WHERE ($1 = tool_run_id)"
       "     AND ($2 = device_id)"
       "     AND ($3 = interface_name)"
       "     AND ($4 = mac_addr)"
       " )");

    db.prepare
      ("insert_raw_device_interfaces_vlan",
       "INSERT INTO raw_device_interfaces_vlans"
       "  (tool_run_id, device_id, interface_name, vlan)"
       " SELECT $1, $2, $3, $4"
       " WHERE NOT EXISTS ("
       "   SELECT 1 FROM raw_device_interfaces_vlans"
       "   WHERE ($1 = tool_run_id)"
       "     AND ($2 = device_id)"
       "     AND ($3 = interface_name)"
       "     AND ($4 = vlan)"
       " )");

    // ----------------------------------------------------------------------
    // TABLE: Ac*Book
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_device_ac_net",
       "INSERT INTO raw_device_ac_nets"
       "  (tool_run_id, device_id, net_set_id, net_set, net_set_data)"
       " SELECT $1, $2, $3, $4, $5"
       " WHERE NOT EXISTS ("
       "   SELECT 1 FROM raw_device_ac_nets"
       "   WHERE ($1 = tool_run_id)"
       "     AND ($2 = device_id)"
       "     AND ($3 = net_set_id)"
       "     AND ($4 = net_set)"
       "     AND ($5 = net_set_data)"
       ")");

    db.prepare
      ("insert_raw_device_ac_service",
       "INSERT INTO raw_device_ac_services"
       "  (tool_run_id, device_id, service_set, service_set_data)"
       " SELECT $1, $2, $3, $4"
       " WHERE NOT EXISTS ("
       "   SELECT 1 FROM raw_device_ac_services"
       "   WHERE ($1 = tool_run_id)"
       "     AND ($2 = device_id)"
       "     AND ($3 = service_set)"
       "     AND ($4 = service_set_data)"
       ")");

    // ----------------------------------------------------------------------
    // TABLE: AcRule
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_device_ac_rule",
       "INSERT INTO raw_device_ac_rules"
       "  (tool_run_id, device_id, enabled, ac_id,"
       "   src_net_set_id, src_net_set, src_iface,"
       "   dst_net_set_id, dst_net_set, dst_iface,"
       "   service_set, action, description)"
       " SELECT $1, $2, $3, $4,"
       "        $5, $6, $7,"
       "        $8, $9, $10,"
       "        $11, $12, nullif($13, '')"
       " WHERE NOT EXISTS ("
       "   SELECT 1 FROM raw_device_ac_rules"
       "   WHERE ($1 = tool_run_id)"
       "     AND ($2 = device_id)"
       "     AND ($3 = enabled)"
       "     AND ($4 = ac_id)"
       "     AND ($5 = src_net_set_id)"
       "     AND ($6 = src_net_set)"
       "     AND ($7 = src_iface)"
       "     AND ($8 = dst_net_set_id)"
       "     AND ($9 = dst_net_set)"
       "     AND ($10 = dst_iface)"
       "     AND ($11 = service_set)"
       "     AND ($12 = action)"
       "     AND ($13 = description)"
       ")");

    // ----------------------------------------------------------------------
    // TABLE: ToolObservations
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_tool_observation",
       "INSERT INTO raw_tool_observations"
       "  (tool_run_id, category, observation)"
       " SELECT $1, $2, $3"
       " WHERE NOT EXISTS ("
       "   SELECT 1 FROM raw_tool_observations"
       "   WHERE ($1 = tool_run_id)"
       "     AND ($2 = category)"
       "     AND ($3 = observation)"
       ")");

    // ----------------------------------------------------------------------
    // SELECT statements
    // ----------------------------------------------------------------------

    db.prepare
      ("select_raw_device_ip_addrs",
       "SELECT * FROM raw_device_ip_addrs"
       "  WHERE ($1 = tool_run_id)"
       "    AND ($2 = device_id)"
       "    AND ($3 = interface_name)"
      );

    // ----------------------------------------------------------------------
    // Ensure that certain table entries are present (create them if absent)
    // ----------------------------------------------------------------------

    if (true) {
      pqxx::work t{db};

      t.exec_prepared("insert_tool_run",
        "32b2fd62-08ff-4d44-8da7-6fbd581a90c6",  // id
        "human",   // tool_name
        "human",   // command_line
        "human",   // data_path
        nullptr,   // execute_time_lower (NULL)
        nullptr);  // execute_time_upper (NULL)

      t.commit();
    }

    // ----------------------------------------------------------------------
  }

}
