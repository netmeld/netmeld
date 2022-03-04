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

CREATE VIEW mac_addrs AS
SELECT DISTINCT
    mac_addr                    AS mac_addr,
    BOOL_OR(is_responding)      AS is_responding
FROM raw_mac_addrs
GROUP BY mac_addr
;


-- ----------------------------------------------------------------------

CREATE VIEW raw_mac_addrs_vendors AS
SELECT
    ma.tool_run_id              AS tool_run_id,
    ma.mac_addr                 AS mac_addr,
    vmp.vendor_name             AS vendor_name
FROM raw_mac_addrs AS ma
JOIN vendor_mac_prefixes AS vmp
ON (trunc(ma.mac_addr) = vmp.mac_prefix)
;


-- ----------------------------------------------------------------------

CREATE VIEW mac_addrs_vendors AS
SELECT DISTINCT
    mac_addr                    AS mac_addr,
    vendor_name                 AS vendor_name
FROM raw_mac_addrs_vendors
;


-- ----------------------------------------------------------------------

CREATE VIEW ip_addrs AS
SELECT DISTINCT
    ip_addr                     AS ip_addr,
    BOOL_OR(is_responding)      AS is_responding
FROM raw_ip_addrs
WHERE (ip_addr != '0.0.0.0') AND
      (ip_addr != '::')
GROUP BY ip_addr
;


-- ----------------------------------------------------------------------

CREATE VIEW ip_addrs_without_devices AS
SELECT DISTINCT
    ia.ip_addr                  AS ip_addr,
    ia.is_responding            AS is_responding
FROM ip_addrs AS ia
LEFT OUTER JOIN raw_device_ip_addrs AS dia
ON (ia.ip_addr = dia.ip_addr)
WHERE (dia.ip_addr IS NULL)
;


-- ----------------------------------------------------------------------

CREATE VIEW raw_bcast_response_ip_addrs AS
SELECT DISTINCT
    ia.tool_run_id              AS tool_run_id,
    ia.ip_addr                  AS ip_addr,
    ia.is_responding            AS is_responding
FROM raw_ip_addrs AS ia
JOIN tool_runs AS tr
ON (ia.tool_run_id = tr.id)
WHERE ((tr.tool_name = 'ping') AND (tr.command_line LIKE '% -b %'))
;


-- ----------------------------------------------------------------------

CREATE VIEW bcast_response_ip_addrs AS
SELECT DISTINCT
    ip_addr                     AS ip_addr,
    BOOL_OR(is_responding)      AS is_responding
FROM raw_bcast_response_ip_addrs
GROUP BY ip_addr
;


-- ----------------------------------------------------------------------

CREATE VIEW raw_router_ip_addrs AS
SELECT DISTINCT
    ia.tool_run_id              AS tool_run_id,
    ia.ip_addr                  AS ip_addr
FROM raw_ip_addrs AS ia
JOIN tool_runs AS tr
  ON (ia.tool_run_id = tr.id)
WHERE ((tr.tool_name = 'ping6') AND (tr.command_line LIKE '% ff02::2'))
UNION
SELECT DISTINCT
    dir.tool_run_id             AS tool_run_id,
    dir.next_hop_ip_addr        AS ip_addr
FROM raw_device_ip_routes AS dir
UNION
SELECT DISTINCT
    rit.tool_run_id             AS tool_run_id,
    rit.next_hop_ip_addr        AS ip_addr
FROM raw_ip_traceroutes AS rit
WHERE (rit.next_hop_ip_addr != rit.dst_ip_addr)
;


-- ----------------------------------------------------------------------

CREATE VIEW router_ip_addrs AS
SELECT DISTINCT
    rria.ip_addr                AS ip_addr,
    BOOL_OR(ia.is_responding)   AS is_responding
FROM raw_router_ip_addrs AS rria
JOIN ip_addrs AS ia
  ON rria.ip_addr = ia.ip_addr
GROUP BY rria.ip_addr
;


-- ----------------------------------------------------------------------

CREATE VIEW mac_addrs_ip_addrs AS
SELECT DISTINCT
    mac_addr                    AS mac_addr,
    ip_addr                     AS ip_addr
FROM raw_mac_addrs_ip_addrs
;


-- ----------------------------------------------------------------------

CREATE VIEW mac_addrs_ipv4_addrs_ipv6_addrs AS
SELECT DISTINCT
    maia4.mac_addr              AS mac_addr,
    maia4.ip_addr               AS ipv4_addr,
    maia6.ip_addr               AS ipv6_addr
FROM mac_addrs_ip_addrs AS maia4
JOIN mac_addrs_ip_addrs AS maia6
ON (maia4.mac_addr = maia6.mac_addr)
WHERE (4 = family(maia4.ip_addr)) AND (6 = family(maia6.ip_addr))
;


-- ----------------------------------------------------------------------

CREATE VIEW mac_addrs_without_ip_addrs AS
SELECT DISTINCT
    ma.mac_addr                 AS mac_addr,
    ma.is_responding            AS is_responding
FROM mac_addrs AS ma
LEFT OUTER JOIN mac_addrs_ip_addrs as maia
ON (ma.mac_addr = maia.mac_addr)
WHERE (maia.ip_addr IS NULL)
;


-- ----------------------------------------------------------------------

CREATE VIEW ip_addrs_without_mac_addrs AS
SELECT DISTINCT
    ia.ip_addr                  AS ip_addr,
    ia.is_responding            AS is_responding
FROM ip_addrs AS ia
LEFT OUTER JOIN mac_addrs_ip_addrs as maia
ON (ia.ip_addr = maia.ip_addr)
WHERE (maia.mac_addr IS NULL)
;


-- ----------------------------------------------------------------------

CREATE VIEW hostnames AS
SELECT DISTINCT
    ip_addr                     AS ip_addr,
    hostname                    AS hostname,
    reason                      AS reason
FROM raw_hostnames
;


-- ----------------------------------------------------------------------

CREATE VIEW dns_lookups AS
SELECT DISTINCT
    resolver_ip_addr            AS resolver_ip_addr,
    resolver_port               AS resolver_port,
    query_fqdn                  AS query_fqdn,
    query_class                 AS query_class,
    query_type                  AS query_type,
    response_status             AS response_status,
    response_section            AS response_section,
    response_fqdn               AS response_fqdn,
    response_class              AS response_class,
    response_type               AS response_type,
    response_ttl                AS response_ttl,
    response_data               AS response_data
FROM raw_dns_lookups
;


-- ----------------------------------------------------------------------

CREATE VIEW dns_ip_addrs AS
WITH RECURSIVE dns_ip_addrs_recursion(
    resolver_ip_addr,
    resolver_port,
    response_fqdn,
    response_data
) AS (
    -- Base case: direct resolutions in any response section
    SELECT DISTINCT
        dns_lookups.resolver_ip_addr  AS resolver_ip_addr,
        dns_lookups.resolver_port     AS resolver_port,
        dns_lookups.response_fqdn     AS response_fqdn,
        dns_lookups.response_data     AS response_data
    FROM dns_lookups
    WHERE (response_status = 'NOERROR') AND
          (response_class = 'IN') AND
          ((response_type = 'A') OR
           (response_type = 'AAAA'))
    UNION
    -- Recursive case: indirect resolutions via CNAME chaining
    SELECT DISTINCT
        dns_lookups.resolver_ip_addr  AS resolver_ip_addr,
        dns_lookups.resolver_port     AS resolver_port,
        dns_lookups.response_fqdn     AS response_fqdn,
        dns_recur.response_data       AS response_data
    FROM dns_ip_addrs_recursion AS dns_recur
    JOIN dns_lookups
    ON (dns_lookups.resolver_ip_addr = dns_recur.resolver_ip_addr) AND
       (dns_lookups.resolver_port = dns_recur.resolver_port) AND
       (dns_lookups.response_type = 'CNAME') AND
       (dns_lookups.response_data = dns_recur.response_fqdn)
)
SELECT DISTINCT
    dns_recur.resolver_ip_addr      	AS resolver_ip_addr,
    dns_recur.resolver_port           AS resolver_port,
    dns_recur.response_fqdn           AS fqdn,
    dns_recur.response_data::INET     AS ip_addr
FROM dns_ip_addrs_recursion AS dns_recur
;


-- ----------------------------------------------------------------------

CREATE VIEW ip_nets AS
SELECT DISTINCT
    ip_net                      AS ip_net,
    description                 AS description
FROM raw_ip_nets
WHERE (((family(ip_net) = 4) AND (masklen(ip_net) <  32)) OR
       ((family(ip_net) = 6) AND (masklen(ip_net) < 128)))
;


-- ----------------------------------------------------------------------

CREATE VIEW ip_nets_overlapping AS
SELECT DISTINCT
    ip_nets_0.ip_net            AS ip_net_larger,
    ip_nets_1.ip_net            AS ip_net_smaller
FROM raw_ip_nets AS ip_nets_0
JOIN raw_ip_nets AS ip_nets_1
ON (ip_nets_0.ip_net >> ip_nets_1.ip_net)
;


-- ----------------------------------------------------------------------
-- Device VLAN views
-- ----------------------------------------------------------------------

CREATE VIEW vlans AS
SELECT DISTINCT
    vlan                        AS vlan,
    description                 AS description
FROM raw_vlans
;


CREATE VIEW vlans_ip_nets AS
SELECT DISTINCT
    vlan                        AS vlan,
    ip_net                      AS ip_net
FROM raw_vlans_ip_nets
;


CREATE VIEW raw_vlans_summaries AS
SELECT DISTINCT
  rvin.tool_run_id              AS tool_run_id,
  rvin.vlan                     AS vlan,
  rvin.ip_net                   AS ip_net,
  rv.description                AS description
FROM raw_vlans_ip_nets AS rvin
JOIN raw_vlans AS rv
  ON (rvin.tool_run_id = rv.tool_run_id)
 AND (rvin.vlan = rv.vlan)
;


CREATE VIEW vlans_summaries AS
SELECT DISTINCT
  vlan                          AS vlan,
  ip_net                        AS ip_net,
  description                   AS description
FROM raw_vlans_summaries
ORDER BY vlan, ip_net
;


-- ----------------------------------------------------------------------

CREATE VIEW ip_traceroutes AS
SELECT DISTINCT
  rd1.device_id AS origin,
  rit1.dst_ip_addr AS last_hop,
  rit1.hop_count,
  rit1.next_hop_ip_addr
FROM raw_ip_traceroutes AS rit1
LEFT JOIN raw_devices AS rd1
  ON rit1.tool_run_id = rd1.tool_run_id
ORDER BY origin, last_hop, hop_count
;

-- ----------------------------------------------------------------------

CREATE VIEW ports AS
SELECT DISTINCT
    ip_addr                     AS ip_addr,
    protocol                    AS protocol,
    port                        AS port,
    port_state                  AS port_state,
    port_reason                 AS port_reason
FROM raw_ports
;


-- ----------------------------------------------------------------------

CREATE VIEW raw_intra_network_ports AS
SELECT
    p.tool_run_id               AS tool_run_id,
    tria.ip_addr                AS src_ip_addr,
    p.ip_addr                   AS dst_ip_addr,
    p.protocol                  AS protocol,
    p.port                      AS port,
    p.port_state                AS port_state,
    p.port_reason               AS port_reason
FROM raw_ports AS p
JOIN tool_run_ip_addrs AS tria
ON (p.tool_run_id = tria.tool_run_id) AND
   (inet_same_family(p.ip_addr, tria.ip_addr)) AND
   (p.ip_addr <<= network(tria.ip_addr)) AND
   (p.ip_addr != host(tria.ip_addr)::INET)
--   (host(p.ip_addr)::INET != host(tria.ip_addr)::INET)
JOIN tool_run_interfaces AS trif
ON (tria.tool_run_id = trif.tool_run_id) AND
   (tria.interface_name = trif.interface_name) AND
   (trif.is_up)
;


-- ----------------------------------------------------------------------

CREATE VIEW intra_network_ports AS
SELECT DISTINCT
    src_ip_addr                 AS src_ip_addr,
    dst_ip_addr                 AS dst_ip_addr,
    protocol                    AS protocol,
    port                        AS port,
    port_state                  AS port_state,
    port_reason                 AS port_reason
FROM raw_intra_network_ports
;


-- ----------------------------------------------------------------------

CREATE VIEW raw_inter_network_ports AS
SELECT
    p.tool_run_id               AS tool_run_id,
    tria.ip_addr                AS src_ip_addr,
    trir.next_hop_ip_addr       AS next_hop_ip_addr,
    p.ip_addr                   AS dst_ip_addr,
    p.protocol                  AS protocol,
    p.port                      AS port,
    p.port_state                AS port_state,
    p.port_reason               AS port_reason
FROM raw_ports AS p
JOIN tool_run_ip_addrs AS tria
ON (p.tool_run_id = tria.tool_run_id) AND
   (inet_same_family(p.ip_addr, tria.ip_addr)) AND
   (NOT (p.ip_addr <<= network(tria.ip_addr)))
JOIN tool_run_interfaces AS trif
ON (tria.tool_run_id = trif.tool_run_id) AND
   (tria.interface_name = trif.interface_name) AND
   (trif.is_up)
JOIN tool_run_ip_routes trir
ON (tria.tool_run_id = trir.tool_run_id) AND
   ((tria.interface_name = trir.interface_name) OR
    (left(tria.interface_name,
          -length(substring(tria.interface_name from '@[^@]*$'))) = trir.interface_name)) AND
   (inet_same_family(tria.ip_addr, trir.next_hop_ip_addr)) AND
   (p.ip_addr <<= trir.dst_ip_net) AND
   (trir.next_hop_ip_addr <<= network(tria.ip_addr) OR tria.ip_addr = host(tria.ip_addr)::INET) AND
--   (trir.next_hop_ip_addr <<= network(tria.ip_addr)) AND
   (trir.next_hop_ip_addr != '0.0.0.0') AND
   (trir.next_hop_ip_addr != '::')
;


-- ----------------------------------------------------------------------

CREATE VIEW inter_network_ports AS
SELECT DISTINCT
    src_ip_addr                 AS src_ip_addr,
    next_hop_ip_addr            AS next_hop_ip_addr,
    dst_ip_addr                 AS dst_ip_addr,
    protocol                    AS protocol,
    port                        AS port,
    port_state                  AS port_state,
    port_reason                 AS port_reason
FROM raw_inter_network_ports
;


-- ----------------------------------------------------------------------

CREATE VIEW network_services AS
SELECT DISTINCT
    ip_addr                     AS ip_addr,
    protocol                    AS protocol,
    port                        AS port,
    service_name                AS service_name,
    service_description         AS service_description,
    service_reason              AS service_reason,
    observer_ip_addr            AS observer_ip_addr
FROM raw_network_services
;


-- ----------------------------------------------------------------------

CREATE VIEW ports_services AS
SELECT DISTINCT
    p.ip_addr                   AS ip_addr,
    p.protocol                  AS protocol,
    p.port                      AS port,
    p.port_state                AS port_state,
    p.port_reason               AS port_reason,
    s.service_name              AS service_name,
    s.service_description       AS service_description,
    s.service_reason            AS service_reason
FROM ports AS p
LEFT OUTER JOIN network_services AS s
ON (p.ip_addr = s.ip_addr) AND
   (p.protocol = s.protocol) AND
   (p.port = s.port)
;


-- ----------------------------------------------------------------------

CREATE VIEW intra_network_ports_services AS
SELECT DISTINCT
    inp.src_ip_addr             AS src_ip_addr,
    inp.dst_ip_addr             AS dst_ip_addr,
    inp.protocol                AS protocol,
    inp.port                    AS port,
    inp.port_state              AS port_state,
    inp.port_reason             AS port_reason,
    s.service_name              AS service_name,
    s.service_description       AS service_description,
    s.service_reason            AS service_reason
FROM intra_network_ports AS inp
LEFT OUTER JOIN network_services AS s
ON (inp.dst_ip_addr = s.ip_addr) AND
   (inp.protocol = s.protocol) AND
   (inp.port = s.port)
;

-- ----------------------------------------------------------------------

CREATE VIEW inter_network_ports_services AS
SELECT DISTINCT
    inp.src_ip_addr             AS src_ip_addr,
    inp.next_hop_ip_addr        AS next_hop_ip_addr,
    inp.dst_ip_addr             AS dst_ip_addr,
    inp.protocol                AS protocol,
    inp.port                    AS port,
    inp.port_state              AS port_state,
    inp.port_reason             AS port_reason,
    s.service_name              AS service_name,
    s.service_description       AS service_description,
    s.service_reason            AS service_reason
FROM inter_network_ports AS inp
LEFT OUTER JOIN network_services AS s
ON (inp.dst_ip_addr = s.ip_addr) AND
   (inp.protocol = s.protocol) AND
   (inp.port = s.port)
;

-- ----------------------------------------------------------------------

CREATE VIEW nessus_results AS
SELECT DISTINCT
    ip_addr                     AS ip_addr,
    protocol                    AS protocol,
    port                        AS port,
    plugin_id                   AS plugin_id,
    plugin_name                 AS plugin_name,
    plugin_family               AS plugin_family,
    plugin_type                 AS plugin_type,
    plugin_output               AS plugin_output,
    severity                    AS severity,
    description                 AS description,
    solution                    AS solution
FROM raw_nessus_results
;


-- ----------------------------------------------------------------------

CREATE VIEW nessus_results_cves AS
SELECT DISTINCT
    ip_addr                     AS ip_addr,
    protocol                    AS protocol,
    port                        AS port,
    plugin_id                   AS plugin_id,
    cve_id                      AS cve_id
FROM raw_nessus_results_cves
;


-- ----------------------------------------------------------------------

CREATE VIEW nessus_results_metasploit_modules AS
SELECT DISTINCT
    ip_addr                     AS ip_addr,
    protocol                    AS protocol,
    port                        AS port,
    plugin_id                   AS plugin_id,
    metasploit_name             AS metasploit_name
FROM raw_nessus_results_metasploit_modules
;


-- ----------------------------------------------------------------------

CREATE VIEW nse_results AS
SELECT DISTINCT
    ip_addr                     AS ip_addr,
    protocol                    AS protocol,
    port                        AS port,
    script_id                   AS script_id,
    script_output               AS script_output
FROM raw_nse_results
;


-- ----------------------------------------------------------------------

CREATE VIEW ssh_host_public_keys AS
SELECT DISTINCT
    ip_addr                     AS ip_addr,
    protocol                    AS protocol,
    port                        AS port,
    ssh_key_type                AS ssh_key_type,
    ssh_key_bits                AS ssh_key_bits,
    ssh_key_fingerprint         AS ssh_key_fingerprint,
    ssh_key_public              AS ssh_key_public
FROM raw_ssh_host_public_keys
;


-- ----------------------------------------------------------------------

CREATE VIEW ssh_host_algorithms AS
SELECT DISTINCT
    ip_addr                     AS ip_addr,
    protocol                    AS protocol,
    port                        AS port,
    ssh_algo_type               AS ssh_algo_type,
    ssh_algo_name               AS ssh_algo_name
FROM raw_ssh_host_algorithms
;


-- ----------------------------------------------------------------------

CREATE VIEW operating_systems AS
SELECT DISTINCT
    ip_addr                     AS ip_addr,
    vendor_name                 AS vendor_name,
    product_name                AS product_name,
    product_version             AS product_version,
    cpe                         AS cpe,
    accuracy                    AS accuracy
FROM raw_operating_systems
;


-- ----------------------------------------------------------------------


CREATE VIEW tool_observations AS
SELECT DISTINCT
  tr.tool_name                  AS tool_name,
  tr.data_path                  AS data_path,
  rto.category                  AS category,
  rto.observation               AS observation
FROM raw_tool_observations AS rto
LEFT OUTER JOIN tool_runs AS tr
   ON (rto.tool_run_id = tr.id)
;

-- ----------------------------------------------------------------------


COMMIT TRANSACTION;
