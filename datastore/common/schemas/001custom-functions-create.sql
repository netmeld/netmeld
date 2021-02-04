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
-- SUB_IF_NULL(PARAM1)
-- * PARAM1: TEXT | FLOAT | PORTNUMBER | INET
-- * RETURNS: TEXT
--
-- These function provide consistent, comparable values for fields
-- which may be NULL.  Specifically, these functions are targeted
-- for usage in unique indexes to guard against duplicates in a table
-- through an expresional index when partial index combinations have
-- high complexity (>2 NULL fields).
-- ----------------------------------------------------------------------
CREATE OR REPLACE FUNCTION sub_if_null(TEXT)
  RETURNS TEXT
  AS
  '
  DECLARE
    sub TEXT := ''~'';
    temp TEXT;
    results TEXT;
  BEGIN
    SELECT COALESCE($1, sub) INTO temp;
    SELECT temp || MD5(temp) INTO results;
    RETURN results;
  END;
  '
  LANGUAGE plpgsql
  CALLED ON NULL INPUT
  PARALLEL SAFE
  IMMUTABLE;

-- ----------------------------------------------------------------------

CREATE OR REPLACE FUNCTION sub_if_null(FLOAT)
  RETURNS TEXT
  AS
  '
  DECLARE
    sub FLOAT := ''-1.0'';
    temp FLOAT;
    results TEXT;
  BEGIN
    SELECT COALESCE($1, sub) INTO temp;
    SELECT temp::TEXT || MD5(temp::TEXT) INTO results;
    RETURN results;
  END;
  '
  LANGUAGE plpgsql
  CALLED ON NULL INPUT
  PARALLEL SAFE
  IMMUTABLE;

-- ----------------------------------------------------------------------

CREATE OR REPLACE FUNCTION sub_if_null(PORTNUMBER)
  RETURNS TEXT
  AS
  '
  DECLARE
    sub PORTNUMBER := ''65536'';
    temp PORTNUMBER;
    results TEXT;
  BEGIN
    SELECT COALESCE($1, sub) INTO temp;
    SELECT temp::TEXT || MD5(temp::TEXT) INTO results;
    RETURN results;
  END;
  '
  LANGUAGE plpgsql
  CALLED ON NULL INPUT
  PARALLEL SAFE
  IMMUTABLE;

-- ----------------------------------------------------------------------

CREATE OR REPLACE FUNCTION sub_if_null(INET)
  RETURNS TEXT
  AS
  '
  DECLARE
    sub INET := ''0.0.0.0/0'';
    temp INET;
    results TEXT;
  BEGIN
    SELECT COALESCE($1, sub) INTO temp;
    SELECT temp::TEXT || MD5(temp::TEXT) INTO results;
    RETURN results;
  END;
  '
  LANGUAGE plpgsql
  CALLED ON NULL INPUT
  PARALLEL SAFE
  IMMUTABLE;


-- ----------------------------------------------------------------------

COMMIT TRANSACTION;
