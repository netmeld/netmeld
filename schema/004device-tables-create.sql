-- =============================================================================
-- Copyright 2017 National Technology & Engineering Solutions of Sandia, LLC
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
    tool_run_id                 UUID            NOT NULL,
    device_id                   TEXT            NOT NULL,
    PRIMARY KEY (tool_run_id, device_id),
    FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);


-- ----------------------------------------------------------------------

CREATE TABLE raw_device_virtualizations (
    tool_run_id                 UUID            NOT NULL,
    host_device_id              TEXT            NOT NULL,
    guest_device_id             TEXT            NOT NULL,
    PRIMARY KEY (tool_run_id, host_device_id, guest_device_id),
    FOREIGN KEY (tool_run_id, host_device_id)
        REFERENCES raw_devices(tool_run_id, device_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE,
    FOREIGN KEY (tool_run_id, guest_device_id)
        REFERENCES raw_devices(tool_run_id, device_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

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

CREATE TABLE raw_device_types (
    tool_run_id                 UUID            NOT NULL,
    device_id                   TEXT            NOT NULL,
    device_type                 TEXT            NOT NULL,
    PRIMARY KEY (tool_run_id, device_id, device_type),
    FOREIGN KEY (tool_run_id, device_id)
        REFERENCES raw_devices(tool_run_id, device_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- ----------------------------------------------------------------------

CREATE TABLE raw_device_hardware (
    tool_run_id                 UUID            NOT NULL,
    device_id                   TEXT            NOT NULL,
    vendor                      TEXT            NOT NULL,
    model                       TEXT            NULL,
    hardware_revision           TEXT            NULL,
    serial_number               TEXT            NULL,
    description                 TEXT            NULL,
    PRIMARY KEY (tool_run_id, 
                 device_id, vendor),
--                 model, hardware_revision, serial_number, description),
    FOREIGN KEY (tool_run_id, device_id)
        REFERENCES raw_devices(tool_run_id, device_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- ----------------------------------------------------------------------

CREATE TABLE device_colors (
    device_id                   TEXT            NOT NULL,
    color                       TEXT            NOT NULL,
    PRIMARY KEY (device_id)
);


-- ----------------------------------------------------------------------

CREATE TABLE device_extra_weights (
    device_id                   TEXT            NOT NULL,
    extra_weight                FLOAT           NOT NULL,
    PRIMARY KEY (device_id)
);


-- ----------------------------------------------------------------------
-- Interfaces from target devices.
-- ----------------------------------------------------------------------

CREATE TABLE raw_device_interfaces (
    tool_run_id                 UUID            NOT NULL,
    device_id                   TEXT            NOT NULL,
    interface_name              TEXT            NOT NULL,
    media_type                  TEXT            NOT NULL,
    is_up                       BOOLEAN         NOT NULL,
    PRIMARY KEY (tool_run_id, device_id, interface_name),
    FOREIGN KEY (tool_run_id, device_id)
        REFERENCES raw_devices(tool_run_id, device_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

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
-- MAC addresses from target devices.
-- ----------------------------------------------------------------------

CREATE TABLE raw_device_mac_addrs (
    tool_run_id                 UUID            NOT NULL,
    device_id                   TEXT            NOT NULL,
    interface_name              TEXT            NOT NULL,
    mac_addr                    MACADDR         NOT NULL,
    PRIMARY KEY (tool_run_id, device_id, interface_name, mac_addr),
    FOREIGN KEY (tool_run_id, device_id, interface_name)
        REFERENCES raw_device_interfaces
                   (tool_run_id, device_id, interface_name)
        ON DELETE CASCADE
        ON UPDATE CASCADE,
    FOREIGN KEY (tool_run_id, mac_addr)
        REFERENCES raw_mac_addrs(tool_run_id, mac_addr)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

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
    tool_run_id                 UUID            NOT NULL,
    device_id                   TEXT            NOT NULL,
    interface_name              TEXT            NOT NULL,
    ip_addr                     INET            NOT NULL,
    PRIMARY KEY (tool_run_id, device_id, interface_name, ip_addr),
    FOREIGN KEY (tool_run_id, device_id, interface_name)
        REFERENCES raw_device_interfaces
                   (tool_run_id, device_id, interface_name)
        ON DELETE CASCADE
        ON UPDATE CASCADE,
    FOREIGN KEY (tool_run_id, ip_addr)
        REFERENCES raw_ip_addrs(tool_run_id, ip_addr)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

CREATE INDEX raw_device_ip_addrs_idx_tool_run_id
ON raw_device_ip_addrs(tool_run_id);

CREATE INDEX raw_device_ip_addrs_idx_device_id
ON raw_device_ip_addrs(device_id);

CREATE INDEX raw_device_ip_addrs_idx_interface_name
ON raw_device_ip_addrs(interface_name);

CREATE INDEX raw_device_ip_addrs_idx_ip_addr
ON raw_device_ip_addrs(ip_addr);

CREATE INDEX raw_device_ip_addrs_idx_device_id_interface_name
ON raw_device_ip_addrs(device_id, interface_name);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_ip_addrs_idx_views
ON raw_device_ip_addrs(device_id, interface_name, ip_addr);


-- ----------------------------------------------------------------------
-- Routing tables from target devices (config files or console access)
-- ----------------------------------------------------------------------

-- dst_ip_net  = '0.0.0.0/0' or '::/0' for the default route.

-- rtr_ip_addr = '0.0.0.0'   or '::'   for directly connected LANs.
-- rtr_ip_addr is NULL                 for "null" routes (not routed).

-- Don't add foreign keys to raw_ip_addrs or raw_ip_nets:
-- * NULL rtr_ip_addr values can't be foreign key into the ip_addrs table.
-- * dst_ip_net is likely to be aggregated networks that don't
--   accurately correspond to actual ip_nets.

CREATE TABLE raw_device_ip_routes (
    tool_run_id                 UUID            NOT NULL,
    device_id                   TEXT            NOT NULL,
    interface_name              TEXT            NULL,
    dst_ip_net                  CIDR            NOT NULL,
    rtr_ip_addr                 INET            NULL,
    FOREIGN KEY (tool_run_id, device_id)
        REFERENCES raw_devices(tool_run_id, device_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE,
    CHECK (family(dst_ip_net) = family(rtr_ip_addr)),
    CHECK ((rtr_ip_addr = host(rtr_ip_addr)::INET))
);

CREATE INDEX raw_device_ip_routes_idx_tool_run_id
ON raw_device_ip_routes(tool_run_id);

CREATE INDEX raw_device_ip_routes_idx_device_id
ON raw_device_ip_routes(device_id);

CREATE INDEX raw_device_ip_routes_idx_interface_name
ON raw_device_ip_routes(interface_name);

CREATE INDEX raw_device_ip_routes_idx_dst_ip_net
ON raw_device_ip_routes(dst_ip_net);

CREATE INDEX raw_device_ip_routes_idx_rtr_ip_addr
ON raw_device_ip_routes(rtr_ip_addr);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_ip_routes_idx_views
ON raw_device_ip_routes(device_id, dst_ip_net, rtr_ip_addr);


-- ----------------------------------------------------------------------
-- Server tables from target devices (config files or console access)
-- Stores device's configured DNS, DHCP, WINS, NTP, ... servers
-- ----------------------------------------------------------------------

CREATE TABLE raw_device_ip_servers (
    tool_run_id                 UUID            NOT NULL,
    device_id                   TEXT            NOT NULL,
    interface_name              TEXT            NULL,
    service_name                TEXT            NOT NULL,
    server_ip_addr              INET            NOT NULL,
    FOREIGN KEY (tool_run_id, device_id)
        REFERENCES raw_devices(tool_run_id, device_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

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

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_ip_servers_idx_views
ON raw_device_ip_servers(device_id, service_name, server_ip_addr);


-- ----------------------------------------------------------------------

CREATE TABLE raw_device_phys_connections (
    tool_run_id                 UUID            NOT NULL,
    self_device_id              TEXT            NOT NULL,
    self_interface_name         TEXT            NOT NULL,
    peer_device_id              TEXT            NOT NULL,
    peer_interface_name         TEXT            NOT NULL,
    PRIMARY KEY (tool_run_id,
                 self_device_id, self_interface_name,
                 peer_device_id, peer_interface_name),
    FOREIGN KEY (tool_run_id, self_device_id, self_interface_name)
        REFERENCES raw_device_interfaces
                   (tool_run_id, device_id, interface_name)
        ON DELETE CASCADE
        ON UPDATE CASCADE,
    FOREIGN KEY (tool_run_id, peer_device_id)
        REFERENCES raw_devices(tool_run_id, device_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

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
ON raw_device_phys_connections(self_device_id, self_interface_name,
                           peer_device_id, peer_interface_name);


-- ----------------------------------------------------------------------

CREATE TABLE raw_device_link_connections (
    tool_run_id                 UUID            NOT NULL,
    self_device_id              TEXT            NOT NULL,
    self_interface_name         TEXT            NOT NULL,
    peer_mac_addr               MACADDR         NOT NULL,
    PRIMARY KEY (tool_run_id,
                 self_device_id, self_interface_name,
                 peer_mac_addr),
    FOREIGN KEY (tool_run_id, self_device_id, self_interface_name)
        REFERENCES raw_device_interfaces
                   (tool_run_id, device_id, interface_name)
        ON DELETE CASCADE
        ON UPDATE CASCADE,
    FOREIGN KEY (tool_run_id, peer_mac_addr)
        REFERENCES raw_mac_addrs(tool_run_id, mac_addr)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

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
    tool_run_id                 UUID            NOT NULL,
    device_id                   TEXT            NOT NULL,
    aaa_command                 TEXT            NOT NULL,
    PRIMARY KEY (tool_run_id, device_id, aaa_command),
    FOREIGN KEY (tool_run_id, device_id)
        REFERENCES raw_devices(tool_run_id, device_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

CREATE INDEX raw_devices_aaa_idx_tool_run_id
ON raw_devices_aaa(tool_run_id);

CREATE INDEX raw_devices_aaa_idx_device_id
ON raw_devices_aaa(device_id);


-- ----------------------------------------------------------------------

CREATE TABLE raw_devices_bpdu (
    tool_run_id                 UUID            NOT NULL,
    device_id                   TEXT            NOT NULL,
    is_bpduguard_enabled        BOOLEAN         NOT NULL,
    is_bpdufilter_enabled       BOOLEAN         NOT NULL,
    PRIMARY KEY (tool_run_id, device_id),
    FOREIGN KEY (tool_run_id, device_id)
        REFERENCES raw_devices(tool_run_id, device_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

CREATE INDEX raw_devices_bpdu_idx_tool_run_id
ON raw_devices_bpdu(tool_run_id);

CREATE INDEX raw_devices_bpdu_idx_device_id
ON raw_devices_bpdu(device_id);

CREATE INDEX raw_devices_bpdu_idx_is_bpduguard_enabled
ON raw_devices_bpdu(is_bpduguard_enabled);

CREATE INDEX raw_devices_bpdu_idx_is_bpdufilter_enabled
ON raw_devices_bpdu(is_bpdufilter_enabled);


-- ----------------------------------------------------------------------

CREATE TABLE raw_devices_cdp (
    tool_run_id                 UUID            NOT NULL,
    device_id                   TEXT            NOT NULL,
    is_cdp_enabled              BOOLEAN         NOT NULL,
    PRIMARY KEY (tool_run_id, device_id),
    FOREIGN KEY (tool_run_id, device_id)
        REFERENCES raw_devices(tool_run_id, device_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

CREATE INDEX raw_devices_cdp_idx_tool_run_id
ON raw_devices_cdp(tool_run_id);

CREATE INDEX raw_devices_cdp_idx_device_id
ON raw_devices_cdp(device_id);

CREATE INDEX raw_devices_cdp_idx_is_cdp_enabled
ON raw_devices_cdp(is_cdp_enabled);


-- ----------------------------------------------------------------------

CREATE TABLE raw_device_interfaces_mode (
    tool_run_id                 UUID            NOT NULL,
    device_id                   TEXT            NOT NULL,
    interface_name              TEXT            NOT NULL,
    interface_mode              TEXT            NOT NULL,
    PRIMARY KEY (tool_run_id, device_id, interface_name),
    FOREIGN KEY (tool_run_id, device_id, interface_name)
        REFERENCES raw_device_interfaces
                   (tool_run_id, device_id, interface_name)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

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

CREATE TABLE raw_device_interfaces_vlans (
    tool_run_id                 UUID            NOT NULL,
    device_id                   TEXT            NOT NULL,
    interface_name              TEXT            NOT NULL,
    vlan_range                  INT4RANGE       NOT NULL,
    PRIMARY KEY (tool_run_id, device_id, interface_name, vlan_range),
    FOREIGN KEY (tool_run_id, device_id, interface_name)
        REFERENCES raw_device_interfaces
                   (tool_run_id, device_id, interface_name)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

CREATE INDEX raw_device_interfaces_vlans_idx_tool_run_id
ON raw_device_interfaces_vlans(tool_run_id);

CREATE INDEX raw_device_interfaces_vlans_idx_device_id
ON raw_device_interfaces_vlans(device_id);

CREATE INDEX raw_device_interfaces_vlans_idx_interface_name
ON raw_device_interfaces_vlans(interface_name);

CREATE INDEX raw_device_interfaces_vlans_idx_vlan_range
ON raw_device_interfaces_vlans(vlan_range);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_interfaces_vlans_idx_views
ON raw_device_interfaces_vlans(device_id, interface_name, vlan_range);


-- ----------------------------------------------------------------------

CREATE TABLE raw_device_interfaces_bpdu (
    tool_run_id                 UUID            NOT NULL,
    device_id                   TEXT            NOT NULL,
    interface_name              TEXT            NOT NULL,
    is_bpduguard_enabled        BOOLEAN         NOT NULL,
    is_bpdufilter_enabled       BOOLEAN         NOT NULL,
    PRIMARY KEY (tool_run_id, device_id, interface_name),
    FOREIGN KEY (tool_run_id, device_id, interface_name)
        REFERENCES raw_device_interfaces
                   (tool_run_id, device_id, interface_name)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

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
    tool_run_id                 UUID            NOT NULL,
    device_id                   TEXT            NOT NULL,
    interface_name              TEXT            NOT NULL,
    is_cdp_enabled              BOOLEAN         NOT NULL,
    PRIMARY KEY (tool_run_id, device_id, interface_name),
    FOREIGN KEY (tool_run_id, device_id, interface_name)
        REFERENCES raw_device_interfaces
                   (tool_run_id, device_id, interface_name)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

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
    tool_run_id                 UUID            NOT NULL,
    device_id                   TEXT            NOT NULL,
    interface_name              TEXT            NOT NULL,
    is_portfast_enabled         BOOLEAN         NOT NULL,
    PRIMARY KEY (tool_run_id, device_id, interface_name),
    FOREIGN KEY (tool_run_id, device_id, interface_name)
        REFERENCES raw_device_interfaces
                   (tool_run_id, device_id, interface_name)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

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
    tool_run_id                 UUID            NOT NULL,
    device_id                   TEXT            NOT NULL,
    interface_name              TEXT            NOT NULL,
    is_port_security_enabled    BOOLEAN         NOT NULL,
    is_mac_addr_sticky          BOOLEAN         NOT NULL,
    max_mac_addrs               INT             NOT NULL,
    violation_action            TEXT            NOT NULL,
    PRIMARY KEY (tool_run_id, device_id, interface_name),
    FOREIGN KEY (tool_run_id, device_id, interface_name)
        REFERENCES raw_device_interfaces
                   (tool_run_id, device_id, interface_name)
        ON DELETE CASCADE
        ON UPDATE CASCADE,
    CHECK (0 < max_mac_addrs)
);

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
    tool_run_id                 UUID            NOT NULL,
    device_id                   TEXT            NOT NULL,
    interface_name              TEXT            NOT NULL,
    mac_addr                    MACADDR         NOT NULL,
    PRIMARY KEY (tool_run_id, device_id, interface_name, mac_addr),
    FOREIGN KEY (tool_run_id, device_id, interface_name)
        REFERENCES raw_device_interfaces
                   (tool_run_id, device_id, interface_name)
        ON DELETE CASCADE
        ON UPDATE CASCADE,
    FOREIGN KEY (tool_run_id, mac_addr)
        REFERENCES raw_mac_addrs(tool_run_id, mac_addr)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

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
-- Device Access Control List (ACL) tables
-- ----------------------------------------------------------------------

CREATE TABLE raw_device_acl_action_sets (
    tool_run_id                 UUID            NOT NULL,
    device_id                   TEXT            NOT NULL,
    action_set                  TEXT            NOT NULL,
    PRIMARY KEY (tool_run_id, device_id, action_set),
    FOREIGN KEY (tool_run_id, device_id)
        REFERENCES raw_devices(tool_run_id, device_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);


-- ----------------------------------------------------------------------

CREATE TABLE raw_device_acl_actions (
    tool_run_id                 UUID            NOT NULL,
    device_id                   TEXT            NOT NULL,
    action_set                  TEXT            NOT NULL,
    action                      TEXT            NOT NULL,
    PRIMARY KEY (tool_run_id, device_id, action_set, action),
    FOREIGN KEY (tool_run_id, device_id, action_set)
        REFERENCES raw_device_acl_action_sets
                   (tool_run_id, device_id, action_set)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);


-- ----------------------------------------------------------------------

CREATE TABLE raw_device_acl_ip_net_sets (
    tool_run_id                 UUID            NOT NULL,
    device_id                   TEXT            NOT NULL,
    ip_net_set                  TEXT            NOT NULL,
    PRIMARY KEY (tool_run_id, device_id, ip_net_set),
    FOREIGN KEY (tool_run_id, device_id)
        REFERENCES raw_devices(tool_run_id, device_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);


-- ----------------------------------------------------------------------

CREATE TABLE raw_device_acl_ip_nets (
    tool_run_id                 UUID            NOT NULL,
    device_id                   TEXT            NOT NULL,
    ip_net_set                  TEXT            NOT NULL,
    ip_net                      CIDR            NOT NULL,
    PRIMARY KEY (tool_run_id, device_id, ip_net_set, ip_net),
    FOREIGN KEY (tool_run_id, device_id, ip_net_set)
        REFERENCES raw_device_acl_ip_net_sets
                   (tool_run_id, device_id, ip_net_set)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);


-- ----------------------------------------------------------------------

CREATE TABLE raw_device_acl_port_range_sets (
    tool_run_id                 UUID            NOT NULL,
    device_id                   TEXT            NOT NULL,
    port_range_set              TEXT            NOT NULL,
    PRIMARY KEY (tool_run_id, device_id, port_range_set),
    FOREIGN KEY (tool_run_id, device_id)
        REFERENCES raw_devices(tool_run_id, device_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);


-- ----------------------------------------------------------------------

CREATE TABLE raw_device_acl_port_ranges (
    tool_run_id                 UUID            NOT NULL,
    device_id                   TEXT            NOT NULL,
    port_range_set              TEXT            NOT NULL,
    protocol                    TEXT            NOT NULL,
    port_range                  INT4RANGE       NOT NULL,
    PRIMARY KEY (tool_run_id, device_id, port_range_set, protocol, port_range),
    FOREIGN KEY (tool_run_id, device_id, port_range_set)
        REFERENCES raw_device_acl_port_range_sets
                   (tool_run_id, device_id, port_range_set)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);


-- ----------------------------------------------------------------------

CREATE TABLE raw_device_acls (
    tool_run_id                 UUID            NOT NULL,
    device_id                   TEXT            NOT NULL,
    acl_set                     TEXT            NOT NULL,
    acl_number                  INT             NOT NULL,
    action_set                  TEXT            NOT NULL,
    src_ip_net_set              TEXT            NOT NULL,
    dst_ip_net_set              TEXT            NOT NULL,
    src_port_range_set          TEXT            NOT NULL,
    dst_port_range_set          TEXT            NOT NULL,
    PRIMARY KEY (tool_run_id, device_id,
                 acl_set, acl_number, action_set,
                 src_ip_net_set, src_port_range_set,
                 dst_ip_net_set, dst_port_range_set),
    FOREIGN KEY (tool_run_id, device_id, action_set)
        REFERENCES raw_device_acl_action_sets
                   (tool_run_id, device_id, action_set)
        ON DELETE CASCADE
        ON UPDATE CASCADE,
    FOREIGN KEY (tool_run_id, device_id, src_ip_net_set)
        REFERENCES raw_device_acl_ip_net_sets
                   (tool_run_id, device_id, ip_net_set)
        ON DELETE CASCADE
        ON UPDATE CASCADE,
    FOREIGN KEY (tool_run_id, device_id, dst_ip_net_set)
        REFERENCES raw_device_acl_ip_net_sets
                   (tool_run_id, device_id, ip_net_set)
        ON DELETE CASCADE
        ON UPDATE CASCADE,
    FOREIGN KEY (tool_run_id, device_id, src_port_range_set)
        REFERENCES raw_device_acl_port_range_sets
                   (tool_run_id, device_id, port_range_set)
        ON DELETE CASCADE
        ON UPDATE CASCADE,
    FOREIGN KEY (tool_run_id, device_id, dst_port_range_set)
        REFERENCES raw_device_acl_port_range_sets
                   (tool_run_id, device_id, port_range_set)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);


-- ----------------------------------------------------------------------

COMMIT TRANSACTION;
