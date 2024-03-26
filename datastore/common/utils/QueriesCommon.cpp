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

#include <netmeld/datastore/utils/QueriesCommon.hpp>


namespace netmeld::datastore::utils {

  void
  dbPrepareAws(pqxx::connection& db)
  {
    // ----------------------------------------------------------------------
    // TABLES: AWS CidrBlock related
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_aws_cidr_block", R"(
          INSERT INTO raw_aws_cidr_blocks
            (tool_run_id, cidr_block)
          VALUES
            ($1, $2)
          ON CONFLICT DO NOTHING
        )");

    db.prepare
      ("insert_raw_aws_cidr_block_detail", R"(
          INSERT INTO raw_aws_cidr_block_details
            (tool_run_id, cidr_block, state, description)
          VALUES
            ($1, $2, $3, $4)
          ON CONFLICT DO NOTHING
        )");

    db.prepare
      ("insert_raw_aws_cidr_block_fqdn", R"(
          INSERT INTO raw_aws_cidr_block_fqdns
            (tool_run_id, cidr_block, fqdn)
          VALUES
            ($1, $2, $3)
          ON CONFLICT DO NOTHING
        )");



    // ----------------------------------------------------------------------
    // TABLES: AWS Instance related
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_aws_instance", R"(
          INSERT INTO raw_aws_instances
            (tool_run_id, instance_id)
          VALUES
            ($1, $2)
          ON CONFLICT DO NOTHING
        )");

    db.prepare
      ("insert_raw_aws_instance_detail", R"(
          INSERT INTO raw_aws_instance_details
            (tool_run_id, instance_id, instance_type, image_id
            , architecture, platform_details, launch_time
            , availability_zone, state_code, state_name
            )
          VALUES
            ($1, $2, $3, $4
            , $5, $6, $7
            , $8, $9, $10
            )
          ON CONFLICT DO NOTHING
        )");


    // ----------------------------------------------------------------------
    // TABLES: AWS Network Interface related
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_aws_network_interface", R"(
          INSERT INTO raw_aws_network_interfaces
            (tool_run_id, interface_id)
          VALUES
            ($1, $2)
          ON CONFLICT DO NOTHING
        )");

    db.prepare
      ("insert_raw_aws_network_interface_detail", R"(
          INSERT INTO raw_aws_network_interface_details
            (tool_run_id, interface_id, interface_type
            , source_destination_check, status, description
            )
          VALUES
            ($1, $2, $3
            , $4, $5, $6
            )
          ON CONFLICT DO NOTHING
        )");

    db.prepare
      ("insert_raw_aws_network_interface_attachment", R"(
          INSERT INTO raw_aws_network_interface_attachments
            (tool_run_id, interface_id, id, status, delete_on_termination)
          VALUES
            ($1, $2, $3, $4, $5)
          ON CONFLICT DO NOTHING
        )");

    db.prepare
      ("insert_raw_aws_network_interface_mac", R"(
          INSERT INTO raw_aws_network_interface_macs
            (tool_run_id, interface_id, mac_address)
          VALUES
            ($1, $2, $3)
          ON CONFLICT DO NOTHING
        )");

    db.prepare
      ("insert_raw_aws_network_interface_ip", R"(
          INSERT INTO raw_aws_network_interface_ips
            (tool_run_id, interface_id, ip_address)
          VALUES
            ($1, $2, $3)
          ON CONFLICT DO NOTHING
        )");


    // ----------------------------------------------------------------------
    // TABLES: AWS VPC related
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_aws_vpc", R"(
          INSERT INTO raw_aws_vpcs
            (tool_run_id, vpc_id)
          VALUES
            ($1, $2)
          ON CONFLICT DO NOTHING
        )");

    db.prepare
      ("insert_raw_aws_vpc_owner", R"(
          INSERT INTO raw_aws_vpc_owners
            (tool_run_id, vpc_id, owner_id)
          VALUES
            ($1, $2, $3)
          ON CONFLICT DO NOTHING
        )");

    db.prepare
      ("insert_raw_aws_vpc_detail", R"(
          INSERT INTO raw_aws_vpc_details
            (tool_run_id, vpc_id, state)
          VALUES
            ($1, $2, $3)
          ON CONFLICT DO NOTHING
        )");

    db.prepare
      ("insert_raw_aws_vpc_cidr_block", R"(
          INSERT INTO raw_aws_vpc_cidr_blocks
            (tool_run_id, vpc_id, cidr_block, state)
          VALUES
            ($1, $2, $3, nullif($4, ''))
          ON CONFLICT DO NOTHING
        )");


    // ----------------------------------------------------------------------
    // TABLES: AWS VPC Peering Connection related
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_aws_vpc_peering_connection", R"(
          INSERT INTO raw_aws_vpc_peering_connections
            (tool_run_id, pcx_id)
          VALUES
            ($1, $2)
          ON CONFLICT DO NOTHING
        )");

    db.prepare
      ("insert_raw_aws_vpc_peering_connection_peer", R"(
          INSERT INTO raw_aws_vpc_peering_connection_peers
            ( tool_run_id, pcx_id
            , accepter_vpc_id, accepter_owner_id
            , requester_vpc_id, requester_owner_id
            )
          VALUES
            ( $1, $2
            , $3, $4
            , $5, $6
            )
          ON CONFLICT DO NOTHING
        )");

    db.prepare
      ("insert_raw_aws_vpc_peering_connection_status", R"(
          INSERT INTO raw_aws_vpc_peering_connection_statuses
            (tool_run_id, pcx_id, code, message)
          VALUES
            ($1, $2, $3, $4)
          ON CONFLICT DO NOTHING
        )");


    // ----------------------------------------------------------------------
    // TABLES: AWS Security Group related
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_aws_security_group", R"(
          INSERT INTO raw_aws_security_groups
            (tool_run_id, security_group_id)
          VALUES
            ($1, $2)
          ON CONFLICT DO NOTHING
        )");

    db.prepare
      ("insert_raw_aws_security_group_detail", R"(
          INSERT INTO raw_aws_security_group_details
            (tool_run_id, security_group_id, group_name, description)
          VALUES
            ($1, $2, $3, $4)
          ON CONFLICT DO NOTHING
        )");

    db.prepare
      ("insert_raw_aws_security_group_rules_port", R"(
          INSERT INTO raw_aws_security_group_rules_ports
            (tool_run_id, security_group_id, egress, protocol
            , from_port , to_port, cidr_block
            )
          VALUES
            ($1, $2, $3, $4
            , $5, $6, $7
            )
          ON CONFLICT DO NOTHING
        )");

    db.prepare
      ("insert_raw_aws_security_group_rules_type_code", R"(
          INSERT INTO raw_aws_security_group_rules_type_codes
            (tool_run_id, security_group_id, egress, protocol
            , type, code, cidr_block
            )
          VALUES
            ($1, $2, $3, $4
            , $5, $6, $7
            )
          ON CONFLICT DO NOTHING
        )");

    db.prepare
      ("insert_raw_aws_security_group_rules_non_ip_detail", R"(
          INSERT INTO raw_aws_security_group_rules_non_ip_details
            (tool_run_id, security_group_id, egress, protocol
            , from_port, to_port, detail
            )
          VALUES
            ($1, $2, $3, $4
            , $5, $6, $7
            )
          ON CONFLICT DO NOTHING
        )");

    db.prepare
      ("insert_raw_aws_security_group_rules_non_ip_port", R"(
          INSERT INTO raw_aws_security_group_rules_non_ip_ports
            (tool_run_id, security_group_id, egress, protocol
            , from_port, to_port, target
            )
          VALUES
            ($1, $2, $3, $4
            , $5, $6, $7
            )
          ON CONFLICT DO NOTHING
        )");

    db.prepare
      ("insert_raw_aws_security_group_rules_non_ip_type_code", R"(
          INSERT INTO raw_aws_security_group_rules_non_ip_type_codes
            (tool_run_id, security_group_id, egress, protocol
            , type, code, target
            )
          VALUES
            ($1, $2, $3, $4
            , $5, $6, $7
            )
          ON CONFLICT DO NOTHING
        )");


    // ----------------------------------------------------------------------
    // TABLES: AWS Network ACL related
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_aws_network_acl", R"(
          INSERT INTO raw_aws_network_acls
            (tool_run_id, network_acl_id)
          VALUES
            ($1, $2)
          ON CONFLICT DO NOTHING
        )");

    db.prepare
      ("insert_raw_aws_network_acl_rule", R"(
          INSERT INTO raw_aws_network_acl_rules
            (tool_run_id, network_acl_id, egress, rule_number, action
            , protocol, cidr_block
            )
          VALUES
            ($1, $2, $3, $4, $5
            , $6, $7
            )
          ON CONFLICT DO NOTHING
        )");

    db.prepare
      ("insert_raw_aws_network_acl_rules_port", R"(
          INSERT INTO raw_aws_network_acl_rules_ports
            (tool_run_id, network_acl_id, egress, rule_number, from_port
            , to_port
            )
          VALUES
            ($1, $2, $3, $4, $5
            , $6
            )
          ON CONFLICT DO NOTHING
        )");

    db.prepare
      ("insert_raw_aws_network_acl_rules_type_code", R"(
          INSERT INTO raw_aws_network_acl_rules_type_codes
            (tool_run_id, network_acl_id, egress, rule_number, type, code)
          VALUES
            ($1, $2, $3, $4, $5, $6)
          ON CONFLICT DO NOTHING
        )");


    // ----------------------------------------------------------------------
    // TABLES: AWS Subnet related
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_aws_subnet", R"(
          INSERT INTO raw_aws_subnets
            (tool_run_id, subnet_id)
          VALUES
            ($1, $2)
          ON CONFLICT DO NOTHING
        )");

    db.prepare
      ("insert_raw_aws_subnet_detail", R"(
          INSERT INTO raw_aws_subnet_details
            (tool_run_id, subnet_id, availability_zone, subnet_arn)
          VALUES
            ($1, $2, $3, $4)
          ON CONFLICT DO NOTHING
        )");

    db.prepare
      ("insert_raw_aws_subnet_cidr_block", R"(
          INSERT INTO raw_aws_subnet_cidr_blocks
            (tool_run_id, subnet_id, cidr_block)
          VALUES
            ($1, $2, $3)
          ON CONFLICT DO NOTHING
        )");


    // ----------------------------------------------------------------------
    // TABLES: AWS Route Table related
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_aws_route_table", R"(
          INSERT INTO raw_aws_route_tables
            (tool_run_id, route_table_id)
          VALUES
            ($1, $2)
          ON CONFLICT DO NOTHING
        )");

    db.prepare
      ("insert_raw_aws_route_table_association", R"(
          INSERT INTO raw_aws_route_table_associations
            (tool_run_id, route_table_id, association_id)
          VALUES
            ($1, $2, $3)
          ON CONFLICT DO NOTHING
        )");

    db.prepare
      ("insert_raw_aws_route_table_route_cidr", R"(
          INSERT INTO raw_aws_route_table_routes_cidr
            (tool_run_id, route_table_id, destination_id, state
            , cidr_block
            )
          VALUES
            ($1, $2, $3, $4
            , $5
            )
          ON CONFLICT DO NOTHING
        )");

    db.prepare
      ("insert_raw_aws_route_table_route_non_cidr", R"(
          INSERT INTO raw_aws_route_table_routes_non_cidr
            (tool_run_id, route_table_id, destination_id, state
            , destination
            )
          VALUES
            ($1, $2, $3, $4
            , $5
            )
          ON CONFLICT DO NOTHING
        )");


    // ----------------------------------------------------------------------
    // TABLES: AWS Transit Gateway related
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_aws_transit_gateway", R"(
          INSERT INTO raw_aws_transit_gateways
            (tool_run_id, tgw_id)
          VALUES
            ($1, $2)
          ON CONFLICT DO NOTHING
        )");

    db.prepare
      ("insert_raw_aws_transit_gateway_owner", R"(
          INSERT INTO raw_aws_transit_gateway_owners
            (tool_run_id, tgw_id, owner_id)
          VALUES
            ($1, $2, $3)
          ON CONFLICT DO NOTHING
        )");

    db.prepare
      ("insert_raw_aws_transit_gateway_attachment", R"(
          INSERT INTO raw_aws_transit_gateway_attachments
            (tool_run_id, tgw_id, tgw_attach_id, state)
          VALUES
            ($1, $2, $3, $4)
          ON CONFLICT DO NOTHING
        )");

    db.prepare
      ("insert_raw_aws_transit_gateway_attachment_detail", R"(
          INSERT INTO raw_aws_transit_gateway_attachment_details
            ( tool_run_id, tgw_id, tgw_attach_id
            , resource_type, resource_id, resource_owner_id
            , association_state
            )
          VALUES
            ( $1, $2, $3
            , $4, $5, $6
            , $7
            )
          ON CONFLICT DO NOTHING
        )");


    // ----------------------------------------------------------------------
    // TABLES: AWS multi-service/component related
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_aws_instance_network_interface", R"(
          INSERT INTO raw_aws_instance_network_interfaces
            (tool_run_id, instance_id, interface_id)
          VALUES
            ($1, $2, $3)
          ON CONFLICT DO NOTHING
        )");

    db.prepare
      ("insert_raw_aws_network_interface_vpc_subnet", R"(
          INSERT INTO raw_aws_network_interface_vpc_subnet
            (tool_run_id, interface_id, vpc_id, subnet_id)
          VALUES
            ($1, $2, $3, $4)
          ON CONFLICT DO NOTHING
        )");

    db.prepare
      ("insert_raw_aws_network_interface_security_group", R"(
          INSERT INTO raw_aws_network_interface_security_groups
            (tool_run_id, interface_id, security_group_id)
          VALUES
            ($1, $2, $3)
          ON CONFLICT DO NOTHING
        )");

    db.prepare
      ("insert_raw_aws_vpc_subnet", R"(
          INSERT INTO raw_aws_vpc_subnets
            (tool_run_id, vpc_id, subnet_id)
          VALUES
            ($1, $2, $3)
          ON CONFLICT DO NOTHING
        )");

    db.prepare
      ("insert_raw_aws_vpc_security_group", R"(
          INSERT INTO raw_aws_vpc_security_groups
            (tool_run_id, vpc_id, security_group_id)
          VALUES
            ($1, $2, $3)
          ON CONFLICT DO NOTHING
        )");

    db.prepare
      ("insert_raw_aws_vpc_network_acl", R"(
          INSERT INTO raw_aws_vpc_network_acls
            (tool_run_id, vpc_id, network_acl_id)
          VALUES
            ($1, $2, $3)
          ON CONFLICT DO NOTHING
        )");

    db.prepare
      ("insert_raw_aws_vpc_route_table", R"(
          INSERT INTO raw_aws_vpc_route_tables
            (tool_run_id, vpc_id, route_table_id, is_default)
          VALUES
            ($1, $2, $3, $4)
          ON CONFLICT DO NOTHING
        )");

    db.prepare
      ("insert_raw_aws_network_acl_subnet", R"(
          INSERT INTO raw_aws_network_acl_subnets
            (tool_run_id, network_acl_id, subnet_id)
          VALUES
            ($1, $2, $3)
          ON CONFLICT DO NOTHING
        )");

    // ----------------------------------------------------------------------
  }

  void
  dbPrepareCommon(pqxx::connection& db)
  {
    // ----------------------------------------------------------------------
    // TABLE: tool_runs
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_tool_run",
       "INSERT INTO tool_runs"
       "  (id, tool_name, command_line, data_path, execute_time)"
       " VALUES ($1, $2, $3, $4, TSRANGE($5, $6, '[]'))"
       " ON CONFLICT"
       " DO NOTHING");

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
       "  (tool_run_id, interface_name, media_type, is_up)"
       " VALUES ($1, $2, $3, $4)"
       " ON CONFLICT"
       " DO NOTHING");

    // ----------------------------------------------------------------------
    // TABLE: tool_run_mac_addrs
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_tool_run_mac_addr",
       "INSERT INTO tool_run_mac_addrs"
       "  (tool_run_id, interface_name, mac_addr)"
       " VALUES ($1, $2, $3)"
       " ON CONFLICT"
       " DO NOTHING");

    // ----------------------------------------------------------------------
    // TABLE: tool_run_ip_addrs
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_tool_run_ip_addr",
       "INSERT INTO tool_run_ip_addrs"
       "  (tool_run_id, interface_name, ip_addr)"
       " VALUES ($1, $2, $3)"
       " ON CONFLICT"
       " DO NOTHING");

    // ----------------------------------------------------------------------
    // TABLE: tool_run_ip_routes
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_tool_run_ip_route",
       "INSERT INTO tool_run_ip_routes"
       "  (tool_run_id, interface_name, dst_ip_net, next_hop_ip_addr)"
       " VALUES ($1, $2, network(($3)::INET), host(($4)::INET)::INET)"
       " ON CONFLICT"
       " DO NOTHING");

    // ----------------------------------------------------------------------
    // TABLE: raw_devices
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_device",
       "INSERT INTO raw_devices"
       "  (tool_run_id, device_id)"
       " VALUES ($1, $2)"
       " ON CONFLICT"
       " DO NOTHING");

    // ----------------------------------------------------------------------
    // TABLE: raw_devices_aaa
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_device_aaa",
       "INSERT INTO raw_devices_aaa"
       "  (tool_run_id, device_id, aaa_command)"
       " VALUES ($1, $2, $3)"
       " ON CONFLICT"
       " DO NOTHING");

    // ----------------------------------------------------------------------
    // TABLE: device_extra_weights
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_device_extra_weight",
       "INSERT INTO device_extra_weights"
       "  (device_id, extra_weight)"
       " VALUES ($1, $2)"
       " ON CONFLICT"
       " DO NOTHING");

    // ----------------------------------------------------------------------
    // TABLE: raw_device_virtualizations
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_device_virtualization",
       "INSERT INTO raw_device_virtualizations"
       "  (tool_run_id, host_device_id, guest_device_id)"
       " VALUES ($1, $2, $3)"
       " ON CONFLICT"
       " DO NOTHING");

    // ----------------------------------------------------------------------
    // TABLE: device_colors
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_device_color",
       "INSERT INTO device_colors"
       "  (device_id, color)"
       " VALUES ($1, $2)"
       " ON CONFLICT"
       " DO NOTHING");

    // ----------------------------------------------------------------------
    // TABLE: raw_device_hardware_information
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_device_hardware_information",
       "INSERT INTO raw_device_hardware_information"
       "  (tool_run_id, device_id, device_type, vendor,"
       "   model, hardware_revision, serial_number, description)"
       " VALUES ($1, $2, nullif($3, ''), nullif($4, ''),"
       "         nullif($5, ''), nullif($6, ''),"
       "         nullif($7, ''), nullif($8, ''))"
       " ON CONFLICT"
       " DO NOTHING");

    // ----------------------------------------------------------------------
    // TABLE: raw_device_interfaces
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_device_interface",
       "INSERT INTO raw_device_interfaces"
       "  (tool_run_id, device_id, interface_name, media_type, is_up, description)"
       " VALUES ($1, $2, $3, $4, $5, nullif($6, ''))"
       " ON CONFLICT"
       " DO NOTHING");

    // ----------------------------------------------------------------------
    // TABLE: raw_device_mac_addrs
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_device_mac_addr",
       "INSERT INTO raw_device_mac_addrs"
       "  (tool_run_id, device_id, interface_name, mac_addr)"
       " VALUES ($1, $2, $3, $4)"
       " ON CONFLICT"
       " DO NOTHING");

    // ----------------------------------------------------------------------
    // TABLE: raw_device_ip_addrs
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_device_ip_addr",
       "INSERT INTO raw_device_ip_addrs"
       "  (tool_run_id, device_id, interface_name, ip_addr, ip_net)"
       " VALUES ($1, $2, $3, host(($4)::INET)::INET, network(($4)::INET))"
       " ON CONFLICT"
       " DO NOTHING");

    // ----------------------------------------------------------------------
    // TABLE: raw_device_vrfs
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_device_vrf",
       "INSERT INTO raw_device_vrfs"
       "  (tool_run_id, device_id, vrf_id)"
       " VALUES ($1, $2, $3)"
       " ON CONFLICT"
       " DO NOTHING");

    // ----------------------------------------------------------------------
    // TABLE: raw_device_vrfs_interfaces
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_device_vrf_interface",
       "INSERT INTO raw_device_vrfs_interfaces"
       "  (tool_run_id, device_id, vrf_id, interface_name)"
       " VALUES ($1, $2, $3, $4)"
       " ON CONFLICT"
       " DO NOTHING");

    // ----------------------------------------------------------------------
    // TABLE: raw_device_ip_routes
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_device_ip_route", R"(
          INSERT INTO raw_device_ip_routes(
                tool_run_id, device_id, vrf_id, table_id, is_active
              , dst_ip_net
              , next_vrf_id
              , next_table_id
              , next_hop_ip_addr
              , outgoing_interface_name
              , protocol
              , administrative_distance, metric
              , description
            )
          VALUES (
                $1, $2, $3, $4, $5
              , network(($6)::INET)
              , nullif($7, '')
              , nullif($8, '')
              , host((nullif($9, ''))::INET)::INET
              , nullif($10, '')
              , nullif($11, '')
              , $12, $13
              , nullif($14, '')
            )
          ON CONFLICT
          DO NOTHING
        )"
      );

    // ----------------------------------------------------------------------
    // TABLE: raw_device_link_connections
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_device_link_connection",
       "INSERT INTO raw_device_link_connections"
       "  (tool_run_id, self_device_id, self_interface_name, peer_mac_addr)"
       " VALUES ($1, $2, $3, $4)"
       " ON CONFLICT"
       " DO NOTHING");

    // ----------------------------------------------------------------------
    // TABLE: raw_device_ip_servers
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_device_ip_server",
       "INSERT INTO raw_device_ip_servers"
       "  (tool_run_id, device_id, interface_name, service_name,"
       "   server_ip_addr, port, local_service, description)"
       " VALUES ($1, $2, $3, $4, host(($5)::INET)::INET,"
       "         ($6)::PortNumber, $7, nullif($8, ''))"
       " ON CONFLICT"
       " DO NOTHING");

    // ----------------------------------------------------------------------
    // TABLE: raw_device_dns_resolvers
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_device_dns_resolver",
       "INSERT INTO raw_device_dns_resolvers"
       "  (tool_run_id, device_id, interface_name, scope_domain,"
       "   src_ip_addr, dst_ip_addr, dst_port)"
       " VALUES ($1, $2, nullif($3, ''), nullif(lower($4), ''),"
       "         (nullif($5, '0.0.0.0/255'))::INET, ($6)::INET, ($7)::PortNumber)");

    // ----------------------------------------------------------------------
    // TABLE: raw_device_dns_search_domains
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_device_dns_search_domain",
       "INSERT INTO raw_device_dns_search_domains"
       "  (tool_run_id, device_id, search_domain)"
       " VALUES ($1, $2, lower($3))"
       " ON CONFLICT"
       " DO NOTHING");

    // ----------------------------------------------------------------------
    // TABLE: raw_device_dns_references
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_device_dns_reference",
       "INSERT INTO raw_device_dns_references"
       "  (tool_run_id, device_id, hostname)"
       " VALUES ($1, $2, lower($3))"
       " ON CONFLICT"
       " DO NOTHING");

    // ----------------------------------------------------------------------
    // TABLE: raw_mac_addrs
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_mac_addr",
       "INSERT INTO raw_mac_addrs AS orig"
       "  (tool_run_id, mac_addr, is_responding)"
       " VALUES ($1, $2, $3)"
       " ON CONFLICT"
       "  (tool_run_id, mac_addr)"
       " DO UPDATE"
       "  SET is_responding = GREATEST(orig.is_responding, $3)");

    // ----------------------------------------------------------------------
    // TABLE: raw_ip_addrs
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_ip_addr",
       "INSERT INTO raw_ip_addrs AS orig"
       "  (tool_run_id, ip_addr, is_responding)"
       " VALUES ($1, host(($2)::INET)::INET, $3)"
       " ON CONFLICT"
       "  (tool_run_id, ip_addr)"
       " DO UPDATE"
       "  SET is_responding = GREATEST(orig.is_responding, $3)");

    // ----------------------------------------------------------------------
    // TABLE: raw_mac_addrs_ip_addrs
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_mac_addr_ip_addr",
       "INSERT INTO raw_mac_addrs_ip_addrs"
       "  (tool_run_id, mac_addr, ip_addr)"
       " VALUES ($1, $2, host(($3)::INET)::INET)"
       " ON CONFLICT"
       " DO NOTHING");

    // ----------------------------------------------------------------------
    // TABLE: raw_hostnames
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_hostname",
       "INSERT INTO raw_hostnames"
       "  (tool_run_id, ip_addr, hostname, reason)"
       " VALUES ($1, host(($2)::INET)::INET, $3, $4)"
       " ON CONFLICT"
       " DO NOTHING");

    // ----------------------------------------------------------------------
    // TABLE: raw_dns_lookups
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_dns_lookup",
       "INSERT INTO raw_dns_lookups"
       "  (tool_run_id, resolver_ip_addr, resolver_port,"
       "   query_fqdn, query_class, query_type,"
       "   response_status, response_section,"
       "   response_fqdn, response_class, response_type,"
       "   response_ttl, response_data)"
       " VALUES ($1, host(($2)::INET)::INET, ($3)::PortNumber,"
       "         lower($4), upper($5), upper($6),"
       "         upper($7), upper($8),"
       "         lower($9), upper($10), upper($11),"
       "         $12, $13)"
       " ON CONFLICT"
       " DO NOTHING");

    // ----------------------------------------------------------------------
    // TABLE: raw_operating_systems
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_operating_system",
       "INSERT INTO raw_operating_systems AS orig"
       "  (tool_run_id, ip_addr,"
       "   vendor_name, product_name, product_version, cpe, accuracy)"
       " VALUES ($1, host(($2)::INET)::INET,"
       "         nullif($3, ''), nullif($4, ''), nullif($5, ''),"
       "         nullif($6, ''), $7)"
       " ON CONFLICT"
       " DO NOTHING");

    // ----------------------------------------------------------------------
    // TABLE: raw_hotfixes
    // ----------------------------------------------------------------------
    db.prepare
      ("insert_raw_hotfixes",
       "INSERT INTO raw_hotfixes"
       "  (tool_run_id, hotfixes)"
       " VALUES ($1, $2)"
       " ON CONFLICT"
       " DO NOTHING");

    // ----------------------------------------------------------------------
    // VLAN Related
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_vlan",
       "INSERT INTO raw_vlans"
       "  (tool_run_id, vlan, description)"
       " VALUES ($1, ($2)::VlanNumber, nullif($3, ''))"
       " ON CONFLICT"
       " DO NOTHING");

    db.prepare
      ("insert_raw_device_vlan",
       "INSERT INTO raw_device_vlans"
       "  (tool_run_id, device_id, vlan, description)"
       " VALUES ($1, $2, ($3)::VlanNumber, nullif($4, ''))"
       " ON CONFLICT"
       " DO NOTHING");

    db.prepare
      ("insert_raw_vlan_ip_net",
       "INSERT INTO raw_vlans_ip_nets"
       "  (tool_run_id, vlan, ip_net)"
       " VALUES ($1, ($2)::VlanNumber, network(($3)::INET))"
       " ON CONFLICT"
       " DO NOTHING");

    db.prepare
      ("insert_raw_device_vlan_ip_net",
       "INSERT INTO raw_device_vlans_ip_nets"
       "  (tool_run_id, device_id, vlan, ip_net)"
       " VALUES ($1, $2, ($3)::VlanNumber, network(($4)::INET))"
       " ON CONFLICT"
       " DO NOTHING");

    // ----------------------------------------------------------------------
    // TABLE: raw_ip_nets
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_ip_net",
       "INSERT INTO raw_ip_nets"
       "  (tool_run_id, ip_net, description)"
       " VALUES ($1, network(($2)::INET), nullif($3, ''))"
       " ON CONFLICT"
       " DO NOTHING");

    db.prepare
      ("update_raw_ip_net_description",
       "UPDATE raw_ip_nets"
       " SET description = nullif($3, '')"
       " WHERE ($1 = tool_run_id)"
       "   AND (network(($2)::INET) = ip_net)");

    // ----------------------------------------------------------------------
    // TABLE: ip_nets_extra_weights
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_ip_net_extra_weight",
       "INSERT INTO ip_nets_extra_weights"
       "  (ip_net, extra_weight)"
       " VALUES (network(($1)::INET), $2)"
       " ON CONFLICT"
       " DO NOTHING");

    // ----------------------------------------------------------------------
    // TABLE: raw_ip_traceroutes
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_ip_traceroute",
       "INSERT INTO raw_ip_traceroutes"
       "  (tool_run_id, hop_count, next_hop_ip_addr, dst_ip_addr)"
       " VALUES ($1, $2, host(($3)::INET)::INET, host(($4)::INET)::INET)"
       " ON CONFLICT"
       " DO NOTHING");

    // ----------------------------------------------------------------------
    // TABLE: raw_packages
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_packages",
       "INSERT INTO raw_packages"
       "  (tool_run_id, package_state, package_name, package_version, package_architecture, package_description)"
       " VALUES ($1, $2, $3, $4, $5, $6)"
       " ON CONFLICT"
       " DO NOTHING");

    // ----------------------------------------------------------------------
    // TABLE: raw_ports
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_port",
       "INSERT INTO raw_ports"
       "  (tool_run_id, ip_addr, protocol, port, port_state, port_reason)"
       " VALUES ($1, host(($2)::INET)::INET, $3, ($4)::PortNumber,"
       "         nullif($5, ''), nullif($6, ''))"
       " ON CONFLICT"
       " DO NOTHING");

    // ----------------------------------------------------------------------
    // TABLE: raw_network_services
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_network_service",
       "INSERT INTO raw_network_services"
       "  (tool_run_id, ip_addr, protocol, port,"
       "   service_name, service_description, service_reason, observer_ip_addr)"
       " VALUES ($1, host(($2)::INET)::INET, $3, ($4)::PortNumber,"
       "         nullif($5, ''), nullif($6, ''), nullif($7, ''),"
       "         (nullif($8, '0.0.0.0/0'))::INET)"
       " ON CONFLICT"
       " DO NOTHING");

    // ----------------------------------------------------------------------
    // TABLE: raw_nessus_results
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_nessus_result",
       "INSERT INTO raw_nessus_results"
       "  (tool_run_id, ip_addr, protocol, port,"
       "   plugin_id, plugin_name, plugin_family, plugin_type,"
       "   plugin_output, severity, description, solution)"
       " VALUES ($1, host(($2)::INET)::INET, $3, ($4)::PortNumber,"
       "         $5, nullif($6, ''), nullif($7, ''), nullif($8, ''),"
       "         nullif($9, ''), $10, nullif($11, ''), nullif($12, ''))"
       " ON CONFLICT"
       " DO NOTHING");

    // ----------------------------------------------------------------------
    // TABLE: raw_nessus_results_cves
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_nessus_result_cve",
       "INSERT INTO raw_nessus_results_cves"
       "  (tool_run_id, ip_addr, protocol, port,"
       "   plugin_id, cve_id)"
       " VALUES ($1, host(($2)::INET)::INET, $3, ($4)::PortNumber,"
       "         $5, ($6)::CVE)"
       " ON CONFLICT"
       " DO NOTHING");

    // ----------------------------------------------------------------------
    // TABLE: raw_nessus_results_metasploit_modules
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_nessus_result_metasploit_module",
       "INSERT INTO raw_nessus_results_metasploit_modules"
       "  (tool_run_id, ip_addr, protocol, port,"
       "   plugin_id, metasploit_name)"
       " VALUES ($1, host(($2)::INET)::INET, $3, ($4)::PortNumber,"
       "         $5, $6)"
       " ON CONFLICT"
       " DO NOTHING");

    // ----------------------------------------------------------------------
    // TABLE: raw_nse_results
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_nse_result",
       "INSERT INTO raw_nse_results"
       "  (tool_run_id, ip_addr, protocol, port,"
       "   script_id, script_output)"
       " VALUES ($1, host(($2)::INET)::INET, $3, ($4)::PortNumber,"
       "         $5, nullif($6, ''))"
       " ON CONFLICT"
       " DO NOTHING");

    // ----------------------------------------------------------------------
    // TABLE: raw_ssh_host_public_keys
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_ssh_host_public_key",
       "INSERT INTO raw_ssh_host_public_keys"
       "  (tool_run_id, ip_addr, protocol, port,"
       "   ssh_key_type, ssh_key_bits, ssh_key_fingerprint, ssh_key_public)"
       " VALUES ($1, host(($2)::INET)::INET, $3, ($4)::PortNumber,"
       "         $5, $6, $7, $8)"
       " ON CONFLICT"
       " DO NOTHING");

    // ----------------------------------------------------------------------
    // TABLE: raw_ssh_host_algorithms
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_ssh_host_algorithm",
       "INSERT INTO raw_ssh_host_algorithms"
       "  (tool_run_id, ip_addr, protocol, port,"
       "   ssh_algo_type, ssh_algo_name)"
       " VALUES ($1, host(($2)::INET)::INET, $3, ($4)::PortNumber,"
       "         $5, $6)"
       " ON CONFLICT"
       " DO NOTHING");

    // ----------------------------------------------------------------------
    // TABLE: InterfaceNetwork
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_device_interfaces_cdp",
       "INSERT INTO raw_device_interfaces_cdp"
       "  (tool_run_id, device_id, interface_name, is_cdp_enabled)"
       " VALUES ($1, $2, $3, $4)"
       " ON CONFLICT"
       " DO NOTHING");

    db.prepare
      ("insert_raw_device_interfaces_bpdu",
       "INSERT INTO raw_device_interfaces_bpdu"
       "  (tool_run_id, device_id, interface_name, "
       "   is_bpduguard_enabled, is_bpdufilter_enabled)"
       " VALUES ($1, $2, $3, $4, $5)"
       " ON CONFLICT"
       " DO NOTHING");

    db.prepare
      ("insert_raw_device_interfaces_portfast",
       "INSERT INTO raw_device_interfaces_portfast"
       "  (tool_run_id, device_id, interface_name, is_portfast_enabled)"
       " VALUES ($1, $2, $3, $4)"
       " ON CONFLICT"
       " DO NOTHING");

    db.prepare
      ("insert_raw_device_interfaces_mode",
       "INSERT INTO raw_device_interfaces_mode"
       "  (tool_run_id, device_id, interface_name, interface_mode)"
       " VALUES ($1, $2, $3, $4)"
       " ON CONFLICT"
       " DO NOTHING");

    db.prepare
      ("insert_raw_device_interfaces_port_security",
       "INSERT INTO raw_device_interfaces_port_security"
       "  (tool_run_id, device_id, interface_name, "
       "   is_port_security_enabled, is_mac_addr_sticky, max_mac_addrs, "
       "   violation_action)"
       " VALUES ($1, $2, $3, $4, $5, $6, $7)"
       " ON CONFLICT"
       " DO NOTHING");

    db.prepare
      ("insert_raw_device_interfaces_port_security_mac_addr",
       "INSERT INTO raw_device_interfaces_port_security_mac_addrs"
       "  (tool_run_id, device_id, interface_name, mac_addr)"
       " VALUES ($1, $2, $3, $4)"
       " ON CONFLICT"
       " DO NOTHING");

    db.prepare
      ("insert_raw_device_interfaces_vlan",
       "INSERT INTO raw_device_interfaces_vlans"
       "  (tool_run_id, device_id, interface_name, vlan)"
       " VALUES ($1, $2, $3, ($4)::VlanNumber)"
       " ON CONFLICT"
       " DO NOTHING");

    db.prepare
      ("insert_raw_device_interface_hierarchy",
       "INSERT INTO raw_device_interface_hierarchies"
       "  (tool_run_id, device_id, underlying_interface_name, virtual_interface_name)"
       " VALUES ($1, $2, $3, $4)"
       " ON CONFLICT"
       " DO NOTHING");

    // ----------------------------------------------------------------------
    // TABLE: Ac*Book
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_device_ac_net",
       "INSERT INTO raw_device_ac_nets"
       "  (tool_run_id, device_id, net_set_id, net_set, net_set_data)"
       " VALUES ($1, $2, $3, $4, $5)"
       " ON CONFLICT"
       " DO NOTHING");

    db.prepare
      ("insert_raw_device_ac_service",
       "INSERT INTO raw_device_ac_services"
       "  (tool_run_id, device_id, service_set, service_set_data)"
       " VALUES ($1, $2, $3, $4)"
       " ON CONFLICT"
       " DO NOTHING");

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
       " VALUES ($1, $2, $3, $4,"
       "         $5, $6, nullif($7, ''),"
       "         $8, $9, nullif($10, ''),"
       "         nullif($11, ''), $12, nullif($13, ''))"
       " ON CONFLICT"
       " DO NOTHING");

    // ----------------------------------------------------------------------
    // TABLE: raw_device_acl_zones_bases
    // ----------------------------------------------------------------------

    db.prepare(
      "insert_raw_device_acl_zone_base",
      "INSERT INTO raw_device_acl_zones_bases"
      "  (tool_run_id, device_id, zone_id)"
      " VALUES ($1, $2, $3)"
      " ON CONFLICT"
      " DO NOTHING"
    );

    // ----------------------------------------------------------------------
    // TABLE: raw_device_acl_zones_interfaces
    // ----------------------------------------------------------------------

    db.prepare(
      "insert_raw_device_acl_zone_interface",
      "INSERT INTO raw_device_acl_zones_interfaces"
      "  (tool_run_id, device_id, zone_id, interface_name)"
      " VALUES ($1, $2, $3, $4)"
      " ON CONFLICT"
      " DO NOTHING"
    );

    // ----------------------------------------------------------------------
    // TABLE: raw_device_acl_zones_includes
    // ----------------------------------------------------------------------

    db.prepare(
      "insert_raw_device_acl_zone_include",
      "INSERT INTO raw_device_acl_zones_includes"
      "  (tool_run_id, device_id, zone_id, included_id)"
      " VALUES ($1, $2, $3, $4)"
      " ON CONFLICT"
      " DO NOTHING"
    );

    // ----------------------------------------------------------------------
    // TABLE: raw_device_acl_ip_nets_bases
    // ----------------------------------------------------------------------

    db.prepare(
      "insert_raw_device_acl_ip_net_base",
      "INSERT INTO raw_device_acl_ip_nets_bases"
      "  (tool_run_id, device_id, ip_net_set_namespace, ip_net_set_id)"
      " VALUES ($1, $2, $3, $4)"
      " ON CONFLICT"
      " DO NOTHING"
    );

    // ----------------------------------------------------------------------
    // TABLE: raw_device_acl_ip_nets_ip_nets
    // ----------------------------------------------------------------------

    db.prepare(
      "insert_raw_device_acl_ip_net_ip_net",
      "INSERT INTO raw_device_acl_ip_nets_ip_nets"
      "  (tool_run_id, device_id, ip_net_set_namespace, ip_net_set_id, ip_net)"
      " VALUES ($1, $2, $3, $4, network(($5)::INET))"
      " ON CONFLICT"
      " DO NOTHING"
    );

    // ----------------------------------------------------------------------
    // TABLE: raw_device_acl_ip_nets_hostnames
    // ----------------------------------------------------------------------

    db.prepare(
      "insert_raw_device_acl_ip_net_hostname",
      "INSERT INTO raw_device_acl_ip_nets_hostnames"
      "  (tool_run_id, device_id, ip_net_set_namespace, ip_net_set_id, hostname)"
      " VALUES ($1, $2, $3, $4, lower($5))"
      " ON CONFLICT"
      " DO NOTHING"
    );

    // ----------------------------------------------------------------------
    // TABLE: raw_device_acl_ip_nets_includes
    // ----------------------------------------------------------------------

    db.prepare(
      "insert_raw_device_acl_ip_net_include",
      "INSERT INTO raw_device_acl_ip_nets_includes"
      "  (tool_run_id, device_id, ip_net_set_namespace, ip_net_set_id,"
      "   included_namespace, included_id)"
      " VALUES ($1, $2, $3, $4, $5, $6)"
      " ON CONFLICT"
      " DO NOTHING"
    );

    // ----------------------------------------------------------------------
    // TABLE: raw_device_acl_ports_bases
    // ----------------------------------------------------------------------

    db.prepare(
      "insert_raw_device_acl_port_base",
      "INSERT INTO raw_device_acl_ports_bases"
      "  (tool_run_id, device_id, port_set_id)"
      " VALUES ($1, $2, $3)"
      " ON CONFLICT"
      " DO NOTHING"
    );

    // ----------------------------------------------------------------------
    // TABLE: raw_device_acl_ports_ports
    // ----------------------------------------------------------------------

    db.prepare(
      "insert_raw_device_acl_port_port",
      "INSERT INTO raw_device_acl_ports_ports"
      "  (tool_run_id, device_id, port_set_id, port_range)"
      " VALUES ($1, $2, $3, $4::PortRange)"
      " ON CONFLICT"
      " DO NOTHING"
    );

    // ----------------------------------------------------------------------
    // TABLE: raw_device_acl_ports_includes
    // ----------------------------------------------------------------------

    db.prepare(
      "insert_raw_device_acl_port_include",
      "INSERT INTO raw_device_acl_ports_includes"
      "  (tool_run_id, device_id, port_set_id, included_id)"
      " VALUES ($1, $2, $3, $4)"
      " ON CONFLICT"
      " DO NOTHING"
    );

    // ----------------------------------------------------------------------
    // TABLE: raw_device_acl_services_bases
    // ----------------------------------------------------------------------

    db.prepare(
      "insert_raw_device_acl_service_base",
      "INSERT INTO raw_device_acl_services_bases"
      "  (tool_run_id, device_id, service_id)"
      " VALUES ($1, $2, $3)"
      " ON CONFLICT"
      " DO NOTHING"
    );

    // ----------------------------------------------------------------------
    // TABLE: raw_device_acl_services_protocols
    // ----------------------------------------------------------------------

    db.prepare(
      "insert_raw_device_acl_service_protocol",
      "INSERT INTO raw_device_acl_services_protocols"
      "  (tool_run_id, device_id, service_id, protocol)"
      " VALUES ($1, $2, $3, $4)"
      " ON CONFLICT"
      " DO NOTHING"
    );

    // ----------------------------------------------------------------------
    // TABLE: raw_device_acl_services_ports
    // ----------------------------------------------------------------------

    db.prepare(
      "insert_raw_device_acl_service_port",
      "INSERT INTO raw_device_acl_services_ports"
      "  (tool_run_id, device_id, service_id, protocol,"
      "   src_port_range, dst_port_range)"
      " VALUES ($1, $2, $3, $4,"
      "  $5::PortRange, $6::PortRange)"
      " ON CONFLICT"
      " DO NOTHING"
    );

    // ----------------------------------------------------------------------
    // TABLE: raw_device_acl_services_includes
    // ----------------------------------------------------------------------

    db.prepare(
      "insert_raw_device_acl_service_include",
      "INSERT INTO raw_device_acl_services_includes"
      "  (tool_run_id, device_id, service_id, included_id)"
      " VALUES ($1, $2, $3, $4)"
      " ON CONFLICT"
      " DO NOTHING"
    );

    // ----------------------------------------------------------------------
    // TABLE: raw_device_acl_rules_ports
    // ----------------------------------------------------------------------

    db.prepare(
      "insert_raw_device_acl_rule_port",
      "INSERT INTO raw_device_acl_rules_ports"
      "  (tool_run_id, device_id, priority, action,"
      "   incoming_zone_id, outgoing_zone_id,"
      "   src_ip_net_set_namespace, src_ip_net_set_id,"
      "   dst_ip_net_set_namespace, dst_ip_net_set_id,"
      "   protocol, src_port_set_id, dst_port_set_id, description)"
      " VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13, $14)"
      " ON CONFLICT"
      " DO NOTHING"
    );

    // ----------------------------------------------------------------------
    // TABLE: raw_device_acl_rules_services
    // ----------------------------------------------------------------------

    db.prepare(
      "insert_raw_device_acl_rule_service",
      "INSERT INTO raw_device_acl_rules_services"
      "  (tool_run_id, device_id, priority, action,"
      "   incoming_zone_id, outgoing_zone_id,"
      "   src_ip_net_set_namespace, src_ip_net_set_id,"
      "   dst_ip_net_set_namespace, dst_ip_net_set_id,"
      "   service_id, description)"
      " VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12)"
      " ON CONFLICT"
      " DO NOTHING"
    );

    // ----------------------------------------------------------------------
    // TABLE: ToolObservations
    // ----------------------------------------------------------------------

    db.prepare
      ("insert_raw_tool_observation",
       "INSERT INTO raw_tool_observations"
       "  (tool_run_id, category, observation)"
       " VALUES ($1, $2, $3)"
       " ON CONFLICT"
       " DO NOTHING");

    // ----------------------------------------------------------------------
    // TABLE: Prowler*
    // ----------------------------------------------------------------------

    db.prepare
    ("insert_raw_prowler_v2_check", R"(
        INSERT INTO raw_prowler_v2_checks
          (tool_run_id, account_number, timestamp, region,
           level, control_id, service,
           status, severity, control, risk, remediation, documentation_link,
           resource_id
           )
        VALUES
          ($1, $2, $3, $4,
           $5, $6, $7,
           $8, $9, $10, $11, $12, $13,
           $14)
        ON CONFLICT
        DO NOTHING
      )");

    db.prepare
    ("insert_raw_prowler_v3_check", R"(
        INSERT INTO raw_prowler_v3_checks
          (tool_run_id, assessment_start_time,
           finding_unique_id, provider, profile, account_id,
           organizations_info, region, check_id, check_title,
           check_types, service_name, sub_service_name, status,
           status_extended, severity, resource_id,
           resource_arn, resource_tags, resource_type,
           resource_details, description, risk, related_url,
           recommendation, recommendation_url, remediation_code,
           categories, notes, compliance
           )
        VALUES
          ( $1, $2, $3
          , nullif($4, ''), nullif($5, ''), nullif($6, ''), nullif($7, '')
          , nullif($8, ''), nullif($9, ''), nullif($10, ''), nullif($11, '')
          , nullif($12, ''), nullif($13, ''), nullif($14, ''), nullif($15, '')
          , $16, nullif($17, ''), nullif($18, ''), nullif($19, '')
          , nullif($20, ''), nullif($21, ''), nullif($22, ''), nullif($23, '')
          , nullif($24, ''), nullif($25, ''), nullif($26, ''), nullif($27, '')
          , nullif($28, ''), nullif($29, ''), nullif($30, '')
          )
        ON CONFLICT
        DO NOTHING
      )");


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


    dbPrepareAws(db);

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
