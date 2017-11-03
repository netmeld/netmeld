# =============================================================================
# Copyright 2017 National Technology & Engineering Solutions of Sandia, LLC
# (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
# Government retains certain rights in this software.
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
# =============================================================================
# Maintained by Sandia National Laboratories <Netmeld@sandia.gov>
# =============================================================================

DB_NAME=site

all:


custom_types_create:
	psql ${DB_NAME} < 000custom-types-create.sql


tables_create:
	psql ${DB_NAME} < 001mac-prefixes-tables-create.sql
	psql ${DB_NAME} < 002tool-runs-tables-create.sql
	psql ${DB_NAME} < 003tool-results-tables-create.sql
	psql ${DB_NAME} < 004device-tables-create.sql

views_create:
	psql ${DB_NAME} < 005tool-runs-views-create.sql
	psql ${DB_NAME} < 006tool-results-views-create.sql
	psql ${DB_NAME} < 007device-views-create.sql


schema_diagram:
	postgresql_autodoc -d ${DB_NAME} -t dot
	dot -Granksep=3.0 -Gnodesep=0.5 -Tpng -o ${DB_NAME}.png ${DB_NAME}.dot


install: custom_types_create tables_create views_create schema_diagram
	./mac-prefixes-load -d ${DB_NAME}

