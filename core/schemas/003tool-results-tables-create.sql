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
-- MAC Addresses of target systems
-- ----------------------------------------------------------------------

CREATE TABLE raw_mac_addrs (
    tool_run_id                 UUID            NOT NULL,
    mac_addr                    MACADDR         NOT NULL,
    is_responding               BOOLEAN         NULL,
    PRIMARY KEY (tool_run_id, mac_addr),
    FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

CREATE INDEX raw_mac_addrs_idx_tool_run_id
ON raw_mac_addrs(tool_run_id);

CREATE INDEX raw_mac_addrs_idx_mac_addr
ON raw_mac_addrs(mac_addr);

CREATE INDEX raw_mac_addrs_idx_is_responding
ON raw_mac_addrs(is_responding);


-- ----------------------------------------------------------------------
-- IP Addresses of target systems
-- ----------------------------------------------------------------------

CREATE TABLE raw_ip_addrs (
    tool_run_id                 UUID            NOT NULL,
    ip_addr                     INET            NOT NULL,
    is_responding               BOOLEAN         NULL,
    PRIMARY KEY (tool_run_id, ip_addr),
    FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE,
    CHECK (ip_addr = host(ip_addr)::INET)
);

CREATE INDEX raw_ip_addrs_idx_tool_run_id
ON raw_ip_addrs(tool_run_id);

CREATE INDEX raw_ip_addrs_idx_ip_addr
ON raw_ip_addrs(ip_addr);

CREATE INDEX raw_ip_addrs_idx_is_responding
ON raw_ip_addrs(is_responding);


-- ----------------------------------------------------------------------
-- MAC-to-IP (from ARP, NDP, configs, MAC sticky, etc)
-- ----------------------------------------------------------------------

CREATE TABLE raw_mac_addrs_ip_addrs (
    tool_run_id                 UUID            NOT NULL,
    mac_addr                    MACADDR         NOT NULL,
    ip_addr                     INET            NOT NULL,
    PRIMARY KEY (tool_run_id, mac_addr, ip_addr),
    FOREIGN KEY (tool_run_id, mac_addr)
        REFERENCES raw_mac_addrs(tool_run_id, mac_addr)
        ON DELETE CASCADE
        ON UPDATE CASCADE,
    FOREIGN KEY (tool_run_id, ip_addr)
        REFERENCES raw_ip_addrs(tool_run_id, ip_addr)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

CREATE INDEX raw_mac_addrs_ip_addrs_idx_tool_run_id
ON raw_mac_addrs_ip_addrs(tool_run_id);

CREATE INDEX raw_mac_addrs_ip_addrs_idx_mac_addr
ON raw_mac_addrs_ip_addrs(mac_addr);

CREATE INDEX raw_mac_addrs_ip_addrs_idx_ip_addr
ON raw_mac_addrs_ip_addrs(ip_addr);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_mac_addrs_ip_addrs_idx_views
ON raw_mac_addrs_ip_addrs(mac_addr, ip_addr);


-- ----------------------------------------------------------------------
-- IP-to-Hostname (from DNS, /etc/hosts, etc)
-- ----------------------------------------------------------------------

CREATE TABLE raw_hostnames (
    tool_run_id                 UUID            NOT NULL,
    ip_addr                     INET            NOT NULL,
    hostname                    TEXT            NOT NULL,
    reason                      TEXT            NOT NULL,
    PRIMARY KEY (tool_run_id, ip_addr, hostname, reason),
    FOREIGN KEY (tool_run_id, ip_addr)
        REFERENCES raw_ip_addrs(tool_run_id, ip_addr)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

CREATE INDEX raw_hostnames_idx_tool_run_id
ON raw_hostnames(tool_run_id);

CREATE INDEX raw_hostnames_idx_ip_addr
ON raw_hostnames(ip_addr);

CREATE INDEX raw_hostnames_idx_hostname
ON raw_hostnames(hostname);

CREATE INDEX raw_hostnames_idx_reason
ON raw_hostnames(reason);

CREATE INDEX raw_hostnames_idx_ip_addr_hostname
ON raw_hostnames(ip_addr, hostname);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_hostnames_idx_views
ON raw_hostnames(ip_addr, hostname, reason);


-- ----------------------------------------------------------------------

CREATE TABLE raw_vlans (
    tool_run_id                 UUID            NOT NULL,
    vlan                        INT             NOT NULL,
    description                 TEXT            NULL,
    PRIMARY KEY (tool_run_id, vlan),
    FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE,
    CHECK (vlan BETWEEN 0 and 4095)
);

CREATE INDEX raw_vlans_idx_tool_run_id
ON raw_vlans(tool_run_id);

CREATE INDEX raw_vlans_idx_vlan
ON raw_vlans(vlan);


-- ----------------------------------------------------------------------

CREATE TABLE raw_ip_nets (
    tool_run_id                 UUID            NOT NULL,
    ip_net                      CIDR            NOT NULL,
    description                 TEXT            NULL,
    PRIMARY KEY (tool_run_id, ip_net),
    FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

CREATE INDEX raw_ip_nets_idx_tool_run_id
ON raw_ip_nets(tool_run_id);

CREATE INDEX raw_ip_nets_idx_ip_net
ON raw_ip_nets(ip_net);


-- ----------------------------------------------------------------------

CREATE TABLE ip_nets_extra_weights (
    ip_net                      CIDR            NOT NULL,
    extra_weight                FLOAT           NOT NULL,
    PRIMARY KEY (ip_net)
);


-- ----------------------------------------------------------------------

CREATE TABLE raw_vlans_ip_nets (
    tool_run_id                 UUID            NOT NULL,
    vlan                        INT             NOT NULL,
    ip_net                      CIDR            NOT NULL,
    PRIMARY KEY (tool_run_id, vlan, ip_net),
    FOREIGN KEY (tool_run_id, vlan)
        REFERENCES raw_vlans(tool_run_id, vlan)
        ON DELETE CASCADE
        ON UPDATE CASCADE,
    FOREIGN KEY (tool_run_id, ip_net)
        REFERENCES raw_ip_nets(tool_run_id, ip_net)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

CREATE INDEX raw_vlans_ip_nets_idx_tool_run_id
ON raw_vlans_ip_nets(tool_run_id);

CREATE INDEX raw_vlans_ip_nets_idx_vlan
ON raw_vlans_ip_nets(vlan);

CREATE INDEX raw_vlans_ip_nets_idx_ip_net
ON raw_vlans_ip_nets(ip_net);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_vlans_ip_nets_idx_views
ON raw_vlans_ip_nets(vlan, ip_net);


-- ----------------------------------------------------------------------

CREATE TABLE raw_ip_traceroutes (
    tool_run_id                 UUID            NOT NULL,
    hop_count                   INT             NOT NULL,
    rtr_ip_addr                 INET            NOT NULL,
    dst_ip_addr                 INET            NOT NULL,
    PRIMARY KEY (tool_run_id, hop_count, rtr_ip_addr, dst_ip_addr),
    FOREIGN KEY (tool_run_id, rtr_ip_addr)
        REFERENCES raw_ip_addrs(tool_run_id, ip_addr)
        ON DELETE CASCADE
        ON UPDATE CASCADE,
    FOREIGN KEY (tool_run_id, dst_ip_addr)
        REFERENCES raw_ip_addrs(tool_run_id, ip_addr)
        ON DELETE CASCADE
        ON UPDATE CASCADE,
    CHECK (hop_count BETWEEN 0 AND 255)
);

CREATE INDEX raw_ip_traceroutes_idx_tool_run_id
ON raw_ip_traceroutes(tool_run_id);

CREATE INDEX raw_ip_traceroutes_idx_hop_count
ON raw_ip_traceroutes(hop_count);

CREATE INDEX raw_ip_traceroutes_idx_rtr_ip_addr
ON raw_ip_traceroutes(rtr_ip_addr);

CREATE INDEX raw_ip_traceroutes_idx_dst_ip_addr
ON raw_ip_traceroutes(dst_ip_addr);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_ip_traceroutes_idx_views
ON raw_ip_traceroutes(hop_count, rtr_ip_addr, dst_ip_addr);


-- ----------------------------------------------------------------------

CREATE TABLE raw_ports (
    tool_run_id                 UUID            NOT NULL,
    ip_addr                     INET            NOT NULL,
    protocol                    TEXT            NOT NULL,
    port                        INT             NOT NULL, -- '-1' = 'other'
    port_state                  TEXT            NULL,
    port_reason                 TEXT            NULL,
    PRIMARY KEY (tool_run_id, ip_addr, protocol, port),
    FOREIGN KEY (tool_run_id, ip_addr)
        REFERENCES raw_ip_addrs(tool_run_id, ip_addr)
        ON DELETE CASCADE
        ON UPDATE CASCADE,
    CHECK (port BETWEEN -1 AND 65535)
);

CREATE INDEX raw_ports_idx_tool_run_id
ON raw_ports(tool_run_id);

CREATE INDEX raw_ports_idx_ip_addr
ON raw_ports(ip_addr);

CREATE INDEX raw_ports_idx_protocol
ON raw_ports(protocol);

CREATE INDEX raw_ports_idx_port
ON raw_ports(port);

CREATE INDEX raw_ports_idx_port_state
ON raw_ports(port_state);

CREATE INDEX raw_ports_idx_port_reason
ON raw_ports(port_reason);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_ports_idx_views
ON raw_ports(ip_addr, protocol, port);


-- ----------------------------------------------------------------------

CREATE TABLE raw_network_services (
    tool_run_id                 UUID            NOT NULL,
    ip_addr                     INET            NOT NULL,
    protocol                    TEXT            NOT NULL,
    port                        INT             NOT NULL,
    service_name                TEXT            NULL,
    service_description         TEXT            NULL,
    service_reason              TEXT            NULL,
    observer_ip_addr            INET            NULL,
    PRIMARY KEY (tool_run_id, ip_addr, protocol, port),
    FOREIGN KEY (tool_run_id, ip_addr, protocol, port)
        REFERENCES raw_ports(tool_run_id, ip_addr, protocol, port)
        ON DELETE CASCADE
        ON UPDATE CASCADE,
    CHECK (port BETWEEN 0 AND 65535)
);

CREATE INDEX raw_network_services_idx_tool_run_id
ON raw_network_services(tool_run_id);

CREATE INDEX raw_network_services_idx_ip_addr
ON raw_network_services(ip_addr);

CREATE INDEX raw_network_services_idx_protocol
ON raw_network_services(protocol);

CREATE INDEX raw_network_services_idx_port
ON raw_network_services(port);

CREATE INDEX raw_network_services_idx_service_name
ON raw_network_services(service_name);

--CREATE INDEX raw_network_services_idx_service_description
--ON raw_network_services USING gin(to_tsvector('english', service_description));

CREATE INDEX raw_network_services_idx_service_reason
ON raw_network_services(service_reason);

CREATE INDEX raw_network_services_idx_observer_ip_addr
ON raw_network_services(observer_ip_addr);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_network_services_idx_views
ON raw_network_services(ip_addr, protocol, port);


-- ----------------------------------------------------------------------
-- Nessus results
-- severity: 0 = None, 1 = Low, 2 = Medium, 3 = High, 4 = Critical.
-- ----------------------------------------------------------------------

CREATE TABLE raw_nessus_results (
    tool_run_id                 UUID            NOT NULL,
    ip_addr                     INET            NOT NULL,
    protocol                    TEXT            NOT NULL,
    port                        INT             NOT NULL,
    plugin_id                   INT             NOT NULL,
    plugin_name                 TEXT            NULL,
    plugin_family               TEXT            NULL,
    plugin_type                 TEXT            NULL,
    plugin_output               TEXT            NULL,
    severity                    INT             NOT NULL,
    description                 TEXT            NULL,
    solution                    TEXT            NULL,
    PRIMARY KEY (tool_run_id, ip_addr, protocol, port, plugin_id),
    FOREIGN KEY (tool_run_id, ip_addr, protocol, port)
        REFERENCES raw_ports(tool_run_id, ip_addr, protocol, port)
        ON DELETE CASCADE
        ON UPDATE CASCADE,
    CHECK (port BETWEEN 0 AND 65535),
    CHECK (severity BETWEEN 0 AND 4)
);

CREATE INDEX raw_nessus_results_idx_tool_run_id
ON raw_nessus_results(tool_run_id);

CREATE INDEX raw_nessus_results_idx_ip_addr
ON raw_nessus_results(ip_addr);

CREATE INDEX raw_nessus_results_idx_protocol
ON raw_nessus_results(protocol);

CREATE INDEX raw_nessus_results_idx_port
ON raw_nessus_results(port);

CREATE INDEX raw_nessus_results_idx_plugin_id
ON raw_nessus_results(plugin_id);

CREATE INDEX raw_nessus_results_idx_plugin_name
ON raw_nessus_results(plugin_name);

CREATE INDEX raw_nessus_results_idx_plugin_family
ON raw_nessus_results(plugin_family);

CREATE INDEX raw_nessus_results_idx_plugin_type
ON raw_nessus_results(plugin_type);

--CREATE INDEX raw_nessus_results_idx_plugin_output
--ON raw_nessus_results USING gin(to_tsvector('english', plugin_output));

CREATE INDEX raw_nessus_results_idx_severity
ON raw_nessus_results(severity);

--CREATE INDEX raw_nessus_results_idx_description
--ON raw_nessus_results USING gin(to_tsvector('english', description));

--CREATE INDEX raw_nessus_results_idx_solution
--ON raw_nessus_results USING gin(to_tsvector('english', solution));

CREATE INDEX raw_nessus_results_idx_ip_addr_protocol_port
ON raw_nessus_results(ip_addr, protocol, port);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_nessus_results_idx_views
ON raw_nessus_results(ip_addr, protocol, port, plugin_id);


-- ----------------------------------------------------------------------

CREATE TABLE raw_nessus_results_cves (
    tool_run_id                 UUID            NOT NULL,
    ip_addr                     INET            NOT NULL,
    protocol                    TEXT            NOT NULL,
    port                        INT             NOT NULL,
    plugin_id                   INT             NOT NULL,
    cve_id                      CVE             NOT NULL,
    PRIMARY KEY (tool_run_id, ip_addr, protocol, port, plugin_id, cve_id),
    FOREIGN KEY (tool_run_id, ip_addr, protocol, port, plugin_id)
        REFERENCES raw_nessus_results
                   (tool_run_id, ip_addr, protocol, port, plugin_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

CREATE INDEX raw_nessus_results_cves_idx_tool_run_id
ON raw_nessus_results_cves(tool_run_id);

CREATE INDEX raw_nessus_results_cves_idx_ip_addr
ON raw_nessus_results_cves(ip_addr);

CREATE INDEX raw_nessus_results_cves_idx_protocol
ON raw_nessus_results_cves(protocol);

CREATE INDEX raw_nessus_results_cves_idx_port
ON raw_nessus_results_cves(port);

CREATE INDEX raw_nessus_results_cves_idx_plugin_id
ON raw_nessus_results_cves(plugin_id);

CREATE INDEX raw_nessus_results_cves_idx_cve_id
ON raw_nessus_results_cves(cve_id);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_nessus_results_cves_idx_views
ON raw_nessus_results_cves(ip_addr, protocol, port, plugin_id, cve_id);


-- ----------------------------------------------------------------------

CREATE TABLE raw_nessus_results_metasploit_modules (
    tool_run_id                 UUID            NOT NULL,
    ip_addr                     INET            NOT NULL,
    protocol                    TEXT            NOT NULL,
    port                        INT             NOT NULL,
    plugin_id                   INT             NOT NULL,
    metasploit_name             TEXT            NOT NULL,
    PRIMARY KEY (tool_run_id, ip_addr, protocol, port, plugin_id,
                 metasploit_name),
    FOREIGN KEY (tool_run_id, ip_addr, protocol, port, plugin_id)
        REFERENCES raw_nessus_results
                   (tool_run_id, ip_addr, protocol, port, plugin_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

CREATE INDEX raw_nessus_results_metasploit_modules_idx_tool_run_id
ON raw_nessus_results_metasploit_modules(tool_run_id);

CREATE INDEX raw_nessus_results_metasploit_modules_idx_ip_addr
ON raw_nessus_results_metasploit_modules(ip_addr);

CREATE INDEX raw_nessus_results_metasploit_modules_idx_protocol
ON raw_nessus_results_metasploit_modules(protocol);

CREATE INDEX raw_nessus_results_metasploit_modules_idx_port
ON raw_nessus_results_metasploit_modules(port);

CREATE INDEX raw_nessus_results_metasploit_modules_idx_plugin_id
ON raw_nessus_results_metasploit_modules(plugin_id);

CREATE INDEX raw_nessus_results_metasploit_modules_idx_metasploit_name
ON raw_nessus_results_metasploit_modules(metasploit_name);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_nessus_results_metasploit_modules_idx_views
ON raw_nessus_results_metasploit_modules
   (ip_addr, protocol, port, plugin_id, metasploit_name);


-- ----------------------------------------------------------------------
-- Nmap Scripting Engine (NSE) results
-- ----------------------------------------------------------------------

CREATE TABLE raw_nse_results (
    tool_run_id                 UUID            NOT NULL,
    ip_addr                     INET            NOT NULL,
    protocol                    TEXT            NOT NULL,
    port                        INT             NOT NULL,
    script_id                   TEXT            NOT NULL,
    script_output               TEXT            NULL,
    PRIMARY KEY (tool_run_id, ip_addr, protocol, port, script_id),
    FOREIGN KEY (tool_run_id, ip_addr, protocol, port)
        REFERENCES raw_ports(tool_run_id, ip_addr, protocol, port)
        ON DELETE CASCADE
        ON UPDATE CASCADE,
    CHECK (port BETWEEN 0 AND 65535)
);

CREATE INDEX raw_nse_results_idx_tool_run_id
ON raw_nse_results(tool_run_id);

CREATE INDEX raw_nse_results_idx_ip_addr
ON raw_nse_results(ip_addr);

CREATE INDEX raw_nse_results_idx_protocol
ON raw_nse_results(protocol);

CREATE INDEX raw_nse_results_idx_port
ON raw_nse_results(port);

CREATE INDEX raw_nse_results_idx_script_id
ON raw_nse_results(script_id);

--CREATE INDEX raw_nse_results_idx_script_output
--ON raw_nse_results USING gin(to_tsvector('english', script_output));

CREATE INDEX raw_nse_results_idx_ip_addr_protocol_port
ON raw_nse_results(ip_addr, protocol, port);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_nse_results_idx_views
ON raw_nse_results(ip_addr, protocol, port, script_id);


-- ----------------------------------------------------------------------

CREATE TABLE raw_ssh_host_public_keys (
    tool_run_id                 UUID            NOT NULL,
    ip_addr                     INET            NOT NULL,
    protocol                    TEXT            NOT NULL,
    port                        INT             NOT NULL,
    ssh_key_type                TEXT            NOT NULL,
    ssh_key_bits                INT             NOT NULL,
    ssh_key_fingerprint         TEXT            NOT NULL,
    ssh_key_public              TEXT            NOT NULL,
    PRIMARY KEY (tool_run_id, ip_addr, protocol, port,
                 ssh_key_type, ssh_key_bits, ssh_key_fingerprint),
    FOREIGN KEY (tool_run_id, ip_addr, protocol, port)
        REFERENCES raw_ports(tool_run_id, ip_addr, protocol, port)
        ON DELETE CASCADE
        ON UPDATE CASCADE,
    CHECK (port BETWEEN 0 AND 65535)
);

CREATE INDEX raw_ssh_host_public_keys_idx_tool_run_id
ON raw_ssh_host_public_keys(tool_run_id);

CREATE INDEX raw_ssh_host_public_keys_idx_ip_addr
ON raw_ssh_host_public_keys(ip_addr);

CREATE INDEX raw_ssh_host_public_keys_idx_protocol
ON raw_ssh_host_public_keys(protocol);

CREATE INDEX raw_ssh_host_public_keys_idx_port
ON raw_ssh_host_public_keys(port);

CREATE INDEX raw_ssh_host_public_keys_idx_ssh_key_type
ON raw_ssh_host_public_keys(ssh_key_type);

CREATE INDEX raw_ssh_host_public_keys_idx_ssh_key_bits
ON raw_ssh_host_public_keys(ssh_key_bits);

CREATE INDEX raw_ssh_host_public_keys_idx_ssh_key_fingerprint
ON raw_ssh_host_public_keys(ssh_key_fingerprint);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_ssh_host_public_keys_idx_views
ON raw_ssh_host_public_keys(ip_addr, protocol, port);


-- ----------------------------------------------------------------------

CREATE TABLE raw_ssh_host_algorithms (
    tool_run_id                 UUID            NOT NULL,
    ip_addr                     INET            NOT NULL,
    protocol                    TEXT            NOT NULL,
    port                        INT             NOT NULL,
    ssh_algo_type               TEXT            NOT NULL,
    ssh_algo_name               TEXT            NOT NULL,
    PRIMARY KEY (tool_run_id, ip_addr, protocol, port,
                 ssh_algo_type, ssh_algo_name),
    FOREIGN KEY (tool_run_id, ip_addr, protocol, port)
        REFERENCES raw_ports(tool_run_id, ip_addr, protocol, port)
        ON DELETE CASCADE
        ON UPDATE CASCADE,
    CHECK (port BETWEEN 0 AND 65535)
);

CREATE INDEX raw_ssh_host_algorithms_idx_tool_run_id
ON raw_ssh_host_algorithms(tool_run_id);

CREATE INDEX raw_ssh_host_algorithms_idx_ip_addr
ON raw_ssh_host_algorithms(ip_addr);

CREATE INDEX raw_ssh_host_algorithms_idx_protocol
ON raw_ssh_host_algorithms(protocol);

CREATE INDEX raw_ssh_host_algorithms_idx_port
ON raw_ssh_host_algorithms(port);

CREATE INDEX raw_ssh_host_algorithms_idx_ssh_algo_type
ON raw_ssh_host_algorithms(ssh_algo_type);

CREATE INDEX raw_ssh_host_algorithms_idx_ssh_algo_name
ON raw_ssh_host_algorithms(ssh_algo_name);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_ssh_host_algorithms_idx_views
ON raw_ssh_host_algorithms(ip_addr, protocol, port);


-- ----------------------------------------------------------------------

CREATE TABLE raw_operating_systems (
    tool_run_id                 UUID            NOT NULL,
    ip_addr                     INET            NOT NULL,
    vendor_name                 TEXT            NULL,
    product_name                TEXT            NULL,
    product_version             TEXT            NULL,
    cpe                         TEXT            NULL,
    accuracy                    FLOAT           NULL,
    FOREIGN KEY (tool_run_id, ip_addr)
        REFERENCES raw_ip_addrs(tool_run_id, ip_addr)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

CREATE INDEX raw_operating_systems_idx_tool_run_id
ON raw_operating_systems(tool_run_id);

CREATE INDEX raw_operating_systems_idx_ip_addr
ON raw_operating_systems(ip_addr);

CREATE INDEX raw_operating_systems_idx_vendor_name
ON raw_operating_systems(vendor_name);

CREATE INDEX raw_operating_systems_idx_product_name
ON raw_operating_systems(product_name);

CREATE INDEX raw_operating_systems_idx_product_version
ON raw_operating_systems(product_version);

CREATE INDEX raw_operating_systems_idx_cpe
ON raw_operating_systems(cpe);

CREATE INDEX raw_operating_systems_idx_accuracy
ON raw_operating_systems(accuracy);


-- ----------------------------------------------------------------------

CREATE TABLE raw_tool_observations (
    tool_run_id                 UUID            NOT NULL,
    category                    TEXT            NOT NULL,
    observation                 TEXT            NOT NULL,
    PRIMARY KEY (tool_run_id, category, observation),
    FOREIGN KEY (tool_run_id)
      REFERENCES tool_runs(id)
      ON DELETE CASCADE
      ON UPDATE CASCADE
);

CREATE INDEX raw_tool_observations_idx_tool_run_id
ON raw_tool_observations(tool_run_id);
CREATE INDEX raw_tool_observations_idx_category
ON raw_tool_observations(category);
CREATE INDEX raw_tool_observations_idx_observation
ON raw_tool_observations(observation);


-- ----------------------------------------------------------------------

COMMIT TRANSACTION;
