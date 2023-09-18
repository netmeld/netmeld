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

set(MODULE_VERSION "2.0")

target_as_library(common)

foreach(ITEM
    #general-tool-template
    nmdb-convert-acls
    nmdb-initialize
    nmdb-remove-tool-run
    nmdb-analyze-data
  )
  target_as_tool(${ITEM})
endforeach()

foreach(ITEM
    exporters
    graphers
    importers
    inserters
  )
  target_as_module(${ITEM})
endforeach()


# ----------------------------------------------------------------------
# Deb Creation
# ----------------------------------------------------------------------
set(desc "This package provides the Datastore module for the Netmeld \
tool suite.  Included are the library and headers necessary to run \
or expand capabilities through tool development, as well as the \
binaries necessary to instantiate and manage a data store compatible \
with the Netmeld tool suite.")
nm_add_deb_description(${desc})
set(deps "netmeld-core, \
libc6 (>= 2.4), libgcc-s1 (>= 3.0), libpqxx-6.4 (>= 6.4), libstdc++6 (>= 9), \
nlohmann-json3-dev (>= 3.9), \
libyaml-cpp0.7 | libyaml-cpp0.8")
nm_add_deb_depends(${deps})
set(deps "postgresql (>= 11), postgresql-client (>= 11)")
nm_add_deb_recommends(${deps})
set(deps "postgresql-contrib (>= 11), postgresql-autodoc (>= 11)")
nm_add_deb_suggests(${deps})
nm_add_deb_control_data(debian-control)
