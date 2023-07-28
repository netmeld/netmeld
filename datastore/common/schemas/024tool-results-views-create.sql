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

COMMIT TRANSACTION;
