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
import subprocess
import tempfile
from string import Template
import re


###
# Datastore process flow graph related
###
def genDatastoreDotFiles():
  print('- Generating Datastore flow graph')
  ds_tmpl = 'datastore.dot.tmpl'
  ds_dot  = 'netmeld-datastore.dot'
  d = {}

  # get bins
  ds_bins_locs = ['', 'exporters', 'graphers', 'importers', 'inserters']
  for folder in ds_bins_locs:
    dict_list   = 'ds_bins_{0}_list'.format(folder)
    dict_count  = 'ds_bins_{0}_count'.format(folder)

    data_list   = getDsBinList(folder, '.cpp').rstrip()
    data_list   = addPorts(data_list)
    data_count  = len(data_list.splitlines())+1

    d[dict_list]  = data_list
    d[dict_count] = data_count

  # get libs
  ds_libs_locs = ['objects', 'parsers', 'tools', 'utils']
  for folder in ds_libs_locs:
    dict_list   = 'ds_libs_{0}_list'.format(folder)
    dict_count  = 'ds_libs_{0}_count'.format(folder)

    data_list   = getDsLibList(folder, '.hpp').rstrip()
    data_list   = addPorts(data_list)
    data_count  = len(data_list.splitlines())+1

    d[dict_list]  = data_list
    d[dict_count] = data_count

  with open(ds_tmpl, 'r') as src:
    with open(ds_dot, 'w') as dst:
      src_tmpl = Template(src.read())
      dst.write(src_tmpl.substitute(d))

  return [ds_dot]

def getDsBinList(path, ext):
  find_args = '../{0} -maxdepth 1 -iname "nmdb-*" -printf "%f\n"'.format(path)
  return getDsList(find_args, ext)

def getDsLibList(path, ext):
  find_args = '../common/{0} -iname "*{1}" -printf "%P\n"'.format(path, ext)
  return getDsList(find_args, ext)

def getDsList(find_args, sed_args):
  find_str  = '/usr/bin/find {0}'.format(find_args)
  sed1_str  = "/usr/bin/sed -e 's/{0}//g'".format(sed_args)
  sort_str  = '/usr/bin/sort -u'
  sed2_str  = '/usr/bin/sed -e \'s/\(.*\)/<tr><td align="left">\\1<\/td><\/tr>/g\''

  p1 = subprocess.run([find_str], shell=True, capture_output=True)
  p2 = subprocess.run([sed1_str], shell=True, capture_output=True, input=p1.stdout)
  p3 = subprocess.run([sort_str], shell=True, capture_output=True, input=p2.stdout)
  p4 = subprocess.run([sed2_str], shell=True, capture_output=True, input=p3.stdout)

  return p4.stdout.decode()

def addPorts(data):
  new_str=data
  sub_list=[
        '<tr><td align="left">nmdb-initialize</td></tr>'
      , '<tr><td align="left" port="init">nmdb-initialize</td></tr>'

      , '<tr><td align="left">nmdb-remove-tool-run</td></tr>'
      , '<tr><td align="left" port="remove">nmdb-remove-tool-run</td></tr>'

      , '<tr><td align="left">AbstractDatastoreTool</td></tr>'
      , '<tr><td align="left" port="dstool">AbstractDatastoreTool</td></tr>'
    ]

  for i in range(0, len(sub_list), 2):
    new_str = re.sub(sub_list[i], sub_list[i+1], new_str)

  return new_str


###
# Datastore schema graph related
###
def initDb(db_name, schema_path):
  print('- (Re-)Initializing DB to just contain Netmeld Datastore schema')

  init_str  = '/usr/bin/yes'
  init_str += ' | /usr/local/bin/nmdb-initialize'
  init_str += '   --db-name {0} --schema-dir {1}'.format(db_name, schema_path)

  ps = subprocess.run( [init_str], shell=True
                     , stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT
                     )

  return


def getSchemaDotFiles(db_name, db_namespace):
  dot_schema_root = '{0}_tables'.format(db_name)

  postgresql_autodoc  = '/usr/bin/postgresql_autodoc'
  postgresql_autodoc += ' -d {0} -f {1} -t dot'


  print('- Generating default graph (just tables)')
  dot_schema_t = '{0}.dot'.format(dot_schema_root)
  cmd_str      = postgresql_autodoc.format(db_name, dot_schema_root)
  p1           = subprocess.run( [cmd_str], shell=True
                               , stdout=subprocess.DEVNULL
                               , stderr=subprocess.STDOUT
                               )


  print('- Generating modified graph (tables+views)')
  dot_schema_tv='{0}-and-views.dot'.format(dot_schema_root)

  # get the schema tables and views
  # - this already generates table edges
  with tempfile.NamedTemporaryFile() as tmp:
    cmd_str   = postgresql_autodoc.format(db_name, tmp.name)
    cmd_str  += ' -l .'
    p1        = subprocess.run( [cmd_str], shell=True
                              , stdout=subprocess.DEVNULL
                              , stderr=subprocess.STDOUT
                              )

    tmp_dot='{0}.dot'.format(tmp.name)
    with open(tmp_dot, 'r') as src:
      with open(dot_schema_tv, 'w') as dst:
        for line in src.readlines()[:-1]: # write all but final/closing '}'
          dst.write(line)


  # add view edges
  edge_style  = '\nedge\n[\n\t\tstyle = "dashed"\n];\n\n'
  edge_str    = '"{0}":rtcol0 -> "{1}":ltcol0 ;\n'
  with open(dot_schema_tv, 'a+') as dst:
    dst.write(edge_style)

    dbconn_args='dbname={0}'.format(db_name)
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
          ORDER BY source_schema, dependent_view, source_table
        """, (db_namespace,)):
          dst.write(edge_str.format(table[0], table[1]))

        conn.rollback();

    dst.write('}\n')

  return [dot_schema_t, dot_schema_tv]


# Generate Graphviz graphics
def genGraphs(files):
  for file in files:
    exts=['pdf', 'png', 'svg']
    print('- Processing {0}'.format(file))
    for ext in exts:
      tgt_out = '{0}{1}'.format(file.removesuffix('dot'), ext)
      print('  - {0}'.format(tgt_out))
      cmd_str   = '/usr/bin/dot'
      cmd_str  += ' -T{0} -o {1} {2}'.format(ext, tgt_out, file)
      p1        = subprocess.run([cmd_str], shell=True)

  return


###
# Start of script logic
###
def main():
  db_name       = 'site'
  db_namespace  = 'public'
  schema_path   = '../common/schemas/'
  graphs        = []

  print('Running for DB-NS: {0}-{1}'.format(db_name, db_namespace))
  print('Generating dot file(s)')
  # Generate Datastore flow graph
  graphs.extend(genDatastoreDotFiles())
  # Generate DB schema graph(s)
  initDb(db_name, schema_path)
  graphs.extend(getSchemaDotFiles(db_name, db_namespace))
  print('Generating graph graphics in various formats')
  genGraphs(graphs)


main()
exit(0)
