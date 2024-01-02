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
-- Common Vulnerabilities and Exposures (CVE)
-- ----------------------------------------------------------------------
-- Historically, CVE identifiers all had the format "CVE-YYYY-NNNN".
-- However, due to an increasing number of reported vulnerabilities,
-- the "NNNN" part can now be longer than four digits when required
-- (such as "NNNNN" or "NNNNNN").
-- This means that CVE identifier strings are no longer guaranteed
-- to sort in the proper order ("CVE-2015-9999" vs "CVE-2015-10000").
--
-- To allow proper sorting of CVE identifiers,
-- and to simplify selecting CVEs in a specific year or range of years,
-- create a custom "CVE" aggregate type in the database.
-- ----------------------------------------------------------------------
CREATE TYPE CVE AS (
    year        SMALLINT
  , number      INT
);


-- ----------------------------------------------------------------------
-- Constrained-value types
-- ----------------------------------------------------------------------

-- TCP, UDP, and SCTP port numbers are unsigned 16-bit fields: 0-65535.
-- * -1: Range includes -1 for cases where a single valid numerical
--   value is not appropriate and expansion of the true value is
--   potentially costly.  For example, nmap scans can identify
--   state (e.g., "resets") for multiple ports, up to 65535, with
--   one entry.
CREATE DOMAIN PortNumber AS INT
CHECK (VALUE BETWEEN -1 AND 65535);


-- IEEE 802.1Q VLAN Identifier (VID) is an unsigned 12-bit field: 0-4095.
-- There are two reserved values:
-- * 0x000: Indicates that the frame does not carry a VLAN ID.
-- * 0xFFF: Reserved for implementation use; must not be configured or
--   transmitted. Can be used for wildcard matching or filtering.
-- The Netmeld database must be able to store all 4096 possible values.
CREATE DOMAIN VlanNumber AS SMALLINT
CHECK (VALUE BETWEEN 0 AND 4095);


-- ----------------------------------------------------------------------
-- Port Range type for use in Access Control Lists (ACLs).
-- Used to perform calculations with ranges like 0-65535 or 9000-9999.
-- ----------------------------------------------------------------------
CREATE TYPE PortRange AS RANGE (
    SUBTYPE = PortNumber
);


-- ----------------------------------------------------------------------
-- Network information about each hop on an IP route.
-- ----------------------------------------------------------------------
CREATE TYPE RouteHop AS (
    device_id                   TEXT
  , vrf_id                      TEXT
  , incoming_interface_name     TEXT
  , incoming_ip_addr            INET
  , incoming_ip_net             CIDR
  , outgoing_interface_name     TEXT
  , outgoing_ip_addr            INET
  , outgoing_ip_net             CIDR
);


-- ----------------------------------------------------------------------
-- Prowler severity enum to aid in sorting.
-- ----------------------------------------------------------------------
CREATE TYPE ProwlerSeverity AS ENUM (
    'critical'
  , 'high'
  , 'medium'
  , 'low'
  , 'informational'
);

-- ----------------------------------------------------------------------

COMMIT TRANSACTION;
