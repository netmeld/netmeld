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

CREATE TABLE raw_aws_instances (
      tool_run_id                   UUID  NOT NULL
    , instance_id                   TEXT  NOT NULL
      PRIMARY KEY (tool_run_id, instance_id)
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
    , availability_zone             TEXT  NOT NULL
    , state_code                    INT   NOT NULL
    , state_name                    TEXT  NOT NULL
      PRIMARY KEY (tool_run_id, instance_id)
    , FOREIGN KEY (tool_run_id, instance_id)
        REFERENCES raw_aws_instances(tool_run_id, instance_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);
CREATE TABLE raw_aws_instance_network_interfaces (
      tool_run_id                   UUID      NOT NULL
    , instance_id                   TEXT      NOT NULL
    , interface_id                  TEXT      NOT NULL
      PRIMARY KEY (tool_run_id, instance_id)
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
    , subnet_id                     TEXT  NOT NULL
    , vpc_id                        TEXT  NOT NULL
      PRIMARY KEY (tool_run_id, instance_id)
    , FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);
CREATE TABLE raw_aws_network_interface_attachments (
      tool_run_id                   UUID  NOT NULL
    , interface_id                  TEXT  NOT NULL
    , id                            TEXT  NOT NULL
    , status                        TEXT  NOT NULL
    , delete_on_termination         BOOL  NOT NULL
      PRIMARY KEY (tool_run_id, instance_id)
    , FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);
CREATE TABLE raw_aws_network_interface_macs (
      tool_run_id                   UUID      NOT NULL
    , interface_id                  TEXT      NOT NULL
    , mac_address                   MACADDR   NOT NULL
      PRIMARY KEY (tool_run_id, instance_id)
    , FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);
CREATE TABLE raw_aws_network_interface_ips (
      tool_run_id                   UUID      NOT NULL
    , interface_id                  TEXT      NOT NULL
    , ip_address                    INET      NOT NULL
    , dns_name                      TEXT      NOT NULL
      PRIMARY KEY (tool_run_id, instance_id)
    , FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);
CREATE TABLE raw_aws_network_interface_security_groups (
      tool_run_id                   UUID      NOT NULL
    , interface_id                  TEXT      NOT NULL
    , security_group                TEXT      NOT NULL
      PRIMARY KEY (tool_run_id, instance_id)
    , FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);



CREATE TABLE raw_aws_vpcs (
      tool_run_id                   UUID  NOT NULL
    , vpc_id                        TEXT  NOT NULL
      PRIMARY KEY (tool_run_id, vpc_id)
    , FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);
CREATE TABLE raw_aws_vpc_details (
      tool_run_id                   UUID  NOT NULL
    , vpc_id                        TEXT  NOT NULL
    , state                         TEXT  NOT NULL
    , cidr_block                    CIDR  NOT NULL
    , block_state                   TEXT  NOT NULL
--    , owner_id                      TEXT  NOT NULL
      PRIMARY KEY (tool_run_id, vpc_id)
    , FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);



CREATE TABLE raw_aws_security_groups (
      tool_run_id                   UUID  NOT NULL
    , security_group_id             TEXT  NOT NULL
      PRIMARY KEY (tool_run_id, security_group_id)
    , FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);
CREATE TABLE raw_aws_security_group_detailss (
      tool_run_id                   UUID  NOT NULL
    , security_group_id             TEXT  NOT NULL
    , group_name                    TEXT  NOT NULL
    , description                   TEXT  NOT NULL
    , vpc_id                        TEXT  NOT NULL
      PRIMARY KEY (tool_run_id, security_group_id)
    , FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);
CREATE TABLE raw_aws_security_group_rules (
      tool_run_id                   UUID  NOT NULL
    , security_group_id             TEXT  NOT NULL
    , egress                        BOOL  NOT NULL
    , protocol                      TEXT  NOT NULL
    , from_port                     PORT  NOT NULL
    , to_port                       PORT  NOT NULL
    , cidr_ip                       INET  NOT NULL
    , description                   TEXT  NULL
      PRIMARY KEY (tool_run_id, security_group_id)
    , FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);



CREATE TABLE raw_aws_network_acls (
      tool_run_id                   UUID  NOT NULL
    , network_acl_id                TEXT  NOT NULL
    , vpc_id                        TEXT  NOT NULL
--    , owner_id                      TEXT  NOT NULL
      PRIMARY KEY (tool_run_id, security_group_id)
    , FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);
CREATE TABLE raw_aws_network_acl_rules (
      tool_run_id                   UUID  NOT NULL
    , network_acl_id                TEXT  NOT NULL
    , egress                        BOOL  NOT NULL
    , rule_number                   INT   NOT NULL
    , action                        TEXT  NOT NULL
    , protocol                      TEXT  NOT NULL
    , cidr_block                    INET  NOT NULL
      PRIMARY KEY (tool_run_id, security_group_id)
    , FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);
CREATE TABLE raw_aws_network_acl_associations (
      tool_run_id                   UUID  NOT NULL
    , network_acl_id                TEXT  NOT NULL
    , subnet_id                     TEXT  NOT NULL
      PRIMARY KEY (tool_run_id, security_group_id)
    , FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);



CREATE TABLE raw_aws_subnets (
      tool_run_id                   UUID  NOT NULL
    , subnet_id                     TEXT  NOT NULL
    , cidr_block                    CIDR  NOT NULL
    , vpc_id                        TEXT  NOT NULL
      PRIMARY KEY (tool_run_id, subnet_id)
    , FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);
CREATE TABLE raw_aws_subnet_details (
      tool_run_id                   UUID  NOT NULL
    , subnet_id                     TEXT  NOT NULL
    , availability_zone             TEXT  NOT NULL
      PRIMARY KEY (tool_run_id, subnet_id)
    , FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);



CREATE TABLE raw_aws_route_tables (
      tool_run_id                   UUID  NOT NULL
    , route_table_id                TEXT  NOT NULL
    , vpc_id                        TEXT  NOT NULL
      PRIMARY KEY (tool_run_id, route_table_id)
    , FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);
CREATE TABLE raw_aws_route_table_associations (
      tool_run_id                   UUID  NOT NULL
    , route_table_id                TEXT  NOT NULL
    , association_id                TEXT  NOT NULL
      PRIMARY KEY (tool_run_id, route_table_id)
    , FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);
CREATE TABLE raw_aws_route_table_routes (
      tool_run_id                   UUID  NOT NULL
    , route_table_id                TEXT  NOT NULL
    , destination_cidr_block        CIDR  NOT NULL
    , state                         TEXT  NOT NULL
    , destination_id                TEXT  NOT NULL
      PRIMARY KEY (tool_run_id, route_table_id)
    , FOREIGN KEY (tool_run_id)
        REFERENCES tool_runs(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- ----------------------------------------------------------------------

COMMIT TRANSACTION;
