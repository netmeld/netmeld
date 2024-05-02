-- =============================================================================
-- Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC
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

CREATE VIEW devices AS
SELECT DISTINCT
    device_id                   AS device_id
FROM raw_devices
;


-- ----------------------------------------------------------------------

CREATE VIEW device_hardware_information AS
SELECT DISTINCT
    device_id                                     AS device_id
  , string_agg(distinct device_type, ',')         AS device_type
  , string_agg(distinct vendor, ',')              AS vendor
  , string_agg(distinct model, ',')               AS model
  , string_agg(distinct hardware_revision, ',')   AS hardware_revision
  , string_agg(distinct serial_number, ',')       AS serial_number
  , string_agg(distinct description, ',')         AS description
FROM raw_device_hardware_information
GROUP BY device_id
;


-- ----------------------------------------------------------------------

CREATE VIEW device_virtualizations AS
SELECT DISTINCT
    host_device_id              AS host_device_id
  , guest_device_id             AS guest_device_id
FROM raw_device_virtualizations
;


-- ----------------------------------------------------------------------

CREATE VIEW device_vrfs AS
SELECT DISTINCT
    device_id                   AS device_id
  , vrf_id                      AS vrf_id
FROM raw_device_vrfs
;


-- ----------------------------------------------------------------------

CREATE VIEW device_vrfs_interfaces AS
SELECT DISTINCT
    device_id                   AS device_id
  , vrf_id                      AS vrf_id
  , interface_name              AS interface_name
FROM raw_device_vrfs_interfaces
;


-- ----------------------------------------------------------------------

CREATE VIEW device_interfaces AS
SELECT DISTINCT
    device_id                   AS device_id
  , interface_name              AS interface_name
  , media_type                  AS media_type
  , is_up                       AS is_up
  , description                 AS description
FROM raw_device_interfaces
;


-- ----------------------------------------------------------------------

CREATE VIEW device_interface_hierarchies AS
SELECT DISTINCT
    device_id                   AS device_id
  , underlying_interface_name   AS underlying_interface_name
  , virtual_interface_name      AS virtual_interface_name
FROM raw_device_interface_hierarchies
;


-- ----------------------------------------------------------------------

CREATE VIEW device_mac_addrs AS
SELECT DISTINCT
    device_id                   AS device_id
  , interface_name              AS interface_name
  , mac_addr                    AS mac_addr
FROM raw_device_mac_addrs
;


-- ----------------------------------------------------------------------

CREATE VIEW device_ip_addrs AS
SELECT DISTINCT
    device_id                   AS device_id
  , interface_name              AS interface_name
  , ip_addr                     AS ip_addr
  , ip_net                      AS ip_net
FROM raw_device_ip_addrs
;


-- ----------------------------------------------------------------------

CREATE VIEW device_vrfs_ip_addrs AS
SELECT DISTINCT
    dia.device_id               AS device_id
  , dvi.vrf_id                  AS vrf_id
  , dia.interface_name          AS interface_name
  , dia.ip_addr                 AS ip_addr
  , dia.ip_net                  AS ip_net
FROM device_ip_addrs AS dia
LEFT JOIN device_vrfs_interfaces AS dvi
  ON (dia.device_id       = dvi.device_id)
 AND (dia.interface_name  = dvi.interface_name)
;


-- ----------------------------------------------------------------------

CREATE VIEW device_mac_addrs_ip_addrs AS
SELECT DISTINCT
    dma.device_id               AS device_id
  , dma.interface_name          AS interface_name
  , dma.mac_addr                AS mac_addr
  , dia.ip_addr                 AS ip_addr
FROM device_mac_addrs AS dma
JOIN device_ip_addrs AS dia
  ON (dma.device_id       = dia.device_id)
 AND (dma.interface_name  = dia.interface_name)
UNION
-- MAC/IP pairs where associated device was unkown
SELECT DISTINCT
    dma.device_id               AS device_id
  , dma.interface_name          AS interface_name
  , maia.mac_addr               AS mac_addr
  , maia.ip_addr                AS ip_addr
FROM device_mac_addrs AS dma
JOIN mac_addrs_ip_addrs AS maia
  ON (dma.mac_addr = maia.mac_addr)
UNION
SELECT DISTINCT
    dia.device_id               AS device_id
  , dia.interface_name          AS interface_name
  , maia.mac_addr               AS mac_addr
  , maia.ip_addr                AS ip_addr
FROM device_ip_addrs AS dia
JOIN mac_addrs_ip_addrs AS maia
  ON (dia.ip_addr = maia.ip_addr)
;


-- ----------------------------------------------------------------------

CREATE VIEW device_ip_routes AS
SELECT DISTINCT
    device_id                                   AS device_id
  , vrf_id                                      AS vrf_id
  , table_id                                    AS table_id
  , is_active                                   AS is_active
  , dst_ip_net                                  AS dst_ip_net
  , next_vrf_id                                 AS next_vrf_id
  , next_table_id                               AS next_table_id
  , next_hop_ip_addr                            AS next_hop_ip_addr
  , nullif(max(COALESCE(outgoing_interface_name, '')), '')
                                                AS outgoing_interface_name
  , protocol                                    AS protocol
  , administrative_distance                     AS administrative_distance
  , metric                                      AS metric
  , nullif(max(COALESCE(description, '')), '')  AS description
FROM raw_device_ip_routes
GROUP BY
    device_id
  , vrf_id
  , table_id
  , is_active
  , dst_ip_net
  , next_vrf_id
  , next_table_id
  , next_hop_ip_addr
  , protocol
  , administrative_distance
  , metric
;


CREATE OR REPLACE VIEW device_ip_route_connections AS
WITH cte1 AS (
  SELECT DISTINCT
      device_id
    , vrf_id
    , interface_name
    , ip_addr
    , ip_net
  FROM device_vrfs_ip_addrs
)
-- Implicit routes to directly-connected routable networks:
SELECT DISTINCT
  -- Destination network:
    self_outgoing_addrs.ip_net          AS dst_ip_net
  -- Self device:
  , self_incoming_addrs.device_id       AS device_id
  , COALESCE(
      self_incoming_addrs.vrf_id
    , self_outgoing_addrs.vrf_id
    )                                   AS vrf_id
  , NULL::TEXT                          AS table_id
  , true                                AS is_active
  , 'local'::TEXT                       AS protocol
  , 0::INT                              AS administrative_distance
  , 0::INT                              AS metric
  , self_incoming_addrs.interface_name  AS incoming_interface_name
  , self_incoming_addrs.ip_addr         AS incoming_ip_addr
  , self_incoming_addrs.ip_net          AS incoming_ip_net
  , self_outgoing_addrs.interface_name  AS outgoing_interface_name
  , self_outgoing_addrs.ip_addr         AS outgoing_ip_addr
  , self_outgoing_addrs.ip_net          AS outgoing_ip_net
  -- Peer device:
  , NULL::TEXT                          AS next_hop_device_id
  , NULL::TEXT                          AS next_hop_vrf_id
  , NULL::TEXT                          AS next_hop_incoming_interface_name
  , NULL::INET                          AS next_hop_incoming_ip_addr
  , NULL::CIDR                          AS next_hop_incoming_ip_net
FROM cte1 AS self_incoming_addrs
JOIN cte1 AS self_outgoing_addrs
  ON (self_incoming_addrs.device_id = self_outgoing_addrs.device_id)
-- AND (  (self_incoming_addrs.vrf_id = self_outgoing_addrs.vrf_id)
--     OR (self_incoming_addrs.vrf_id IS NULL)
--     OR (self_outgoing_addrs.vrf_id IS NULL)
--     )
 AND (self_incoming_addrs.interface_name != self_outgoing_addrs.interface_name)
 AND (inet_same_family(self_incoming_addrs.ip_net, self_outgoing_addrs.ip_net))
 AND (NOT self_incoming_addrs.ip_net && self_outgoing_addrs.ip_net)
-- Since these are one-hop routes, both directly-connected networks
-- MUST be routable (not link-local) networks.
WHERE (NOT self_incoming_addrs.ip_net <<= 'fe80::/10'::CIDR)
  AND (NOT self_outgoing_addrs.ip_net <<= 'fe80::/10'::CIDR)
  AND (NOT self_incoming_addrs.ip_net <<= '169.254.0.0/16'::CIDR)
  AND (NOT self_outgoing_addrs.ip_net <<= '169.254.0.0/16'::CIDR)
UNION
-- Explicit routes:
SELECT DISTINCT
  -- Destination network:
    self_routes.dst_ip_net              AS dst_ip_net
  -- Self device:
  , self_routes.device_id               AS device_id
  , self_routes.vrf_id                  AS vrf_id
  , self_routes.table_id                AS table_id
  , self_routes.is_active               AS is_active
  , self_routes.protocol                AS protocol
  , self_routes.administrative_distance AS administrative_distance
  , self_routes.metric                  AS metric
  , self_incoming_addrs.interface_name  AS incoming_interface_name
  , self_incoming_addrs.ip_addr         AS incoming_ip_addr
  , self_incoming_addrs.ip_net          AS incoming_ip_net
  , COALESCE(
        self_routes.outgoing_interface_name
      , self_outgoing_addrs.interface_name
    )                                   AS outgoing_interface_name
  , self_outgoing_addrs.ip_addr         AS outgoing_ip_addr
  , self_outgoing_addrs.ip_net          AS outgoing_ip_net
  -- Peer device:
  , peer_incoming_addrs.device_id       AS next_hop_device_id
  , peer_incoming_addrs.vrf_id          AS next_hop_vrf_id
  , peer_incoming_addrs.interface_name  AS next_hop_incoming_interface_name
  , self_routes.next_hop_ip_addr        AS next_hop_incoming_ip_addr
  , peer_incoming_addrs.ip_net          AS next_hop_incoming_ip_net
FROM device_ip_routes AS self_routes
JOIN cte1 AS self_incoming_addrs
  ON (self_routes.device_id = self_incoming_addrs.device_id)
 AND (  (self_routes.vrf_id = self_incoming_addrs.vrf_id)
     OR (self_incoming_addrs.vrf_id IS NULL)
     )
 AND (inet_same_family(self_routes.dst_ip_net, self_incoming_addrs.ip_net))
 AND (  (self_routes.next_hop_ip_addr != self_incoming_addrs.ip_addr)
     OR (self_routes.next_hop_ip_addr IS NULL)
     )
 AND (NOT self_routes.dst_ip_net <<= self_incoming_addrs.ip_net)
JOIN cte1 AS self_outgoing_addrs
  ON (self_routes.device_id = self_outgoing_addrs.device_id)
-- AND (  (self_routes.vrf_id = self_outgoing_addrs.vrf_id)
--     OR (self_outgoing_addrs.vrf_id IS NULL)
--     )
 AND (inet_same_family(self_routes.dst_ip_net, self_outgoing_addrs.ip_net))
 AND (self_routes.next_hop_ip_addr <<= self_outgoing_addrs.ip_net)
 AND (self_routes.next_hop_ip_addr != self_outgoing_addrs.ip_addr)
 AND (NOT self_routes.dst_ip_net <<= self_outgoing_addrs.ip_net)
 AND (  (self_outgoing_addrs.interface_name != self_incoming_addrs.interface_name)
     OR (self_outgoing_addrs.interface_name IS NULL)
     )
LEFT JOIN cte1 AS peer_incoming_addrs
  ON (self_routes.next_hop_ip_addr = peer_incoming_addrs.ip_addr)
;


-- ----------------------------------------------------------------------

CREATE OR REPLACE FUNCTION ip_route_paths (
    arg_src_ip_net INET
  , arg_dst_ip_net INET
  , arg_device_ids TEXT[] DEFAULT '{}'
)
RETURNS TABLE (
    src_ip_net              CIDR
  , dst_ip_net              CIDR
  , next_hop_ip_addr        INET
  , hop_count               INT
  , route_path              TEXT[]
  , route_path_detail       RouteHop[]
) AS $$
WITH RECURSIVE device_ip_route_recursion (
  -- Source network:
    src_ip_net
  -- Destination network:
  , dst_ip_net
  -- Next router in the path:
  , next_hop_device_id
  , next_hop_vrf_id
  , next_hop_incoming_interface_name
  , next_hop_incoming_ip_addr
  , next_hop_incoming_ip_net
  -- ARRAY of all hops along the path:
  , route_path
  , route_path_detail
) AS (
    WITH cte1 AS (
      SELECT DISTINCT
          *
      FROM device_ip_route_connections
      WHERE (is_active = true)
        AND ( (   (incoming_interface_name NOT LIKE 'management%')
              AND (incoming_interface_name NOT LIKE 'mgmt%')
              )
            OR (incoming_interface_name IS NULL)
            )
        AND ( (   (outgoing_interface_name NOT LIKE 'management%')
              AND (outgoing_interface_name NOT LIKE 'mgmt%')
              )
            OR (outgoing_interface_name IS NULL)
            )
    )
    -- Base case: First router in path to destination.
    SELECT DISTINCT
     -- Source network:
        route_conns.incoming_ip_net                   AS src_ip_net
     -- Destination network:
     ,  route_conns.dst_ip_net                        AS dst_ip_net
     -- Next router in the path:
     ,  route_conns.next_hop_device_id                AS next_hop_device_id
     ,  route_conns.next_hop_vrf_id                   AS next_hop_vrf_id
     ,  route_conns.next_hop_incoming_interface_name  AS next_hop_incoming_interface_name
     ,  route_conns.next_hop_incoming_ip_addr         AS next_hop_incoming_ip_addr
     ,  route_conns.next_hop_incoming_ip_net          AS next_hop_incoming_ip_net
     -- Insert current router's information into the path:
     ,  ARRAY[route_conns.device_id]                  AS route_path
     ,  ARRAY[(
            route_conns.device_id
          , route_conns.vrf_id
          , route_conns.incoming_interface_name
          , route_conns.incoming_ip_addr
          , route_conns.incoming_ip_net
          , route_conns.outgoing_interface_name
          , route_conns.outgoing_ip_addr
          , route_conns.outgoing_ip_net
        )::RouteHop]                                  AS route_path_detail
    FROM cte1 AS route_conns
    WHERE ($1 && route_conns.incoming_ip_net)
      AND ($2 && route_conns.dst_ip_net)
    UNION ALL
    -- Recursive case: Move current router one hop downstream.
    SELECT DISTINCT
      -- Source network:
        route_recur.src_ip_net                        AS src_ip_net
      -- Destination network:
      , CASE WHEN (route_recur.dst_ip_net <<= route_conns.dst_ip_net) THEN route_recur.dst_ip_net
             WHEN (route_conns.dst_ip_net <<= route_recur.dst_ip_net) THEN route_conns.dst_ip_net
             ELSE NULL
        END                                           AS dst_ip_net
      -- Next router in the path:
      , route_conns.next_hop_device_id                AS next_hop_device_id
      , route_conns.next_hop_vrf_id                   AS next_hop_vrf_id
      , route_conns.next_hop_incoming_interface_name  AS next_hop_incoming_interface_name
      , route_conns.next_hop_incoming_ip_addr         AS next_hop_incoming_ip_addr
      , route_conns.next_hop_incoming_ip_net          AS next_hop_incoming_ip_net
      -- Insert current router's information into the path:
      , route_path || route_conns.device_id           AS route_path
      , route_path_detail || (
            route_conns.device_id
          , route_conns.vrf_id
          , route_conns.incoming_interface_name
          , route_conns.incoming_ip_addr
          , route_conns.incoming_ip_net
          , route_conns.outgoing_interface_name
          , route_conns.outgoing_ip_addr
          , route_conns.outgoing_ip_net
        )::RouteHop                                   AS route_path_detail
    FROM device_ip_route_recursion AS route_recur
    JOIN cte1 AS route_conns
      ON (route_recur.next_hop_device_id = route_conns.device_id)
     AND (  (route_recur.next_hop_vrf_id = route_conns.vrf_id)
         OR (route_recur.next_hop_vrf_id IS NULL)
         OR (route_conns.vrf_id IS NULL)
         )
     AND (route_recur.next_hop_incoming_interface_name = route_conns.incoming_interface_name)
     AND (route_recur.next_hop_incoming_ip_net && route_conns.incoming_ip_net)
     AND (route_recur.dst_ip_net && route_conns.dst_ip_net)
    WHERE (route_conns.device_id != ALL(route_path))
      AND ($2 && route_conns.dst_ip_net)
)
-- Filter results
SELECT
    route_recur.src_ip_net                        AS src_ip_net
  , route_recur.dst_ip_net                        AS dst_ip_net
  , route_recur.next_hop_incoming_ip_addr         AS next_hop_ip_addr
  , array_length(route_recur.route_path, 1)       AS hop_count
  , route_recur.route_path                        AS route_path
  , route_recur.route_path_detail                 AS route_path_detail
FROM device_ip_route_recursion AS route_recur
WHERE (route_recur.next_hop_device_id IS NULL)
  AND (route_recur.route_path @> $3)
  -- The route endpoints src_ip_net and dst_ip_net
  -- MUST be routable (not link-local) networks.
  AND (NOT route_recur.src_ip_net <<= 'fe80::/10'::CIDR)
  AND (NOT route_recur.dst_ip_net <<= 'fe80::/10'::CIDR)
  AND (NOT route_recur.src_ip_net <<= '169.254.0.0/16'::CIDR)
  AND (NOT route_recur.dst_ip_net <<= '169.254.0.0/16'::CIDR)
ORDER BY
    masklen(dst_ip_net) DESC
  , hop_count ASC
  , dst_ip_net ASC
  , src_ip_net ASC
  , route_path ASC
$$
LANGUAGE SQL
;


-- ----------------------------------------------------------------------

CREATE VIEW device_ip_servers AS
SELECT DISTINCT
    device_id                   AS device_id
  , interface_name              AS interface_name
  , service_name                AS service_name
  , server_ip_addr              AS server_ip_addr
  , port                        AS port
  , local_service               AS local_service
  , description                 AS description
FROM raw_device_ip_servers
;


-- ----------------------------------------------------------------------

CREATE VIEW device_dns_resolvers AS
SELECT DISTINCT
    device_id                   AS device_id
  , interface_name              AS interface_name
  , scope_domain                AS scope_domain
  , src_ip_addr                 AS src_ip_addr
  , dst_ip_addr                 AS dst_ip_addr
  , dst_port                    AS dst_port
FROM raw_device_dns_resolvers
;


-- ----------------------------------------------------------------------

CREATE VIEW device_dns_search_domains AS
SELECT DISTINCT
    device_id                   AS device_id
  , search_domain               AS search_domain
FROM raw_device_dns_search_domains
;


-- ----------------------------------------------------------------------

CREATE VIEW device_dns_references AS
SELECT DISTINCT
    ddr.device_id               AS device_id
  , ddr.hostname                AS hostname
  , CASE WHEN (ddr.hostname LIKE '%.%')   THEN (ddr.hostname)
         WHEN (dds.search_domain IS NULL) THEN (ddr.hostname)
         ELSE (ddr.hostname || '.' || dds.search_domain)
    END                         AS fqdn
FROM raw_device_dns_references AS ddr
LEFT JOIN device_dns_search_domains AS dds
  ON (ddr.device_id = dds.device_id)
;


-- ----------------------------------------------------------------------

CREATE VIEW device_dns_lookups_required AS
SELECT DISTINCT
    device_dns_references.device_id     AS device_id
  , device_dns_resolvers.dst_ip_addr    AS resolver_ip_addr
  , device_dns_resolvers.dst_port       AS resolver_port
  , device_dns_references.hostname      AS query_hostname
  , device_dns_references.fqdn          AS query_fqdn
FROM device_dns_references
LEFT JOIN device_dns_resolvers
  ON (device_dns_references.device_id = device_dns_resolvers.device_id)
 AND (  (device_dns_references.hostname LIKE ('%.' || device_dns_resolvers.scope_domain))
     OR (device_dns_resolvers.scope_domain IS NULL)
     )
ORDER BY device_id, query_fqdn, query_hostname, resolver_ip_addr, resolver_port
;


-- ----------------------------------------------------------------------

CREATE VIEW device_dns_lookups AS
SELECT DISTINCT
    dns_required.device_id              AS device_id
  , dns_required.resolver_ip_addr       AS resolver_ip_addr
  , dns_required.resolver_port          AS resolver_port
  , dns_required.query_hostname         AS query_hostname
  , dns_required.query_fqdn             AS query_fqdn
  , dns_completed.query_class           AS query_class
  , dns_completed.query_type            AS query_type
  , dns_completed.response_status       AS response_status
  , dns_completed.response_section      AS response_section
  , dns_completed.response_fqdn         AS response_fqdn
  , dns_completed.response_class        AS response_class
  , dns_completed.response_type         AS response_type
  , dns_completed.response_ttl          AS response_ttl
  , dns_completed.response_data         AS response_data
FROM device_dns_lookups_required AS dns_required
JOIN dns_lookups AS dns_completed
  ON (dns_required.resolver_ip_addr = dns_completed.resolver_ip_addr)
 AND (dns_required.resolver_port    = dns_completed.resolver_port)
 AND (dns_required.query_fqdn       = dns_completed.query_fqdn)
;


-- ----------------------------------------------------------------------

CREATE VIEW device_dns_ip_addrs AS
SELECT DISTINCT
    dns_required.device_id              AS device_id
  , dns_required.resolver_ip_addr       AS resolver_ip_addr
  , dns_required.resolver_port          AS resolver_port
  , dns_required.query_hostname         AS hostname
  , dns_required.query_fqdn             AS fqdn
  , dns_ip_addrs.ip_addr                AS ip_addr
FROM device_dns_lookups_required AS dns_required
JOIN dns_ip_addrs
  ON (dns_required.resolver_ip_addr = dns_ip_addrs.resolver_ip_addr)
 AND (dns_required.resolver_port    = dns_ip_addrs.resolver_port)
 AND (dns_required.query_fqdn       = dns_ip_addrs.fqdn)
;


-- ----------------------------------------------------------------------

CREATE VIEW device_phys_connections AS
SELECT DISTINCT
    self_device_id              AS self_device_id
  , self_interface_name         AS self_interface_name
  , peer_device_id              AS peer_device_id
  , peer_interface_name         AS peer_interface_name
FROM raw_device_phys_connections
;


-- ----------------------------------------------------------------------

CREATE VIEW device_link_connections AS
SELECT DISTINCT
    self_device_id              AS self_device_id
  , self_interface_name         AS self_interface_name
  , peer_mac_addr               AS peer_mac_addr
FROM raw_device_link_connections
;


-- ----------------------------------------------------------------------

CREATE VIEW device_connections AS
SELECT
    self_device_id              AS self_device_id
  , self_interface_name         AS self_interface_name
  , peer_device_id              AS peer_device_id
  , peer_interface_name         AS peer_interface_name
FROM device_phys_connections
UNION
SELECT
    dlc.self_device_id          AS self_device_id
  , dlc.self_interface_name     AS self_interface_name
  , dma.device_id               AS peer_device_id
  , dma.interface_name          AS peer_interface_name
FROM device_link_connections AS dlc
JOIN device_mac_addrs AS dma
  ON (dlc.peer_mac_addr = dma.mac_addr)
UNION
SELECT
    dlc.self_device_id          AS self_device_id
  , dlc.self_interface_name     AS self_interface_name
  , dia.device_id               AS peer_device_id
  , dia.interface_name          AS peer_interface_name
FROM device_link_connections AS dlc
JOIN mac_addrs_ip_addrs AS maia
  ON (dlc.peer_mac_addr = maia.mac_addr)
JOIN device_ip_addrs AS dia
  ON (maia.ip_addr = dia.ip_addr)
UNION
SELECT *
FROM (
  SELECT
      dlc.self_device_id          AS self_device_id
    , dlc.self_interface_name     AS self_interface_name
    , dlc.peer_mac_addr::text     AS peer_device_id
    , 'UNKNOWN'                   AS peer_interface_name
  FROM device_link_connections AS dlc
  LEFT JOIN device_mac_addrs AS dma
  ON (dlc.peer_mac_addr = dma.mac_addr)
  WHERE (dma.mac_addr IS NULL) AND (dlc.self_interface_name != 'CPU')
) dlcdma
WHERE NOT EXISTS (
  SELECT 1
  FROM device_mac_addrs_ip_addrs dmaia
  WHERE (dlcdma.peer_device_id = dmaia.mac_addr::text)
)
;


-- ----------------------------------------------------------------------
-- Cisco and Cisco-like device and interface information
-- ----------------------------------------------------------------------

CREATE VIEW devices_aaa AS
SELECT DISTINCT
    device_id                   AS device_id
  , aaa_command                 AS aaa_command
FROM raw_devices_aaa
;


-- ----------------------------------------------------------------------

CREATE VIEW device_interfaces_mode AS
SELECT DISTINCT
    device_id                   AS device_id
  , interface_name              AS interface_name
  , interface_mode              AS interface_mode
FROM raw_device_interfaces_mode
;


-- ----------------------------------------------------------------------

CREATE VIEW device_interfaces_bpdu AS
SELECT DISTINCT
    device_id                   AS device_id
  , interface_name              AS interface_name
  , is_bpduguard_enabled        AS is_bpduguard_enabled
  , is_bpdufilter_enabled       AS is_bpdufilter_enabled
FROM raw_device_interfaces_bpdu
;


-- ----------------------------------------------------------------------

CREATE VIEW device_interfaces_cdp AS
SELECT DISTINCT
    device_id                   AS device_id
  , interface_name              AS interface_name
  , is_cdp_enabled              AS is_cdp_enabled
FROM raw_device_interfaces_cdp
;


-- ----------------------------------------------------------------------

CREATE VIEW device_interfaces_portfast AS
SELECT DISTINCT
    device_id                   AS device_id
  , interface_name              AS interface_name
  , is_portfast_enabled         AS is_portfast_enabled
FROM raw_device_interfaces_portfast
;


-- ----------------------------------------------------------------------

CREATE VIEW device_interfaces_port_security AS
SELECT DISTINCT
    device_id                   AS device_id
  , interface_name              AS interface_name
  , is_port_security_enabled    AS is_port_security_enabled
  , is_mac_addr_sticky          AS is_mac_addr_sticky
  , max_mac_addrs               AS max_mac_addrs
  , violation_action            AS violation_action
FROM raw_device_interfaces_port_security
;


-- ----------------------------------------------------------------------

CREATE VIEW device_interfaces_port_security_mac_addrs AS
SELECT DISTINCT
    device_id                   AS device_id
  , interface_name              AS interface_name
  , mac_addr                    AS mac_addr
FROM raw_device_interfaces_port_security_mac_addrs
;


-- ----------------------------------------------------------------------

CREATE VIEW raw_device_interfaces_summaries AS
SELECT
    di.tool_run_id              AS tool_run_id
  , di.device_id                AS device_id
  , di.interface_name           AS interface_name
  , di.media_type               AS media_type
  , di.is_up                    AS is_up
  , di_mode.interface_mode      AS interface_mode
  , di_pf.is_portfast_enabled   AS is_portfast_enabled
  , di_cdp.is_cdp_enabled       AS is_cdp_enabled
  , (di_bpdu.is_bpduguard_enabled OR di_pf.is_portfast_enabled)
                                AS is_bpduguard_enabled
  , (di_bpdu.is_bpdufilter_enabled OR di_pf.is_portfast_enabled)
                                AS is_bpdufilter_enabled
  , di_ps.is_port_security_enabled AS is_port_security_enabled
  , di_ps.is_mac_addr_sticky    AS is_mac_addr_sticky
  , di_ps.max_mac_addrs         AS max_mac_addrs
  , count(di_ps_ma.mac_addr)    AS learned_mac_addrs
  , di_ps.violation_action      AS violation_action
FROM raw_device_interfaces AS di

LEFT JOIN raw_device_interfaces_mode AS di_mode
  ON (di.tool_run_id    = di_mode.tool_run_id)
 AND (di.device_id      = di_mode.device_id)
 AND (di.interface_name = di_mode.interface_name)

LEFT JOIN raw_device_interfaces_portfast AS di_pf
  ON (di.tool_run_id    = di_pf.tool_run_id)
 AND (di.device_id      = di_pf.device_id)
 AND (di.interface_name = di_pf.interface_name)

LEFT JOIN raw_device_interfaces_cdp AS di_cdp
  ON (di.tool_run_id    = di_cdp.tool_run_id)
 AND (di.device_id      = di_cdp.device_id)
 AND (di.interface_name = di_cdp.interface_name)

LEFT JOIN raw_device_interfaces_bpdu AS di_bpdu
  ON (di.tool_run_id    = di_bpdu.tool_run_id)
 AND (di.device_id      = di_bpdu.device_id)
 AND (di.interface_name = di_bpdu.interface_name)

LEFT JOIN raw_device_interfaces_port_security AS di_ps
  ON (di.tool_run_id    = di_ps.tool_run_id)
 AND (di.device_id      = di_ps.device_id)
 AND (di.interface_name = di_ps.interface_name)

LEFT JOIN raw_device_interfaces_port_security_mac_addrs AS di_ps_ma
  ON (di.tool_run_id    = di_ps_ma.tool_run_id)
 AND (di.device_id      = di_ps_ma.device_id)
 AND (di.interface_name = di_ps_ma.interface_name)

GROUP BY
    di.tool_run_id
  , di.device_id
  , di.interface_name
  , di.media_type
  , di.is_up
  , di_mode.interface_mode
  , di_pf.is_portfast_enabled
  , di_cdp.is_cdp_enabled
  , (di_bpdu.is_bpduguard_enabled OR di_pf.is_portfast_enabled)
  , (di_bpdu.is_bpdufilter_enabled OR di_pf.is_portfast_enabled)
  , di_ps.is_port_security_enabled
  , di_ps.is_mac_addr_sticky
  , di_ps.max_mac_addrs
  , di_ps.violation_action
;


-- ----------------------------------------------------------------------

CREATE VIEW device_interfaces_summaries AS
SELECT DISTINCT
    device_id                   AS device_id
  , interface_name              AS interface_name
  , media_type                  AS media_type
  , is_up                       AS is_up
  , interface_mode              AS interface_mode
  , is_portfast_enabled         AS is_portfast_enabled
  , is_cdp_enabled              AS is_cdp_enabled
  , is_bpduguard_enabled        AS is_bpduguard_enabled
  , is_bpdufilter_enabled       AS is_bpdufilter_enabled
  , is_port_security_enabled    AS is_port_security_enabled
  , is_mac_addr_sticky          AS is_mac_addr_sticky
  , max_mac_addrs               AS max_mac_addrs
  , learned_mac_addrs           AS learned_mac_addrs
  , violation_action            AS violation_action
FROM raw_device_interfaces_summaries
;


-- ----------------------------------------------------------------------
-- Device Access Control views
-- ----------------------------------------------------------------------

CREATE VIEW device_ac_nets AS
SELECT DISTINCT
    device_id     AS device_id
  , net_set_id    AS net_set_id
  , net_set       AS net_set
  , net_set_data  AS net_set_data
FROM raw_device_ac_nets
;


CREATE VIEW device_ac_services AS
SELECT DISTINCT
    device_id         AS device_id
  , service_set       AS service_set
  , service_set_data  AS service_set_data
FROM raw_device_ac_services
;


CREATE VIEW device_ac_rules AS
SELECT DISTINCT
    device_id       AS device_id
  , enabled         AS enabled
  , ac_id           AS ac_id
  , src_net_set_id  AS src_net_set_id
  , src_net_set     AS src_net_set
  , src_iface       AS src_iface
  , dst_net_set_id  AS dst_net_set_id
  , dst_net_set     AS dst_net_set
  , dst_iface       AS dst_iface
  , service_set     AS service_set
  , action          AS action
  , description     AS description
FROM raw_device_ac_rules
ORDER BY
    device_id, src_iface, dst_iface
  , description, ac_id
  , src_net_set, dst_net_set
;

CREATE VIEW device_ac_rules_known_applied AS
SELECT DISTINCT
    device_id       AS device_id
  , enabled         AS enabled
  , ac_id           AS ac_id
  , src_net_set_id  AS src_net_set_id
  , src_net_set     AS src_net_set
  , src_iface       AS src_iface
  , dst_net_set_id  AS dst_net_set_id
  , dst_net_set     AS dst_net_set
  , dst_iface       AS dst_iface
  , service_set     AS service_set
  , action          AS action
  , description     AS description
FROM raw_device_ac_rules
WHERE src_iface IS NOT NULL
  AND dst_iface IS NOT NULL
ORDER BY
    device_id, src_iface, dst_iface
  , description, ac_id
;

CREATE VIEW device_ac_rules_unknown_applied AS
SELECT DISTINCT
    device_id       AS device_id
  , enabled         AS enabled
  , ac_id           AS ac_id
  , src_net_set_id  AS src_net_set_id
  , src_net_set     AS src_net_set
  , src_iface       AS src_iface
  , dst_net_set_id  AS dst_net_set_id
  , dst_net_set     AS dst_net_set
  , dst_iface       AS dst_iface
  , service_set     AS service_set
  , action          AS action
  , description     AS description
FROM raw_device_ac_rules
WHERE src_iface IS NULL
   OR dst_iface IS NULL
ORDER BY
    device_id, description, ac_id
  , src_net_set, dst_net_set
;


CREATE VIEW raw_device_ac_nets_flattened AS
WITH RECURSIVE raw_device_ac_nets_recursion(
    tool_run_id
  , device_id
  , net_set_id
  , net_set
  , net_set_data
  , nested_net_set
) AS (
    SELECT DISTINCT
        base.tool_run_id                AS tool_run_id
      , base.device_id                  AS device_id
      , base.net_set_id                 AS net_set_id
      , base.net_set                    AS net_set
      , base.net_set_data               AS net_set_data
      , nested.net_set                  AS nested_net_set
    FROM raw_device_ac_nets AS base
    LEFT JOIN device_ac_nets AS nested
      ON (base.device_id = nested.device_id)
     AND (base.net_set_id = nested.net_set_id)
     AND (base.net_set_data = nested.net_set)
     AND (nested.net_set != nested.net_set_data)
    UNION
    SELECT DISTINCT
        recur.tool_run_id               AS tool_run_id
      , recur.device_id                 AS device_id
      , recur.net_set_id                AS net_set_id
      , recur.net_set                   AS net_set
      , nested.net_set_data             AS net_set_data
      , nested.net_set                  AS nested_net_set
    FROM raw_device_ac_nets_recursion AS recur
    LEFT JOIN device_ac_nets AS nested
      ON (recur.device_id = nested.device_id)
     AND (recur.net_set_id = nested.net_set_id)
     AND (recur.net_set_data = nested.net_set)
     AND (nested.net_set != nested.net_set_data)
    WHERE (nested.net_set IS NOT NULL)
)
SELECT DISTINCT
    tool_run_id
  , device_id
  , net_set_id
  , net_set
  , net_set_data
FROM raw_device_ac_nets_recursion
WHERE (net_set_data IS NOT NULL)
  AND (  (nested_net_set IS NULL)
      OR (net_set_data != nested_net_set)
      )
;


CREATE VIEW raw_device_ac_services_flattened AS
WITH RECURSIVE raw_device_ac_services_recursion(
    tool_run_id
  , device_id
  , service_set
  , service_set_data
  , nested_service_set
) AS (
    SELECT DISTINCT
        base.tool_run_id                AS tool_run_id
      , base.device_id                  AS device_id
      , base.service_set                AS service_set
      , base.service_set_data           AS service_set_data
      , nested.service_set              AS nested_service_set
    FROM raw_device_ac_services AS base
    LEFT JOIN device_ac_services AS nested
      ON (base.device_id = nested.device_id)
     AND (base.service_set_data = nested.service_set)
     AND (nested.service_set != nested.service_set_data)
    UNION
    SELECT DISTINCT
        recur.tool_run_id               AS tool_run_id
      , recur.device_id                 AS device_id
      , recur.service_set               AS service_set
      , nested.service_set_data         AS service_set_data
      , nested.service_set              AS nested_service_set
    FROM raw_device_ac_services_recursion AS recur
    LEFT JOIN device_ac_services AS nested
      ON (recur.device_id = nested.device_id)
     AND (recur.service_set_data = nested.service_set)
     AND (nested.service_set != nested.service_set_data)
    WHERE (nested.service_set IS NOT NULL)
)
SELECT DISTINCT
    tool_run_id
  , device_id
  , service_set
  , service_set_data
FROM raw_device_ac_services_recursion
WHERE (service_set_data IS NOT NULL)
  AND (  (nested_service_set IS NULL)
      OR (service_set_data != nested_service_set)
      )
;


-- ----------------------------------------------------------------------

CREATE VIEW device_acl_zones AS
WITH RECURSIVE device_acl_zones_recursion(
    device_id
  , zone_id
  , interface_name
) AS (
    -- Base case:
    SELECT DISTINCT
        acl_ifaces.device_id        AS device_id
      , acl_ifaces.zone_id          AS zone_id
      , acl_ifaces.interface_name   AS interface_name
    FROM raw_device_acl_zones_interfaces AS acl_ifaces
    UNION
    -- Recursive case:
    SELECT DISTINCT
        acl_includes.device_id      AS device_id
      , acl_includes.zone_id        AS zone_id
      , acl_recur.interface_name    AS interface_name
    FROM device_acl_zones_recursion AS acl_recur
    JOIN raw_device_acl_zones_includes AS acl_includes
      ON (acl_recur.device_id = acl_includes.device_id)
     AND (acl_recur.zone_id   = acl_includes.included_id)
)
SELECT DISTINCT
    acl_bases.device_id             AS device_id
  , acl_bases.zone_id               AS zone_id
  , acl_recur.interface_name        AS interface_name
FROM raw_device_acl_zones_bases AS acl_bases
LEFT JOIN device_acl_zones_recursion AS acl_recur
  ON (acl_bases.device_id = acl_recur.device_id)
 AND (acl_bases.zone_id   = acl_recur.zone_id)
;


-- ----------------------------------------------------------------------

CREATE VIEW device_acl_ip_nets AS
WITH RECURSIVE device_acl_ip_nets_recursion(
    device_id
  , ip_net_set_namespace
  , ip_net_set_id
  , hostname
  , fqdn
  , ip_net
) AS (
    -- Base case:
    SELECT DISTINCT
        acl_ip_nets.device_id               AS device_id
      , acl_ip_nets.ip_net_set_namespace    AS ip_net_set_namespace
      , acl_ip_nets.ip_net_set_id           AS ip_net_set_id
      , NULL::TEXT                          AS hostname
      , NULL::TEXT                          AS fqdn
      , acl_ip_nets.ip_net                  AS ip_net
    FROM raw_device_acl_ip_nets_ip_nets AS acl_ip_nets
    UNION
    SELECT DISTINCT
        acl_hostnames.device_id             AS device_id
      , acl_hostnames.ip_net_set_namespace  AS ip_net_set_namespace
      , acl_hostnames.ip_net_set_id         AS ip_net_set_id
      , acl_hostnames.hostname              AS hostname
      , COALESCE(dns_ip_addrs.fqdn, dns_required.query_fqdn) AS fqdn
      , dns_ip_addrs.ip_addr::CIDR  AS ip_net
    FROM raw_device_acl_ip_nets_hostnames AS acl_hostnames
    LEFT JOIN device_dns_lookups_required AS dns_required
      ON (acl_hostnames.device_id = dns_required.device_id)
     AND (acl_hostnames.hostname  = dns_required.query_hostname)
    LEFT JOIN device_dns_ip_addrs AS dns_ip_addrs
      ON (acl_hostnames.device_id = dns_ip_addrs.device_id)
     AND (acl_hostnames.hostname  = dns_ip_addrs.hostname)
    UNION
    -- Recursive case:
    SELECT DISTINCT
        acl_includes.device_id              AS device_id
      , acl_includes.ip_net_set_namespace   AS ip_net_set_namespace
      , acl_includes.ip_net_set_id          AS ip_net_set_id
      , acl_recur.hostname                  AS hostname
      , acl_recur.fqdn                      AS fqdn
      , acl_recur.ip_net                    AS ip_net
    FROM device_acl_ip_nets_recursion AS acl_recur
    JOIN raw_device_acl_ip_nets_includes AS acl_includes
      ON (acl_recur.device_id             = acl_includes.device_id)
     AND (  (acl_recur.ip_net_set_namespace  = acl_includes.included_namespace)
         -- NOTE: More complicated than this, but current revised ACL logic
         --       doesn't appropriately support the recursive logic.
         --       See AcNetworkBook and the Parser for juniper-conf;
         --       effectively it is a matter of walk up the tree and first
         --       match wins (same applies to services).
         OR (acl_includes.included_namespace = '')
         )
     AND (acl_recur.ip_net_set_id         = acl_includes.included_id)
)
SELECT DISTINCT
    acl_bases.device_id                     AS device_id
  , acl_bases.ip_net_set_namespace          AS ip_net_set_namespace
  , acl_bases.ip_net_set_id                 AS ip_net_set_id
  , acl_recur.hostname                      AS hostname
  , acl_recur.fqdn                          AS fqdn
  , acl_recur.ip_net                        AS ip_net
FROM raw_device_acl_ip_nets_bases AS acl_bases
LEFT JOIN device_acl_ip_nets_recursion AS acl_recur
  ON (acl_bases.device_id             = acl_recur.device_id)
 AND (acl_bases.ip_net_set_namespace  = acl_recur.ip_net_set_namespace)
 AND (acl_bases.ip_net_set_id         = acl_recur.ip_net_set_id)
;


-- ----------------------------------------------------------------------

CREATE VIEW device_acl_ports AS
WITH RECURSIVE device_acl_ports_recursion(
    device_id
  , port_set_id
  , port_range
) AS (
    -- Base case:
    SELECT DISTINCT
        acl_ports.device_id         AS device_id
      , acl_ports.port_set_id       AS port_set_id
      , acl_ports.port_range        AS port_range
    FROM raw_device_acl_ports_ports AS acl_ports
    UNION
    -- Recursive case:
    SELECT DISTINCT
        acl_includes.device_id      AS device_id
      , acl_includes.port_set_id    AS port_set_id
      , acl_recur.port_range        AS port_range
    FROM device_acl_ports_recursion AS acl_recur
    JOIN raw_device_acl_ports_includes AS acl_includes
      ON (acl_recur.device_id   = acl_includes.device_id)
     AND (acl_recur.port_set_id = acl_includes.included_id)
)
SELECT DISTINCT
    acl_bases.device_id             AS device_id
  , acl_bases.port_set_id           AS port_set_id
  , acl_recur.port_range            AS port_range
FROM raw_device_acl_ports_bases AS acl_bases
LEFT JOIN device_acl_ports_recursion AS acl_recur
  ON (acl_bases.device_id   = acl_recur.device_id)
 AND (acl_bases.port_set_id = acl_recur.port_set_id)
;


-- ----------------------------------------------------------------------

CREATE VIEW device_acl_services AS
WITH RECURSIVE device_acl_services_recursion(
    device_id
  , service_id
  , protocol
  , src_port_range
  , dst_port_range
) AS (
    -- Base case:
    SELECT DISTINCT
        acl_protocols.device_id     AS device_id
      , acl_protocols.service_id    AS service_id
      , acl_protocols.protocol      AS protocol
      , acl_ports.src_port_range    AS src_port_range
      , acl_ports.dst_port_range    AS dst_port_range
    FROM raw_device_acl_services_protocols AS acl_protocols
    LEFT JOIN raw_device_acl_services_ports AS acl_ports
      ON (acl_protocols.device_id  = acl_ports.device_id)
     AND (acl_protocols.service_id = acl_ports.service_id)
     AND (acl_protocols.protocol   = acl_ports.protocol)
    UNION
    -- Recursive case:
    SELECT DISTINCT
        acl_includes.device_id      AS device_id
      , acl_includes.service_id     AS service_id
      , acl_recur.protocol          AS protocol
      , acl_recur.src_port_range    AS src_port_range
      , acl_recur.dst_port_range    AS dst_port_range
    FROM device_acl_services_recursion AS acl_recur
    JOIN raw_device_acl_services_includes AS acl_includes
      ON (acl_recur.device_id  = acl_includes.device_id)
     AND (acl_recur.service_id = acl_includes.included_id)
)
SELECT DISTINCT
    acl_bases.device_id             AS device_id
  , acl_bases.service_id            AS service_id
  , acl_recur.protocol              AS protocol
  , acl_recur.src_port_range        AS src_port_range
  , acl_recur.dst_port_range        AS dst_port_range
FROM raw_device_acl_services_bases AS acl_bases
LEFT JOIN device_acl_services_recursion AS acl_recur
  ON (acl_bases.device_id  = acl_recur.device_id)
 AND (acl_bases.service_id = acl_recur.service_id)
;


-- ----------------------------------------------------------------------

CREATE VIEW device_acl_rules_ports AS
SELECT DISTINCT
    device_id                   AS device_id
  , priority                    AS priority
  , action                      AS action
  , incoming_zone_id            AS incoming_zone_id
  , outgoing_zone_id            AS outgoing_zone_id
  , src_ip_net_set_namespace    AS src_ip_net_set_namespace
  , src_ip_net_set_id           AS src_ip_net_set_id
  , dst_ip_net_set_namespace    AS dst_ip_net_set_namespace
  , dst_ip_net_set_id           AS dst_ip_net_set_id
  , protocol                    AS protocol
  , src_port_set_id             AS src_port_set_id
  , dst_port_set_id             AS dst_port_set_id
  , description                 AS description
FROM raw_device_acl_rules_ports
;


-- ----------------------------------------------------------------------

CREATE VIEW device_acl_rules_services AS
SELECT DISTINCT
    device_id                   AS device_id
  , priority                    AS priority
  , action                      AS action
  , incoming_zone_id            AS incoming_zone_id
  , outgoing_zone_id            AS outgoing_zone_id
  , src_ip_net_set_namespace    AS src_ip_net_set_namespace
  , src_ip_net_set_id           AS src_ip_net_set_id
  , dst_ip_net_set_namespace    AS dst_ip_net_set_namespace
  , dst_ip_net_set_id           AS dst_ip_net_set_id
  , service_id                  AS service_id
  , description                 AS description
FROM raw_device_acl_rules_services
;


-- ----------------------------------------------------------------------

CREATE VIEW device_acl_rules_all AS
SELECT DISTINCT
    device_id                   AS device_id
  , priority                    AS priority
  , action                      AS action
  , incoming_zone_id            AS incoming_zone_id
  , outgoing_zone_id            AS outgoing_zone_id
  , src_ip_net_set_namespace    AS src_ip_net_set_namespace
  , src_ip_net_set_id           AS src_ip_net_set_id
  , dst_ip_net_set_namespace    AS dst_ip_net_set_namespace
  , dst_ip_net_set_id           AS dst_ip_net_set_id
  , NULL                        AS service_id
  , protocol                    AS protocol
  , src_port_set_id             AS src_port_set_id
  , dst_port_set_id             AS dst_port_set_id
  , description                 AS description
FROM raw_device_acl_rules_ports
UNION
SELECT DISTINCT
    device_id                   AS device_id
  , priority                    AS priority
  , action                      AS action
  , incoming_zone_id            AS incoming_zone_id
  , outgoing_zone_id            AS outgoing_zone_id
  , src_ip_net_set_namespace    AS src_ip_net_set_namespace
  , src_ip_net_set_id           AS src_ip_net_set_id
  , dst_ip_net_set_namespace    AS dst_ip_net_set_namespace
  , dst_ip_net_set_id           AS dst_ip_net_set_id
  , service_id                  AS service_id
  , NULL                        AS protocol
  , NULL                        AS src_port_set_id
  , NULL                        AS dst_port_set_id
  , description                 AS description
FROM raw_device_acl_rules_services
;


-- ----------------------------------------------------------------------
CREATE OR REPLACE VIEW device_acl_rules AS
WITH cte1 AS (
  SELECT DISTINCT * FROM device_acl_zones
),
cte2 AS (
  SELECT DISTINCT * FROM device_acl_ip_nets
),
cte3 AS (
  SELECT DISTINCT * FROM device_acl_ports
),
cte4 AS (
  SELECT DISTINCT * FROM device_acl_services
)
SELECT DISTINCT
    acl_rules.device_id                 AS device_id
  , acl_rules.priority                  AS priority
  , acl_rules.action                    AS action
  , acl_rules.incoming_zone_id          AS incoming_zone_id
  , acl_incoming_zones.interface_name   AS incoming_interface_name
  , acl_rules.outgoing_zone_id          AS outgoing_zone_id
  , acl_outgoing_zones.interface_name   AS outgoing_interface_name
  , acl_rules.src_ip_net_set_namespace  AS src_ip_net_set_namespace
  , acl_rules.src_ip_net_set_id         AS src_ip_net_set_id
  , acl_src_ip_nets.fqdn                AS src_fqdn
  , acl_src_ip_nets.ip_net              AS src_ip_net
  , acl_rules.dst_ip_net_set_namespace  AS dst_ip_net_set_namespace
  , acl_rules.dst_ip_net_set_id         AS dst_ip_net_set_id
  , acl_dst_ip_nets.fqdn                AS dst_fqdn
  , acl_dst_ip_nets.ip_net              AS dst_ip_net
  , NULL                                AS service_id
  , acl_rules.protocol                  AS protocol
  , acl_rules.src_port_set_id           AS src_port_set_id
  , acl_src_ports.port_range            AS src_port_range
  , acl_rules.dst_port_set_id           AS dst_port_set_id
  , acl_dst_ports.port_range            AS dst_port_range
  , acl_rules.description               AS description
FROM device_acl_rules_ports AS acl_rules
LEFT JOIN cte1 AS acl_incoming_zones
  ON (acl_rules.device_id         = acl_incoming_zones.device_id)
 AND (acl_rules.incoming_zone_id  = acl_incoming_zones.zone_id)
LEFT JOIN cte1 AS acl_outgoing_zones
  ON (acl_rules.device_id         = acl_outgoing_zones.device_id)
 AND (acl_rules.outgoing_zone_id  = acl_outgoing_zones.zone_id)
LEFT JOIN cte2 AS acl_src_ip_nets
  ON (acl_rules.device_id                 = acl_src_ip_nets.device_id)
 AND (acl_rules.src_ip_net_set_namespace  = acl_src_ip_nets.ip_net_set_namespace)
 AND (acl_rules.src_ip_net_set_id         = acl_src_ip_nets.ip_net_set_id)
LEFT JOIN cte2 AS acl_dst_ip_nets
  ON (acl_rules.device_id                 = acl_dst_ip_nets.device_id)
 AND (acl_rules.dst_ip_net_set_namespace  = acl_dst_ip_nets.ip_net_set_namespace)
 AND (acl_rules.dst_ip_net_set_id         = acl_dst_ip_nets.ip_net_set_id)
 AND (inet_same_family(acl_src_ip_nets.ip_net, acl_dst_ip_nets.ip_net))
LEFT JOIN cte3 AS acl_src_ports
  ON (acl_rules.device_id         = acl_src_ports.device_id)
 AND (acl_rules.src_port_set_id   = acl_src_ports.port_set_id)
LEFT JOIN cte3 AS acl_dst_ports
  ON (acl_rules.device_id         = acl_dst_ports.device_id)
 AND (acl_rules.dst_port_set_id   = acl_dst_ports.port_set_id)
UNION
SELECT DISTINCT
    acl_rules.device_id                 AS device_id
  , acl_rules.priority                  AS priority
  , acl_rules.action                    AS action
  , acl_rules.incoming_zone_id          AS incoming_zone_id
  , acl_incoming_zones.interface_name   AS incoming_interface_name
  , acl_rules.outgoing_zone_id          AS outgoing_zone_id
  , acl_outgoing_zones.interface_name   AS outgoing_interface_name
  , acl_rules.src_ip_net_set_namespace  AS src_ip_net_set_namespace
  , acl_rules.src_ip_net_set_id         AS src_ip_net_set_id
  , acl_src_ip_nets.fqdn                AS src_fqdn
  , acl_src_ip_nets.ip_net              AS src_ip_net
  , acl_rules.dst_ip_net_set_namespace  AS dst_ip_net_set_namespace
  , acl_rules.dst_ip_net_set_id         AS dst_ip_net_set_id
  , acl_dst_ip_nets.fqdn                AS dst_fqdn
  , acl_dst_ip_nets.ip_net              AS dst_ip_net
  , acl_rules.service_id                AS service_id
  , acl_services.protocol               AS protocol
  , NULL                                AS src_port_set_id
  , acl_services.src_port_range         AS src_port_range
  , NULL                                AS dst_port_set_id
  , acl_services.dst_port_range         AS dst_port_range
  , acl_rules.description               AS description
FROM device_acl_rules_services AS acl_rules
LEFT JOIN cte1 AS acl_incoming_zones
  ON (acl_rules.device_id         = acl_incoming_zones.device_id)
 AND (acl_rules.incoming_zone_id  = acl_incoming_zones.zone_id)
LEFT JOIN cte1 AS acl_outgoing_zones
  ON (acl_rules.device_id         = acl_outgoing_zones.device_id)
 AND (acl_rules.outgoing_zone_id  = acl_outgoing_zones.zone_id)
LEFT JOIN cte2 AS acl_src_ip_nets
  ON (acl_rules.device_id                 = acl_src_ip_nets.device_id)
 AND (acl_rules.src_ip_net_set_namespace  = acl_src_ip_nets.ip_net_set_namespace)
 AND (acl_rules.src_ip_net_set_id         = acl_src_ip_nets.ip_net_set_id)
LEFT JOIN cte2 AS acl_dst_ip_nets
  ON (acl_rules.device_id                 = acl_dst_ip_nets.device_id)
 AND (acl_rules.dst_ip_net_set_namespace  = acl_dst_ip_nets.ip_net_set_namespace)
 AND (acl_rules.dst_ip_net_set_id         = acl_dst_ip_nets.ip_net_set_id)
 AND (inet_same_family(acl_src_ip_nets.ip_net, acl_dst_ip_nets.ip_net))
LEFT JOIN cte4 AS acl_services
  ON (acl_rules.device_id         = acl_services.device_id)
 AND (acl_rules.service_id        = acl_services.service_id)
;


-- ----------------------------------------------------------------------

CREATE FUNCTION ip_route_path_acls(
    arg_src_ip_net INET
  , arg_dst_ip_net INET
  , arg_route_path_detail RouteHop[]
)
RETURNS TABLE (
    action              TEXT
  , src_ip_net          CIDR
  , dst_ip_net          CIDR
  , protocol            TEXT
  , src_port_range      PortRange
  , dst_port_range      PortRange
  , priority            INT[]
  , description         TEXT[]
) AS $$
WITH RECURSIVE ip_route_path_acls_recursion(
    action
  , src_ip_net
  , dst_ip_net
  , protocol
  , src_port_range
  , dst_port_range
  , priority
  , description
  , hop_index
) AS (
    -- Base case: ACLs from first (hop_index = 1) router in the IP route path.
    SELECT DISTINCT
        acl_rules.action                    AS action
      , acl_rules.src_ip_net                AS src_ip_net
      , acl_rules.dst_ip_net                AS dst_ip_net
      , acl_rules.protocol                  AS protocol
      , acl_rules.src_port_range            AS src_port_range
      , acl_rules.dst_port_range            AS dst_port_range
      , ARRAY[acl_rules.priority]           AS priority
      , ARRAY[acl_rules.description]        AS description
      , 1                                   AS hop_index
    FROM device_acl_rules AS acl_rules
    WHERE ($3[1].device_id = acl_rules.device_id)
      AND (  ($3[1].incoming_interface_name = acl_rules.incoming_interface_name)
          OR (   acl_rules.incoming_zone_id = 'any'
             AND acl_rules.incoming_interface_name IS NULL
             )
          )
      AND (  ($3[1].outgoing_interface_name = acl_rules.outgoing_interface_name)
          OR (   acl_rules.outgoing_zone_id = 'any'
             AND acl_rules.outgoing_interface_name IS NULL
             )
          )
      AND ($1 && acl_rules.src_ip_net)
      AND ($2 && acl_rules.dst_ip_net)
    UNION ALL
    -- Recursive case: Move current router one hop downstream.
    -- Calculate intersection between accumulated ACLs and current router ACLs.
    SELECT DISTINCT
        CASE WHEN (acl_recur.action = acl_rules.action) THEN acl_recur.action
             WHEN ('block' = acl_recur.action) THEN 'block'
             WHEN ('block' = acl_rules.action) THEN 'block'
             ELSE 'unknown'
        END                                 AS action
      , CASE WHEN (acl_recur.src_ip_net <<= acl_rules.src_ip_net) THEN acl_recur.src_ip_net
             WHEN (acl_rules.src_ip_net <<= acl_recur.src_ip_net) THEN acl_rules.src_ip_net
             ELSE NULL
        END                                 AS src_ip_net
      , CASE WHEN (acl_recur.dst_ip_net <<= acl_rules.dst_ip_net) THEN acl_recur.dst_ip_net
             WHEN (acl_rules.dst_ip_net <<= acl_recur.dst_ip_net) THEN acl_rules.dst_ip_net
             ELSE NULL
        END                                 AS dst_ip_net
      , CASE WHEN ('any' = acl_recur.protocol) THEN acl_rules.protocol
             ELSE acl_recur.protocol
        END                                 AS protocol
      , (acl_recur.src_port_range * acl_rules.src_port_range)
                                            AS src_port_range
      , (acl_recur.dst_port_range * acl_rules.dst_port_range)
                                            AS dst_port_range
      , (acl_recur.priority || acl_rules.priority)
                                            AS priority
      , (acl_recur.description || acl_rules.description)
                                            AS description
      , (acl_recur.hop_index + 1)           AS hop_index
    FROM ip_route_path_acls_recursion AS acl_recur
    JOIN device_acl_rules AS acl_rules
      ON ($3[acl_recur.hop_index + 1].device_id = acl_rules.device_id)
     AND (  ($3[acl_recur.hop_index + 1].incoming_interface_name = acl_rules.incoming_interface_name)
         OR (   acl_rules.incoming_zone_id = 'any'
            AND acl_rules.incoming_interface_name IS NULL
            )
         )
     AND (  ($3[acl_recur.hop_index + 1].outgoing_interface_name = acl_rules.outgoing_interface_name)
         OR (   acl_rules.outgoing_zone_id = 'any'
            AND acl_rules.outgoing_interface_name IS NULL
            )
         )
     AND (acl_recur.src_ip_net && acl_rules.src_ip_net)
     AND ($1 && acl_rules.src_ip_net)
     AND (acl_recur.dst_ip_net && acl_rules.dst_ip_net)
     AND ($2 && acl_rules.dst_ip_net)
     AND (  (acl_recur.protocol = acl_rules.protocol)
         OR ('any' = acl_recur.protocol)
         OR ('any' = acl_rules.protocol)
         )
     AND (acl_recur.src_port_range && acl_rules.src_port_range)
     AND (acl_recur.dst_port_range && acl_rules.dst_port_range)
    WHERE ((acl_recur.hop_index + 1) <= array_length($3, 1))
)
-- Filter results
SELECT
    acl_recur.action                        AS action
  , acl_recur.src_ip_net                    AS src_ip_net
  , acl_recur.dst_ip_net                    AS dst_ip_net
  , acl_recur.protocol                      AS protocol
  , acl_recur.src_port_range                AS src_port_range
  , acl_recur.dst_port_range                AS dst_port_range
  , acl_recur.priority                      AS priority
  , acl_recur.description                   AS description
FROM ip_route_path_acls_recursion AS acl_recur
WHERE (acl_recur.hop_index = array_length($3, 1))
ORDER BY
    priority ASC
  , src_ip_net ASC
  , dst_ip_net ASC
  , protocol ASC
  , src_port_range ASC
  , dst_port_range ASC
  , action ASC
$$
LANGUAGE SQL
;


-- ----------------------------------------------------------------------
-- Device VLAN views
-- ----------------------------------------------------------------------

CREATE VIEW device_vlans AS
SELECT DISTINCT
    device_id                   AS device_id
  , vlan                        AS vlan
  , description                 AS description
FROM raw_device_vlans
ORDER BY device_id, vlan
;


CREATE VIEW device_vlans_ip_nets AS
SELECT DISTINCT
    device_id                   AS device_id
  , vlan                        AS vlan
  , ip_net                      AS ip_net
FROM raw_device_vlans_ip_nets
ORDER BY device_id, vlan
;


CREATE VIEW device_vlans_ip_nets_conflicting AS
SELECT DISTINCT
    dvin0.vlan                  AS vlan
  , dvin0.device_id             AS device0_id
  , dvin0.ip_net                AS device0_ip_net
  , dvin1.device_id             AS device1_id
  , dvin1.ip_net                AS device1_ip_net
FROM device_vlans_ip_nets AS dvin0
INNER JOIN device_vlans_ip_nets AS dvin1
  ON (dvin0.vlan = dvin1.vlan)
 AND (dvin0.device_id < dvin1.device_id) -- ensures only one copy of entry
 AND (inet_same_family(dvin0.ip_net, dvin1.ip_net))
 AND (dvin0.ip_net != dvin1.ip_net)
ORDER BY device0_id, device1_id, vlan
;


CREATE VIEW device_interfaces_vlans AS
SELECT DISTINCT
    device_id                   AS device_id
  , interface_name              AS interface_name
  , vlan                        AS vlan
FROM raw_device_interfaces_vlans
ORDER BY device_id, interface_name, vlan
;


CREATE VIEW raw_device_vlans_summaries AS
SELECT DISTINCT
    rdvin.tool_run_id           AS tool_run_id
  , rdvin.device_id             AS device_id
  , rdvin.vlan                  AS vlan
  , rdvin.ip_net                AS ip_net
  , rdv.description             AS description
FROM raw_device_vlans_ip_nets AS rdvin
JOIN raw_device_vlans AS rdv
  ON (rdvin.tool_run_id = rdv.tool_run_id)
 AND (rdvin.device_id = rdv.device_id)
 AND (rdvin.vlan = rdv.vlan)
;


CREATE VIEW device_vlans_summaries AS
SELECT DISTINCT
    device_id                   AS device_id
  , vlan                        AS vlan
  , ip_net                      AS ip_net
  , description                 AS description
FROM raw_device_vlans_summaries
ORDER BY device_id, vlan, ip_net
;


-- ----------------------------------------------------------------------

COMMIT TRANSACTION;
