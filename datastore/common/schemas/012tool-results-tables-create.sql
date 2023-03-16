-- =============================================================================
-- Copyright 2022 National Technology & Engineering Solutions of Sandia, LLC
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

-- Partial indexes
CREATE INDEX raw_mac_addrs_idx_tool_run_id
ON raw_mac_addrs(tool_run_id);

CREATE INDEX raw_mac_addrs_idx_mac_addr
ON raw_mac_addrs(mac_addr);

CREATE INDEX raw_mac_addrs_idx_is_responding
ON raw_mac_addrs(is_responding);



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

-- Partial indexes
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

-- Partial indexes
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

-- Partial indexes
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

CREATE TABLE raw_dns_lookups (
    tool_run_id                 UUID            NOT NULL,
    resolver_ip_addr            INET            NOT NULL,
    resolver_port               PortNumber      NOT NULL,
    query_fqdn                  TEXT            NOT NULL,
    query_class                 TEXT            NOT NULL,
    query_type                  TEXT            NOT NULL,
    response_status             TEXT            NOT NULL,
    response_section            TEXT            NOT NULL,
    response_fqdn               TEXT            NOT NULL,
    response_class              TEXT            NOT NULL,
    response_type               TEXT            NOT NULL,
    response_ttl                INT             NOT NULL,
    response_data               TEXT            NOT NULL,
    PRIMARY KEY (tool_run_id, resolver_ip_addr, resolver_port,
                 query_fqdn, query_class, query_type,
                 response_status, response_section,
                 response_fqdn, response_class, response_type,
                 response_ttl, response_data),
    FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE,
    CHECK (resolver_ip_addr = host(resolver_ip_addr)::INET),
    CHECK (query_fqdn  = lower(query_fqdn)),
    CHECK (query_class = upper(query_class)),
    CHECK (query_type  = upper(query_type)),
    CHECK (response_status  = upper(response_status)),
    CHECK (response_section = upper(response_section)),
    CHECK (response_fqdn  = lower(response_fqdn)),
    CHECK (response_class = upper(response_class)),
    CHECK (response_type  = upper(response_type))
);

-- Partial indexes
CREATE INDEX raw_dns_lookups_idx_tool_run_id
ON raw_dns_lookups(tool_run_id);

CREATE INDEX raw_dns_lookups_idx_resolver_ip_addr
ON raw_dns_lookups(resolver_ip_addr);

CREATE INDEX raw_dns_lookups_idx_resolver_port
ON raw_dns_lookups(resolver_port);

CREATE INDEX raw_dns_lookups_idx_query_fqdn
ON raw_dns_lookups(query_fqdn);

CREATE INDEX raw_dns_lookups_idx_query_class
ON raw_dns_lookups(query_class);

CREATE INDEX raw_dns_lookups_idx_query_type
ON raw_dns_lookups(query_type);

CREATE INDEX raw_dns_lookups_idx_response_status
ON raw_dns_lookups(response_status);

CREATE INDEX raw_dns_lookups_idx_response_section
ON raw_dns_lookups(response_section);

CREATE INDEX raw_dns_lookups_idx_response_fqdn
ON raw_dns_lookups(response_fqdn);

CREATE INDEX raw_dns_lookups_idx_response_class
ON raw_dns_lookups(response_class);

CREATE INDEX raw_dns_lookups_idx_response_type
ON raw_dns_lookups(response_type);

CREATE INDEX raw_dns_lookups_idx_response_ttl
ON raw_dns_lookups(response_ttl);

CREATE INDEX raw_dns_lookups_idx_response_data
ON raw_dns_lookups(response_data);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_dns_lookups_idx_views
ON raw_dns_lookups
   (resolver_ip_addr, resolver_port,
    query_fqdn, query_class, query_type,
    response_status, response_section,
    response_fqdn, response_class, response_type,
    response_ttl, response_data);


-- ----------------------------------------------------------------------

CREATE TABLE raw_vlans (
    tool_run_id                 UUID            NOT NULL,
    vlan                        VlanNumber      NOT NULL,
    description                 TEXT            NULL,
    PRIMARY KEY (tool_run_id, vlan),
    FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
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

-- Partial indexes
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
    vlan                        VlanNumber      NOT NULL,
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

-- Partial indexes
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
    next_hop_ip_addr            INET            NOT NULL,
    dst_ip_addr                 INET            NOT NULL,
    PRIMARY KEY (tool_run_id, hop_count, next_hop_ip_addr, dst_ip_addr),
    FOREIGN KEY (tool_run_id, next_hop_ip_addr)
        REFERENCES raw_ip_addrs(tool_run_id, ip_addr)
        ON DELETE CASCADE
        ON UPDATE CASCADE,
    FOREIGN KEY (tool_run_id, dst_ip_addr)
        REFERENCES raw_ip_addrs(tool_run_id, ip_addr)
        ON DELETE CASCADE
        ON UPDATE CASCADE,
    CHECK (hop_count BETWEEN 0 AND 255)
);

-- Partial indexes
CREATE INDEX raw_ip_traceroutes_idx_tool_run_id
ON raw_ip_traceroutes(tool_run_id);

CREATE INDEX raw_ip_traceroutes_idx_hop_count
ON raw_ip_traceroutes(hop_count);

CREATE INDEX raw_ip_traceroutes_idx_next_hop_ip_addr
ON raw_ip_traceroutes(next_hop_ip_addr);

CREATE INDEX raw_ip_traceroutes_idx_dst_ip_addr
ON raw_ip_traceroutes(dst_ip_addr);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_ip_traceroutes_idx_views
ON raw_ip_traceroutes(hop_count, next_hop_ip_addr, dst_ip_addr);

-- ----------------------------------------------------------------------

CREATE TABLE raw_packages (
    tool_run_id                  UUID            NOT NULL,
    package_state                TEXT            NULL,
    package_name                 TEXT            NOT NULL,
    package_version              TEXT            NOT NULL,
    package_architecture         TEXT            NOT NULL,
    package_description          TEXT            NOT NULL,
    PRIMARY KEY (tool_run_id, package_state, package_name, package_version, package_architecture, package_description),
    FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
CREATE INDEX raw_packages_idx_tool_run_id
ON raw_packages(tool_run_id);

CREATE INDEX raw_packages_idx_package_state
ON raw_packages(package_state);

CREATE INDEX raw_packages_idx_package_name
ON raw_packages(package_name);

CREATE INDEX raw_packages_idx_package_version
ON raw_packages(package_version);

CREATE INDEX raw_packages_idx_package_architecture
ON raw_packages(package_architecture);

CREATE INDEX raw_packages_idx_package_description
ON raw_packages(package_description);

-- Index the primary key without tool_run_id (if not already indexed).
-- Helps the views that ignore the tool_run_id.
CREATE INDEX raw_packages_idx_views
ON raw_packages(package_state, package_name, package_architecture);


-- ----------------------------------------------------------------------

CREATE TABLE raw_ports (
    tool_run_id                 UUID            NOT NULL,
    ip_addr                     INET            NOT NULL,
    protocol                    TEXT            NOT NULL,
    port                        PortNumber      NOT NULL, -- '-1' = 'other'
    port_state                  TEXT            NULL,
    port_reason                 TEXT            NULL,
    PRIMARY KEY (tool_run_id, ip_addr, protocol, port),
    FOREIGN KEY (tool_run_id, ip_addr)
        REFERENCES raw_ip_addrs(tool_run_id, ip_addr)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
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
    port                        PortNumber      NOT NULL,
    service_name                TEXT            NULL,
    service_description         TEXT            NULL,
    service_reason              TEXT            NULL,
    observer_ip_addr            INET            NULL,
    PRIMARY KEY (tool_run_id, ip_addr, protocol, port),
    FOREIGN KEY (tool_run_id, ip_addr, protocol, port)
        REFERENCES raw_ports(tool_run_id, ip_addr, protocol, port)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
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
    port                        PortNumber      NOT NULL,
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
    CHECK (severity BETWEEN 0 AND 4)
);

-- Partial indexes
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
    port                        PortNumber      NOT NULL,
    plugin_id                   INT             NOT NULL,
    cve_id                      CVE             NOT NULL,
    PRIMARY KEY (tool_run_id, ip_addr, protocol, port, plugin_id, cve_id),
    FOREIGN KEY (tool_run_id, ip_addr, protocol, port, plugin_id)
        REFERENCES raw_nessus_results
                   (tool_run_id, ip_addr, protocol, port, plugin_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
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
    port                        PortNumber      NOT NULL,
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

-- Partial indexes
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
    port                        PortNumber      NOT NULL,
    script_id                   TEXT            NOT NULL,
    script_output               TEXT            NULL,
    PRIMARY KEY (tool_run_id, ip_addr, protocol, port, script_id),
    FOREIGN KEY (tool_run_id, ip_addr, protocol, port)
        REFERENCES raw_ports(tool_run_id, ip_addr, protocol, port)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
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
    port                        PortNumber      NOT NULL,
    ssh_key_type                TEXT            NOT NULL,
    ssh_key_bits                INT             NOT NULL,
    ssh_key_fingerprint         TEXT            NOT NULL,
    ssh_key_public              TEXT            NOT NULL,
    PRIMARY KEY (tool_run_id, ip_addr, protocol, port,
                 ssh_key_type, ssh_key_bits, ssh_key_fingerprint),
    FOREIGN KEY (tool_run_id, ip_addr, protocol, port)
        REFERENCES raw_ports(tool_run_id, ip_addr, protocol, port)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
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
    port                        PortNumber      NOT NULL,
    ssh_algo_type               TEXT            NOT NULL,
    ssh_algo_name               TEXT            NOT NULL,
    PRIMARY KEY (tool_run_id, ip_addr, protocol, port,
                 ssh_algo_type, ssh_algo_name),
    FOREIGN KEY (tool_run_id, ip_addr, protocol, port)
        REFERENCES raw_ports(tool_run_id, ip_addr, protocol, port)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Partial indexes
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

-- Since this table lacks a PRIMARY KEY and allows NULLs (>2):
-- Create UNIQUE expressional index with substitutions of NULL values
-- for use with `ON CONFLICT` guards against duplicate data.
CREATE UNIQUE INDEX raw_operating_systems_idx_unique
ON raw_operating_systems
  ((HASH_CHAIN(
      tool_run_id::TEXT, ip_addr::TEXT,
      vendor_name, product_name, product_version,
      cpe, accuracy::TEXT
    )
  ));

-- Partial indexes
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

-- Partial indexes
CREATE INDEX raw_tool_observations_idx_tool_run_id
ON raw_tool_observations(tool_run_id);

CREATE INDEX raw_tool_observations_idx_category
ON raw_tool_observations(category);

CREATE INDEX raw_tool_observations_idx_observation
ON raw_tool_observations(observation);


-- ----------------------------------------------------------------------


CREATE TABLE raw_prowler_checks (
      tool_run_id                 UUID            NOT NULL
    , account_number              TEXT            NULL
    , timestamp                   TIMESTAMP       NULL
    , region                      TEXT            NULL
    , level                       TEXT            NULL
    , control_id                  TEXT            NULL
    , service                     TEXT            NULL
    , status                      TEXT            NULL
    , severity                    ProwlerSeverity NULL
    , control                     TEXT            NULL
    , risk                        TEXT            NULL
    , remediation                 TEXT            NULL
    , documentation_link          TEXT            NULL
    , resource_id                 TEXT            NULL
    , FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);
    

-- ----------------------------------------------------------------------

COMMIT TRANSACTION;
