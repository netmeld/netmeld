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
-- AWS Logic
-- ----------------------------------------------------------------------
-- Generic view of data flow and guard points; for reference.
--  Instance <-> Network Interface <-> Security Group <-> Network ACL <-> Route Table
--               |- Subnet ->
--  |- VPC -> (hooked at all levels)

-- ----------------------------------------------------------------------
-- AWS base service/components
-- ----------------------------------------------------------------------


-- AWS CidrBlock
CREATE TABLE raw_aws_cidr_blocks (
      tool_run_id                   UUID  NOT NULL
    , cidr_block                    INET  NOT NULL
    , PRIMARY KEY (tool_run_id, cidr_block)
    , FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);
CREATE TABLE raw_aws_cidr_block_details (
      tool_run_id                   UUID  NOT NULL
    , cidr_block                    INET  NOT NULL
    , state                         TEXT  NOT NULL
    , PRIMARY KEY (tool_run_id, cidr_block, state)
    , FOREIGN KEY (tool_run_id, cidr_block)
        REFERENCES raw_aws_cidr_blocks(tool_run_id, cidr_block)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);


-- AWS Instances
CREATE TABLE raw_aws_instances (
      tool_run_id                   UUID  NOT NULL
    , instance_id                   TEXT  NOT NULL
    , PRIMARY KEY (tool_run_id, instance_id)
    , FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);
CREATE TABLE raw_aws_instance_details (
      tool_run_id                   UUID  NOT NULL
    , instance_id                   TEXT  NOT NULL
    , instance_type                 TEXT  NOT NULL
    , image_id                      TEXT  NOT NULL
    , architecture                  TEXT  NOT NULL
    , platform_details              TEXT  NOT NULL
    , launch_time                   TEXT  NOT NULL
    , availability_zone             TEXT  NOT NULL
    , state_code                    INT   NOT NULL
    , state_name                    TEXT  NOT NULL
    , PRIMARY KEY (tool_run_id, instance_id, instance_type, image_id
                  , architecture, platform_details, launch_time
                  , availability_zone, state_code, state_name
                  )
    , FOREIGN KEY (tool_run_id, instance_id)
        REFERENCES raw_aws_instances(tool_run_id, instance_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);


-- AWS Network Interfaces
CREATE TABLE raw_aws_network_interfaces (
      tool_run_id                   UUID      NOT NULL
    , interface_id                  TEXT      NOT NULL
    , PRIMARY KEY (tool_run_id, interface_id)
    , FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);
CREATE TABLE raw_aws_network_interface_details (
      tool_run_id                   UUID  NOT NULL
    , interface_id                  TEXT  NOT NULL
    , interface_type                TEXT  NOT NULL
    , source_destination_check      BOOL  NOT NULL
    , status                        TEXT  NOT NULL
    , description                   TEXT  NOT NULL
    , PRIMARY KEY (tool_run_id, interface_id, interface_type
                  ,  source_destination_check, status
                  )
    , FOREIGN KEY (tool_run_id, interface_id)
        REFERENCES raw_aws_network_interfaces(tool_run_id, interface_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);
CREATE TABLE raw_aws_network_interface_attachments (
      tool_run_id                   UUID  NOT NULL
    , interface_id                  TEXT  NOT NULL
    , id                            TEXT  NOT NULL
    , status                        TEXT  NOT NULL
    , delete_on_termination         BOOL  NOT NULL
    , PRIMARY KEY (tool_run_id, interface_id, id, status
                  , delete_on_termination
                  )
    , FOREIGN KEY (tool_run_id, interface_id)
        REFERENCES raw_aws_network_interfaces(tool_run_id, interface_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);
CREATE TABLE raw_aws_network_interface_macs (
      tool_run_id                   UUID      NOT NULL
    , interface_id                  TEXT      NOT NULL
    , mac_address                   MACADDR   NOT NULL
    , PRIMARY KEY (tool_run_id, interface_id, mac_address)
    , FOREIGN KEY (tool_run_id, interface_id)
        REFERENCES raw_aws_network_interfaces(tool_run_id, interface_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
    , FOREIGN KEY (tool_run_id, mac_address)
        REFERENCES raw_mac_addrs(tool_run_id, mac_addr)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);
CREATE TABLE raw_aws_network_interface_ips (
      tool_run_id                   UUID      NOT NULL
    , interface_id                  TEXT      NOT NULL
    , ip_address                    INET      NOT NULL
    , dns_name                      TEXT      NOT NULL
    , PRIMARY KEY (tool_run_id, interface_id, ip_address, dns_name)
    , FOREIGN KEY (tool_run_id, interface_id)
        REFERENCES raw_aws_network_interfaces(tool_run_id, interface_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
    , FOREIGN KEY (tool_run_id, ip_address)
        REFERENCES raw_ip_addrs(tool_run_id, ip_addr)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);


-- AWS VPCs
CREATE TABLE raw_aws_vpcs (
      tool_run_id                   UUID  NOT NULL
    , vpc_id                        TEXT  NOT NULL
    , PRIMARY KEY (tool_run_id, vpc_id)
    , FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);
CREATE TABLE raw_aws_vpc_details (
      tool_run_id                   UUID  NOT NULL
    , vpc_id                        TEXT  NOT NULL
    , state                         TEXT  NOT NULL
--    , owner_id                      TEXT  NOT NULL
    , PRIMARY KEY (tool_run_id, vpc_id, state)
    , FOREIGN KEY (tool_run_id, vpc_id)
        REFERENCES raw_aws_vpcs(tool_run_id, vpc_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);
CREATE TABLE raw_aws_vpc_cidr_blocks (
      tool_run_id                   UUID  NOT NULL
    , vpc_id                        TEXT  NOT NULL
    , cidr_block                    CIDR  NOT NULL
    , state                         TEXT  NOT NULL
    , PRIMARY KEY (tool_run_id, vpc_id, cidr_block, state)
    , FOREIGN KEY (tool_run_id, vpc_id)
        REFERENCES raw_aws_vpcs(tool_run_id, vpc_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
    , FOREIGN KEY (tool_run_id, cidr_block)
        REFERENCES raw_aws_cidr_blocks(tool_run_id, cidr_block)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);


-- AWS Security Groups
CREATE TABLE raw_aws_security_groups (
      tool_run_id                   UUID  NOT NULL
    , security_group_id             TEXT  NOT NULL
    , PRIMARY KEY (tool_run_id, security_group_id)
    , FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);
CREATE TABLE raw_aws_security_group_details (
      tool_run_id                   UUID  NOT NULL
    , security_group_id             TEXT  NOT NULL
    , group_name                    TEXT  NOT NULL
    , description                   TEXT  NOT NULL
    , PRIMARY KEY (tool_run_id, security_group_id, group_name, description)
    , FOREIGN KEY (tool_run_id, security_group_id)
        REFERENCES raw_aws_security_groups(tool_run_id, security_group_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);
CREATE TABLE raw_aws_security_group_rules (
      tool_run_id                   UUID  NOT NULL
    , security_group_id             TEXT  NOT NULL
    , egress                        BOOL  NOT NULL
    , protocol                      TEXT  NOT NULL
    , from_port                     INT   NOT NULL
    , to_port                       INT   NOT NULL
    , cidr_block                    INET  NOT NULL
    , PRIMARY KEY (tool_run_id, security_group_id, egress, protocol
                  , from_port, to_port, cidr_block
                  )
    , FOREIGN KEY (tool_run_id, security_group_id)
        REFERENCES raw_aws_security_groups(tool_run_id, security_group_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
    , FOREIGN KEY (tool_run_id, cidr_block)
        REFERENCES raw_aws_cidr_blocks(tool_run_id, cidr_block)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);
CREATE TABLE raw_aws_security_group_rules_non_ip (
      tool_run_id                   UUID  NOT NULL
    , security_group_id             TEXT  NOT NULL
    , egress                        BOOL  NOT NULL
    , protocol                      TEXT  NOT NULL
    , from_port                     INT   NOT NULL
    , to_port                       INT   NOT NULL
    , target                        JSONB NOT NULL
    , PRIMARY KEY (tool_run_id, security_group_id, egress, protocol
                  , from_port, to_port, target
                  )
    , FOREIGN KEY (tool_run_id, security_group_id)
        REFERENCES raw_aws_security_groups(tool_run_id, security_group_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);


-- AWS Network ACLs
CREATE TABLE raw_aws_network_acls (
      tool_run_id                   UUID  NOT NULL
    , network_acl_id                TEXT  NOT NULL
    , PRIMARY KEY (tool_run_id, network_acl_id)
    , FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);
-- CREATE TABLE raw_aws_network_acl_details (
--       tool_run_id                   UUID  NOT NULL
--     , network_acl_id                TEXT  NOT NULL
--     , owner_id                      TEXT  NOT NULL
--     , PRIMARY KEY (tool_run_id, network_acl_id, owner_id)
--     , FOREIGN KEY (tool_run_id, network_acl_id)
--         REFERENCES raw_aws_network_acls(tool_run_id, network_acl_id)
--         ON DELETE CASCADE
--         ON UPDATE CASCADE
-- );
CREATE TABLE raw_aws_network_acl_rules (
      tool_run_id                   UUID  NOT NULL
    , network_acl_id                TEXT  NOT NULL
    , egress                        BOOL  NOT NULL
    , rule_number                   INT   NOT NULL
    , action                        TEXT  NOT NULL
    , protocol                      TEXT  NOT NULL
    , cidr_block                    INET  NOT NULL
    , PRIMARY KEY (tool_run_id, network_acl_id, egress, rule_number)
    , FOREIGN KEY (tool_run_id, network_acl_id)
        REFERENCES raw_aws_network_acls(tool_run_id, network_acl_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
    , FOREIGN KEY (tool_run_id, cidr_block)
        REFERENCES raw_aws_cidr_blocks(tool_run_id, cidr_block)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);
CREATE TABLE raw_aws_network_acl_rule_ports (
      tool_run_id                   UUID  NOT NULL
    , network_acl_id                TEXT  NOT NULL
    , egress                        BOOL  NOT NULL
    , rule_number                   INT   NOT NULL
    , from_port                     INT   NOT NULL
    , to_port                       INT   NOT NULL
    , PRIMARY KEY (tool_run_id, network_acl_id, rule_number, from_port
                  , to_port
                  )
    , FOREIGN KEY (tool_run_id, network_acl_id, egress, rule_number)
        REFERENCES raw_aws_network_acl_rules(
          tool_run_id, network_acl_id, egress, rule_number)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);
CREATE TABLE raw_aws_network_acl_rule_type_codes (
      tool_run_id                   UUID  NOT NULL
    , network_acl_id                TEXT  NOT NULL
    , egress                        BOOL  NOT NULL
    , rule_number                   INT   NOT NULL
    , type                          INT   NOT NULL
    , code                          INT   NOT NULL
    , PRIMARY KEY (tool_run_id, network_acl_id, rule_number, type, code)
    , FOREIGN KEY (tool_run_id, network_acl_id, egress, rule_number)
        REFERENCES raw_aws_network_acl_rules(
          tool_run_id, network_acl_id, egress, rule_number)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);


-- AWS Subnets
CREATE TABLE raw_aws_subnets (
      tool_run_id                   UUID  NOT NULL
    , subnet_id                     TEXT  NOT NULL
    , PRIMARY KEY (tool_run_id, subnet_id)
    , FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);
CREATE TABLE raw_aws_subnet_details (
      tool_run_id                   UUID  NOT NULL
    , subnet_id                     TEXT  NOT NULL
    , availability_zone             TEXT  NOT NULL
    , subnet_arn                    TEXT  NOT NULL
    , PRIMARY KEY (tool_run_id, subnet_id, availability_zone, subnet_arn)
    , FOREIGN KEY (tool_run_id, subnet_id)
        REFERENCES raw_aws_subnets(tool_run_id, subnet_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);
CREATE TABLE raw_aws_subnet_cidr_blocks (
      tool_run_id                   UUID  NOT NULL
    , subnet_id                     TEXT  NOT NULL
    , cidr_block                    CIDR  NOT NULL
    , PRIMARY KEY (tool_run_id, subnet_id, cidr_block)
    , FOREIGN KEY (tool_run_id, subnet_id)
        REFERENCES raw_aws_subnets(tool_run_id, subnet_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
    , FOREIGN KEY (tool_run_id, cidr_block)
        REFERENCES raw_aws_cidr_blocks(tool_run_id, cidr_block)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);


-- AWS Route Tables
CREATE TABLE raw_aws_route_tables (
      tool_run_id                   UUID  NOT NULL
    , route_table_id                TEXT  NOT NULL
    , PRIMARY KEY (tool_run_id, route_table_id)
    , FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);
CREATE TABLE raw_aws_route_table_associations (
      tool_run_id                   UUID  NOT NULL
    , route_table_id                TEXT  NOT NULL
    , association_id                TEXT  NOT NULL -- aka subnet_id
    , PRIMARY KEY (tool_run_id, route_table_id, association_id)
    , FOREIGN KEY (tool_run_id, route_table_id)
        REFERENCES raw_aws_route_tables(tool_run_id, route_table_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);
CREATE TABLE raw_aws_route_table_routes_cidr (
      tool_run_id                   UUID  NOT NULL
    , route_table_id                TEXT  NOT NULL
    , destination_id                TEXT  NOT NULL
    , state                         TEXT  NOT NULL
    , cidr_block                    CIDR  NOT NULL
    , PRIMARY KEY (tool_run_id, route_table_id, destination_id, state
                  , cidr_block
                  )
    , FOREIGN KEY (tool_run_id, route_table_id)
        REFERENCES raw_aws_route_tables(tool_run_id, route_table_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
    , FOREIGN KEY (tool_run_id, cidr_block)
        REFERENCES raw_aws_cidr_blocks(tool_run_id, cidr_block)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);
CREATE TABLE raw_aws_route_table_routes_non_cidr (
      tool_run_id                   UUID  NOT NULL
    , route_table_id                TEXT  NOT NULL
    , destination_id                TEXT  NOT NULL
    , state                         TEXT  NOT NULL
    , destination                   TEXT  NOT NULL
    , PRIMARY KEY (tool_run_id, route_table_id, destination_id, state
                  , destination
                  )
    , FOREIGN KEY (tool_run_id, route_table_id)
        REFERENCES raw_aws_route_tables(tool_run_id, route_table_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);


-- ----------------------------------------------------------------------
-- AWS cross component/service ties
-- ----------------------------------------------------------------------

-- 1 to many
CREATE TABLE raw_aws_instance_network_interfaces (
      tool_run_id                   UUID      NOT NULL
    , instance_id                   TEXT      NOT NULL
    , interface_id                  TEXT      NOT NULL
    , PRIMARY KEY (tool_run_id, instance_id, interface_id)
    , FOREIGN KEY (tool_run_id, instance_id)
        REFERENCES raw_aws_instances(tool_run_id, instance_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
    , FOREIGN KEY (tool_run_id, interface_id)
        REFERENCES raw_aws_network_interfaces(tool_run_id, interface_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- 1 to 1 to 1
CREATE TABLE raw_aws_network_interface_vpc_subnet (
      tool_run_id                   UUID  NOT NULL
    , interface_id                  TEXT  NOT NULL
    , vpc_id                        TEXT  NOT NULL
    , subnet_id                     TEXT  NOT NULL
    , PRIMARY KEY (tool_run_id, interface_id, subnet_id, vpc_id)
    , FOREIGN KEY (tool_run_id, interface_id)
        REFERENCES raw_aws_network_interfaces(tool_run_id, interface_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
    , FOREIGN KEY (tool_run_id, subnet_id)
        REFERENCES raw_aws_subnets(tool_run_id, subnet_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
    , FOREIGN KEY (tool_run_id, vpc_id)
        REFERENCES raw_aws_vpcs(tool_run_id, vpc_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- 1 to many
CREATE TABLE raw_aws_network_interface_security_groups (
      tool_run_id                   UUID      NOT NULL
    , interface_id                  TEXT      NOT NULL
    , security_group_id             TEXT      NOT NULL
    , PRIMARY KEY (tool_run_id, interface_id, security_group_id)
    , FOREIGN KEY (tool_run_id, interface_id)
        REFERENCES raw_aws_network_interfaces(tool_run_id, interface_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
    , FOREIGN KEY (tool_run_id, security_group_id)
        REFERENCES raw_aws_security_groups(tool_run_id, security_group_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- 1 to many
CREATE TABLE raw_aws_vpc_subnets (
      tool_run_id                   UUID  NOT NULL
    , vpc_id                        TEXT  NOT NULL
    , subnet_id                     TEXT  NOT NULL
    , PRIMARY KEY (tool_run_id, vpc_id, subnet_id)
    , FOREIGN KEY (tool_run_id, vpc_id)
        REFERENCES raw_aws_vpcs(tool_run_id, vpc_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
    , FOREIGN KEY (tool_run_id, subnet_id)
        REFERENCES raw_aws_subnets(tool_run_id, subnet_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- 1 to many
CREATE TABLE raw_aws_vpc_security_groups (
      tool_run_id                   UUID  NOT NULL
    , vpc_id                        TEXT  NOT NULL
    , security_group_id             TEXT  NOT NULL
    , PRIMARY KEY (tool_run_id, vpc_id, security_group_id)
    , FOREIGN KEY (tool_run_id, vpc_id)
        REFERENCES raw_aws_vpcs(tool_run_id, vpc_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
    , FOREIGN KEY (tool_run_id, security_group_id)
        REFERENCES raw_aws_security_groups(tool_run_id, security_group_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- 1 to many
CREATE TABLE raw_aws_vpc_network_acls (
      tool_run_id                   UUID  NOT NULL
    , vpc_id                        TEXT  NOT NULL
    , network_acl_id                TEXT  NOT NULL
    , PRIMARY KEY (tool_run_id, vpc_id, network_acl_id)
    , FOREIGN KEY (tool_run_id, vpc_id)
        REFERENCES raw_aws_vpcs(tool_run_id, vpc_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
    , FOREIGN KEY (tool_run_id, network_acl_id)
        REFERENCES raw_aws_network_acls(tool_run_id, network_acl_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- 1 to many
CREATE TABLE raw_aws_vpc_route_tables (
      tool_run_id                   UUID  NOT NULL
    , vpc_id                        TEXT  NOT NULL
    , route_table_id                TEXT  NOT NULL
    , PRIMARY KEY (tool_run_id, vpc_id, route_table_id)
    , FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- 1 to many
CREATE TABLE raw_aws_network_acl_subnets (
      tool_run_id                   UUID  NOT NULL
    , network_acl_id                TEXT  NOT NULL
    , subnet_id                     TEXT  NOT NULL
    , PRIMARY KEY (tool_run_id, network_acl_id, subnet_id)
    , FOREIGN KEY (tool_run_id, network_acl_id)
        REFERENCES raw_aws_network_acls(tool_run_id, network_acl_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
    , FOREIGN KEY (tool_run_id, subnet_id)
        REFERENCES raw_aws_subnets(tool_run_id, subnet_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);


-- ----------------------------------------------------------------------


COMMIT TRANSACTION;
