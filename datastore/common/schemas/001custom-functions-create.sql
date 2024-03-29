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
-- ----------------------------------------------------------------------
-- HASH_CHAIN()
--
-- This function provides consistent, comparable values for fields
-- which may be NULL.  Specifically, it is targeted for usage in
-- unique indexes to guard against duplicates in a table through
-- an expressional index when partial index combinations have
-- high complexity (>2 NULL fields).
-- ----------------------------------------------------------------------
CREATE OR REPLACE FUNCTION hash_chain(
    VARIADIC args TEXT[]
)
RETURNS TEXT
AS $$
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
$$
LANGUAGE plpgsql
CALLED ON NULL INPUT
PARALLEL SAFE
IMMUTABLE
;


-- ----------------------------------------------------------------------
-- ----------------------------------------------------------------------
-- AS_TEXT(ProwlerSeverity)
--
-- This function provides consistent, comparable textual values for
-- the enumeration values defined in the custom type ProwlerSeverity.
-- ----------------------------------------------------------------------

CREATE OR REPLACE FUNCTION as_text(
    severity    ProwlerSeverity
)
RETURNS TEXT
AS $$
DECLARE
  val TEXT;
BEGIN
  CASE severity
  WHEN 'critical'       THEN val='critical';
  WHEN 'high'           THEN val='high';
  WHEN 'medium'         THEN val='medium';
  WHEN 'low'            THEN val='low';
  WHEN 'informational'  THEN val='informational';
  ELSE                       val='undefined';
  END CASE;
  RETURN val;
END;
$$
LANGUAGE plpgsql
CALLED ON NULL INPUT
PARALLEL SAFE
IMMUTABLE
;


-- ----------------------------------------------------------------------
-- ----------------------------------------------------------------------
-- AS_TEXT(TIMESTAMP)
--
-- This function provides consistent, comparable textual values for
-- a timestamp value defined in TIMESTAMP.  Generally speaking, the
-- value is standardized on UTC, converted to text, and returned.
-- ----------------------------------------------------------------------

CREATE OR REPLACE FUNCTION as_text(
    ts        TIMESTAMP
)
RETURNS TEXT
AS $$
DECLARE
  val TEXT;
BEGIN
  SELECT timezone('UTC', ts)::TEXT INTO val;
  RETURN val;
END;
$$
LANGUAGE plpgsql
PARALLEL SAFE
IMMUTABLE
;


-- ----------------------------------------------------------------------

COMMIT TRANSACTION;
