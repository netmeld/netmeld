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

-- vpc default route table list
CREATE OR REPLACE VIEW aws_vpc_route_table_defaults AS
SELECT DISTINCT
  vpc_id, route_table_id
FROM raw_aws_vpc_route_tables AS ravrt
WHERE NOT EXISTS (SELECT FROM raw_aws_route_table_associations WHERE route_table_id = ravrt.route_table_id)
;
-- eni public ips
CREATE OR REPLACE VIEW aws_network_interface_public_ips AS
SELECT DISTINCT
  interface_id, ip_address
FROM raw_aws_network_interface_ips
WHERE NOT
  (    ip_address <<= '10/8'
    OR ip_address <<= '172.16/12'
    OR ip_address <<= '192.168/16'
  )
;
-- acl rule aggregation
CREATE OR REPLACE VIEW aws_network_acl_rules_joined AS
SELECT DISTINCT
    t1.network_acl_id , t1.egress , t1.rule_number , t1.action , t1.protocol , t1.cidr_block
--  , t2.from_port , t2.to_port
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

------------
-- ENI Based
------------

-- stitch; start eni,vpc,subnet add sg,nacl,route
CREATE OR REPLACE VIEW aws_eni_sg_subnet_nacl_rt_vpc_join AS
SELECT DISTINCT
  ranivs.interface_id AS iface_id
  , ranisg.security_group_id AS sg_id
  , ranivs.subnet_id AS net_id
  , ranas.network_acl_id AS nacl_id
  , COALESCE(rarta.route_table_id, avrtd.route_table_id) AS rt_id
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

-- egress
-- eni -> sg
CREATE OR REPLACE VIEW aws_eni_egress_t1 AS
SELECT DISTINCT
  aevsjsnr.iface_id
    , ranii.ip_address AS eni_src
  , aevsjsnr.sg_id
    , rasgrp.cidr_block AS sg_dest
    , rasgrp.protocol AS sg_protocol
    , PortRange(GREATEST(NULLIF(rasgrp.from_port, -1), 0), LEAST(NULLIF(rasgrp.to_port,-1),65535), '[]') AS sg_ports
  , aevsjsnr.net_id
  , aevsjsnr.nacl_id
  , aevsjsnr.rt_id
  , aevsjsnr.vpc_id
FROM aws_eni_vpc_subnet_join_sg_nacl_route AS aevsjsnr
LEFT JOIN raw_aws_network_interface_ips AS ranii
  ON aevsjsnr.iface_id = ranii.interface_id
LEFT JOIN raw_aws_security_group_rules_ports AS rasgrp
  ON aevsjsnr.sg_id = rasgrp.security_group_id
 AND rasgrp.egress = 'true'
;
-- prior cleaned
CREATE OR REPLACE VIEW aws_eni_egress_t1_cleaned AS
SELECT DISTINCT
    t1.iface_id
    , t1.eni_src
  , t1.sg_id
    , t1.sg_dest , t1.sg_protocol , t1.sg_ports
  , t1.net_id
  , t1.nacl_id
  , t1.rt_id
  , t1.vpc_id
FROM aws_eni_egress_t1 AS t1
LEFT JOIN aws_eni_egress_t1 AS t2
  ON (t1.iface_id = t2.iface_id)
 AND ((t1.sg_dest IS NULL AND t2.sg_dest IS NOT NULL) OR (t1.sg_dest << t2.sg_dest))
 AND (t1.sg_protocol = t2.sg_protocol OR t1.sg_protocol = '-1' OR t2.sg_protocol = '-1' OR t2.sg_protocol IS NULL)
 AND (t1.sg_ports = t2.sg_ports OR t1.sg_ports <@ t2.sg_ports)
WHERE t2.sg_dest IS NULL      -- AWS SGs are least restrictive rules
  AND t1.sg_id IS NOT NULL    -- TODO confirm still when tgws filled in
--  AND t1.sg_dest IS NOT NULL  -- no explicit egress rules
;
-- prior + subnet
CREATE OR REPLACE VIEW aws_eni_egress_t2 AS
SELECT DISTINCT
  aeet1.iface_id
    , aeet1.eni_src
  , aeet1.sg_id
    , aeet1.sg_dest , aeet1.sg_protocol , aeet1.sg_ports
  , aeet1.net_id
    , rascb.cidr_block AS subnet
  , aeet1.nacl_id
  , aeet1.rt_id
  , aeet1.vpc_id
FROM aws_eni_egress_t1_cleaned AS aeet1
LEFT JOIN raw_aws_subnet_cidr_blocks AS rascb
  ON aeet1.net_id = rascb.subnet_id
;
-- prior -> nacl and ports
CREATE OR REPLACE VIEW aws_eni_egress_t3 AS
SELECT DISTINCT
  aeet2.iface_id
    , aeet2.eni_src
  , aeet2.sg_id
    , aeet2.sg_dest , aeet2.sg_protocol , aeet2.sg_ports
  , aeet2.net_id
    , aeet2.subnet
  , aeet2.nacl_id
    , anarj.cidr_block AS nacl_dest
    , anarj.rule_number
    , anarj.action
    , anarj.protocol AS nacl_protocol
    , anarj.ports AS nacl_ports
  , aeet2.rt_id
  , aeet2.vpc_id
FROM aws_eni_egress_t2 AS aeet2
LEFT JOIN aws_network_acl_rules_joined AS anarj
  ON aeet2.nacl_id = anarj.network_acl_id
 AND anarj.egress = 'true'
 AND (   (aeet2.sg_protocol IS NULL) -- stateful allow case
      OR (aeet2.sg_protocol = anarj.protocol)
      OR (aeet2.sg_protocol = '-1' OR anarj.protocol = '-1') -- -1 means any
      OR (aeet2.sg_protocol = 'tcp' AND anarj.protocol = '6')
      OR (aeet2.sg_protocol = 'udp' AND anarj.protocol = '17')
     )
 AND NOT (anarj.rule_number = '32767' and anarj.action = 'deny')
;
-- prior cleaned
CREATE OR REPLACE VIEW aws_eni_egress_t3_cleaned AS
SELECT DISTINCT
  aeet3.iface_id
    , aeet3.eni_src
  , aeet3.sg_id
    , aeet3.sg_dest , aeet3.sg_protocol , aeet3.sg_ports
  , aeet3.net_id
    , aeet3.subnet
  , aeet3.nacl_id
    , aeet3.nacl_dest , aeet3.rule_number , aeet3.action , aeet3.nacl_protocol , aeet3.nacl_ports
  , aeet3.rt_id
  , aeet3.vpc_id
FROM aws_eni_egress_t3 AS aeet3
WHERE (aeet3.sg_ports <@ aeet3.nacl_ports OR aeet3.nacl_ports <@ aeet3.sg_ports)
;
-- prior -> route
CREATE OR REPLACE VIEW aws_eni_egress_t4 AS
SELECT DISTINCT
  aeet3.iface_id
    , aeet3.eni_src
  , aeet3.sg_id
    , aeet3.sg_dest , aeet3.sg_protocol , aeet3.sg_ports
  , aeet3.net_id
    , aeet3.subnet
  , aeet3.nacl_id
    , aeet3.nacl_dest , aeet3.rule_number , aeet3.action , aeet3.nacl_protocol , aeet3.nacl_ports
  , aeet3.rt_id
    , rartrc.cidr_block AS next_hop
    , rartrc.destination_id AS next_hop_id
  , aeet3.vpc_id
FROM aws_eni_egress_t3_cleaned AS aeet3
LEFT JOIN raw_aws_route_table_routes_cidr AS rartrc
  ON (aeet3.rt_id = rartrc.route_table_id)
 AND (aeet3.nacl_dest && rartrc.cidr_block)
 AND ((aeet3.sg_dest IS NULL) OR (aeet3.sg_dest && rartrc.cidr_block))
;
-- final stitch
CREATE OR REPLACE VIEW aws_eni_egress_t4_cleaned AS
SELECT DISTINCT
  t1.iface_id
    , t1.eni_src
  , t1.sg_id
    , t1.sg_dest , t1.sg_protocol , t1.sg_ports
  , t1.net_id
    , t1.subnet
  , t1.nacl_id
    , t1.nacl_dest , t1.rule_number , t1.action , t1.nacl_protocol , t1.nacl_ports
  , t1.rt_id
    , t1.next_hop
    , t1.next_hop_id
  , t1.vpc_id
FROM aws_eni_egress_t4 AS t1
WHERE (t1.next_hop IS NOT NULL) -- remove non-routable (already applied vpc default routes)
;

CREATE OR REPLACE VIEW aws_eni_vpc_subnet_join_sg_nacl_route AS
SELECT DISTINCT
  ranivs.interface_id AS iface_id
  , ranisg.security_group_id AS sg_id
  , ranas.network_acl_id AS nacl_id
  , COALESCE(rarta.route_table_id, avrtd.route_table_id) AS rt_id
  , ranivs.subnet_id AS net_id
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

-- prior cleaned
CREATE OR REPLACE VIEW aws_eni_egress_t5 AS
SELECT DISTINCT
    aeet4.iface_id
  , aeet4.eni_src
  , aeet4.subnet
  , dest
  , GREATEST(aeet4.sg_protocol , aeet4.nacl_protocol) AS protocol
  , aeet4.rule_number
  , aeet4.action
  , aeet4.sg_ports * aeet4.nacl_ports AS ports
  , aeet4.next_hop
  , aeet4.next_hop_id
--  , array_agg(DISTINCT aeet4.next_hop || ' via ' || aeet4.next_hop_id) AS routing
FROM (
  WITH dest_usage AS (
      SELECT * FROM (SELECT *, (sg_dest <<= nacl_dest) AS use_sg, (nacl_dest << sg_dest) AS use_nacl FROM aws_eni_egress_t4_cleaned) AS t1
    )
  SELECT
    *, sg_dest AS dest
  FROM dest_usage
  WHERE use_sg = 't'
  UNION
  SELECT
    *, nacl_dest AS dest
  FROM dest_usage
  WHERE use_sg = 'f'
) AS aeet4
-- GROUP BY 1,2,3,4,5,6,7,8
;


-- ingress; can build up same way as just what rules are tighter
-- eni -> sg
CREATE OR REPLACE VIEW aws_eni_ingress_t1 AS
SELECT DISTINCT
  aevsjsnr.iface_id
    , ranii.ip_address AS eni_dest
  , aevsjsnr.sg_id
    , rasgrp.cidr_block AS sg_src
    , rasgrp.protocol AS sg_protocol
    , PortRange(GREATEST(NULLIF(rasgrp.from_port, -1), 0), LEAST(NULLIF(rasgrp.to_port,-1),65535), '[]') AS sg_ports
  , aevsjsnr.net_id
  , aevsjsnr.nacl_id
  , aevsjsnr.rt_id
  , aevsjsnr.vpc_id
FROM aws_eni_vpc_subnet_join_sg_nacl_route AS aevsjsnr
LEFT JOIN raw_aws_network_interface_ips AS ranii
  ON aevsjsnr.iface_id = ranii.interface_id
LEFT JOIN raw_aws_security_group_rules_ports AS rasgrp
  ON aevsjsnr.sg_id = rasgrp.security_group_id
 AND rasgrp.egress = 'false'
;
-- prior cleaned
CREATE OR REPLACE VIEW aws_eni_ingress_t1_cleaned AS
SELECT DISTINCT
  t1.iface_id
    , t1.eni_dest
  , t1.sg_id
    , t1.sg_src , t1.sg_protocol , t1.sg_ports
  , t1.net_id
  , t1.nacl_id
  , t1.rt_id
  , t1.vpc_id
FROM aws_eni_ingress_t1 AS t1
LEFT JOIN aws_eni_ingress_t1 AS t2
  ON (t1.iface_id = t2.iface_id)
 AND ((t1.sg_src IS NULL AND t2.sg_src IS NOT NULL) OR (t1.sg_src << t2.sg_src))
 AND (t1.sg_protocol = t2.sg_protocol OR t1.sg_protocol = '-1' OR t2.sg_protocol = '-1')
 AND (t1.sg_ports = t2.sg_ports OR t1.sg_ports <@ t2.sg_ports)
WHERE t2.sg_src IS NULL     -- AWS SGs are least restrictive rules
  AND t1.sg_id IS NOT NULL  -- TODO confirm still when tgws filled in
--  AND t1.sg_src IS NOT NULL -- no explicit ingress rules
;
-- prior + subnet
CREATE OR REPLACE VIEW aws_eni_ingress_t2 AS
SELECT DISTINCT
  aeet1.iface_id
    , aeet1.eni_dest
  , aeet1.sg_id
    , aeet1.sg_src , aeet1.sg_protocol , aeet1.sg_ports
  , aeet1.net_id
    , rascb.cidr_block AS subnet
  , aeet1.nacl_id
  , aeet1.rt_id
  , aeet1.vpc_id
FROM aws_eni_ingress_t1_cleaned AS aeet1
LEFT JOIN raw_aws_subnet_cidr_blocks AS rascb
  ON aeet1.net_id = rascb.subnet_id
;
-- prior -> nacl and ports
CREATE OR REPLACE VIEW aws_eni_ingress_t3 AS
SELECT DISTINCT
  aeit2.iface_id
    , aeit2.eni_dest
  , aeit2.sg_id
    , aeit2.sg_src , aeit2.sg_protocol , aeit2.sg_ports
  , aeit2.net_id
    , aeit2.subnet
  , aeit2.nacl_id
    , anarj.cidr_block AS nacl_src
    , anarj.rule_number
    , anarj.action
    , anarj.protocol AS nacl_protocol
    , anarj.ports AS nacl_ports
  , aeit2.rt_id
  , aeit2.vpc_id
FROM aws_eni_ingress_t2 AS aeit2
LEFT JOIN aws_network_acl_rules_joined AS anarj
  ON aeit2.nacl_id = anarj.network_acl_id
 AND anarj.egress = 'false'
 AND (   (aeit2.sg_protocol IS NULL) -- stateful allow case 
      OR (aeit2.sg_protocol = anarj.protocol)
      OR (aeit2.sg_protocol = '-1' OR anarj.protocol = '-1') -- -1 means any
      OR (aeit2.sg_protocol = 'tcp' AND anarj.protocol = '6')
      OR (aeit2.sg_protocol = 'udp' AND anarj.protocol = '17')
     )
 AND NOT (anarj.rule_number = '32767' and anarj.action = 'deny')
;
-- prior -> cleaned
CREATE OR REPLACE VIEW aws_eni_ingress_t3_cleaned AS
SELECT DISTINCT
  aeit3.iface_id
    , aeit3.eni_dest
  , aeit3.sg_id
    , aeit3.sg_src , aeit3.sg_protocol , aeit3.sg_ports
  , aeit3.net_id
    , aeit3.subnet
  , aeit3.nacl_id
    , aeit3.nacl_src , aeit3.rule_number , aeit3.action , aeit3.nacl_protocol , aeit3.nacl_ports
  , aeit3.rt_id
  , aeit3.vpc_id
FROM aws_eni_ingress_t3 AS aeit3
WHERE (aeit3.sg_ports <@ aeit3.nacl_ports OR aeit3.nacl_ports <@ aeit3.sg_ports)
;
-- prior -> route
CREATE OR REPLACE VIEW aws_eni_ingress_t4 AS
SELECT DISTINCT
  aeit3.iface_id
    , aeit3.eni_dest
  , aeit3.sg_id
    , aeit3.sg_src , aeit3.sg_protocol , aeit3.sg_ports
  , aeit3.net_id
    , aeit3.subnet
  , aeit3.nacl_id
    , aeit3.nacl_src , aeit3.rule_number , aeit3.action , aeit3.nacl_protocol , aeit3.nacl_ports
  , aeit3.rt_id
    , rartrc.cidr_block AS next_hop
    , rartrc.destination_id AS next_hop_id
  , aeit3.vpc_id
FROM aws_eni_ingress_t3_cleaned AS aeit3
LEFT JOIN raw_aws_route_table_routes_cidr AS rartrc
  ON aeit3.rt_id = rartrc.route_table_id
 AND (aeit3.nacl_src && rartrc.cidr_block)
 AND ((aeit3.sg_src IS NULL) OR (aeit3.sg_src && rartrc.cidr_block))
;
-- prior -> route
CREATE OR REPLACE VIEW aws_eni_ingress_t4_cleaned AS
SELECT DISTINCT
  t1.iface_id
    , t1.eni_dest
  , t1.sg_id
    , t1.sg_src , t1.sg_protocol , t1.sg_ports
  , t1.net_id
    , t1.subnet
  , t1.nacl_id
    , t1.nacl_src , t1.rule_number , t1.action , t1.nacl_protocol , t1.nacl_ports
  , t1.rt_id
    , t1.next_hop , t1.next_hop_id
  , t1.vpc_id
FROM aws_eni_ingress_t4 AS t1
WHERE (t1.next_hop IS NOT NULL)
;
-- prior, cleaned; only required fields
CREATE OR REPLACE VIEW aws_eni_ingress_t5 AS
SELECT DISTINCT
    aeit4.iface_id
  , src
  , aeit4.subnet
  , aeit4.eni_dest
  , GREATEST(aeit4.sg_protocol , aeit4.nacl_protocol) AS protocol
  , aeit4.rule_number
  , aeit4.action
  , aeit4.sg_ports * aeit4.nacl_ports AS ports
  , aeit4.next_hop
  , aeit4.next_hop_id
--  , array_agg(DISTINCT aeit4.next_hop || ' via ' || aeit4.next_hop_id) AS routing
FROM (
  WITH src_usage AS (
      SELECT * FROM (SELECT *, (sg_src <<= nacl_src) AS use_sg, (nacl_src << sg_src) AS use_nacl FROM aws_eni_ingress_t4_cleaned) AS t1
    )
  SELECT
    *, sg_src AS src
  FROM src_usage
  WHERE use_sg = 't'
  UNION
  SELECT
    *, nacl_src AS src
  FROM src_usage
  WHERE use_sg = 'f'
) AS aeit4
--GROUP BY 1,2,3,4,5,6,7,8
;


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
--CREATE OR REPLACE VIEW aws_security_group_rules AS
--SELECT DISTINCT
--    security_group_id
--  , egress
--  , protocol
--  , from_port
--  , to_port
--  , -1 AS type
--  , -1 AS code
--  , cidr_block
--FROM raw_aws_security_group_rules_ports
--UNION
--SELECT DISTINCT
--    security_group_id
--  , egress
--  , protocol
--  , -1 AS from_port
--  , -1 AS to_port
--  , type
--  , code
--  , cidr_block
--FROM raw_aws_security_group_rules_type_codes
--;
CREATE OR REPLACE VIEW aws_security_group_rules_ports AS
SELECT DISTINCT
    security_group_id
  , egress
  , protocol
  , PortRange(GREATEST(NULLIF(from_port, -1), 0), LEAST(NULLIF(to_port,-1),65535), '[]') AS ports
  , cidr_block
FROM raw_aws_security_group_rules_ports
;
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
CREATE OR REPLACE VIEW aws_eni_security_group_rules_ports AS
WITH cte1 AS (
SELECT DISTINCT
    t1.interface_id
  , t1.security_group_id
  , t2.egress
  , t2.protocol
  , t2.ports
  , t2.cidr_block
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
FROM raw_aws_network_interface_security_groups AS t1
LEFT JOIN aws_security_group_rules_non_ip_ports AS t2
  ON (t1.security_group_id = t2.security_group_id)
)
SELECT DISTINCT
    t1.interface_id
  , t1.egress
  , t1.protocol
--  , CASE WHEN t1.protocol = '-1' THEN 'any' ELSE t1.protocol END
  , t1.ports
--  , CASE WHEN t1.protocol != 'tcp' OR t1.protocol != 'udp' THEN '' ELSE t1.ports END
  , t1.cidr_block
FROM cte1 AS t1
LEFT JOIN cte1 AS t2
  ON (t1.interface_id = t2.interface_id)
 AND (t1.egress = t2.egress)
 AND (t1.protocol = t2.protocol OR t2.protocol = '-1')
 AND NOT (t1.cidr_block = t2.cidr_block AND t1.ports = t2.ports)
 AND (t1.cidr_block <<= t2.cidr_block)
 AND (t1.ports = t2.ports OR t1.ports <@ t2.ports)
WHERE (t2.interface_id IS NULL) -- eliminate overlaps
;
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
FROM raw_aws_network_interface_security_groups AS t1
LEFT JOIN aws_security_group_rules_type_codes AS t2
  ON (t1.security_group_id = t2.security_group_id)
)
SELECT DISTINCT
    t1.interface_id
  , t1.egress
  , t1.protocol
  , t1.type
  , t1.code
  , t1.cidr_block
FROM cte1 AS t1
LEFT JOIN cte1 AS t2
  ON (t1.interface_id = t2.interface_id)
 AND (t1.egress = t2.egress)
 AND (t1.protocol = t2.protocol OR t2.protocol = '-1')
 AND NOT (t1.cidr_block = t2.cidr_block AND t1.type = t2.type AND t1.code = t2.code)
 AND (t1.cidr_block <<= t2.cidr_block)
 AND ((t1.type != '-1' AND t2.type = '-1') OR (t1.code != '-1' AND t2.code = '-1'))
WHERE (t2.interface_id IS NULL) -- eliminate overlaps
;

-------------------------------------------------------------------------------
CREATE OR REPLACE VIEW aws_eni_security_group_rules_full AS
SELECT DISTINCT
    interface_id
  , egress
--  , protocol
  , CASE WHEN protocol = '-1' THEN 'any' ELSE protocol END
--  , ports
  , CASE WHEN (protocol = '-1' OR (protocol != 'tcp' AND protocol != 'udp' AND protocol != 'icmp')) THEN 'any' ELSE ports END
  , cidr_block
FROM (
SELECT DISTINCT
    interface_id
  , egress
  , protocol
  , ports::text AS ports
  , cidr_block
FROM aws_eni_security_group_rules_ports
UNION
SELECT DISTINCT
    interface_id
  , egress
  , protocol
  , type||':'||code AS ports
  , cidr_block
FROM aws_eni_security_group_rules_type_codes
) AS t1
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
FROM aws_eni_security_group_rules_type_codes
) AS t1
;
CREATE OR REPLACE VIEW aws_eni_network_acl_rules_full_machine AS
SELECT DISTINCT
    t1.iface_id
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
  ON t1.nacl_id = t2.network_acl_id
UNION
SELECT DISTINCT
    t1.iface_id
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
        t1.iface_id
      , t2.cidr_block
    FROM aws_eni_sg_subnet_nacl_rt_vpc_join AS t1
    LEFT JOIN raw_aws_subnet_cidr_blocks AS t2
      ON t1.net_id = t2.subnet_id
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


-------------------------------------------------------------------------------


COMMIT TRANSACTION;
