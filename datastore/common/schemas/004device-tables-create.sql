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

CREATE TABLE raw_device_hardware_information (
    tool_run_id                 UUID            NOT NULL,
    device_id                   TEXT            NOT NULL,
    device_type                 TEXT            NULL,
    vendor                      TEXT            NULL,
    model                       TEXT            NULL,
    hardware_revision           TEXT            NULL,
    serial_number               TEXT            NULL,
    description                 TEXT            NULL,
-- TODO [#114] Primary key here is hard as this table is a data conglomorate
    FOREIGN KEY (tool_run_id, device_id)
        REFERENCES raw_devices(tool_run_id, device_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);
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
CREATE INDEX raw_device_hardware_information_idx_views
ON raw_device_hardware_information(device_id, device_type, vendor, model,
    hardware_revision, serial_number, description);

-- Since this table lacks a PRIMARY KEY and allows NULLs:
-- Create UNIQUE partial indexes over likely combinations of columns
-- for use with `ON CONFLICT` guards against duplicate data.

-- The vendor and hardware information is increasingly specific.
-- If less specific information (like vendor) is NULL,
-- then more specific information is very likely also NULL.

CREATE UNIQUE INDEX raw_device_hardware_information_idx_unique2
ON raw_device_hardware_information(tool_run_id, device_id)
WHERE (vendor IS NULL) AND (model IS NULL) AND
      (hardware_revision IS NULL) AND (serial_number IS NULL);

CREATE UNIQUE INDEX raw_device_hardware_information_idx_unique3
ON raw_device_hardware_information(tool_run_id, device_id,
       vendor)
WHERE (vendor IS NOT NULL) AND (model IS NULL) AND
      (hardware_revision IS NULL) AND (serial_number IS NULL);

CREATE UNIQUE INDEX raw_device_hardware_information_idx_unique4_model
ON raw_device_hardware_information(tool_run_id, device_id,
       vendor, model)
WHERE (vendor IS NOT NULL) AND (model IS NOT NULL) AND
      (hardware_revision IS NULL) AND (serial_number IS NULL);

CREATE UNIQUE INDEX raw_device_hardware_information_idx_unique5_model
ON raw_device_hardware_information(tool_run_id, device_id,
       vendor, model, hardware_revision)
WHERE (vendor IS NOT NULL) AND (model IS NOT NULL) AND
      (hardware_revision IS NOT NULL) AND (serial_number IS NULL);

CREATE UNIQUE INDEX raw_device_hardware_information_idx_unique6
ON raw_device_hardware_information(tool_run_id, device_id,
       vendor, model, hardware_revision, serial_number)
WHERE (vendor IS NOT NULL) AND (model IS NOT NULL) AND
      (hardware_revision IS NOT NULL) AND (serial_number IS NOT NULL);

-- It is possible to have the vendor and serial_number,
-- while missing model or hardware_revision information.
CREATE UNIQUE INDEX raw_device_hardware_information_idx_unique4_serial
ON raw_device_hardware_information(tool_run_id, device_id,
       vendor, serial_number)
WHERE (vendor IS NOT NULL) AND (model IS NULL) AND
      (hardware_revision IS NULL) AND (serial_number IS NOT NULL);

CREATE UNIQUE INDEX raw_device_hardware_information_idx_unique5_serial
ON raw_device_hardware_information(tool_run_id, device_id,
       vendor, model, serial_number)
WHERE (vendor IS NOT NULL) AND (model IS NOT NULL) AND
      (hardware_revision IS NULL) AND (serial_number IS NOT NULL);


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
    CHECK (inet_same_family(dst_ip_net, rtr_ip_addr)),
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

-- Since this table lacks a PRIMARY KEY and allows NULLs:
-- Create UNIQUE partial indexes over likely combinations of columns
-- for use with `ON CONFLICT` guards against duplicate data.

CREATE UNIQUE INDEX raw_device_ip_routes_idx_unique3
ON raw_device_ip_routes(tool_run_id, device_id, dst_ip_net)
WHERE (interface_name IS NULL) and (rtr_ip_addr IS NULL);

CREATE UNIQUE INDEX raw_device_ip_routes_idx_unique4_interface
ON raw_device_ip_routes(tool_run_id, device_id, dst_ip_net,
       interface_name)
WHERE (interface_name IS NOT NULL) and (rtr_ip_addr IS NULL);

CREATE UNIQUE INDEX raw_device_ip_routes_idx_unique4_rtr_ip_addr
ON raw_device_ip_routes(tool_run_id, device_id, dst_ip_net,
       rtr_ip_addr)
WHERE (interface_name IS NULL) and (rtr_ip_addr IS NOT NULL);

CREATE UNIQUE INDEX raw_device_ip_routes_idx_unique5
ON raw_device_ip_routes(tool_run_id, device_id, dst_ip_net,
       interface_name, rtr_ip_addr)
WHERE (interface_name IS NOT NULL) and (rtr_ip_addr IS NOT NULL);

-- ----------------------------------------------------------------------
-- Server tables from target devices (config files or console access)
-- Stores device's configured DNS, DHCP, WINS, NTP, ... servers
-- ----------------------------------------------------------------------

CREATE TABLE raw_device_ip_servers (
    tool_run_id                 UUID            NOT NULL,
    device_id                   TEXT            NOT NULL,
    interface_name              TEXT            NOT NULL,
    service_name                TEXT            NOT NULL,
    server_ip_addr              INET            NOT NULL,
    port                        PortNumber      NULL,
    local_service               BOOLEAN         NOT NULL,
    description                 TEXT            NULL,
    PRIMARY KEY (tool_run_id, device_id, interface_name, service_name,
                server_ip_addr),
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
-- Device Access Control tables
-- ----------------------------------------------------------------------

CREATE TABLE raw_device_ac_nets (
    tool_run_id   UUID    NOT NULL,
    device_id     TEXT    NOT NULL,
    net_set_id    TEXT    NOT NULL,
    net_set       TEXT    NOT NULL,
    net_set_data  TEXT    NULL,
    FOREIGN KEY (tool_run_id, device_id)
      REFERENCES raw_devices(tool_run_id, device_id)
      ON DELETE CASCADE
      ON UPDATE CASCADE
);
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
CREATE INDEX raw_device_ac_nets_idx_views
ON raw_device_ac_nets(device_id, net_set_id, net_set_data);

-- Since this table lacks a PRIMARY KEY and allows NULLs:
-- Create UNIQUE partial indexes over likely combinations of columns
-- for use with `ON CONFLICT` guards against duplicate data.

CREATE UNIQUE INDEX raw_device_ac_nets_idx_unique4
ON raw_device_ac_nets(tool_run_id, device_id, net_set_id, net_set)
WHERE (net_set_data IS NULL);

CREATE UNIQUE INDEX raw_device_ac_nets_idx_unique5
ON raw_device_ac_nets(tool_run_id, device_id, net_set_id, net_set,
       net_set_data)
WHERE (net_set_data IS NOT NULL);


CREATE TABLE raw_device_ac_services (
    tool_run_id       UUID    NOT NULL,
    device_id         TEXT    NOT NULL,
    service_set       TEXT    NOT NULL,
    service_set_data  TEXT    NULL,
    FOREIGN KEY (tool_run_id, device_id)
      REFERENCES raw_devices(tool_run_id, device_id)
      ON DELETE CASCADE
      ON UPDATE CASCADE
);
CREATE INDEX raw_device_ac_services_idx_tool_run_id
ON raw_device_ac_services(tool_run_id);
CREATE INDEX raw_device_ac_services_idx_device_id
ON raw_device_ac_services(device_id);
CREATE INDEX raw_device_ac_services_idx_service_set
ON raw_device_ac_services(service_set);
CREATE INDEX raw_device_ac_services_idx_service_set_data
ON raw_device_ac_services(service_set_data);
CREATE INDEX raw_device_ac_services_idx_views
ON raw_device_ac_services(device_id, service_set, service_set_data);

-- Since this table lacks a PRIMARY KEY and allows NULLs:
-- Create UNIQUE partial indexes over likely combinations of columns
-- for use with `ON CONFLICT` guards against duplicate data.

CREATE UNIQUE INDEX raw_device_ac_services_idx_unique3
ON raw_device_ac_services(tool_run_id, device_id, service_set)
WHERE (service_set_data IS NULL);

CREATE UNIQUE INDEX raw_device_ac_services_idx_unique4
ON raw_device_ac_services(tool_run_id, device_id, service_set,
       service_set_data)
WHERE (service_set_data IS NOT NULL);


CREATE TABLE raw_device_ac_rules (
    tool_run_id     UUID    NOT NULL,
    device_id       TEXT    NOT NULL,
    enabled         BOOLEAN NOT NULL,
    ac_id           INT     NOT NULL,
    src_net_set_id  TEXT    NOT NULL,
    src_net_set     TEXT    NOT NULL,
    src_iface       TEXT    NULL,
    dst_net_set_id  TEXT    NOT NULL,
    dst_net_set     TEXT    NOT NULL,
    dst_iface       TEXT    NULL,
    service_set     TEXT    NULL,
    action          TEXT    NOT NULL,
    description     TEXT    NULL,
    FOREIGN KEY (tool_run_id, device_id)
      REFERENCES raw_devices(tool_run_id, device_id)
      ON DELETE CASCADE
      ON UPDATE CASCADE
);
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
CREATE INDEX raw_device_ac_rules_idx_views
ON raw_device_ac_rules(device_id, enabled, ac_id,
    src_net_set_id, src_net_set, src_iface,
    dst_net_set_id, dst_net_set, dst_iface,
    service_set, action, description);

-- Since this table lacks a PRIMARY KEY and allows NULLs:
-- Create UNIQUE partial indexes over likely combinations of columns
-- for use with `ON CONFLICT` guards against duplicate data.

CREATE UNIQUE INDEX raw_device_ac_rules_idx_unique9
ON raw_device_ac_rules(tool_run_id, device_id, enabled, ac_id, action,
       src_net_set_id, src_net_set,
       dst_net_set_id, dst_net_set)
WHERE (src_iface IS NULL) AND (dst_iface IS NULL) AND
      (service_set IS NULL);

CREATE UNIQUE INDEX raw_device_ac_rules_idx_unique10_src
ON raw_device_ac_rules(tool_run_id, device_id, enabled, ac_id, action,
       src_net_set_id, src_net_set, src_iface,
       dst_net_set_id, dst_net_set)
WHERE (src_iface IS NOT NULL) AND (dst_iface IS NULL) AND
      (service_set IS NULL);

CREATE UNIQUE INDEX raw_device_ac_rules_idx_unique10_dst
ON raw_device_ac_rules(tool_run_id, device_id, enabled, ac_id, action,
       src_net_set_id, src_net_set,
       dst_net_set_id, dst_net_set, dst_iface)
WHERE (src_iface IS NULL) AND (dst_iface IS NOT NULL) AND
      (service_set IS NULL);

CREATE UNIQUE INDEX raw_device_ac_rules_idx_unique10_service
ON raw_device_ac_rules(tool_run_id, device_id, enabled, ac_id, action,
       src_net_set_id, src_net_set,
       dst_net_set_id, dst_net_set,
       service_set)
WHERE (src_iface IS NULL) AND (dst_iface IS NULL) AND
      (service_set IS NOT NULL);

CREATE UNIQUE INDEX raw_device_ac_rules_idx_unique11_src_dst
ON raw_device_ac_rules(tool_run_id, device_id, enabled, ac_id, action,
       src_net_set_id, src_net_set, src_iface,
       dst_net_set_id, dst_net_set, dst_iface)
WHERE (src_iface IS NOT NULL) AND (dst_iface IS NOT NULL) AND
      (service_set IS NULL);

CREATE UNIQUE INDEX raw_device_ac_rules_idx_unique11_src_service
ON raw_device_ac_rules(tool_run_id, device_id, enabled, ac_id, action,
       src_net_set_id, src_net_set, src_iface,
       dst_net_set_id, dst_net_set,
       service_set)
WHERE (src_iface IS NOT NULL) AND (dst_iface IS NULL) AND
      (service_set IS NOT NULL);

CREATE UNIQUE INDEX raw_device_ac_rules_idx_unique11_dst_service
ON raw_device_ac_rules(tool_run_id, device_id, enabled, ac_id, action,
       src_net_set_id, src_net_set,
       dst_net_set_id, dst_net_set, dst_iface,
       service_set)
WHERE (src_iface IS NULL) AND (dst_iface IS NOT NULL) AND
      (service_set IS NOT NULL);

CREATE UNIQUE INDEX raw_device_ac_rules_idx_unique12
ON raw_device_ac_rules(tool_run_id, device_id, enabled, ac_id, action,
       src_net_set_id, src_net_set, src_iface,
       dst_net_set_id, dst_net_set, dst_iface,
       service_set)
WHERE (src_iface IS NOT NULL) AND (dst_iface IS NOT NULL) AND
      (service_set IS NOT NULL);


-- ----------------------------------------------------------------------
-- Device VLAN tables
-- ----------------------------------------------------------------------
CREATE TABLE raw_device_vlans (
    tool_run_id                 UUID            NOT NULL,
    device_id                   TEXT            NOT NULL,
    vlan                        VlanNumber      NOT NULL,
    description                 TEXT            NULL,
    PRIMARY KEY (tool_run_id, device_id, vlan),
    FOREIGN KEY (tool_run_id, device_id)
        REFERENCES raw_devices(tool_run_id, device_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);
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


CREATE TABLE raw_device_vlans_ip_nets (
    tool_run_id                 UUID            NOT NULL,
    device_id                   TEXT            NOT NULL,
    vlan                        VlanNumber      NOT NULL,
    ip_net                      CIDR            NOT NULL,
    PRIMARY KEY (tool_run_id, device_id, vlan, ip_net),
    FOREIGN KEY (tool_run_id, device_id, vlan)
        REFERENCES raw_device_vlans(tool_run_id, device_id, vlan)
        ON DELETE CASCADE
        ON UPDATE CASCADE,
    FOREIGN KEY (tool_run_id, ip_net)
        REFERENCES raw_ip_nets(tool_run_id, ip_net)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);
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


CREATE TABLE raw_device_interfaces_vlans (
    tool_run_id                 UUID            NOT NULL,
    device_id                   TEXT            NOT NULL,
    interface_name              TEXT            NOT NULL,
    vlan                        VlanNumber      NOT NULL,
    PRIMARY KEY (tool_run_id, device_id, interface_name, vlan),
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
CREATE INDEX raw_device_interfaces_vlans_idx_vlan
ON raw_device_interfaces_vlans(vlan);
-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_device_interfaces_vlans_idx_views
ON raw_device_interfaces_vlans(device_id, interface_name, vlan);


-- ----------------------------------------------------------------------

COMMIT TRANSACTION;
