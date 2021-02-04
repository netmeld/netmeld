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
-- Information about a given run of a Red Team tool.
-- The "tool_run_*" tables contain the context in which a tool was run.
-- ----------------------------------------------------------------------

CREATE TABLE tool_runs (
    id                          UUID            NOT NULL,
    tool_name                   TEXT            NOT NULL,
    command_line                TEXT            NOT NULL,
    data_path                   TEXT            NOT NULL,
    execute_time                TSRANGE         NOT NULL,
    PRIMARY KEY (id)
);

-- Partial indexes
CREATE INDEX tool_runs_idx_tool_name
ON tool_runs(tool_name);

CREATE INDEX tool_runs_idx_execute_time
ON tool_runs(execute_time);


-- ----------------------------------------------------------------------
-- Interfaces of the Red Team system that is running the tool.
-- ----------------------------------------------------------------------

CREATE TABLE tool_run_interfaces (
    tool_run_id                 UUID            NOT NULL,
    interface_name              TEXT            NOT NULL,
    media_type                  TEXT            NOT NULL,
    is_up                       BOOLEAN         NOT NULL,
    PRIMARY KEY (tool_run_id, interface_name),
    FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
CREATE INDEX tool_run_interfaces_idx_tool_run_id
ON tool_run_interfaces(tool_run_id);

CREATE INDEX tool_run_interfaces_idx_interface_name
ON tool_run_interfaces(interface_name);

CREATE INDEX tool_run_interfaces_idx_media_type
ON tool_run_interfaces(media_type);

CREATE INDEX tool_run_interfaces_idx_is_up
ON tool_run_interfaces(is_up);


-- ----------------------------------------------------------------------
-- MAC addresses of the Red Team system that is running the tool.
-- ----------------------------------------------------------------------

CREATE TABLE tool_run_mac_addrs (
    tool_run_id                 UUID            NOT NULL,
    interface_name              TEXT            NOT NULL,
    mac_addr                    MACADDR         NOT NULL,
    PRIMARY KEY (tool_run_id, interface_name, mac_addr),
    FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
CREATE INDEX tool_run_mac_addrs_idx_tool_run_id_interface_name
ON tool_run_mac_addrs(tool_run_id, interface_name);

CREATE INDEX tool_run_mac_addrs_idx_tool_run_id
ON tool_run_mac_addrs(tool_run_id);

CREATE INDEX tool_run_mac_addrs_idx_interface_name
ON tool_run_mac_addrs(interface_name);

CREATE INDEX tool_run_mac_addrs_idx_mac_addr
ON tool_run_mac_addrs(mac_addr);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX tool_run_mac_addrs_idx_views
ON tool_run_mac_addrs(interface_name, mac_addr);


-- ----------------------------------------------------------------------
-- IP addresses of the Red Team system that is running the tool.
-- ----------------------------------------------------------------------

CREATE TABLE tool_run_ip_addrs (
    tool_run_id                 UUID            NOT NULL,
    interface_name              TEXT            NOT NULL,
    ip_addr                     INET            NOT NULL,
    PRIMARY KEY (tool_run_id, interface_name, ip_addr),
    FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
CREATE INDEX tool_run_ip_addrs_idx_tool_run_id_interface_name
ON tool_run_ip_addrs(tool_run_id, interface_name);

CREATE INDEX tool_run_ip_addrs_idx_tool_run_id
ON tool_run_ip_addrs(tool_run_id);

CREATE INDEX tool_run_ip_addrs_idx_interface_name
ON tool_run_ip_addrs(interface_name);

CREATE INDEX tool_run_ip_addrs_idx_ip_addr
ON tool_run_ip_addrs(ip_addr);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX tool_run_ip_addrs_idx_views
ON tool_run_ip_addrs(interface_name, ip_addr);


-- ----------------------------------------------------------------------
-- Routing tables of the Red Team system that is running the tool.
-- ----------------------------------------------------------------------

-- dst_ip_net  = '0.0.0.0/0' or '::/0' for the default route.
-- rtr_ip_addr = '0.0.0.0'   or '::'   for directly connected LANs.

CREATE TABLE tool_run_ip_routes (
    tool_run_id                 UUID            NOT NULL,
    interface_name              TEXT            NOT NULL,
    dst_ip_net                  CIDR            NOT NULL,
    rtr_ip_addr                 INET            NOT NULL,
    PRIMARY KEY (tool_run_id, interface_name, dst_ip_net, rtr_ip_addr),
    FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE,
    CHECK (inet_same_family(dst_ip_net, rtr_ip_addr)),
    CHECK ((rtr_ip_addr = host(rtr_ip_addr)::INET))
);

-- Partial indexes
CREATE INDEX tool_run_ip_routes_idx_tool_run_id_interface_name
ON tool_run_ip_routes(tool_run_id, interface_name);

CREATE INDEX tool_run_ip_routes_idx_tool_run_id
ON tool_run_ip_routes(tool_run_id);

CREATE INDEX tool_run_ip_routes_idx_interface_name
ON tool_run_ip_routes(interface_name);

CREATE INDEX tool_run_ip_routes_idx_dst_ip_net
ON tool_run_ip_routes(dst_ip_net);

CREATE INDEX tool_run_ip_routes_idx_rtr_ip_addr
ON tool_run_ip_routes(rtr_ip_addr);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX tool_run_ip_routes_idx_views
ON tool_run_ip_routes(interface_name, dst_ip_net, rtr_ip_addr);


-- ----------------------------------------------------------------------

COMMIT TRANSACTION;
