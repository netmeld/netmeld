# =============================================================================
# Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
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

import os
import psycopg
import tempfile

tgt_db='site'
tgt_ns='public'
tgt_schema='../common/schemas/'

print('Running for DB-NS: {0}-{1}'.format(tgt_db, tgt_ns))

print('(Re-)Initializing DB to just contain Netmeld Datastore schema')
os.system('/usr/bin/yes | /usr/local/bin/nmdb-initialize --db-name {0} --schema-dir {1}'.format(tgt_db, tgt_schema))


print('Generating dot file(s)')
src_root='{0}_tables'.format(tgt_db)
postgresql_autodoc='/usr/bin/postgresql_autodoc -d {0} -f {1} -t dot {2}'

# Generate from default template
print('Generating default graph (just tables)')
src_dot='{0}.dot'.format(src_root)
os.system(postgresql_autodoc.format(tgt_db, src_root, ''))


# Generate with modified template
print('Generating modified graph (tables+views)')
tgt_dot='{0}-and-views.dot'.format(src_root)
with tempfile.NamedTemporaryFile() as tmp:
  os.system(postgresql_autodoc.format(tgt_db, tmp.name, '-l .'))

  tmp_dot='{0}.dot'.format(tmp.name)
  with open(tmp_dot, 'r') as src:
    with open(tgt_dot, 'w') as dst:
      for line in src.readlines()[:-1]:
        dst.write(line)


with open(tgt_dot, 'a+') as dst:
  dst.write('\nedge\n[\n\t\tstyle = "dashed"\n];\n\n')

  dbconn_args='dbname={0}'.format(tgt_db)
  with psycopg.connect(dbconn_args) as conn:
    with conn.cursor() as cur:
      for table in conn.execute("""
        WITH RECURSIVE view_deps AS (
          SELECT DISTINCT
              dependent_ns.nspname as dependent_schema
            , dependent_view.relname as dependent_view
            , source_ns.nspname as source_schema
            , source_table.relname as source_table
          FROM pg_depend
          JOIN pg_rewrite
            ON pg_depend.objid = pg_rewrite.oid
          JOIN pg_class as dependent_view
            ON pg_rewrite.ev_class = dependent_view.oid
          JOIN pg_class as source_table
            ON pg_depend.refobjid = source_table.oid
          JOIN pg_namespace dependent_ns
            ON dependent_ns.oid = dependent_view.relnamespace
          JOIN pg_namespace source_ns
            ON source_ns.oid = source_table.relnamespace
          WHERE NOT (   dependent_ns.nspname    = source_ns.nspname
                    AND dependent_view.relname  = source_table.relname
                    )
          UNION
          SELECT DISTINCT
              dependent_ns.nspname as dependent_schema
            , dependent_view.relname as dependent_view
            , source_ns.nspname as source_schema
            , source_table.relname as source_table
          FROM pg_depend
          JOIN pg_rewrite
            ON pg_depend.objid = pg_rewrite.oid
          JOIN pg_class as dependent_view
            ON pg_rewrite.ev_class = dependent_view.oid
          JOIN pg_class as source_table
            ON pg_depend.refobjid = source_table.oid
          JOIN pg_namespace dependent_ns
            ON dependent_ns.oid = dependent_view.relnamespace
          JOIN pg_namespace source_ns
            ON source_ns.oid = source_table.relnamespace
          INNER JOIN view_deps vd
              ON vd.dependent_schema = source_ns.nspname
             AND vd.dependent_view = source_table.relname
             AND NOT (   dependent_ns.nspname   = vd.dependent_schema
                     AND dependent_view.relname = vd.dependent_view
                     )
        )

        SELECT
            dependent_view
          , source_table
        FROM view_deps
        WHERE source_schema = %s
        ORDER BY source_schema, source_table
      """, (tgt_ns,)):
        dst.write('"{0}":rtcol0 -> "{1}":ltcol0 ;\n'.format(table[0], table[1]))

      conn.rollback();

  dst.write('}\n')


# Generate Graphviz graphics
files=[src_dot, tgt_dot]
for file in files:
  exts=['pdf', 'png', 'svg']
  print('Generating graphics for {0}'.format(file))
  for ext in exts:
    print('- {0}'.format(ext))
    os.system('/usr/bin/dot -T{0} -o {1}{0} {2}'.format(ext, file.removesuffix('dot'), file))


exit(0)
