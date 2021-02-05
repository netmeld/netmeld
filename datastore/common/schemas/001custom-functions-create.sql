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
-- ----------------------------------------------------------------------
-- HASH_CHAIN()
--
-- This function provides consistent, comparable values for fields
-- which may be NULL.  Specifically, it is targeted for usage in
-- unique indexes to guard against duplicates in a table through
-- an expresional index when partial index combinations have
-- high complexity (>2 NULL fields).
-- ----------------------------------------------------------------------
CREATE OR REPLACE FUNCTION hash_chain(VARIADIC args TEXT[])
  RETURNS TEXT
  AS
  '
  DECLARE
    sub TEXT := ''~'';
    temp TEXT := '''';
    results TEXT;
  BEGIN
    FOR i IN 1 .. ARRAY_UPPER(args, 1) LOOP
      temp := MD5(temp || COALESCE(args[i], sub));
    END LOOP;
    SELECT temp INTO results;
    RETURN results;
  END;
  '
  LANGUAGE plpgsql
  CALLED ON NULL INPUT
  PARALLEL SAFE
  IMMUTABLE;


-- ----------------------------------------------------------------------

COMMIT TRANSACTION;
