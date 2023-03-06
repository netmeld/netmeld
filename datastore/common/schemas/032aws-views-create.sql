-- =============================================================================
-- Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
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

-------------------------------------------------------------------------------
-- Table views; no inter-dependencies
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------

CREATE OR REPLACE VIEW aws_network_interface_mac_ips AS
SELECT DISTINCT
    t1.interface_id
  , t2.mac_address
  , t1.ip_address
FROM raw_aws_network_interface_ips AS t1
LEFT JOIN raw_aws_network_interface_macs AS t2
  ON t1.interface_id = t2.interface_id
;

-------------------------------------------------------------------------------

CREATE OR REPLACE VIEW aws_vpc_route_table_defaults AS
SELECT DISTINCT
  vpc_id, route_table_id
FROM raw_aws_vpc_route_tables AS ravrt
WHERE NOT EXISTS (
  SELECT
  FROM raw_aws_route_table_associations
  WHERE route_table_id = ravrt.route_table_id
)
;

-------------------------------------------------------------------------------

CREATE OR REPLACE VIEW aws_network_acl_rules_joined AS
SELECT DISTINCT
    t1.network_acl_id , t1.egress , t1.rule_number , t1.action , t1.protocol , t1.cidr_block
  , CASE
      WHEN t1.protocol != '1' THEN
        PortRange(GREATEST(NULLIF(t2.from_port, -1), 0), LEAST(NULLIF(t2.to_port,-1),65535), '[]')
      WHEN t1.protocol = '1' THEN
        NULL
    END ports
  , CASE
      WHEN t1.protocol = '1' OR t1.protocol = '-1' THEN
        GREATEST(t3.type, -1)
      WHEN t1.protocol != '1' THEN
        NULL
    END type
  , CASE
      WHEN t1.protocol = '1' OR t1.protocol = '-1' THEN
        GREATEST(t3.code, -1)
      WHEN t1.protocol != '1' THEN
        NULL
    END code
FROM raw_aws_network_acl_rules AS t1
LEFT JOIN raw_aws_network_acl_rules_ports AS t2
  ON t1.network_acl_id = t2.network_acl_id
 AND t1.egress = t2.egress
 AND t1.rule_number = t2.rule_number
LEFT JOIN raw_aws_network_acl_rules_type_codes AS t3
  ON t1.network_acl_id = t3.network_acl_id
 AND t1.egress = t3.egress
 AND t1.rule_number = t3.rule_number
;

-------------------------------------------------------------------------------

CREATE OR REPLACE VIEW aws_active_instance_details AS
SELECT DISTINCT
    instance_id
  , instance_type
  , image_id
  , architecture
  , platform_details
  , launch_time
  , availability_zone
  , state_code
  , state_name
FROM raw_aws_instance_details
WHERE state_name NOT IN ('terminated','shutting down')
;

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Additional views; inter-dependencies
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Stiching IDs
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------

CREATE OR REPLACE VIEW aws_eni_sg_subnet_nacl_rt_vpc_join AS
SELECT DISTINCT
  ranivs.interface_id
  , ranisg.security_group_id
  , ranivs.subnet_id
  , ranas.network_acl_id
  , COALESCE(rarta.route_table_id, avrtd.route_table_id)
  , ranivs.vpc_id
FROM raw_aws_network_interface_vpc_subnet AS ranivs
LEFT JOIN raw_aws_network_interface_security_groups AS ranisg
  ON ranivs.interface_id = ranisg.interface_id
LEFT JOIN raw_aws_network_acl_subnets AS ranas
  ON ranivs.subnet_id = ranas.subnet_id
LEFT JOIN raw_aws_route_table_associations AS rarta
  ON ranivs.subnet_id = rarta.association_id
LEFT JOIN aws_vpc_route_table_defaults AS avrtd
  ON ranivs.vpc_id = avrtd.vpc_id
;

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Routing
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------

CREATE OR REPLACE VIEW aws_route_table_routes AS
SELECT DISTINCT
    route_table_id
  , destination_id AS next_hop_id
  , cidr_block::text AS next_hop
  , state
FROM raw_aws_route_table_routes_cidr
UNION
SELECT DISTINCT
    route_table_id
  , destination_id AS next_hop_id
  , destination AS next_hop
  , state
FROM raw_aws_route_table_routes_non_cidr
;

-------------------------------------------------------------------------------

CREATE OR REPLACE VIEW aws_subnet_route_tables AS
SELECT DISTINCT
    t1.vpc_id
  , t1.subnet_id
  , COALESCE(t2.route_table_id , t3.route_table_id) AS route_table_id
FROM raw_aws_vpc_subnets AS t1
LEFT JOIN raw_aws_route_table_associations AS t2
  ON t1.subnet_id = t2.association_id
LEFT JOIN raw_aws_vpc_route_tables AS t3
  ON t1.vpc_id = t3.vpc_id
 AND t3.is_default
;

-------------------------------------------------------------------------------

CREATE OR REPLACE VIEW aws_subnet_route_table_next_hops AS
SELECT DISTINCT
    t1.vpc_id , t1.subnet_id , t1.route_table_id
  , t2.next_hop_id , t2.next_hop , t2.state
FROM aws_subnet_route_tables AS t1
LEFT JOIN aws_route_table_routes AS t2
  ON t1.route_table_id = t2.route_table_id
;

-------------------------------------------------------------------------------

CREATE OR REPLACE VIEW aws_route_table_next_hops_unknown_in_db AS
SELECT DISTINCT
    next_hop_id
  , next_hop
FROM aws_route_table_routes
WHERE next_hop IN (
  SELECT DISTINCT
      cidr_block::text
  FROM raw_aws_cidr_blocks
  WHERE cidr_block NOT IN (
    SELECT DISTINCT cidr_block FROM raw_aws_route_table_routes_cidr WHERE destination_id = 'local'
    UNION
    SELECT DISTINCT ip_address FROM raw_aws_network_interface_ips
    UNION
    SELECT DISTINCT cidr_block FROM raw_aws_vpc_cidr_blocks
    UNION
    SELECT DISTINCT cidr_block FROM raw_aws_subnet_cidr_blocks
  )
  UNION
  SELECT DISTINCT
      destination
  FROM raw_aws_route_table_routes_non_cidr
)
;

-------------------------------------------------------------------------------


-------------------------------------------------------------------------------
-- Security Groups
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------

CREATE OR REPLACE VIEW aws_security_group_rules_ports AS
SELECT DISTINCT
    security_group_id
  , egress
  , protocol
  , PortRange(GREATEST(NULLIF(from_port, -1), 0), LEAST(NULLIF(to_port,-1),65535), '[]') AS ports
  , cidr_block
FROM raw_aws_security_group_rules_ports
;

-------------------------------------------------------------------------------

CREATE OR REPLACE VIEW aws_security_group_rules_non_ip_ports AS
SELECT DISTINCT
    t1.security_group_id
  , t1.egress
  , t1.protocol
  , PortRange(GREATEST(NULLIF(t1.from_port, -1), 0), LEAST(NULLIF(t1.to_port,-1),65535), '[]') AS ports
  , t1.target
  , t3.ip_address AS cidr_block
FROM raw_aws_security_group_rules_non_ip_ports AS t1
LEFT JOIN raw_aws_network_interface_security_groups AS t2
  ON t1.target = t2.security_group_id
LEFT JOIN raw_aws_network_interface_ips AS t3
  ON t2.interface_id = t3.interface_id
;

-------------------------------------------------------------------------------

CREATE OR REPLACE VIEW aws_security_group_rules_type_codes AS
SELECT DISTINCT
    security_group_id
  , egress
  , protocol
  , type
  , code
  , cidr_block
FROM raw_aws_security_group_rules_type_codes
;

-------------------------------------------------------------------------------

CREATE OR REPLACE VIEW aws_security_group_rules_non_ip_type_codes AS
SELECT DISTINCT
    t1.security_group_id
  , t1.egress
  , t1.protocol
  , t1.type
  , t1.code
  , t1.target
  , t3.ip_address AS cidr_block
FROM raw_aws_security_group_rules_non_ip_type_codes AS t1
LEFT JOIN raw_aws_network_interface_security_groups AS t2
  ON t1.target = t2.security_group_id
LEFT JOIN raw_aws_network_interface_ips AS t3
  ON t2.interface_id = t3.interface_id
;

-------------------------------------------------------------------------------

CREATE OR REPLACE VIEW aws_security_group_rules_full_machine AS
SELECT DISTINCT
    t1.security_group_id
  , t2.egress
  , t2.protocol
  , PortRange(GREATEST(NULLIF(t2.from_port, -1), 0), LEAST(NULLIF(t2.to_port,-1),65535), '[]') AS ports
  , NULL::NUMERIC AS type
  , NULL::NUMERIC AS code
  , t2.cidr_block
  , NULL::TEXT AS target
FROM raw_aws_security_groups AS t1
LEFT JOIN raw_aws_security_group_rules_ports AS t2
  ON t1.security_group_id = t2.security_group_id
UNION
SELECT DISTINCT
    t1.security_group_id
  , t2.egress
  , t2.protocol
  , PortRange(GREATEST(NULLIF(t2.from_port, -1), 0), LEAST(NULLIF(t2.to_port,-1),65535), '[]') AS ports
  , NULL::NUMERIC AS type
  , NULL::NUMERIC AS code
  , NULL::INET AS cidr_block
  , t2.target
FROM raw_aws_security_groups AS t1
LEFT JOIN raw_aws_security_group_rules_non_ip_ports AS t2
  ON t1.security_group_id = t2.security_group_id
UNION
SELECT DISTINCT
    t1.security_group_id
  , t2.egress
  , t2.protocol
  , NULL::PortRange AS ports
  , t2.type
  , t2.code
  , t2.cidr_block
  , NULL::TEXT AS target
FROM raw_aws_security_groups AS t1
LEFT JOIN raw_aws_security_group_rules_type_codes AS t2
  ON t1.security_group_id = t2.security_group_id
UNION
SELECT DISTINCT
    t1.security_group_id
  , t2.egress
  , t2.protocol
  , NULL::PortRange AS ports
  , t2.type
  , t2.code
  , NULL::INET AS cidr_block
  , t2.target
FROM raw_aws_security_groups AS t1
LEFT JOIN raw_aws_security_group_rules_non_ip_type_codes AS t2
  ON t1.security_group_id = t2.security_group_id
;

-------------------------------------------------------------------------------

CREATE OR REPLACE VIEW aws_eni_security_group_rules_ports AS
WITH cte1 AS (
SELECT DISTINCT
    t1.interface_id
  , t1.security_group_id
  , t2.egress
  , t2.protocol
  , t2.ports
  , t2.cidr_block
  , NULL AS target
FROM raw_aws_network_interface_security_groups AS t1
LEFT JOIN aws_security_group_rules_ports AS t2
  ON (t1.security_group_id = t2.security_group_id)
UNION
SELECT DISTINCT
    t1.interface_id
  , t1.security_group_id
  , t2.egress
  , t2.protocol
  , t2.ports
  , t2.cidr_block
  , t2.target
FROM raw_aws_network_interface_security_groups AS t1
LEFT JOIN aws_security_group_rules_non_ip_ports AS t2
  ON (t1.security_group_id = t2.security_group_id)
)
SELECT DISTINCT
    t1.interface_id
  , t1.egress
  , t1.protocol
  , t1.ports
  , t1.cidr_block
  , t1.target
FROM cte1 AS t1
LEFT JOIN cte1 AS t2
  ON (t1.interface_id = t2.interface_id)
 AND (t1.egress = t2.egress)
 AND (t1.protocol = t2.protocol OR t2.protocol = '-1')
 AND NOT (t1.cidr_block = t2.cidr_block AND t1.ports = t2.ports)
 AND ((t1.cidr_block <<= t2.cidr_block) OR (t1.target = t2.target))
 AND (t1.ports = t2.ports OR t1.ports <@ t2.ports)
WHERE (t2.interface_id IS NULL) -- eliminate overlaps
;

-------------------------------------------------------------------------------

CREATE OR REPLACE VIEW aws_eni_security_group_rules_type_codes AS
WITH cte1 AS (
SELECT DISTINCT
    t1.interface_id
  , t1.security_group_id
  , t2.egress
  , t2.protocol
  , t2.type
  , t2.code
  , t2.cidr_block
  , NULL AS target
FROM raw_aws_network_interface_security_groups AS t1
LEFT JOIN aws_security_group_rules_type_codes AS t2
  ON (t1.security_group_id = t2.security_group_id)
UNION
SELECT DISTINCT
    t1.interface_id
  , t1.security_group_id
  , t2.egress
  , t2.protocol
  , t2.type
  , t2.code
  , t2.cidr_block
  , t2.target
FROM raw_aws_network_interface_security_groups AS t1
LEFT JOIN aws_security_group_rules_non_ip_type_codes AS t2
  ON (t1.security_group_id = t2.security_group_id)
)
SELECT DISTINCT
    t1.interface_id
  , t1.egress
  , t1.protocol
  , t1.type
  , t1.code
  , t1.cidr_block
  , t1.target
FROM cte1 AS t1
LEFT JOIN cte1 AS t2
  ON (t1.interface_id = t2.interface_id)
 AND (t1.egress = t2.egress)
 AND (t1.protocol = t2.protocol OR t2.protocol = '-1')
 AND NOT (t1.cidr_block = t2.cidr_block AND t1.type = t2.type AND t1.code = t2.code)
 AND ((t1.cidr_block <<= t2.cidr_block) OR (t1.target = t2.target))
 AND ((t1.type != '-1' AND t2.type = '-1') OR (t1.code != '-1' AND t2.code = '-1'))
WHERE (t2.interface_id IS NULL) -- eliminate overlaps
;

-------------------------------------------------------------------------------

CREATE OR REPLACE VIEW aws_eni_security_group_rules_full_machine AS
SELECT DISTINCT
    interface_id
  , egress
  , protocol
  , ports
  , type
  , code
  , cidr_block
  , target
FROM (
SELECT DISTINCT
    interface_id
  , egress
  , CASE
    WHEN protocol = 'icmp' THEN 1
    WHEN protocol = 'tcp' THEN 6
    WHEN protocol = 'udp' THEN 17
    ELSE protocol::NUMERIC
    END AS protocol
  , ports
  , CASE
    WHEN protocol = '-1' THEN -1
    ELSE NULL::NUMERIC
    END AS type
  , CASE
    WHEN protocol = '-1' THEN -1
    ELSE NULL::NUMERIC
    END AS code
  , cidr_block
  , target
FROM aws_eni_security_group_rules_ports
UNION
SELECT DISTINCT
    interface_id
  , egress
  , CASE
    WHEN protocol = 'icmp' THEN 1
    WHEN protocol = 'tcp' THEN 6
    WHEN protocol = 'udp' THEN 17
    ELSE protocol::NUMERIC
    END AS protocol
  , CASE
    WHEN protocol = '-1' THEN PortRange(0,65535,'[]')
    ELSE NULL::PortRange
    END AS ports
  , type
  , code
  , cidr_block
  , target
FROM aws_eni_security_group_rules_type_codes
) AS t1
;

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- NACLs
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------

CREATE OR REPLACE VIEW aws_eni_network_acl_rules_full_machine AS
SELECT DISTINCT
    t1.interface_id
  , t2.egress
  , t2.rule_number::NUMERIC
  , t2.action
  , t2.protocol::NUMERIC
  , t2.cidr_block
  , t2.ports::PortRange
  , t2.type::NUMERIC
  , t2.code::NUMERIC
FROM aws_eni_sg_subnet_nacl_rt_vpc_join AS t1
LEFT JOIN aws_network_acl_rules_joined AS t2
  ON t1.network_acl_id = t2.network_acl_id
UNION
SELECT DISTINCT
    t1.interface_id
  , t1.egress
  , -32767 AS rule_number
  , 'allow' AS action
  , -1 AS protocol
  , t1.cidr_block
  , PortRange(0,65535,'[]') AS ports
  , -1 AS type
  , -1 AS code
FROM (
  WITH cte1 AS (
    SELECT DISTINCT
        t1.interface_id
      , t2.cidr_block
    FROM aws_eni_sg_subnet_nacl_rt_vpc_join AS t1
    LEFT JOIN raw_aws_subnet_cidr_blocks AS t2
      ON t1.subnet_id = t2.subnet_id
  )
  SELECT DISTINCT
      *, true AS egress
  FROM cte1
  UNION
  SELECT DISTINCT
      *, false AS egress
  FROM cte1
) AS t1
;

-------------------------------------------------------------------------------

CREATE OR REPLACE VIEW aws_subnet_network_acl_rules_full_machine AS
SELECT DISTINCT
    t1.network_acl_id
  , t1.subnet_id
  , t2.egress
  , t2.rule_number::NUMERIC
  , t2.action
  , t2.protocol::NUMERIC
  , t2.cidr_block
  , t2.ports::PortRange
  , t2.type::NUMERIC
  , t2.code::NUMERIC
FROM raw_aws_network_acl_subnets AS t1
LEFT JOIN aws_network_acl_rules_joined AS t2
  ON t1.network_acl_id = t2.network_acl_id
;

-------------------------------------------------------------------------------


COMMIT TRANSACTION;
