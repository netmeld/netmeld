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

CREATE VIEW playbook_intra_network_dashboard AS
SELECT DISTINCT
    pins.playbook_source_id     AS playbook_source_id,
    pins.is_completed           AS is_completed,
    pins.playbook_stage         AS playbook_stage,
    pins.interface_name         AS interface_name,
    pins.vlan                   AS vlan,
    pins.mac_addr               AS mac_addr,
    pins.ip_addr                AS ip_addr,
    pins.description            AS description,
    count(DISTINCT tria.tool_run_id) AS tool_run_count,
    count(DISTINCT ia.ip_addr)       AS responding_ip_addr_count,
    count(DISTINCT pre.error_type)   AS error_count
FROM playbook_intra_network_sources AS pins
LEFT OUTER JOIN tool_run_ip_addrs AS tria
ON (pins.interface_name = tria.interface_name) AND
   (pins.ip_addr = tria.ip_addr)
LEFT OUTER JOIN raw_ip_addrs AS ia
ON (tria.tool_run_id = ia.tool_run_id)
LEFT OUTER JOIN playbook_runtime_errors AS pre
ON (pins.playbook_source_id = pre.playbook_source_id)
WHERE (ia.is_responding) OR (ia.is_responding IS NULL)
GROUP BY
    pins.playbook_source_id,
    pins.is_completed,
    pins.playbook_stage,
    pins.interface_name,
    pins.vlan,
    pins.mac_addr,
    pins.ip_addr,
    pins.description
ORDER BY
    pins.playbook_stage,
    pins.is_completed,
    pins.ip_addr
;


-- ----------------------------------------------------------------------

CREATE VIEW playbook_inter_network_dashboard AS
SELECT DISTINCT
    pins.playbook_source_id     AS playbook_source_id,
    pins.is_completed           AS is_completed,
    pins.playbook_stage         AS playbook_stage,
    pins.interface_name         AS interface_name,
    pins.vlan                   AS vlan,
    pins.mac_addr               AS mac_addr,
    pins.ip_addr                AS ip_addr,
    coalesce(pins.ptp_rtr_ip_addr,
             pir.rtr_ip_addr)   AS rtr_ip_addr,
    pins.description            AS description,
    count(DISTINCT tria.tool_run_id)    AS tool_run_count,
    count(DISTINCT ia.ip_addr)          AS responding_ip_addr_count,
    count(DISTINCT pre.error_type)   AS error_count
FROM playbook_inter_network_sources AS pins
LEFT OUTER JOIN playbook_ip_routers AS pir
ON (pir.rtr_ip_addr <<= network(pins.ip_addr))
LEFT OUTER JOIN tool_run_ip_addrs AS tria
ON (pins.interface_name = tria.interface_name) AND
   (pins.ip_addr = tria.ip_addr)
LEFT OUTER JOIN tool_run_ip_routes AS trir
ON (tria.tool_run_id = trir.tool_run_id) AND
   (pins.interface_name = trir.interface_name) AND
   (pir.rtr_ip_addr = trir.next_hop_ip_addr)
LEFT OUTER JOIN raw_ip_addrs AS ia
ON (tria.tool_run_id = ia.tool_run_id)
LEFT OUTER JOIN playbook_runtime_errors AS pre
ON (pins.playbook_source_id = pre.playbook_source_id)
WHERE ((ia.is_responding) OR (ia.is_responding IS NULL)) AND
      ((pir.rtr_ip_addr != host(pins.ip_addr)::INET) OR
       (pir.rtr_ip_addr IS NULL))
GROUP BY
    pins.playbook_source_id,
    pins.is_completed,
    pins.playbook_stage,
    pins.interface_name,
    pins.vlan,
    pins.mac_addr,
    pins.ip_addr,
    coalesce(pins.ptp_rtr_ip_addr,
             pir.rtr_ip_addr),
    pins.description
ORDER BY
    pins.playbook_stage,
    pins.is_completed,
    pins.ip_addr,
    coalesce(pins.ptp_rtr_ip_addr,
             pir.rtr_ip_addr)
;


-- ----------------------------------------------------------------------

COMMIT TRANSACTION;
