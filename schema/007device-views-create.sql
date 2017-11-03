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

CREATE VIEW devices AS
SELECT DISTINCT
    device_id                   AS device_id
FROM raw_devices
;


-- ----------------------------------------------------------------------

CREATE VIEW device_types AS
SELECT DISTINCT
    ad.device_id                    AS device_id,
    rdt.device_type                 AS type,
    rdh.vendor                      AS vendor,
    rdh.model                       AS model
FROM 
  (
   SELECT device_id FROM raw_device_types
   UNION
   SELECT device_id FROM raw_device_hardware
  ) AS ad

LEFT OUTER JOIN raw_device_types AS rdt
ON (ad.device_id = rdt.device_id)

LEFT OUTER JOIN 
  (
   SELECT DISTINCT
       device_id,
       string_agg(distinct vendor, ',') AS vendor,
       string_agg(distinct model, ',') AS model,
       string_agg(distinct hardware_revision, ',') AS hardware_revision,
       string_agg(distinct serial_number, ',') AS serial_number,
       string_agg(distinct description, ',') AS description
   FROM raw_device_hardware
   GROUP BY device_id
  ) AS rdh
ON (ad.device_id = rdh.device_id)
;


-- ----------------------------------------------------------------------

CREATE VIEW device_virtualizations AS
SELECT DISTINCT
    host_device_id              AS host_device_id,
    guest_device_id             AS guest_device_id
FROM raw_device_virtualizations
;


-- ----------------------------------------------------------------------

CREATE VIEW device_interfaces AS
SELECT DISTINCT
    device_id                   AS device_id,
    interface_name              AS interface_name,
    media_type                  AS media_type,
    is_up                       AS is_up
FROM raw_device_interfaces
;


-- ----------------------------------------------------------------------

CREATE VIEW device_mac_addrs AS
SELECT DISTINCT
    device_id                   AS device_id,
    interface_name              AS interface_name,
    mac_addr                    AS mac_addr
FROM raw_device_mac_addrs
;


-- ----------------------------------------------------------------------

CREATE VIEW device_ip_addrs AS
SELECT DISTINCT
    device_id                   AS device_id,
    interface_name              AS interface_name,
    ip_addr                     AS ip_addr
FROM raw_device_ip_addrs
;


-- ----------------------------------------------------------------------

CREATE VIEW device_mac_addrs_ip_addrs AS
SELECT DISTINCT
    dma.device_id               AS device_id,
    dma.interface_name          AS interface_name,
    dma.mac_addr                AS mac_addr,
    dia.ip_addr                 AS ip_addr
FROM device_mac_addrs AS dma
JOIN device_ip_addrs AS dia
ON (dma.device_id      = dia.device_id) AND
   (dma.interface_name = dia.interface_name)
UNION
SELECT DISTINCT
    dma.device_id               AS device_id,
    dma.interface_name          AS interface_name,
    maia.mac_addr               AS mac_addr,
    maia.ip_addr                AS ip_addr
FROM device_mac_addrs AS dma
JOIN mac_addrs_ip_addrs AS maia
ON (dma.mac_addr = maia.mac_addr)
UNION
SELECT DISTINCT
    dia.device_id               AS device_id,
    dia.interface_name          AS interface_name,
    maia.mac_addr               AS mac_addr,
    maia.ip_addr                AS ip_addr
FROM device_ip_addrs AS dia
JOIN mac_addrs_ip_addrs AS maia
ON (dia.ip_addr = maia.ip_addr)
;


-- ----------------------------------------------------------------------

CREATE VIEW device_ip_routes AS
SELECT DISTINCT
    device_id                   AS device_id,
    interface_name              AS interface_name,
    dst_ip_net                  AS dst_ip_net,
    rtr_ip_addr                 AS rtr_ip_addr
FROM raw_device_ip_routes
;


-- ----------------------------------------------------------------------

CREATE VIEW device_ip_servers AS
SELECT DISTINCT
    device_id                   AS device_id,
    interface_name              AS interface_name,
    service_name                AS service_name,
    server_ip_addr              AS server_ip_addr
FROM raw_device_ip_servers
;


-- ----------------------------------------------------------------------

CREATE VIEW device_phys_connections AS
SELECT DISTINCT
    self_device_id              AS self_device_id,
    self_interface_name         AS self_interface_name,
    peer_device_id              AS peer_device_id,
    peer_interface_name         AS peer_interface_name
FROM raw_device_phys_connections
;


-- ----------------------------------------------------------------------

CREATE VIEW device_link_connections AS
SELECT DISTINCT
    self_device_id              AS self_device_id,
    self_interface_name         AS self_interface_name,
    peer_mac_addr               AS peer_mac_addr
FROM raw_device_link_connections
;


-- ----------------------------------------------------------------------

CREATE VIEW device_connections AS
SELECT
    self_device_id              AS self_device_id,
    self_interface_name         AS self_interface_name,
    peer_device_id              AS peer_device_id,
    peer_interface_name         AS peer_interface_name
FROM device_phys_connections
UNION
SELECT
    dlc.self_device_id          AS self_device_id,
    dlc.self_interface_name     AS self_interface_name,
    dma.device_id               AS peer_device_id,
    dma.interface_name          AS peer_interface_name
FROM device_link_connections AS dlc
JOIN device_mac_addrs AS dma
ON (dlc.peer_mac_addr = dma.mac_addr)
UNION
SELECT
    dlc.self_device_id          AS self_device_id,
    dlc.self_interface_name     AS self_interface_name,
    dia.device_id               AS peer_device_id,
    dia.interface_name          AS peer_interface_name
FROM device_link_connections AS dlc
JOIN mac_addrs_ip_addrs AS maia
ON (dlc.peer_mac_addr = maia.mac_addr)
JOIN device_ip_addrs AS dia
ON (maia.ip_addr = dia.ip_addr)
UNION
SELECT
    dlc.self_device_id          AS self_device_id,
    dlc.self_interface_name     AS self_interface_name,
    dlc.peer_mac_addr::text     AS peer_device_id,
    'UNKNOWN'                   AS peer_interface_name
FROM device_link_connections AS dlc 
LEFT OUTER JOIN device_mac_addrs AS dma
ON (dlc.peer_mac_addr = dma.mac_addr)
WHERE (dma.mac_addr IS NULL) AND (dlc.self_interface_name != 'CPU')
;


-- ----------------------------------------------------------------------
-- Cisco and Cisco-like device and interface information
-- ----------------------------------------------------------------------

CREATE VIEW devices_aaa AS
SELECT DISTINCT
    device_id                   AS device_id,
    aaa_command                 AS aaa_command
FROM raw_devices_aaa
;


-- ----------------------------------------------------------------------

CREATE VIEW devices_bpdu AS
SELECT DISTINCT
    device_id                   AS device_id,
    is_bpduguard_enabled        AS is_bpduguard_enabled,
    is_bpdufilter_enabled       AS is_bpdufilter_enabled
FROM raw_devices_bpdu
;


-- ----------------------------------------------------------------------

CREATE VIEW devices_cdp AS
SELECT DISTINCT
    device_id                   AS device_id,
    is_cdp_enabled              AS is_cdp_enabled
FROM raw_devices_cdp
;


-- ----------------------------------------------------------------------

CREATE VIEW device_interfaces_mode AS
SELECT DISTINCT
    device_id                   AS device_id,
    interface_name              AS interface_name,
    interface_mode              AS interface_mode
FROM raw_device_interfaces_mode
;


-- ----------------------------------------------------------------------

CREATE VIEW device_interfaces_bpdu AS
SELECT DISTINCT
    device_id                   AS device_id,
    interface_name              AS interface_name,
    is_bpduguard_enabled        AS is_bpduguard_enabled,
    is_bpdufilter_enabled       AS is_bpdufilter_enabled
FROM raw_device_interfaces_bpdu
;


-- ----------------------------------------------------------------------

CREATE VIEW device_interfaces_cdp AS
SELECT DISTINCT
    device_id                   AS device_id,
    interface_name              AS interface_name,
    is_cdp_enabled              AS is_cdp_enabled
FROM raw_device_interfaces_cdp
;


-- ----------------------------------------------------------------------

CREATE VIEW device_interfaces_portfast AS
SELECT DISTINCT
    device_id                   AS device_id,
    interface_name              AS interface_name,
    is_portfast_enabled         AS is_portfast_enabled
FROM raw_device_interfaces_portfast
;


-- ----------------------------------------------------------------------

CREATE VIEW device_interfaces_port_security AS
SELECT DISTINCT
    device_id                   AS device_id,
    interface_name              AS interface_name,
    is_port_security_enabled    AS is_port_security_enabled,
    is_mac_addr_sticky          AS is_mac_addr_sticky,
    max_mac_addrs               AS max_mac_addrs,
    violation_action            AS violation_action
FROM raw_device_interfaces_port_security
;


-- ----------------------------------------------------------------------

CREATE VIEW device_interfaces_port_security_mac_addrs AS
SELECT DISTINCT
    device_id                   AS device_id,
    interface_name              AS interface_name,
    mac_addr                    AS mac_addr
FROM raw_device_interfaces_port_security_mac_addrs
;


-- ----------------------------------------------------------------------

CREATE VIEW raw_device_interfaces_summaries AS
SELECT
    di.tool_run_id              AS tool_run_id,
    di.device_id                AS device_id,
    di.interface_name           AS interface_name,
    di.is_up                    AS is_up,
    di_mode.interface_mode      AS interface_mode,
    di_pf.is_portfast_enabled   AS is_portfast_enabled,
    (d_cdp.is_cdp_enabled AND di_cdp.is_cdp_enabled)
                                AS is_cdp_enabled,
    ((di_bpdu.is_bpduguard_enabled) OR
     (di_pf.is_portfast_enabled AND d_bpdu.is_bpduguard_enabled))
                                AS is_bpduguard_enabled,
    ((di_bpdu.is_bpdufilter_enabled) OR
     (di_pf.is_portfast_enabled AND d_bpdu.is_bpdufilter_enabled))
                                AS is_bpdufilter_enabled,
    di_ps.is_port_security_enabled AS is_port_security_enabled,
    di_ps.is_mac_addr_sticky    AS is_mac_addr_sticky,
    di_ps.max_mac_addrs         AS max_mac_addrs,
    count(di_ps_ma.mac_addr)    AS learned_mac_addrs,
    di_ps.violation_action      AS violation_action
FROM raw_device_interfaces AS di

LEFT OUTER JOIN raw_device_interfaces_mode AS di_mode
ON (di.tool_run_id    = di_mode.tool_run_id) AND
   (di.device_id      = di_mode.device_id) AND
   (di.interface_name = di_mode.interface_name)

LEFT OUTER JOIN raw_device_interfaces_portfast AS di_pf
ON (di.tool_run_id    = di_pf.tool_run_id) AND
   (di.device_id      = di_pf.device_id) AND
   (di.interface_name = di_pf.interface_name)

LEFT OUTER JOIN raw_devices_cdp AS d_cdp
ON (di.tool_run_id = d_cdp.tool_run_id) AND
   (di.device_id   = d_cdp.device_id)

LEFT OUTER JOIN raw_device_interfaces_cdp AS di_cdp
ON (di.tool_run_id    = di_cdp.tool_run_id) AND
   (di.device_id      = di_cdp.device_id) AND
   (di.interface_name = di_cdp.interface_name)

LEFT OUTER JOIN raw_devices_bpdu AS d_bpdu
ON (di.tool_run_id = d_bpdu.tool_run_id) AND
   (di.device_id   = d_bpdu.device_id)

LEFT OUTER JOIN raw_device_interfaces_bpdu AS di_bpdu
ON (di.tool_run_id    = di_bpdu.tool_run_id) AND
   (di.device_id      = di_bpdu.device_id) AND
   (di.interface_name = di_bpdu.interface_name)

LEFT OUTER JOIN raw_device_interfaces_port_security AS di_ps
ON (di.tool_run_id    = di_ps.tool_run_id) AND
   (di.device_id      = di_ps.device_id) AND
   (di.interface_name = di_ps.interface_name)

LEFT OUTER JOIN raw_device_interfaces_port_security_mac_addrs AS di_ps_ma
ON (di.tool_run_id    = di_ps_ma.tool_run_id) AND
   (di.device_id      = di_ps_ma.device_id) AND
   (di.interface_name = di_ps_ma.interface_name)

GROUP BY
    di.tool_run_id,
    di.device_id,
    di.interface_name,
    di.is_up,
    di_mode.interface_mode,
    di_pf.is_portfast_enabled,
    (d_cdp.is_cdp_enabled AND di_cdp.is_cdp_enabled),
    ((di_bpdu.is_bpduguard_enabled) OR
     (di_pf.is_portfast_enabled AND d_bpdu.is_bpduguard_enabled)),
    ((di_bpdu.is_bpdufilter_enabled) OR
     (di_pf.is_portfast_enabled AND d_bpdu.is_bpdufilter_enabled)),
    di_ps.is_port_security_enabled,
    di_ps.is_mac_addr_sticky,
    di_ps.max_mac_addrs,
    di_ps.violation_action
;


-- ----------------------------------------------------------------------

CREATE VIEW device_interfaces_summaries AS
SELECT DISTINCT
    device_id                   AS device_id,
    interface_name              AS interface_name,
    is_up                       AS is_up,
    interface_mode              AS interface_mode,
    is_portfast_enabled         AS is_portfast_enabled,
    is_cdp_enabled              AS is_cdp_enabled,
    is_bpduguard_enabled        AS is_bpduguard_enabled,
    is_bpdufilter_enabled       AS is_bpdufilter_enabled,
    is_port_security_enabled    AS is_port_security_enabled,
    is_mac_addr_sticky          AS is_mac_addr_sticky,
    max_mac_addrs               AS max_mac_addrs,
    learned_mac_addrs           AS learned_mac_addrs,
    violation_action            AS violation_action
FROM raw_device_interfaces_summaries
;


-- ----------------------------------------------------------------------
-- Device Access Control List (ACL) views
-- ----------------------------------------------------------------------

CREATE VIEW device_acl_action_sets AS
SELECT DISTINCT
    device_id                   AS device_id,
    action_set                  AS action_set
FROM raw_device_acl_action_sets
;


-- ----------------------------------------------------------------------

CREATE VIEW device_acl_actions AS
SELECT DISTINCT
    device_id                   AS device_id,
    action_set                  AS action_set,
    action                      AS action
FROM raw_device_acl_actions
;


-- ----------------------------------------------------------------------

CREATE VIEW device_acl_ip_net_sets AS
SELECT DISTINCT
    device_id                   AS device_id,
    ip_net_set                  AS ip_net_set
FROM raw_device_acl_ip_net_sets
;


-- ----------------------------------------------------------------------

CREATE VIEW device_acl_ip_nets AS
SELECT DISTINCT
    device_id                   AS device_id,
    ip_net_set                  AS ip_net_set,
    ip_net                      AS ip_net
FROM raw_device_acl_ip_nets
;


-- ----------------------------------------------------------------------

CREATE VIEW device_acl_port_range_sets AS
SELECT DISTINCT
    device_id                   AS device_id,
    port_range_set              AS port_range_set
FROM raw_device_acl_port_range_sets
;


-- ----------------------------------------------------------------------

CREATE VIEW device_acl_port_ranges AS
SELECT DISTINCT
    device_id                   AS device_id,
    port_range_set              AS port_range_set,
    protocol                    AS protocol,
    port_range                  AS port_range
FROM raw_device_acl_port_ranges
;


-- ----------------------------------------------------------------------

CREATE VIEW device_acls AS
SELECT DISTINCT
    device_id                   AS device_id,
    acl_set                     AS acl_set,
    acl_number                  AS acl_number,
    action_set                  AS action_set,
    src_ip_net_set              AS src_ip_net_set,
    dst_ip_net_set              AS dst_ip_net_set,
    src_port_range_set          AS src_port_range_set,
    dst_port_range_set          AS dst_port_range_set
FROM raw_device_acls
;


-- ----------------------------------------------------------------------

CREATE VIEW raw_device_acls_detail AS
SELECT DISTINCT
    acls.tool_run_id            AS tool_run_id,
    acls.device_id              AS device_id,
    acls.acl_set                AS acl_set,
    acls.acl_number             AS acl_number,
    acls.action_set             AS action_set,
    actions.action              AS action,
    acls.src_ip_net_set         AS src_ip_net_set,
    src_ip_nets.ip_net          AS src_ip_net,
    acls.dst_ip_net_set         AS dst_ip_net_set,
    dst_ip_nets.ip_net          AS dst_ip_net,
    dst_ports.protocol          AS protocol,
    acls.src_port_range_set     AS src_port_range_set,
    src_ports.port_range        AS src_port_range,
    acls.dst_port_range_set     AS dst_port_range_set,
    dst_ports.port_range        AS dst_port_range
FROM raw_device_acls AS acls
JOIN raw_device_acl_actions AS actions
ON (acls.tool_run_id = actions.tool_run_id) AND
   (acls.device_id   = actions.device_id) AND
   (acls.action_set  = actions.action_set)
JOIN raw_device_acl_ip_nets AS src_ip_nets
ON (acls.tool_run_id    = src_ip_nets.tool_run_id) AND
   (acls.device_id      = src_ip_nets.device_id) AND
   (acls.src_ip_net_set = src_ip_nets.ip_net_set)
JOIN raw_device_acl_ip_nets AS dst_ip_nets
ON (acls.tool_run_id    = dst_ip_nets.tool_run_id) AND
   (acls.device_id      = dst_ip_nets.device_id) AND
   (acls.dst_ip_net_set = dst_ip_nets.ip_net_set) AND
   (family(src_ip_nets.ip_net) = family(dst_ip_nets.ip_net))
JOIN raw_device_acl_port_ranges AS src_ports
ON (acls.tool_run_id        = src_ports.tool_run_id) AND
   (acls.device_id          = src_ports.device_id) AND
   (acls.src_port_range_set = src_ports.port_range_set)
JOIN raw_device_acl_port_ranges AS dst_ports
ON (acls.tool_run_id        = dst_ports.tool_run_id) AND
   (acls.device_id          = dst_ports.device_id) AND
   (acls.dst_port_range_set = dst_ports.port_range_set) AND
   (src_ports.protocol      = dst_ports.protocol)
;


-- ----------------------------------------------------------------------

CREATE VIEW device_acls_detail AS
SELECT DISTINCT
    device_id                   AS device_id,
    acl_set                     AS acl_set,
    acl_number                  AS acl_number,
    action_set                  AS action_set,
    action                      AS action,
    src_ip_net_set              AS src_ip_net_set,
    src_ip_net                  AS src_ip_net,
    dst_ip_net_set              AS dst_ip_net_set,
    dst_ip_net                  AS dst_ip_net,
    protocol                    AS protocol,
    src_port_range_set          AS src_port_range_set,
    src_port_range              AS src_port_range,
    dst_port_range_set          AS dst_port_range_set,
    dst_port_range              AS dst_port_range
FROM raw_device_acls_detail
;


-- ----------------------------------------------------------------------

COMMIT TRANSACTION;
