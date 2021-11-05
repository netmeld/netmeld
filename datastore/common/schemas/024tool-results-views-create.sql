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

CREATE VIEW mac_addrs_without_devices AS
SELECT DISTINCT
    ma.mac_addr                 AS mac_addr,
    ma.is_responding            AS is_responding
FROM raw_mac_addrs AS ma
LEFT OUTER JOIN raw_device_mac_addrs AS dma
ON (ma.mac_addr = dma.mac_addr)
WHERE (dma.mac_addr IS NULL)
EXCEPT
SELECT DISTINCT
    ma.mac_addr                 AS mac_addr,
    ma.is_responding            AS is_responding
FROM raw_mac_addrs AS ma
LEFT OUTER JOIN raw_device_link_connections AS dlc
ON (ma.mac_addr = dlc.peer_mac_addr)
WHERE (dlc.self_interface_name = 'CPU')
EXCEPT
SELECT DISTINCT
    ma.mac_addr                 AS mac_addr,
    ma.is_responding            AS is_responding
FROM raw_mac_addrs AS ma
JOIN device_mac_addrs_ip_addrs as dmaia
ON (ma.mac_addr = dmaia.mac_addr)
;


-- ----------------------------------------------------------------------

CREATE FUNCTION ip_route_paths(
    arg_src_ip_net INET,
    arg_dst_ip_net INET,
    arg_device_ids TEXT[] DEFAULT '{}'
)
RETURNS TABLE (
    src_ip_net              CIDR,
    dst_ip_net              CIDR,
    next_hop_ip_addr        INET,
    hop_count               INT,
    route_path              TEXT[],
    route_path_detail       RouteHop[]
) AS $$
WITH RECURSIVE device_ip_route_recursion(
    -- Source network:
    src_ip_net,
    -- Destination network:
    dst_ip_net,
    -- Next router in the path:
    next_hop_device_id,
    next_hop_vrf_id,
    next_hop_incoming_interface_name,
    next_hop_incoming_ip_addr,
    next_hop_incoming_ip_net,
    -- ARRAY of all hops along the path:
    route_path,
    route_path_detail
) AS (
    -- Base case: First router in path to destination.
    SELECT DISTINCT
        -- Source network:
        route_conns.incoming_ip_net                   AS src_ip_net,
        -- Destination network:
        route_conns.dst_ip_net                        AS dst_ip_net,
        -- Next router in the path:
        route_conns.next_hop_device_id                AS next_hop_device_id,
        route_conns.next_hop_vrf_id                   AS next_hop_vrf_id,
        route_conns.next_hop_incoming_interface_name  AS next_hop_incoming_interface_name,
        route_conns.next_hop_incoming_ip_addr         AS next_hop_incoming_ip_addr,
        route_conns.next_hop_incoming_ip_net          AS next_hop_incoming_ip_net,
        -- Insert current router's information into the path:
        ARRAY[route_conns.device_id]                  AS route_path,
        ARRAY[(
            route_conns.device_id,
            route_conns.vrf_id,
            route_conns.incoming_interface_name,
            route_conns.incoming_ip_addr,
            route_conns.incoming_ip_net,
            route_conns.outgoing_interface_name,
            route_conns.outgoing_ip_addr,
            route_conns.outgoing_ip_net
        )::RouteHop]                                  AS route_path_detail
    FROM device_ip_route_connections AS route_conns
    WHERE ($1 && route_conns.incoming_ip_net) AND
          ($2 && route_conns.dst_ip_net) AND
          (route_conns.is_active = true) AND
          (((route_conns.incoming_interface_name NOT LIKE 'management%') AND
            (route_conns.incoming_interface_name NOT LIKE 'mgmt%')) OR
           (route_conns.incoming_interface_name IS NULL)) AND
          (((route_conns.outgoing_interface_name NOT LIKE 'management%') AND
            (route_conns.outgoing_interface_name NOT LIKE 'mgmt%')) OR
           (route_conns.outgoing_interface_name IS NULL))
    UNION ALL
    -- Recursive case: Move current router one hop downstream.
    SELECT DISTINCT
        -- Source network:
        route_recur.src_ip_net                        AS src_ip_net,
        -- Destination network:
        CASE WHEN (route_recur.dst_ip_net <<= route_conns.dst_ip_net) THEN route_recur.dst_ip_net
             WHEN (route_conns.dst_ip_net <<= route_recur.dst_ip_net) THEN route_conns.dst_ip_net
             ELSE NULL
        END                                           AS dst_ip_net,
        -- Next router in the path:
        route_conns.next_hop_device_id                AS next_hop_device_id,
        route_conns.next_hop_vrf_id                   AS next_hop_vrf_id,
        route_conns.next_hop_incoming_interface_name  AS next_hop_incoming_interface_name,
        route_conns.next_hop_incoming_ip_addr         AS next_hop_incoming_ip_addr,
        route_conns.next_hop_incoming_ip_net          AS next_hop_incoming_ip_net,
        -- Insert current router's information into the path:
        route_path || route_conns.device_id           AS route_path,
        route_path_detail || (
            route_conns.device_id,
            route_conns.vrf_id,
            route_conns.incoming_interface_name,
            route_conns.incoming_ip_addr,
            route_conns.incoming_ip_net,
            route_conns.outgoing_interface_name,
            route_conns.outgoing_ip_addr,
            route_conns.outgoing_ip_net
        )::RouteHop                                   AS route_path_detail
    FROM device_ip_route_recursion AS route_recur
    JOIN device_ip_route_connections AS route_conns
    ON (route_recur.next_hop_device_id = route_conns.device_id) AND
       ((route_recur.next_hop_vrf_id = route_conns.vrf_id) OR
        (route_recur.next_hop_vrf_id IS NULL) OR (route_conns.vrf_id IS NULL)) AND
       (route_recur.next_hop_incoming_interface_name = route_conns.incoming_interface_name) AND
       (route_recur.next_hop_incoming_ip_net && route_conns.incoming_ip_net) AND
       (route_recur.dst_ip_net && route_conns.dst_ip_net)
    WHERE (route_conns.device_id != ALL(route_path)) AND
          ($2 && route_conns.dst_ip_net) AND
          (route_conns.is_active = true) AND
          (((route_conns.incoming_interface_name NOT LIKE 'management%') AND
            (route_conns.incoming_interface_name NOT LIKE 'mgmt%')) OR
           (route_conns.incoming_interface_name IS NULL)) AND
          (((route_conns.outgoing_interface_name NOT LIKE 'management%') AND
            (route_conns.outgoing_interface_name NOT LIKE 'mgmt%')) OR
           (route_conns.outgoing_interface_name IS NULL))
)
-- Filter results
SELECT
    route_recur.src_ip_net                        AS src_ip_net,
    route_recur.dst_ip_net                        AS dst_ip_net,
    route_recur.next_hop_incoming_ip_addr         AS next_hop_ip_addr,
    array_length(route_recur.route_path, 1)       AS hop_count,
    route_recur.route_path                        AS route_path,
    route_recur.route_path_detail                 AS route_path_detail
FROM device_ip_route_recursion AS route_recur
WHERE (route_recur.next_hop_device_id IS NULL) AND
      (route_recur.route_path @> $3) AND
      -- The route endpoints src_ip_net and dst_ip_net
      -- MUST be routable (not link-local) networks.
      (NOT route_recur.src_ip_net <<= 'fe80::/10'::CIDR) AND
      (NOT route_recur.dst_ip_net <<= 'fe80::/10'::CIDR) AND
      (NOT route_recur.src_ip_net <<= '169.254.0.0/16'::CIDR) AND
      (NOT route_recur.dst_ip_net <<= '169.254.0.0/16'::CIDR)
ORDER BY
    masklen(dst_ip_net) DESC,
    hop_count ASC,
    dst_ip_net ASC,
    src_ip_net ASC,
    route_path ASC
$$
LANGUAGE SQL
;


-- ----------------------------------------------------------------------

CREATE FUNCTION ip_route_path_acls(
    arg_src_ip_net INET,
    arg_dst_ip_net INET,
    arg_route_path_detail RouteHop[]
)
RETURNS TABLE (
    action              TEXT,
    src_ip_net          CIDR,
    dst_ip_net          CIDR,
    protocol            TEXT,
    src_port_range      PortRange,
    dst_port_range      PortRange,
    priority            INT[],
    description         TEXT[]
) AS $$
WITH RECURSIVE ip_route_path_acls_recursion(
    action,
    src_ip_net,
    dst_ip_net,
    protocol,
    src_port_range,
    dst_port_range,
    priority,
    description,
    hop_index
) AS (
    -- Base case: ACLs from first (hop_index = 1) router in the IP route path.
    SELECT DISTINCT
        acl_rules.action                    AS action,
        acl_rules.src_ip_net                AS src_ip_net,
        acl_rules.dst_ip_net                AS dst_ip_net,
        acl_rules.protocol                  AS protocol,
        acl_rules.src_port_range            AS src_port_range,
        acl_rules.dst_port_range            AS dst_port_range,
        ARRAY[acl_rules.priority]           AS priority,
        ARRAY[acl_rules.description]        AS description,
        1                                   AS hop_index
    FROM device_acl_rules AS acl_rules
    WHERE ($3[1].device_id = acl_rules.device_id) AND
          (($3[1].incoming_interface_name = acl_rules.incoming_interface_name) OR
           (acl_rules.incoming_zone_id = 'any' AND acl_rules.incoming_interface_name IS NULL)) AND
          (($3[1].outgoing_interface_name = acl_rules.outgoing_interface_name) OR
           (acl_rules.outgoing_zone_id = 'any' AND acl_rules.outgoing_interface_name IS NULL)) AND
          ($1 && acl_rules.src_ip_net) AND
          ($2 && acl_rules.dst_ip_net)
    UNION ALL
    -- Recursive case: Move current router one hop downstream.
    -- Calculate intersection between accumulated ACLs and current router ACLs.
    SELECT DISTINCT
        CASE WHEN (acl_recur.action = acl_rules.action) THEN acl_recur.action
             WHEN ('block' = acl_recur.action) THEN 'block'
             WHEN ('block' = acl_rules.action) THEN 'block'
             ELSE 'unknown'
        END                                 AS action,
        CASE WHEN (acl_recur.src_ip_net <<= acl_rules.src_ip_net) THEN acl_recur.src_ip_net
             WHEN (acl_rules.src_ip_net <<= acl_recur.src_ip_net) THEN acl_rules.src_ip_net
             ELSE NULL
        END                                 AS src_ip_net,
        CASE WHEN (acl_recur.dst_ip_net <<= acl_rules.dst_ip_net) THEN acl_recur.dst_ip_net
             WHEN (acl_rules.dst_ip_net <<= acl_recur.dst_ip_net) THEN acl_rules.dst_ip_net
             ELSE NULL
        END                                 AS dst_ip_net,
        CASE WHEN ('any' = acl_recur.protocol) THEN acl_rules.protocol
             ELSE acl_recur.protocol
        END                                 AS protocol,
        (acl_recur.src_port_range * acl_rules.src_port_range)
                                            AS src_port_range,
        (acl_recur.dst_port_range * acl_rules.dst_port_range)
                                            AS dst_port_range,
        (acl_recur.priority || acl_rules.priority)
                                            AS priority,
        (acl_recur.description || acl_rules.description)
                                            AS description,
        (acl_recur.hop_index + 1)           AS hop_index
    FROM ip_route_path_acls_recursion AS acl_recur
    JOIN device_acl_rules AS acl_rules
    ON ($3[acl_recur.hop_index + 1].device_id = acl_rules.device_id) AND
       (($3[acl_recur.hop_index + 1].incoming_interface_name = acl_rules.incoming_interface_name) OR
        (acl_rules.incoming_zone_id = 'any' AND acl_rules.incoming_interface_name IS NULL)) AND
       (($3[acl_recur.hop_index + 1].outgoing_interface_name = acl_rules.outgoing_interface_name) OR
        (acl_rules.outgoing_zone_id = 'any' AND acl_rules.outgoing_interface_name IS NULL)) AND
       (acl_recur.src_ip_net && acl_rules.src_ip_net) AND ($1 && acl_rules.src_ip_net) AND
       (acl_recur.dst_ip_net && acl_rules.dst_ip_net) AND ($2 && acl_rules.dst_ip_net) AND
       ( (acl_recur.protocol = acl_rules.protocol) OR
         ('any' = acl_recur.protocol) OR ('any' = acl_rules.protocol) ) AND
       (acl_recur.src_port_range && acl_rules.src_port_range) AND
       (acl_recur.dst_port_range && acl_rules.dst_port_range)
    WHERE ((acl_recur.hop_index + 1) <= array_length($3, 1))
)
-- Filter results
SELECT
    acl_recur.action                        AS action,
    acl_recur.src_ip_net                    AS src_ip_net,
    acl_recur.dst_ip_net                    AS dst_ip_net,
    acl_recur.protocol                      AS protocol,
    acl_recur.src_port_range                AS src_port_range,
    acl_recur.dst_port_range                AS dst_port_range,
    acl_recur.priority                      AS priority,
    acl_recur.description                   AS description
FROM ip_route_path_acls_recursion AS acl_recur
WHERE (acl_recur.hop_index = array_length($3, 1))
ORDER BY
    priority ASC,
    src_ip_net ASC,
    dst_ip_net ASC,
    protocol ASC,
    src_port_range ASC,
    dst_port_range ASC,
    action ASC
$$
LANGUAGE SQL
;


-- ----------------------------------------------------------------------

COMMIT TRANSACTION;
