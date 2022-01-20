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
-- IP networks that are "in-scope" or "out-of-scope"
-- per the rules of engagement (RoE).
-- ----------------------------------------------------------------------

CREATE TABLE playbook_roe_ip_nets (
    ip_net                      CIDR            NOT NULL,
    in_scope                    BOOLEAN         NOT NULL,
    PRIMARY KEY (ip_net)
);

CREATE INDEX playbook_roe_ip_nets_idx_in_scope
ON playbook_roe_ip_nets(in_scope);


-- ----------------------------------------------------------------------
-- Locations in the target system from which to run tests.
-- ----------------------------------------------------------------------

CREATE TABLE playbook_intra_network_sources (
    playbook_source_id          UUID            NOT NULL,
    is_completed                BOOLEAN         NOT NULL,
    playbook_stage              INT             NOT NULL,
    interface_name              TEXT            NOT NULL,
    vlan                        INT             NOT NULL,
    mac_addr                    MACADDR         NULL,
    ip_addr                     INET            NOT NULL,
    description                 TEXT            NULL,
    PRIMARY KEY (playbook_source_id),
    UNIQUE (playbook_stage, interface_name, vlan, ip_addr),
    CHECK ((vlan BETWEEN 0 and 4095) or (vlan = 65535))
);


-- ----------------------------------------------------------------------

CREATE TABLE playbook_inter_network_sources (
    playbook_source_id          UUID            NOT NULL,
    is_completed                BOOLEAN         NOT NULL,
    playbook_stage              INT             NOT NULL,
    interface_name              TEXT            NOT NULL,
    vlan                        INT             NOT NULL,
    mac_addr                    MACADDR         NULL,
    ip_addr                     INET            NOT NULL,
    ptp_next_hop_ip_addr        INET            NULL,
    description                 TEXT            NULL,
    PRIMARY KEY (playbook_source_id),
    UNIQUE (playbook_stage, interface_name, vlan, ip_addr),
    CHECK ((vlan BETWEEN 0 and 4095) or (vlan = 65535))
);


-- ----------------------------------------------------------------------
-- Known and possible IP routers in the target system.
-- ----------------------------------------------------------------------

CREATE TABLE playbook_ip_routers (
    next_hop_ip_addr            INET            NOT NULL,
    PRIMARY KEY (next_hop_ip_addr),
    CHECK ((next_hop_ip_addr = host(next_hop_ip_addr)::INET))
);


-- ----------------------------------------------------------------------
-- Tracking for errors which occured during execution.
-- ----------------------------------------------------------------------

CREATE TABLE playbook_runtime_errors (
    playbook_source_id          UUID            NOT NULL,
    error_type                  TEXT            NOT NULL,
    PRIMARY KEY (playbook_source_id, error_type),
    FOREIGN KEY (playbook_source_id)
      REFERENCES playbook_intra_network_sources(playbook_source_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE,
    FOREIGN KEY (playbook_source_id)
      REFERENCES playbook_inter_network_sources(playbook_source_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);


-- ----------------------------------------------------------------------

COMMIT TRANSACTION;
