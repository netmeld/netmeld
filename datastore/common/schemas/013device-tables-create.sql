-- =============================================================================
-- Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
-- (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
-- Government retains certain rights in this software.
--
-- Permission is hereby granted, free of charge, to any person obtaining a copy
-- of this software and associated documentation files (the "Software"), to deal
-- in the Software without restriction, including without limitation the rights
-- to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
-- copies of the Software, and to permit persons to whom the Software is
-- furnished to do so, subject to the following conditions:
--
-- The above copyright notice and this permission notice shall be included in
-- all copies or substantial portions of the Software.
--
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
-- IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
-- FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
-- AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
-- LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
-- OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
-- SOFTWARE.
-- =============================================================================
-- Maintained by Sandia National Laboratories <Netmeld@sandia.gov>
-- =============================================================================

BEGIN TRANSACTION;

-- ----------------------------------------------------------------------
-- Information obtained about target devices.
-- ----------------------------------------------------------------------

CREATE TABLE raw_devices (
    tool_run_id                 UUID            NOT NULL
  , device_id                   TEXT            NOT NULL
  , PRIMARY KEY (tool_run_id, device_id)
  , FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
CREATE INDEX raw_devices_idx_tool_run_id
ON raw_devices(tool_run_id);

CREATE INDEX raw_devices_idx_device_id
ON raw_devices(device_id);


-- ----------------------------------------------------------------------

CREATE TABLE raw_device_virtualizations (
    tool_run_id                 UUID            NOT NULL
  , host_device_id              TEXT            NOT NULL
  , guest_device_id             TEXT            NOT NULL
  , PRIMARY KEY (tool_run_id, host_device_id, guest_device_id)
  , FOREIGN KEY (tool_run_id, host_device_id)
        REFERENCES raw_devices(tool_run_id, device_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
  , FOREIGN KEY (tool_run_id, guest_device_id)
        REFERENCES raw_devices(tool_run_id, device_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
CREATE INDEX raw_device_virtualizations_idx_tool_run_id
ON raw_device_virtualizations(tool_run_id);

CREATE INDEX raw_device_virtualizations_idx_host_device_id
ON raw_device_virtualizations(host_device_id);

CREATE INDEX raw_device_virtualizations_idx_guest_device_id
ON raw_device_virtualizations(guest_device_id);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_virtualizations_idx_views
ON raw_device_virtualizations(host_device_id, guest_device_id);


-- ----------------------------------------------------------------------

CREATE TABLE raw_device_hardware_information (
    tool_run_id                 UUID            NOT NULL
  , device_id                   TEXT            NOT NULL
  , device_type                 TEXT            NULL
  , vendor                      TEXT            NULL
  , model                       TEXT            NULL
  , hardware_revision           TEXT            NULL
  , serial_number               TEXT            NULL
  , description                 TEXT            NULL
  , FOREIGN KEY (tool_run_id, device_id)
        REFERENCES raw_devices(tool_run_id, device_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Since this table lacks a PRIMARY KEY and allows NULLs (>2):
-- Create UNIQUE expressional index with substitutions of NULL values
-- for use with `ON CONFLICT` guards against duplicate data.
CREATE UNIQUE INDEX raw_device_hardware_information_idx_unique
ON raw_device_hardware_information(
  HASH_CHAIN(
      tool_run_id::TEXT, device_id, device_type, vendor, model
    , hardware_revision, serial_number, description
    )
);

-- Partial indexes
CREATE INDEX raw_device_hardware_information_idx_tool_run_id
ON raw_device_hardware_information(tool_run_id);

CREATE INDEX raw_device_hardware_information_idx_device_id
ON raw_device_hardware_information(device_id);

CREATE INDEX raw_device_hardware_information_idx_device_type
ON raw_device_hardware_information(device_type);

CREATE INDEX raw_device_hardware_information_idx_vendor
ON raw_device_hardware_information(vendor);

CREATE INDEX raw_device_hardware_information_idx_model
ON raw_device_hardware_information(model);

CREATE INDEX raw_device_hardware_information_idx_hardware_revision
ON raw_device_hardware_information(hardware_revision);

CREATE INDEX raw_device_hardware_information_idx_serial_number
ON raw_device_hardware_information(serial_number);

CREATE INDEX raw_device_hardware_information_idx_description
ON raw_device_hardware_information(description);

-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_hardware_information_idx_views
ON raw_device_hardware_information(
    device_id, device_type, vendor, model, hardware_revision, serial_number
  , description
);


-- ----------------------------------------------------------------------

CREATE TABLE device_colors (
    device_id                   TEXT            NOT NULL
  , color                       TEXT            NOT NULL
  , PRIMARY KEY (device_id)
);


-- ----------------------------------------------------------------------

CREATE TABLE device_extra_weights (
    device_id                   TEXT            NOT NULL
  , extra_weight                FLOAT           NOT NULL
  , PRIMARY KEY (device_id)
);


-- ----------------------------------------------------------------------
-- Interfaces from target devices.
-- ----------------------------------------------------------------------

CREATE TABLE raw_device_interfaces (
    tool_run_id                 UUID            NOT NULL
  , device_id                   TEXT            NOT NULL
  , interface_name              TEXT            NOT NULL
  , media_type                  TEXT            NOT NULL
  , is_up                       BOOLEAN         NOT NULL
  , description                 TEXT            NULL
  , PRIMARY KEY (tool_run_id, device_id, interface_name)
  , FOREIGN KEY (tool_run_id, device_id)
        REFERENCES raw_devices(tool_run_id, device_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
CREATE INDEX raw_device_interfaces_idx_tool_run_id
ON raw_device_interfaces(tool_run_id);

CREATE INDEX raw_device_interfaces_idx_device_id
ON raw_device_interfaces(device_id);

CREATE INDEX raw_device_interfaces_idx_interface_name
ON raw_device_interfaces(interface_name);

CREATE INDEX raw_device_interfaces_idx_media_type
ON raw_device_interfaces(media_type);

CREATE INDEX raw_device_interfaces_idx_is_up
ON raw_device_interfaces(is_up);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_interfaces_idx_views
ON raw_device_interfaces(device_id, interface_name);


-- ----------------------------------------------------------------------
-- Interface hierarchies from target devices.
-- ----------------------------------------------------------------------

CREATE TABLE raw_device_interface_hierarchies (
    tool_run_id                 UUID            NOT NULL
  , device_id                   TEXT            NOT NULL
  , underlying_interface_name   TEXT            NOT NULL
  , virtual_interface_name      TEXT            NOT NULL
  , PRIMARY KEY ( tool_run_id, device_id, underlying_interface_name
                , virtual_interface_name
                )
  , FOREIGN KEY (tool_run_id, device_id, underlying_interface_name)
        REFERENCES raw_device_interfaces(tool_run_id, device_id, interface_name)
        ON DELETE CASCADE
        ON UPDATE CASCADE
  , FOREIGN KEY (tool_run_id, device_id, virtual_interface_name)
        REFERENCES raw_device_interfaces(tool_run_id, device_id, interface_name)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
CREATE INDEX raw_device_interface_hierarchies_idx_tool_run_id
ON raw_device_interface_hierarchies(tool_run_id);

CREATE INDEX raw_device_interface_hierarchies_idx_device_id
ON raw_device_interface_hierarchies(device_id);

CREATE INDEX raw_device_interface_hierarchies_idx_underlying_interface_name
ON raw_device_interface_hierarchies(underlying_interface_name);

CREATE INDEX raw_device_interface_hierarchies_idx_virtual_interface_name
ON raw_device_interface_hierarchies(virtual_interface_name);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_interface_hierarchies_idx_views
ON raw_device_interface_hierarchies(
  device_id, underlying_interface_name, virtual_interface_name
);


-- ----------------------------------------------------------------------
-- MAC addresses from target devices.
-- ----------------------------------------------------------------------

CREATE TABLE raw_device_mac_addrs (
    tool_run_id                 UUID            NOT NULL
  , device_id                   TEXT            NOT NULL
  , interface_name              TEXT            NOT NULL
  , mac_addr                    MACADDR         NOT NULL
  , PRIMARY KEY (tool_run_id, device_id, interface_name, mac_addr)
  , FOREIGN KEY (tool_run_id, device_id, interface_name)
        REFERENCES raw_device_interfaces
                   (tool_run_id, device_id, interface_name)
        ON DELETE CASCADE
        ON UPDATE CASCADE
  , FOREIGN KEY (tool_run_id, mac_addr)
        REFERENCES raw_mac_addrs(tool_run_id, mac_addr)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
CREATE INDEX raw_device_mac_addrs_idx_tool_run_id
ON raw_device_mac_addrs(tool_run_id);

CREATE INDEX raw_device_mac_addrs_idx_device_id
ON raw_device_mac_addrs(device_id);

CREATE INDEX raw_device_mac_addrs_idx_interface_name
ON raw_device_mac_addrs(interface_name);

CREATE INDEX raw_device_mac_addrs_idx_mac_addr
ON raw_device_mac_addrs(mac_addr);

CREATE INDEX raw_device_mac_addrs_idx_device_id_interface_name
ON raw_device_mac_addrs(device_id, interface_name);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_mac_addrs_idx_views
ON raw_device_mac_addrs(device_id, interface_name, mac_addr);


-- ----------------------------------------------------------------------
-- IP addresses from target devices.
-- ----------------------------------------------------------------------

CREATE TABLE raw_device_ip_addrs (
    tool_run_id                 UUID            NOT NULL
  , device_id                   TEXT            NOT NULL
  , interface_name              TEXT            NOT NULL
  , ip_addr                     INET            NOT NULL
  , ip_net                      CIDR            NOT NULL
  , PRIMARY KEY (tool_run_id, device_id, interface_name, ip_addr, ip_net)
  , FOREIGN KEY (tool_run_id, device_id, interface_name)
        REFERENCES raw_device_interfaces
                   (tool_run_id, device_id, interface_name)
        ON DELETE CASCADE
        ON UPDATE CASCADE
  , FOREIGN KEY (tool_run_id, ip_addr)
        REFERENCES raw_ip_addrs(tool_run_id, ip_addr)
        ON DELETE CASCADE
        ON UPDATE CASCADE
  , CHECK (ip_addr = host(ip_addr)::INET)
  , CHECK (ip_addr <<= ip_net)
);

-- Partial indexes
CREATE INDEX raw_device_ip_addrs_idx_tool_run_id
ON raw_device_ip_addrs(tool_run_id);

CREATE INDEX raw_device_ip_addrs_idx_device_id
ON raw_device_ip_addrs(device_id);

CREATE INDEX raw_device_ip_addrs_idx_interface_name
ON raw_device_ip_addrs(interface_name);

CREATE INDEX raw_device_ip_addrs_idx_ip_addr
ON raw_device_ip_addrs(ip_addr);

CREATE INDEX raw_device_ip_addrs_idx_ip_net
ON raw_device_ip_addrs(ip_net);

CREATE INDEX raw_device_ip_addrs_idx_device_id_interface_name
ON raw_device_ip_addrs(device_id, interface_name);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_ip_addrs_idx_views
ON raw_device_ip_addrs(device_id, interface_name, ip_addr, ip_net);


-- ----------------------------------------------------------------------
-- VRF (Virtual Routing and Forwarding) tables from target devices.
-- ----------------------------------------------------------------------

CREATE TABLE raw_device_vrfs (
    tool_run_id                 UUID            NOT NULL
  , device_id                   TEXT            NOT NULL
  , vrf_id                      TEXT            NOT NULL
  , PRIMARY KEY (tool_run_id, device_id, vrf_id)
  , FOREIGN KEY (tool_run_id, device_id)
        REFERENCES raw_devices(tool_run_id, device_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
CREATE INDEX raw_device_vrfs_idx_tool_run_id
ON raw_device_vrfs(tool_run_id);

CREATE INDEX raw_device_vrfs_idx_device_id
ON raw_device_vrfs(device_id);

CREATE INDEX raw_device_vrfs_idx_vrf_id
ON raw_device_vrfs(vrf_id);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_vrfs_idx_views
ON raw_device_vrfs(device_id, vrf_id);


-- ----------------------------------------------------------------------
-- VRF member interfaces from target devices.
-- ----------------------------------------------------------------------

CREATE TABLE raw_device_vrfs_interfaces (
    tool_run_id                 UUID            NOT NULL
  , device_id                   TEXT            NOT NULL
  , vrf_id                      TEXT            NOT NULL
  , interface_name              TEXT            NOT NULL
  , PRIMARY KEY (tool_run_id, device_id, vrf_id, interface_name)
  , FOREIGN KEY (tool_run_id, device_id, vrf_id)
        REFERENCES raw_device_vrfs(tool_run_id, device_id, vrf_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
  , FOREIGN KEY (tool_run_id, device_id, interface_name)
        REFERENCES raw_device_interfaces(tool_run_id, device_id, interface_name)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
CREATE INDEX raw_device_vrfs_interfaces_idx_tool_run_id
ON raw_device_vrfs_interfaces(tool_run_id);

CREATE INDEX raw_device_vrfs_interfaces_idx_device_id
ON raw_device_vrfs_interfaces(device_id);

CREATE INDEX raw_device_vrfs_interfaces_idx_vrf_id
ON raw_device_vrfs_interfaces(vrf_id);

CREATE INDEX raw_device_vrfs_interfaces_idx_interface_name
ON raw_device_vrfs_interfaces(interface_name);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_vrfs_interfaces_idx_views
ON raw_device_vrfs_interfaces(device_id, vrf_id, interface_name);


-- ----------------------------------------------------------------------
-- Routing tables from target devices (config files or console access)
-- ----------------------------------------------------------------------

-- dst_ip_net  = '0.0.0.0/0' or '::/0' for the default route.

-- next_vrf_id and next_table_id are used when redirecting to
-- another routing table on same router.

-- next_hop_ip_addr = '0.0.0.0'   or '::'   for directly connected LANs.
-- next_hop_ip_addr is NULL                 for "null" routes (not routed).

-- Don't add foreign keys to raw_ip_addrs or raw_ip_nets:
-- * NULL next_hop_ip_addr values can't be foreign key into the ip_addrs table.
-- * dst_ip_net is likely to be aggregated networks that don't
--   accurately correspond to actual ip_nets.

CREATE TABLE raw_device_ip_routes (
    tool_run_id                 UUID            NOT NULL
  , device_id                   TEXT            NOT NULL
  , vrf_id                      TEXT            NOT NULL
  , table_id                    TEXT            NOT NULL
  , is_active                   BOOLEAN         NOT NULL
  , dst_ip_net                  CIDR            NOT NULL
  , next_vrf_id                 TEXT            NULL
  , next_table_id               TEXT            NULL
  , next_hop_ip_addr            INET            NULL
  , outgoing_interface_name     TEXT            NULL
  , protocol                    TEXT            NULL
  , administrative_distance     INT             NOT NULL
  , metric                      INT             NOT NULL
  , description                 TEXT            NULL
  , FOREIGN KEY (tool_run_id, device_id)
        REFERENCES raw_devices(tool_run_id, device_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
  , CHECK (next_hop_ip_addr = host(next_hop_ip_addr)::INET)
  , CHECK (inet_same_family(dst_ip_net, next_hop_ip_addr))
  , CHECK (  ((next_vrf_id IS NULL)     AND (next_table_id IS NULL))
          OR ((next_vrf_id IS NOT NULL) AND (next_table_id IS NOT NULL))
          )
  , CHECK (  ((next_vrf_id IS NULL)      AND (next_table_id IS NULL))
          OR ((next_hop_ip_addr IS NULL) AND (outgoing_interface_name IS NULL))
          )
);

-- Partial indexes
CREATE INDEX raw_device_ip_routes_idx_tool_run_id
ON raw_device_ip_routes(tool_run_id);

CREATE INDEX raw_device_ip_routes_idx_device_id
ON raw_device_ip_routes(device_id);

CREATE INDEX raw_device_ip_routes_idx_vrf_id
ON raw_device_ip_routes(vrf_id);

CREATE INDEX raw_device_ip_routes_idx_table_id
ON raw_device_ip_routes(table_id);

CREATE INDEX raw_device_ip_routes_idx_active
ON raw_device_ip_routes(is_active);

CREATE INDEX raw_device_ip_routes_idx_dst_ip_net
ON raw_device_ip_routes(dst_ip_net);

CREATE INDEX raw_device_ip_routes_idx_next_vrf_id
ON raw_device_ip_routes(next_vrf_id);

CREATE INDEX raw_device_ip_routes_idx_next_table_id
ON raw_device_ip_routes(next_table_id);

CREATE INDEX raw_device_ip_routes_idx_next_hop_ip_addr
ON raw_device_ip_routes(next_hop_ip_addr);

CREATE INDEX raw_device_ip_routes_idx_outgoing_interface_name
ON raw_device_ip_routes(outgoing_interface_name);

CREATE INDEX raw_device_ip_routes_idx_protocol
ON raw_device_ip_routes(protocol);

CREATE INDEX raw_device_ip_routes_idx_administrative_distance
ON raw_device_ip_routes(administrative_distance);

CREATE INDEX raw_device_ip_routes_idx_metric
ON raw_device_ip_routes(metric);

-- Index important combinations without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_ip_routes_idx_views4
ON raw_device_ip_routes(device_id, vrf_id, is_active, dst_ip_net);

CREATE INDEX raw_device_ip_routes_idx_views5_next_vrf_id
ON raw_device_ip_routes(
  device_id, vrf_id, is_active, dst_ip_net, next_vrf_id
);

CREATE INDEX raw_device_ip_routes_idx_views5_next_hop_ip_addr
ON raw_device_ip_routes(
  device_id, vrf_id, is_active, dst_ip_net, next_hop_ip_addr
);

CREATE INDEX raw_device_ip_routes_idx_views5_outgoing_interface
ON raw_device_ip_routes(device_id, vrf_id, is_active, dst_ip_net
  , outgoing_interface_name);

CREATE INDEX raw_device_ip_routes_idx_views6
ON raw_device_ip_routes(device_id, vrf_id, is_active, dst_ip_net
  , next_hop_ip_addr, outgoing_interface_name);


-- ----------------------------------------------------------------------
-- Server tables from target devices (config files or console access)
-- Stores device's configured DNS, DHCP, WINS, NTP, ... servers
-- ----------------------------------------------------------------------

CREATE TABLE raw_device_ip_servers (
    tool_run_id                 UUID            NOT NULL
  , device_id                   TEXT            NOT NULL
  , interface_name              TEXT            NOT NULL
  , service_name                TEXT            NOT NULL
  , server_ip_addr              INET            NOT NULL
  , port                        PortNumber      NULL
  , local_service               BOOLEAN         NOT NULL
  , description                 TEXT            NULL
  , PRIMARY KEY ( tool_run_id, device_id, interface_name, service_name
                , server_ip_addr
                )
  , FOREIGN KEY (tool_run_id, device_id)
        REFERENCES raw_devices(tool_run_id, device_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
CREATE INDEX raw_device_ip_servers_idx_tool_run_id
ON raw_device_ip_servers(tool_run_id);

CREATE INDEX raw_device_ip_servers_idx_device_id
ON raw_device_ip_servers(device_id);

CREATE INDEX raw_device_ip_servers_idx_interface_name
ON raw_device_ip_servers(interface_name);

CREATE INDEX raw_device_ip_servers_idx_service_name
ON raw_device_ip_servers(service_name);

CREATE INDEX raw_device_ip_servers_idx_server_ip_addr
ON raw_device_ip_servers(server_ip_addr);

CREATE INDEX raw_device_ip_servers_idx_port
ON raw_device_ip_servers(port);

CREATE INDEX raw_device_ip_servers_idx_local_service
ON raw_device_ip_servers(local_service);

CREATE INDEX raw_device_ip_servers_idx_description
ON raw_device_ip_servers(description);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_ip_servers_idx_views
ON raw_device_ip_servers(device_id, service_name, server_ip_addr);


-- ----------------------------------------------------------------------

CREATE TABLE raw_device_dns_resolvers (
    tool_run_id                 UUID            NOT NULL
  , device_id                   TEXT            NOT NULL
  , interface_name              TEXT            NULL
  , scope_domain                TEXT            NULL
  , src_ip_addr                 INET            NULL
  , dst_ip_addr                 INET            NOT NULL
  , dst_port                    PortNumber      NOT NULL
  , FOREIGN KEY (tool_run_id, device_id)
        REFERENCES raw_devices(tool_run_id, device_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
  , CHECK (scope_domain = lower(scope_domain))
  , CHECK (src_ip_addr = host(src_ip_addr)::INET)
  , CHECK (dst_ip_addr = host(dst_ip_addr)::INET)
);

-- Since this table lacks a PRIMARY KEY and allows NULLs (>2):
-- Create UNIQUE expressional index with substitutions of NULL values
-- for use with `ON CONFLICT` guards against duplicate data.
CREATE UNIQUE INDEX raw_device_dns_resolvers_idx_unique
ON raw_device_dns_resolvers(
  HASH_CHAIN(
      tool_run_id::TEXT, device_id, interface_name, scope_domaini
    , src_ip_addr::TEXT, dst_ip_addr::TEXT, dst_port::TEXT
    )
);

-- Partial indexes
CREATE INDEX raw_device_dns_resolvers_idx_tool_run_id
ON raw_device_dns_resolvers(tool_run_id);

CREATE INDEX raw_device_dns_resolvers_idx_device_id
ON raw_device_dns_resolvers(device_id);

CREATE INDEX raw_device_dns_resolvers_idx_interface_name
ON raw_device_dns_resolvers(interface_name);

CREATE INDEX raw_device_dns_resolvers_idx_scope_domain
ON raw_device_dns_resolvers(scope_domain);

CREATE INDEX raw_device_dns_resolvers_idx_src_ip_addr
ON raw_device_dns_resolvers(src_ip_addr);

CREATE INDEX raw_device_dns_resolvers_idx_dst_ip_addr
ON raw_device_dns_resolvers(dst_ip_addr);

CREATE INDEX raw_device_dns_resolvers_idx_dst_port
ON raw_device_dns_resolvers(dst_port);


-- ----------------------------------------------------------------------

CREATE TABLE raw_device_dns_search_domains (
    tool_run_id                 UUID            NOT NULL
  , device_id                   TEXT            NOT NULL
  , search_domain               TEXT            NOT NULL
  , PRIMARY KEY (tool_run_id, device_id, search_domain)
  , FOREIGN KEY (tool_run_id, device_id)
        REFERENCES raw_devices(tool_run_id, device_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
  , CHECK (search_domain = lower(search_domain))
);

-- Partial indexes
CREATE INDEX raw_device_dns_search_domains_idx_tool_run_id
ON raw_device_dns_search_domains(tool_run_id);

CREATE INDEX raw_device_dns_search_domains_idx_device_id
ON raw_device_dns_search_domains(device_id);

CREATE INDEX raw_device_dns_search_domains_idx_search_domain
ON raw_device_dns_search_domains(search_domain);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_dns_search_domains_idx_views
ON raw_device_dns_search_domains(device_id, search_domain);


-- ----------------------------------------------------------------------

CREATE TABLE raw_device_dns_references (
    tool_run_id                 UUID            NOT NULL
  , device_id                   TEXT            NOT NULL
  , hostname                    TEXT            NOT NULL
  , PRIMARY KEY (tool_run_id, device_id, hostname)
  , FOREIGN KEY (tool_run_id, device_id)
        REFERENCES raw_devices(tool_run_id, device_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
  , CHECK (hostname = lower(hostname))
);

-- Partial indexes
CREATE INDEX raw_device_dns_references_idx_tool_run_id
ON raw_device_dns_references(tool_run_id);

CREATE INDEX raw_device_dns_references_idx_device_id
ON raw_device_dns_references(device_id);

CREATE INDEX raw_device_dns_references_idx_hostname
ON raw_device_dns_references(hostname);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_dns_references_idx_views
ON raw_device_dns_references(device_id, hostname);


-- ----------------------------------------------------------------------

CREATE TABLE raw_device_phys_connections (
    tool_run_id                 UUID            NOT NULL
  , self_device_id              TEXT            NOT NULL
  , self_interface_name         TEXT            NOT NULL
  , peer_device_id              TEXT            NOT NULL
  , peer_interface_name         TEXT            NOT NULL
  , PRIMARY KEY ( tool_run_id
                , self_device_id, self_interface_name
                , peer_device_id, peer_interface_name
                )
  , FOREIGN KEY (tool_run_id, self_device_id, self_interface_name)
        REFERENCES raw_device_interfaces
                   (tool_run_id, device_id, interface_name)
        ON DELETE CASCADE
        ON UPDATE CASCADE
  , FOREIGN KEY (tool_run_id, peer_device_id)
        REFERENCES raw_devices(tool_run_id, device_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
CREATE INDEX raw_device_phys_connections_idx_tool_run_id
ON raw_device_phys_connections(tool_run_id);

CREATE INDEX raw_device_phys_connections_idx_self_device_id
ON raw_device_phys_connections(self_device_id);

CREATE INDEX raw_device_phys_connections_idx_self_interface_name
ON raw_device_phys_connections(self_interface_name);

CREATE INDEX raw_device_phys_connections_idx_peer_device_id
ON raw_device_phys_connections(peer_device_id);

CREATE INDEX raw_device_phys_connections_idx_peer_interface_name
ON raw_device_phys_connections(peer_interface_name);

CREATE INDEX raw_device_phys_connections_idx_self
ON raw_device_phys_connections(self_device_id, self_interface_name);

CREATE INDEX raw_device_phys_connections_idx_peer
ON raw_device_phys_connections(peer_device_id, peer_interface_name);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_phys_connections_idx_views
ON raw_device_phys_connections(self_device_id, self_interface_name
                         , peer_device_id, peer_interface_name);


-- ----------------------------------------------------------------------

CREATE TABLE raw_device_link_connections (
    tool_run_id                 UUID            NOT NULL
  , self_device_id              TEXT            NOT NULL
  , self_interface_name         TEXT            NOT NULL
  , peer_mac_addr               MACADDR         NOT NULL
  , PRIMARY KEY ( tool_run_id
                , self_device_id, self_interface_name
                , peer_mac_addr
                )
  , FOREIGN KEY (tool_run_id, self_device_id, self_interface_name)
        REFERENCES raw_device_interfaces
                   (tool_run_id, device_id, interface_name)
        ON DELETE CASCADE
        ON UPDATE CASCADE
  , FOREIGN KEY (tool_run_id, peer_mac_addr)
        REFERENCES raw_mac_addrs(tool_run_id, mac_addr)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
CREATE INDEX raw_device_link_connections_idx_tool_run_id
ON raw_device_link_connections(tool_run_id);

CREATE INDEX raw_device_link_connections_idx_self_device_id
ON raw_device_link_connections(self_device_id);

CREATE INDEX raw_device_link_connections_idx_self_interface_name
ON raw_device_link_connections(self_interface_name);

CREATE INDEX raw_device_link_connections_idx_peer_mac_addr
ON raw_device_link_connections(peer_mac_addr);

CREATE INDEX raw_device_link_connections_idx_self
ON raw_device_link_connections(self_device_id, self_interface_name);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_link_connections_idx_views
ON raw_device_link_connections
   (self_device_id, self_interface_name, peer_mac_addr);


-- ----------------------------------------------------------------------
-- Cisco and Cisco-like device and interface information
-- ----------------------------------------------------------------------

CREATE TABLE raw_devices_aaa (
    tool_run_id                 UUID            NOT NULL
  , device_id                   TEXT            NOT NULL
  , aaa_command                 TEXT            NOT NULL
  , PRIMARY KEY (tool_run_id, device_id, aaa_command)
  , FOREIGN KEY (tool_run_id, device_id)
        REFERENCES raw_devices(tool_run_id, device_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
CREATE INDEX raw_devices_aaa_idx_tool_run_id
ON raw_devices_aaa(tool_run_id);

CREATE INDEX raw_devices_aaa_idx_device_id
ON raw_devices_aaa(device_id);


-- ----------------------------------------------------------------------

CREATE TABLE raw_device_interfaces_mode (
    tool_run_id                 UUID            NOT NULL
  , device_id                   TEXT            NOT NULL
  , interface_name              TEXT            NOT NULL
  , interface_mode              TEXT            NOT NULL
  , PRIMARY KEY (tool_run_id, device_id, interface_name)
  , FOREIGN KEY (tool_run_id, device_id, interface_name)
        REFERENCES raw_device_interfaces
                   (tool_run_id, device_id, interface_name)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
CREATE INDEX raw_device_interfaces_mode_idx_tool_run_id
ON raw_device_interfaces_mode(tool_run_id);

CREATE INDEX raw_device_interfaces_mode_idx_device_id
ON raw_device_interfaces_mode(device_id);

CREATE INDEX raw_device_interfaces_mode_idx_interface_name
ON raw_device_interfaces_mode(interface_name);

CREATE INDEX raw_device_interfaces_mode_idx_interface_mode
ON raw_device_interfaces_mode(interface_mode);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_interfaces_mode_idx_views
ON raw_device_interfaces_mode(device_id, interface_name);


-- ----------------------------------------------------------------------

CREATE TABLE raw_device_interfaces_bpdu (
    tool_run_id                 UUID            NOT NULL
  , device_id                   TEXT            NOT NULL
  , interface_name              TEXT            NOT NULL
  , is_bpduguard_enabled        BOOLEAN         NOT NULL
  , is_bpdufilter_enabled       BOOLEAN         NOT NULL
  , PRIMARY KEY (tool_run_id, device_id, interface_name)
  , FOREIGN KEY (tool_run_id, device_id, interface_name)
        REFERENCES raw_device_interfaces
                   (tool_run_id, device_id, interface_name)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
CREATE INDEX raw_device_interfaces_bpdu_idx_tool_run_id
ON raw_device_interfaces_bpdu(tool_run_id);

CREATE INDEX raw_device_interfaces_bpdu_idx_device_id
ON raw_device_interfaces_bpdu(device_id);

CREATE INDEX raw_device_interfaces_bpdu_idx_interface_name
ON raw_device_interfaces_bpdu(interface_name);

CREATE INDEX raw_device_interfaces_bpdu_idx_is_bpduguard_enabled
ON raw_device_interfaces_bpdu(is_bpduguard_enabled);

CREATE INDEX raw_device_interfaces_bpdu_idx_is_bpdufilter_enabled
ON raw_device_interfaces_bpdu(is_bpdufilter_enabled);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_interfaces_bpdu_idx_views
ON raw_device_interfaces_bpdu(device_id, interface_name);


-- ----------------------------------------------------------------------

CREATE TABLE raw_device_interfaces_cdp (
    tool_run_id                 UUID            NOT NULL
  , device_id                   TEXT            NOT NULL
  , interface_name              TEXT            NOT NULL
  , is_cdp_enabled              BOOLEAN         NOT NULL
  , PRIMARY KEY (tool_run_id, device_id, interface_name)
  , FOREIGN KEY (tool_run_id, device_id, interface_name)
        REFERENCES raw_device_interfaces
                   (tool_run_id, device_id, interface_name)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
CREATE INDEX raw_device_interfaces_cdp_idx_tool_run_id
ON raw_device_interfaces_cdp(tool_run_id);

CREATE INDEX raw_device_interfaces_cdp_idx_device_id
ON raw_device_interfaces_cdp(device_id);

CREATE INDEX raw_device_interfaces_cdp_idx_interface_name
ON raw_device_interfaces_cdp(interface_name);

CREATE INDEX raw_device_interfaces_cdp_idx_is_cdp_enabled
ON raw_device_interfaces_cdp(is_cdp_enabled);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_interfaces_cdp_idx_views
ON raw_device_interfaces_cdp(device_id, interface_name);


-- ----------------------------------------------------------------------

CREATE TABLE raw_device_interfaces_portfast (
    tool_run_id                 UUID            NOT NULL
  , device_id                   TEXT            NOT NULL
  , interface_name              TEXT            NOT NULL
  , is_portfast_enabled         BOOLEAN         NOT NULL
  , PRIMARY KEY (tool_run_id, device_id, interface_name)
  , FOREIGN KEY (tool_run_id, device_id, interface_name)
        REFERENCES raw_device_interfaces
                   (tool_run_id, device_id, interface_name)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
CREATE INDEX raw_device_interfaces_portfast_idx_tool_run_id
ON raw_device_interfaces_portfast(tool_run_id);

CREATE INDEX raw_device_interfaces_portfast_idx_device_id
ON raw_device_interfaces_portfast(device_id);

CREATE INDEX raw_device_interfaces_portfast_idx_interface_name
ON raw_device_interfaces_portfast(interface_name);

CREATE INDEX raw_device_interfaces_portfast_idx_is_portfast_enabled
ON raw_device_interfaces_portfast(is_portfast_enabled);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_interfaces_portfast_idx_views
ON raw_device_interfaces_portfast(device_id, interface_name);


-- ----------------------------------------------------------------------

CREATE TABLE raw_device_interfaces_port_security (
    tool_run_id                 UUID            NOT NULL
  , device_id                   TEXT            NOT NULL
  , interface_name              TEXT            NOT NULL
  , is_port_security_enabled    BOOLEAN         NOT NULL
  , is_mac_addr_sticky          BOOLEAN         NOT NULL
  , max_mac_addrs               INT             NOT NULL
  , violation_action            TEXT            NOT NULL
  , PRIMARY KEY (tool_run_id, device_id, interface_name)
  , FOREIGN KEY (tool_run_id, device_id, interface_name)
        REFERENCES raw_device_interfaces
                   (tool_run_id, device_id, interface_name)
        ON DELETE CASCADE
        ON UPDATE CASCADE
  , CHECK (0 <= max_mac_addrs)
);

-- Partial indexes
CREATE INDEX raw_device_interfaces_port_security_idx_tool_run_id
ON raw_device_interfaces_port_security(tool_run_id);

CREATE INDEX raw_device_interfaces_port_security_idx_device_id
ON raw_device_interfaces_port_security(device_id);

CREATE INDEX raw_device_interfaces_port_security_idx_interface_name
ON raw_device_interfaces_port_security(interface_name);

CREATE INDEX raw_device_interfaces_port_security_idx_is_port_sec_enabled
ON raw_device_interfaces_port_security(is_port_security_enabled);

CREATE INDEX raw_device_interfaces_port_security_idx_is_mac_addr_sticky
ON raw_device_interfaces_port_security(is_mac_addr_sticky);

CREATE INDEX raw_device_interfaces_port_security_idx_max_mac_addrs
ON raw_device_interfaces_port_security(max_mac_addrs);

CREATE INDEX raw_device_interfaces_port_security_idx_violation_action
ON raw_device_interfaces_port_security(violation_action);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_interfaces_port_security_idx_views
ON raw_device_interfaces_port_security(device_id, interface_name);


-- ----------------------------------------------------------------------

CREATE TABLE raw_device_interfaces_port_security_mac_addrs (
    tool_run_id                 UUID            NOT NULL
  , device_id                   TEXT            NOT NULL
  , interface_name              TEXT            NOT NULL
  , mac_addr                    MACADDR         NOT NULL
  , PRIMARY KEY (tool_run_id, device_id, interface_name, mac_addr)
  , FOREIGN KEY (tool_run_id, device_id, interface_name)
        REFERENCES raw_device_interfaces
                   (tool_run_id, device_id, interface_name)
        ON DELETE CASCADE
        ON UPDATE CASCADE
  , FOREIGN KEY (tool_run_id, mac_addr)
        REFERENCES raw_mac_addrs(tool_run_id, mac_addr)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
CREATE INDEX raw_device_interfaces_port_security_mac_addrs_idx_tool_run_id
ON raw_device_interfaces_port_security_mac_addrs(tool_run_id);

CREATE INDEX raw_device_interfaces_port_security_mac_addrs_idx_device_id
ON raw_device_interfaces_port_security_mac_addrs(device_id);

CREATE INDEX raw_device_interfaces_port_security_mac_addrs_idx_interface
ON raw_device_interfaces_port_security_mac_addrs(interface_name);

CREATE INDEX raw_device_interfaces_port_security_mac_addrs_idx_mac_addr
ON raw_device_interfaces_port_security_mac_addrs(mac_addr);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_interfaces_port_security_mac_addrs_idx_views
ON raw_device_interfaces_port_security_mac_addrs
   (device_id, interface_name, mac_addr);


-- ----------------------------------------------------------------------
-- Device Access Control tables
-- ----------------------------------------------------------------------

CREATE TABLE raw_device_ac_nets (
    tool_run_id   UUID    NOT NULL
  , device_id     TEXT    NOT NULL
  , net_set_id    TEXT    NOT NULL
  , net_set       TEXT    NOT NULL
  , net_set_data  TEXT    NULL
  , FOREIGN KEY (tool_run_id, device_id)
      REFERENCES raw_devices(tool_run_id, device_id)
      ON DELETE CASCADE
      ON UPDATE CASCADE
);

-- Since this table lacks a PRIMARY KEY and allows NULLs (<3):
-- Create UNIQUE partial indexes over likely combinations of columns
-- for use with `ON CONFLICT` guards against duplicate data.
CREATE UNIQUE INDEX raw_device_ac_nets_idx_unique1
ON raw_device_ac_nets(tool_run_id, device_id, net_set_id, net_set)
WHERE (net_set_data IS NULL);

CREATE UNIQUE INDEX raw_device_ac_nets_idx_unique2
ON raw_device_ac_nets(tool_run_id, device_id, net_set_id, net_set, net_set_data)
WHERE (net_set_data IS NOT NULL);

-- Partial indexes
CREATE INDEX raw_device_ac_nets_idx_tool_run_id
ON raw_device_ac_nets(tool_run_id);

CREATE INDEX raw_device_ac_nets_idx_device_id
ON raw_device_ac_nets(device_id);

CREATE INDEX raw_device_ac_nets_idx_net_set_id
ON raw_device_ac_nets(net_set_id);

CREATE INDEX raw_device_ac_nets_idx_net_set
ON raw_device_ac_nets(net_set);

CREATE INDEX raw_device_ac_nets_idx_net_set_data
ON raw_device_ac_nets(net_set_data);

-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_ac_nets_idx_views
ON raw_device_ac_nets(device_id, net_set_id, net_set_data);


-- ----------------------------------------------------------------------

CREATE TABLE raw_device_ac_services (
    tool_run_id       UUID    NOT NULL
  , device_id         TEXT    NOT NULL
  , service_set       TEXT    NOT NULL
  , service_set_data  TEXT    NULL
  , FOREIGN KEY (tool_run_id, device_id)
      REFERENCES raw_devices(tool_run_id, device_id)
      ON DELETE CASCADE
      ON UPDATE CASCADE
);

-- Since this table lacks a PRIMARY KEY and allows NULLs (<3):
-- Create UNIQUE partial indexes over likely combinations of columns
-- for use with `ON CONFLICT` guards against duplicate data.
CREATE UNIQUE INDEX raw_device_ac_services_idx_unique1
ON raw_device_ac_services(tool_run_id, device_id, service_set)
WHERE (service_set_data IS NULL);

CREATE UNIQUE INDEX raw_device_ac_services_idx_unique2
ON raw_device_ac_services(tool_run_id, device_id, service_set, service_set_data)
WHERE (service_set_data IS NOT NULL);

-- Partial indexes
CREATE INDEX raw_device_ac_services_idx_tool_run_id
ON raw_device_ac_services(tool_run_id);

CREATE INDEX raw_device_ac_services_idx_device_id
ON raw_device_ac_services(device_id);

CREATE INDEX raw_device_ac_services_idx_service_set
ON raw_device_ac_services(service_set);

CREATE INDEX raw_device_ac_services_idx_service_set_data
ON raw_device_ac_services(service_set_data);

-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_ac_services_idx_views
ON raw_device_ac_services(device_id, service_set, service_set_data);

-- ----------------------------------------------------------------------

CREATE TABLE raw_device_ac_rules (
    tool_run_id     UUID    NOT NULL
  , device_id       TEXT    NOT NULL
  , enabled         BOOLEAN NOT NULL
  , ac_id           INT     NOT NULL
  , src_net_set_id  TEXT    NOT NULL
  , src_net_set     TEXT    NOT NULL
  , src_iface       TEXT    NULL
  , dst_net_set_id  TEXT    NOT NULL
  , dst_net_set     TEXT    NOT NULL
  , dst_iface       TEXT    NULL
  , service_set     TEXT    NULL
  , action          TEXT    NOT NULL
  , description     TEXT    NULL
  , FOREIGN KEY (tool_run_id, device_id)
      REFERENCES raw_devices(tool_run_id, device_id)
      ON DELETE CASCADE
      ON UPDATE CASCADE
);

-- Since this table lacks a PRIMARY KEY and allows NULLs (>2):
-- Create UNIQUE expressional index with substitutions of NULL values
-- for use with `ON CONFLICT` guards against duplicate data.
CREATE UNIQUE INDEX raw_device_ac_rules_idx_unique
ON raw_device_ac_rules(
  HASH_CHAIN(
      tool_run_id::TEXT, device_id, enabled::TEXT, ac_id::TEXT
    , src_net_set_id, src_net_set, src_iface
    , dst_net_set_id, dst_net_set, dst_iface
    , service_set, action, description
    )
);

-- Partial indexes
CREATE INDEX raw_device_ac_rules_idx_tool_run_id
ON raw_device_ac_rules(tool_run_id);

CREATE INDEX raw_device_ac_rules_idx_device_id
ON raw_device_ac_rules(device_id);

CREATE INDEX raw_device_ac_rules_idx_enabled
ON raw_device_ac_rules(enabled);

CREATE INDEX raw_device_ac_rules_idx_ac_id
ON raw_device_ac_rules(ac_id);

CREATE INDEX raw_device_ac_rules_idx_src_net_set_id
ON raw_device_ac_rules(src_net_set_id);

CREATE INDEX raw_device_ac_rules_idx_src_net_set
ON raw_device_ac_rules(src_net_set);

CREATE INDEX raw_device_ac_rules_idx_src_iface
ON raw_device_ac_rules(src_iface);

CREATE INDEX raw_device_ac_rules_idx_dst_net_set_id
ON raw_device_ac_rules(dst_net_set_id);

CREATE INDEX raw_device_ac_rules_idx_dst_net_set
ON raw_device_ac_rules(dst_net_set);

CREATE INDEX raw_device_ac_rules_idx_dst_iface
ON raw_device_ac_rules(dst_iface);

CREATE INDEX raw_device_ac_rules_idx_service_set
ON raw_device_ac_rules(service_set);

CREATE INDEX raw_device_ac_rules_idx_action
ON raw_device_ac_rules(action);

CREATE INDEX raw_device_ac_rules_idx_description
ON raw_device_ac_rules(description);

-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_ac_rules_idx_views
ON raw_device_ac_rules(
    device_id, enabled, ac_id
  , src_net_set_id, src_net_set, src_iface
  , dst_net_set_id, dst_net_set, dst_iface
  , service_set, action, description
);


-- ----------------------------------------------------------------------

CREATE TABLE raw_device_acl_zones_bases (
    tool_run_id             UUID        NOT NULL
  , device_id               TEXT        NOT NULL
  , zone_id                 TEXT        NOT NULL
  , PRIMARY KEY (tool_run_id, device_id, zone_id)
  , FOREIGN KEY (tool_run_id, device_id)
        REFERENCES raw_devices(tool_run_id, device_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
CREATE INDEX raw_device_acl_zones_bases_idx_tool_run_id
ON raw_device_acl_zones_bases(tool_run_id);

CREATE INDEX raw_device_acl_zones_bases_idx_device_id
ON raw_device_acl_zones_bases(device_id);

CREATE INDEX raw_device_acl_zones_bases_idx_zone_id
ON raw_device_acl_zones_bases(zone_id);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_acl_zones_bases_idx_views
ON raw_device_acl_zones_bases(device_id, zone_id);


-- ----------------------------------------------------------------------

CREATE TABLE raw_device_acl_zones_interfaces (
    tool_run_id             UUID        NOT NULL
  , device_id               TEXT        NOT NULL
  , zone_id                 TEXT        NOT NULL
  , interface_name          TEXT        NOT NULL
  , PRIMARY KEY (tool_run_id, device_id, zone_id, interface_name)
  , FOREIGN KEY (tool_run_id, device_id, zone_id)
        REFERENCES raw_device_acl_zones_bases(tool_run_id, device_id, zone_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
CREATE INDEX raw_device_acl_zones_interfaces_idx_tool_run_id
ON raw_device_acl_zones_interfaces(tool_run_id);

CREATE INDEX raw_device_acl_zones_interfaces_idx_device_id
ON raw_device_acl_zones_interfaces(device_id);

CREATE INDEX raw_device_acl_zones_interfaces_idx_zone_id
ON raw_device_acl_zones_interfaces(zone_id);

CREATE INDEX raw_device_acl_zones_interfaces_idx_interface_name
ON raw_device_acl_zones_interfaces(interface_name);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_acl_zones_interfaces_idx_views
ON raw_device_acl_zones_interfaces(device_id, zone_id, interface_name);

CREATE INDEX raw_device_acl_zones_interfaces_idx_views_fkey
ON raw_device_acl_zones_interfaces(device_id, zone_id);


-- ----------------------------------------------------------------------

CREATE TABLE raw_device_acl_zones_includes (
    tool_run_id             UUID        NOT NULL
  , device_id               TEXT        NOT NULL
  , zone_id                 TEXT        NOT NULL
  , included_id             TEXT        NOT NULL
  , PRIMARY KEY (tool_run_id, device_id, zone_id, included_id)
  , FOREIGN KEY (tool_run_id, device_id, zone_id)
        REFERENCES raw_device_acl_zones_bases(tool_run_id, device_id, zone_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
CREATE INDEX raw_device_acl_zones_includes_idx_tool_run_id
ON raw_device_acl_zones_includes(tool_run_id);

CREATE INDEX raw_device_acl_zones_includes_idx_device_id
ON raw_device_acl_zones_includes(device_id);

CREATE INDEX raw_device_acl_zones_includes_idx_zone_id
ON raw_device_acl_zones_includes(zone_id);

CREATE INDEX raw_device_acl_zones_includes_idx_included_id
ON raw_device_acl_zones_includes(included_id);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_acl_zones_includes_idx_views
ON raw_device_acl_zones_includes(device_id, zone_id, included_id);

CREATE INDEX raw_device_acl_zones_includes_idx_views_fkey
ON raw_device_acl_zones_includes(device_id, zone_id);


-- ----------------------------------------------------------------------

CREATE TABLE raw_device_acl_ip_nets_bases (
    tool_run_id             UUID        NOT NULL
  , device_id               TEXT        NOT NULL
  , ip_net_set_namespace    TEXT        NOT NULL
  , ip_net_set_id           TEXT        NOT NULL
  , PRIMARY KEY (tool_run_id, device_id, ip_net_set_namespace, ip_net_set_id)
  , FOREIGN KEY (tool_run_id, device_id)
        REFERENCES raw_devices(tool_run_id, device_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
CREATE INDEX raw_device_acl_ip_nets_idx_tool_run_id
ON raw_device_acl_ip_nets_bases(tool_run_id);

CREATE INDEX raw_device_acl_ip_nets_idx_device_id
ON raw_device_acl_ip_nets_bases(device_id);

CREATE INDEX raw_device_acl_ip_nets_idx_ip_net_set_namespace
ON raw_device_acl_ip_nets_bases(ip_net_set_namespace);

CREATE INDEX raw_device_acl_ip_nets_idx_ip_net_set_id
ON raw_device_acl_ip_nets_bases(ip_net_set_id);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_acl_ip_nets_idx_views
ON raw_device_acl_ip_nets_bases
    (device_id, ip_net_set_namespace, ip_net_set_id);


-- ----------------------------------------------------------------------

CREATE TABLE raw_device_acl_ip_nets_ip_nets (
    tool_run_id             UUID        NOT NULL
  , device_id               TEXT        NOT NULL
  , ip_net_set_namespace    TEXT        NOT NULL
  , ip_net_set_id           TEXT        NOT NULL
  , ip_net                  CIDR        NOT NULL
  , PRIMARY KEY ( tool_run_id, device_id, ip_net_set_namespace, ip_net_set_id
                , ip_net
                )
  , FOREIGN KEY (tool_run_id, device_id, ip_net_set_namespace, ip_net_set_id)
        REFERENCES raw_device_acl_ip_nets_bases
            (tool_run_id, device_id, ip_net_set_namespace, ip_net_set_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
CREATE INDEX raw_device_acl_ip_nets_ip_nets_idx_tool_run_id
ON raw_device_acl_ip_nets_ip_nets(tool_run_id);

CREATE INDEX raw_device_acl_ip_nets_ip_nets_idx_device_id
ON raw_device_acl_ip_nets_ip_nets(device_id);

CREATE INDEX raw_device_acl_ip_nets_ip_nets_idx_ip_net_set_namespace
ON raw_device_acl_ip_nets_ip_nets(ip_net_set_namespace);

CREATE INDEX raw_device_acl_ip_nets_ip_nets_idx_ip_net_set_id
ON raw_device_acl_ip_nets_ip_nets(ip_net_set_id);

CREATE INDEX raw_device_acl_ip_nets_ip_nets_idx_ip_net
ON raw_device_acl_ip_nets_ip_nets(ip_net);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_acl_ip_nets_ip_nets_idx_views
ON raw_device_acl_ip_nets_ip_nets
    (device_id, ip_net_set_namespace, ip_net_set_id, ip_net);

CREATE INDEX raw_device_acl_ip_nets_ip_nets_idx_views_fkey
ON raw_device_acl_ip_nets_ip_nets
    (device_id, ip_net_set_namespace, ip_net_set_id);


-- ----------------------------------------------------------------------

CREATE TABLE raw_device_acl_ip_nets_hostnames (
    tool_run_id             UUID        NOT NULL
  , device_id               TEXT        NOT NULL
  , ip_net_set_namespace    TEXT        NOT NULL
  , ip_net_set_id           TEXT        NOT NULL
  , hostname                TEXT        NOT NULL
  , PRIMARY KEY ( tool_run_id, device_id, ip_net_set_namespace, ip_net_set_id
                , hostname
                )
  , FOREIGN KEY (tool_run_id, device_id, ip_net_set_namespace, ip_net_set_id)
        REFERENCES raw_device_acl_ip_nets_bases
            (tool_run_id, device_id, ip_net_set_namespace, ip_net_set_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
  , FOREIGN KEY (tool_run_id, device_id, hostname)
        REFERENCES raw_device_dns_references
            (tool_run_id, device_id, hostname)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
CREATE INDEX raw_device_acl_ip_nets_hostnames_idx_tool_run_id
ON raw_device_acl_ip_nets_hostnames(tool_run_id);

CREATE INDEX raw_device_acl_ip_nets_hostnames_idx_device_id
ON raw_device_acl_ip_nets_hostnames(device_id);

CREATE INDEX raw_device_acl_ip_nets_hostnames_idx_ip_net_set_namespace
ON raw_device_acl_ip_nets_hostnames(ip_net_set_namespace);

CREATE INDEX raw_device_acl_ip_nets_hostnames_idx_ip_net_set_id
ON raw_device_acl_ip_nets_hostnames(ip_net_set_id);

CREATE INDEX raw_device_acl_ip_nets_hostnames_idx_ip_net
ON raw_device_acl_ip_nets_hostnames(hostname);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_acl_ip_nets_hostnames_idx_views
ON raw_device_acl_ip_nets_hostnames
    (device_id, ip_net_set_namespace, ip_net_set_id, hostname);

CREATE INDEX raw_device_acl_ip_nets_hostnames_idx_views_fkey
ON raw_device_acl_ip_nets_hostnames
    (device_id, ip_net_set_namespace, ip_net_set_id);


-- ----------------------------------------------------------------------

CREATE TABLE raw_device_acl_ip_nets_includes (
    tool_run_id             UUID        NOT NULL
  , device_id               TEXT        NOT NULL
  , ip_net_set_namespace    TEXT        NOT NULL
  , ip_net_set_id           TEXT        NOT NULL
  , included_namespace      TEXT        NOT NULL
  , included_id             TEXT        NOT NULL
  , PRIMARY KEY ( tool_run_id, device_id, ip_net_set_namespace, ip_net_set_id
                , included_namespace, included_id
                )
  , FOREIGN KEY (tool_run_id, device_id, ip_net_set_namespace, ip_net_set_id)
        REFERENCES raw_device_acl_ip_nets_bases
            (tool_run_id, device_id, ip_net_set_namespace, ip_net_set_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
CREATE INDEX raw_device_acl_ip_nets_includes_idx_tool_run_id
ON raw_device_acl_ip_nets_includes(tool_run_id);

CREATE INDEX raw_device_acl_ip_nets_includes_idx_device_id
ON raw_device_acl_ip_nets_includes(device_id);

CREATE INDEX raw_device_acl_ip_nets_includes_idx_ip_net_set_namespace
ON raw_device_acl_ip_nets_includes(ip_net_set_namespace);

CREATE INDEX raw_device_acl_ip_nets_includes_idx_ip_net_set_id
ON raw_device_acl_ip_nets_includes(ip_net_set_id);

CREATE INDEX raw_device_acl_ip_nets_includes_idx_included_namespace
ON raw_device_acl_ip_nets_includes(included_namespace);

CREATE INDEX raw_device_acl_ip_nets_includes_idx_included_id
ON raw_device_acl_ip_nets_includes(included_id);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_acl_ip_nets_includes_idx_views
ON raw_device_acl_ip_nets_includes(
    device_id, ip_net_set_namespace, ip_net_set_id, included_namespace
  , included_id
);

CREATE INDEX raw_device_acl_ip_nets_includes_idx_views_fkey
ON raw_device_acl_ip_nets_includes(
    device_id, ip_net_set_namespace, ip_net_set_id
);


-- ----------------------------------------------------------------------

CREATE TABLE raw_device_acl_ports_bases (
    tool_run_id             UUID        NOT NULL
  , device_id               TEXT        NOT NULL
  , port_set_id             TEXT        NOT NULL
  , PRIMARY KEY (tool_run_id, device_id, port_set_id)
  , FOREIGN KEY (tool_run_id, device_id)
        REFERENCES raw_devices(tool_run_id, device_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
CREATE INDEX raw_device_acl_ports_bases_idx_tool_run_id
ON raw_device_acl_ports_bases(tool_run_id);

CREATE INDEX raw_device_acl_ports_bases_idx_device_id
ON raw_device_acl_ports_bases(device_id);

CREATE INDEX raw_device_acl_ports_bases_idx_port_set_id
ON raw_device_acl_ports_bases(port_set_id);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_acl_ports_bases_idx_views
ON raw_device_acl_ports_bases(device_id, port_set_id);


-- ----------------------------------------------------------------------

CREATE TABLE raw_device_acl_ports_ports (
    tool_run_id             UUID        NOT NULL
  , device_id               TEXT        NOT NULL
  , port_set_id             TEXT        NOT NULL
  , port_range              PortRange   NOT NULL
  , PRIMARY KEY (tool_run_id, device_id, port_set_id, port_range)
  , FOREIGN KEY (tool_run_id, device_id, port_set_id)
        REFERENCES raw_device_acl_ports_bases(
              tool_run_id, device_id, port_set_id
          )
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
CREATE INDEX raw_device_acl_ports_ports_idx_tool_run_id
ON raw_device_acl_ports_ports(tool_run_id);

CREATE INDEX raw_device_acl_ports_ports_idx_device_id
ON raw_device_acl_ports_ports(device_id);

CREATE INDEX raw_device_acl_ports_ports_idx_port_set_id
ON raw_device_acl_ports_ports(port_set_id);

CREATE INDEX raw_device_acl_ports_ports_idx_port_range
ON raw_device_acl_ports_ports(port_range);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_acl_ports_ports_idx_views
ON raw_device_acl_ports_ports(device_id, port_set_id, port_range);

CREATE INDEX raw_device_acl_ports_ports_idx_views_fkey
ON raw_device_acl_ports_ports(device_id, port_set_id);


-- ----------------------------------------------------------------------

CREATE TABLE raw_device_acl_ports_includes (
    tool_run_id             UUID        NOT NULL
  , device_id               TEXT        NOT NULL
  , port_set_id             TEXT        NOT NULL
  , included_id             TEXT        NOT NULL
  , PRIMARY KEY (tool_run_id, device_id, port_set_id, included_id)
  , FOREIGN KEY (tool_run_id, device_id, port_set_id)
        REFERENCES raw_device_acl_ports_bases(
              tool_run_id, device_id, port_set_id
          )
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
CREATE INDEX raw_device_acl_ports_includes_idx_tool_run_id
ON raw_device_acl_ports_includes(tool_run_id);

CREATE INDEX raw_device_acl_ports_includes_idx_device_id
ON raw_device_acl_ports_includes(device_id);

CREATE INDEX raw_device_acl_ports_includes_idx_port_set_id
ON raw_device_acl_ports_includes(port_set_id);

CREATE INDEX raw_device_acl_ports_includes_idx_included_id
ON raw_device_acl_ports_includes(included_id);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_acl_ports_includes_idx_views
ON raw_device_acl_ports_includes(device_id, port_set_id, included_id);

CREATE INDEX raw_device_acl_ports_includes_idx_views_fkey
ON raw_device_acl_ports_includes(device_id, port_set_id);


-- ----------------------------------------------------------------------

CREATE TABLE raw_device_acl_services_bases (
    tool_run_id             UUID        NOT NULL
  , device_id               TEXT        NOT NULL
  , service_id              TEXT        NOT NULL
  , PRIMARY KEY (tool_run_id, device_id, service_id)
  , FOREIGN KEY (tool_run_id, device_id)
        REFERENCES raw_devices(tool_run_id, device_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
CREATE INDEX raw_device_acl_services_bases_idx_tool_run_id
ON raw_device_acl_services_bases(tool_run_id);

CREATE INDEX raw_device_acl_services_bases_idx_device_id
ON raw_device_acl_services_bases(device_id);

CREATE INDEX raw_device_acl_services_bases_idx_service_id
ON raw_device_acl_services_bases(service_id);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_acl_services_bases_idx_views
ON raw_device_acl_services_bases(device_id, service_id);


-- ----------------------------------------------------------------------

CREATE TABLE raw_device_acl_services_protocols (
    tool_run_id             UUID        NOT NULL
  , device_id               TEXT        NOT NULL
  , service_id              TEXT        NOT NULL
  , protocol                TEXT        NOT NULL
  , PRIMARY KEY (tool_run_id, device_id, service_id, protocol)
  , FOREIGN KEY (tool_run_id, device_id, service_id)
        REFERENCES raw_device_acl_services_bases(
              tool_run_id, device_id, service_id
          )
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
CREATE INDEX raw_device_acl_services_protocols_idx_tool_run_id
ON raw_device_acl_services_protocols(tool_run_id);

CREATE INDEX raw_device_acl_services_protocols_idx_device_id
ON raw_device_acl_services_protocols(device_id);

CREATE INDEX raw_device_acl_services_protocols_idx_service_id
ON raw_device_acl_services_protocols(service_id);

CREATE INDEX raw_device_acl_services_protocols_idx_protocol
ON raw_device_acl_services_protocols(protocol);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_acl_services_protocols_idx_views
ON raw_device_acl_services_protocols(device_id, service_id, protocol);

CREATE INDEX raw_device_acl_services_protocols_idx_views_fkey
ON raw_device_acl_services_protocols(device_id, service_id);


-- ----------------------------------------------------------------------

CREATE TABLE raw_device_acl_services_ports (
    tool_run_id             UUID        NOT NULL
  , device_id               TEXT        NOT NULL
  , service_id              TEXT        NOT NULL
  , protocol                TEXT        NOT NULL
  , src_port_range          PortRange   NOT NULL
  , dst_port_range          PortRange   NOT NULL
  , PRIMARY KEY ( tool_run_id, device_id, service_id, protocol
                , src_port_range, dst_port_range
                )
  , FOREIGN KEY (tool_run_id, device_id, service_id, protocol)
        REFERENCES raw_device_acl_services_protocols(
              tool_run_id, device_id, service_id, protocol
          )
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
CREATE INDEX raw_device_acl_services_ports_idx_tool_run_id
ON raw_device_acl_services_ports(tool_run_id);

CREATE INDEX raw_device_acl_services_ports_idx_device_id
ON raw_device_acl_services_ports(device_id);

CREATE INDEX raw_device_acl_services_ports_idx_service_id
ON raw_device_acl_services_ports(service_id);

CREATE INDEX raw_device_acl_services_ports_idx_protocol
ON raw_device_acl_services_ports(protocol);

CREATE INDEX raw_device_acl_services_ports_idx_src_port_range
ON raw_device_acl_services_ports(src_port_range);

CREATE INDEX raw_device_acl_services_ports_idx_dst_port_range
ON raw_device_acl_services_ports(dst_port_range);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_acl_services_ports_idx_views
ON raw_device_acl_services_ports(
    device_id, service_id, protocol, src_port_range, dst_port_range
);

CREATE INDEX raw_device_acl_services_ports_idx_views_fkey
ON raw_device_acl_services_ports(device_id, service_id, protocol);


-- ----------------------------------------------------------------------

CREATE TABLE raw_device_acl_services_includes (
    tool_run_id             UUID        NOT NULL
  , device_id               TEXT        NOT NULL
  , service_id              TEXT        NOT NULL
  , included_id             TEXT        NOT NULL
  , PRIMARY KEY (tool_run_id, device_id, service_id, included_id)
  , FOREIGN KEY (tool_run_id, device_id, service_id)
        REFERENCES raw_device_acl_services_bases(
              tool_run_id, device_id, service_id
          )
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
CREATE INDEX raw_device_acl_services_includes_idx_tool_run_id
ON raw_device_acl_services_includes(tool_run_id);

CREATE INDEX raw_device_acl_services_includes_idx_device_id
ON raw_device_acl_services_includes(device_id);

CREATE INDEX raw_device_acl_services_includes_idx_service_id
ON raw_device_acl_services_includes(service_id);

CREATE INDEX raw_device_acl_services_includes_idx_included_id
ON raw_device_acl_services_includes(included_id);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_acl_services_includes_idx_views
ON raw_device_acl_services_includes(device_id, service_id, included_id);

CREATE INDEX raw_device_acl_services_includes_idx_views_fkey
ON raw_device_acl_services_includes(device_id, service_id);


-- ----------------------------------------------------------------------
-- Table for port-based rules
-- ----------------------------------------------------------------------

CREATE TABLE raw_device_acl_rules_ports (
    tool_run_id                 UUID        NOT NULL
  , device_id                   TEXT        NOT NULL
  , priority                    INT         NOT NULL
  , action                      TEXT        NOT NULL
  , incoming_zone_id            TEXT        NOT NULL
  , outgoing_zone_id            TEXT        NOT NULL
  , src_ip_net_set_namespace    TEXT        NOT NULL
  , src_ip_net_set_id           TEXT        NOT NULL
  , dst_ip_net_set_namespace    TEXT        NOT NULL
  , dst_ip_net_set_id           TEXT        NOT NULL
  , protocol                    TEXT        NOT NULL
  , src_port_set_id             TEXT        NOT NULL
  , dst_port_set_id             TEXT        NOT NULL
  , description                 TEXT        NULL
  , PRIMARY KEY ( tool_run_id, device_id, priority, action
                , incoming_zone_id, outgoing_zone_id
                , src_ip_net_set_namespace, src_ip_net_set_id
                , dst_ip_net_set_namespace, dst_ip_net_set_id
                , protocol, src_port_set_id, dst_port_set_id
                )
  , FOREIGN KEY
        (tool_run_id, device_id, incoming_zone_id)
        REFERENCES raw_device_acl_zones_bases
            (tool_run_id, device_id, zone_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
  , FOREIGN KEY
        (tool_run_id, device_id, outgoing_zone_id)
        REFERENCES raw_device_acl_zones_bases
            (tool_run_id, device_id, zone_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
  , FOREIGN KEY
        (tool_run_id, device_id, src_ip_net_set_namespace, src_ip_net_set_id)
        REFERENCES raw_device_acl_ip_nets_bases
            (tool_run_id, device_id, ip_net_set_namespace, ip_net_set_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
  , FOREIGN KEY
        (tool_run_id, device_id, dst_ip_net_set_namespace, dst_ip_net_set_id)
        REFERENCES raw_device_acl_ip_nets_bases
            (tool_run_id, device_id, ip_net_set_namespace, ip_net_set_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
  , FOREIGN KEY (tool_run_id, device_id, src_port_set_id)
        REFERENCES raw_device_acl_ports_bases
            (tool_run_id, device_id, port_set_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
  , FOREIGN KEY (tool_run_id, device_id, dst_port_set_id)
        REFERENCES raw_device_acl_ports_bases
            (tool_run_id, device_id, port_set_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
CREATE INDEX raw_device_acl_rules_ports_idx_tool_run_id
ON raw_device_acl_rules_ports(tool_run_id);

CREATE INDEX raw_device_acl_rules_ports_idx_device_id
ON raw_device_acl_rules_ports(device_id);

CREATE INDEX raw_device_acl_rules_ports_idx_priority
ON raw_device_acl_rules_ports(priority);

CREATE INDEX raw_device_acl_rules_ports_idx_action
ON raw_device_acl_rules_ports(action);

CREATE INDEX raw_device_acl_rules_ports_idx_incoming_zone_id
ON raw_device_acl_rules_ports(incoming_zone_id);

CREATE INDEX raw_device_acl_rules_ports_idx_outgoing_zone_id
ON raw_device_acl_rules_ports(outgoing_zone_id);

CREATE INDEX raw_device_acl_rules_ports_idx_src_ip_net_set_namespace
ON raw_device_acl_rules_ports(src_ip_net_set_namespace);

CREATE INDEX raw_device_acl_rules_ports_idx_src_ip_net_set_id
ON raw_device_acl_rules_ports(src_ip_net_set_id);

CREATE INDEX raw_device_acl_rules_ports_idx_dst_ip_net_set_namespace
ON raw_device_acl_rules_ports(dst_ip_net_set_namespace);

CREATE INDEX raw_device_acl_rules_ports_idx_dst_ip_net_set_id
ON raw_device_acl_rules_ports(dst_ip_net_set_id);

CREATE INDEX raw_device_acl_rules_ports_idx_protocol
ON raw_device_acl_rules_ports(protocol);

CREATE INDEX raw_device_acl_rules_ports_idx_src_port_set_id
ON raw_device_acl_rules_ports(src_port_set_id);

CREATE INDEX raw_device_acl_rules_ports_idx_dst_port_set_id
ON raw_device_acl_rules_ports(dst_port_set_id);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_acl_rules_ports_idx_views
ON raw_device_acl_rules_ports(
    device_id, priority, action
  , incoming_zone_id, outgoing_zone_id
  , src_ip_net_set_namespace, src_ip_net_set_id
  , dst_ip_net_set_namespace, dst_ip_net_set_id
  , protocol, src_port_set_id, dst_port_set_id
);

CREATE INDEX raw_device_acl_rules_ports_idx_views_fkey_incoming_zone
ON raw_device_acl_rules_ports(device_id, incoming_zone_id);

CREATE INDEX raw_device_acl_rules_ports_idx_views_fkey_outgoing_zone
ON raw_device_acl_rules_ports(device_id, outgoing_zone_id);

CREATE INDEX raw_device_acl_rules_ports_idx_views_fkey_src_ip_net_set_namespace
ON raw_device_acl_rules_ports(device_id, src_ip_net_set_namespace);

CREATE INDEX raw_device_acl_rules_ports_idx_views_fkey_src_ip_net_set_id
ON raw_device_acl_rules_ports(device_id, src_ip_net_set_id);

CREATE INDEX raw_device_acl_rules_ports_idx_views_fkey_dst_ip_net_set_namespace
ON raw_device_acl_rules_ports(device_id, dst_ip_net_set_namespace);

CREATE INDEX raw_device_acl_rules_ports_idx_views_fkey_dst_ip_net_set_id
ON raw_device_acl_rules_ports(device_id, dst_ip_net_set_id);

CREATE INDEX raw_device_acl_rules_ports_idx_views_fkey_src_port
ON raw_device_acl_rules_ports(device_id, src_port_set_id);

CREATE INDEX raw_device_acl_rules_ports_idx_views_fkey_dst_port
ON raw_device_acl_rules_ports(device_id, dst_port_set_id);


-- ----------------------------------------------------------------------
-- Table for service-based rules
-- ----------------------------------------------------------------------

CREATE TABLE raw_device_acl_rules_services (
    tool_run_id                 UUID        NOT NULL
  , device_id                   TEXT        NOT NULL
  , priority                    INT         NOT NULL
  , action                      TEXT        NOT NULL
  , incoming_zone_id            TEXT        NOT NULL
  , outgoing_zone_id            TEXT        NOT NULL
  , src_ip_net_set_namespace    TEXT        NOT NULL
  , src_ip_net_set_id           TEXT        NOT NULL
  , dst_ip_net_set_namespace    TEXT        NOT NULL
  , dst_ip_net_set_id           TEXT        NOT NULL
  , service_id                  TEXT        NOT NULL
  , description                 TEXT        NULL
  , PRIMARY KEY ( tool_run_id, device_id, priority, action
                , incoming_zone_id, outgoing_zone_id
                , src_ip_net_set_namespace, src_ip_net_set_id
                , dst_ip_net_set_namespace, dst_ip_net_set_id
                , service_id
                )
  , FOREIGN KEY
        (tool_run_id, device_id, incoming_zone_id)
        REFERENCES raw_device_acl_zones_bases
            (tool_run_id, device_id, zone_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
  , FOREIGN KEY
        (tool_run_id, device_id, outgoing_zone_id)
        REFERENCES raw_device_acl_zones_bases
            (tool_run_id, device_id, zone_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
  , FOREIGN KEY
        (tool_run_id, device_id, src_ip_net_set_namespace, src_ip_net_set_id)
        REFERENCES raw_device_acl_ip_nets_bases
            (tool_run_id, device_id, ip_net_set_namespace, ip_net_set_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
  , FOREIGN KEY
        (tool_run_id, device_id, dst_ip_net_set_namespace, dst_ip_net_set_id)
        REFERENCES raw_device_acl_ip_nets_bases
            (tool_run_id, device_id, ip_net_set_namespace, ip_net_set_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
    -- No foreign key for services yet.
    -- junos-default services are currently under a different tool_run_id
    -- and so don't match up with their use in the runtime config.
    --FOREIGN KEY (tool_run_id, device_id, service_id)
    --    REFERENCES raw_device_acl_services_bases(tool_run_id, device_id, service_id)
    --    ON DELETE CASCADE
    --    ON UPDATE CASCADE
);

-- Partial indexes
CREATE INDEX raw_device_acl_rules_services_idx_tool_run_id
ON raw_device_acl_rules_services(tool_run_id);

CREATE INDEX raw_device_acl_rules_services_idx_device_id
ON raw_device_acl_rules_services(device_id);

CREATE INDEX raw_device_acl_rules_services_idx_priority
ON raw_device_acl_rules_services(priority);

CREATE INDEX raw_device_acl_rules_services_idx_action
ON raw_device_acl_rules_services(action);

CREATE INDEX raw_device_acl_rules_services_idx_incoming_zone_id
ON raw_device_acl_rules_services(incoming_zone_id);

CREATE INDEX raw_device_acl_rules_services_idx_outgoing_zone_id
ON raw_device_acl_rules_services(outgoing_zone_id);

CREATE INDEX raw_device_acl_rules_services_idx_src_ip_net_set_namespace
ON raw_device_acl_rules_services(src_ip_net_set_namespace);

CREATE INDEX raw_device_acl_rules_services_idx_src_ip_net_set_id
ON raw_device_acl_rules_services(src_ip_net_set_id);

CREATE INDEX raw_device_acl_rules_services_idx_dst_ip_net_set_namespace
ON raw_device_acl_rules_services(dst_ip_net_set_namespace);

CREATE INDEX raw_device_acl_rules_services_idx_dst_ip_net_set_id
ON raw_device_acl_rules_services(dst_ip_net_set_id);

CREATE INDEX raw_device_acl_rules_services_idx_service_id
ON raw_device_acl_rules_services(service_id);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_acl_rules_services_idx_views
ON raw_device_acl_rules_services(
    device_id, priority, action
  , incoming_zone_id, outgoing_zone_id
  , src_ip_net_set_namespace, src_ip_net_set_id
  , dst_ip_net_set_namespace, dst_ip_net_set_id
  , service_id
);

CREATE INDEX raw_device_acl_rules_services_idx_views_fkey_incoming_zone
ON raw_device_acl_rules_services(device_id, incoming_zone_id);

CREATE INDEX raw_device_acl_rules_services_idx_views_fkey_outgoing_zone
ON raw_device_acl_rules_services(device_id, outgoing_zone_id);

CREATE INDEX raw_device_acl_rules_services_idx_views_fkey_src_ip_net_set_namespace
ON raw_device_acl_rules_services(device_id, src_ip_net_set_namespace);

CREATE INDEX raw_device_acl_rules_services_idx_views_fkey_src_ip_net_set_id
ON raw_device_acl_rules_services(device_id, src_ip_net_set_id);

CREATE INDEX raw_device_acl_rules_services_idx_views_fkey_dst_ip_net_set_namespace
ON raw_device_acl_rules_services(device_id, dst_ip_net_set_namespace);

CREATE INDEX raw_device_acl_rules_services_idx_views_fkey_dst_ip_net_set_id
ON raw_device_acl_rules_services(device_id, dst_ip_net_set_id);

CREATE INDEX raw_device_acl_rules_services_idx_views_fkey_service
ON raw_device_acl_rules_services(device_id, service_id);


-- ----------------------------------------------------------------------
-- Device VLAN tables
-- ----------------------------------------------------------------------

CREATE TABLE raw_device_vlans (
    tool_run_id                 UUID            NOT NULL
  , device_id                   TEXT            NOT NULL
  , vlan                        VlanNumber      NOT NULL
  , description                 TEXT            NULL
  , PRIMARY KEY (tool_run_id, device_id, vlan)
  , FOREIGN KEY (tool_run_id, device_id)
        REFERENCES raw_devices(tool_run_id, device_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
CREATE INDEX raw_device_vlans_idx_tool_run_id
ON raw_device_vlans(tool_run_id);

CREATE INDEX raw_device_vlans_idx_device_id
ON raw_device_vlans(device_id);

CREATE INDEX raw_device_vlans_idx_vlan
ON raw_device_vlans(vlan);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_vlans_idx_views
ON raw_device_vlans(device_id, vlan);

-- ----------------------------------------------------------------------

CREATE TABLE raw_device_vlans_ip_nets (
    tool_run_id                 UUID            NOT NULL
  , device_id                   TEXT            NOT NULL
  , vlan                        VlanNumber      NOT NULL
  , ip_net                      CIDR            NOT NULL
  , PRIMARY KEY (tool_run_id, device_id, vlan, ip_net)
  , FOREIGN KEY (tool_run_id, device_id, vlan)
        REFERENCES raw_device_vlans(tool_run_id, device_id, vlan)
        ON DELETE CASCADE
        ON UPDATE CASCADE
  , FOREIGN KEY (tool_run_id, ip_net)
        REFERENCES raw_ip_nets(tool_run_id, ip_net)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
CREATE INDEX raw_device_vlans_ip_nets_idx_tool_run_id
ON raw_device_vlans_ip_nets(tool_run_id);

CREATE INDEX raw_device_vlans_ip_nets_idx_device_id
ON raw_device_vlans_ip_nets(device_id);

CREATE INDEX raw_device_vlans_ip_nets_idx_vlan
ON raw_device_vlans_ip_nets(vlan);

CREATE INDEX raw_device_vlans_ip_nets_idx_ip_net
ON raw_device_vlans_ip_nets(ip_net);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_vlans_ip_nets_idx_views
ON raw_device_vlans_ip_nets(device_id, vlan, ip_net);

-- ----------------------------------------------------------------------

CREATE TABLE raw_device_interfaces_vlans (
    tool_run_id                 UUID            NOT NULL
  , device_id                   TEXT            NOT NULL
  , interface_name              TEXT            NOT NULL
  , vlan                        VlanNumber      NOT NULL
  , PRIMARY KEY (tool_run_id, device_id, interface_name, vlan)
  , FOREIGN KEY (tool_run_id, device_id, interface_name)
        REFERENCES raw_device_interfaces
                   (tool_run_id, device_id, interface_name)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
CREATE INDEX raw_device_interfaces_vlans_idx_tool_run_id
ON raw_device_interfaces_vlans(tool_run_id);

CREATE INDEX raw_device_interfaces_vlans_idx_device_id
ON raw_device_interfaces_vlans(device_id);

CREATE INDEX raw_device_interfaces_vlans_idx_interface_name
ON raw_device_interfaces_vlans(interface_name);

CREATE INDEX raw_device_interfaces_vlans_idx_vlan
ON raw_device_interfaces_vlans(vlan);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_interfaces_vlans_idx_views
ON raw_device_interfaces_vlans(device_id, interface_name, vlan);


-- ----------------------------------------------------------------------

COMMIT TRANSACTION;
