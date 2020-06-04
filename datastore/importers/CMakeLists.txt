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
set(MODULE_VERSION "2.0")

foreach(ITEM
    #import-tool-template
    nmdb-import-brocade
    nmdb-import-cisco
    nmdb-import-cisco-wireless
    nmdb-import-clw
    nmdb-import-hosts
    nmdb-import-ip-addr-show
    nmdb-import-ipconfig
    nmdb-import-ip-route-show
    nmdb-import-iptables-save
    nmdb-import-juniper-conf
    nmdb-import-juniper-set
    nmdb-import-nessus
    nmdb-import-nmap
    nmdb-import-pcap
    nmdb-import-ping
    nmdb-import-powerconnect
    nmdb-import-show-cdp-neighbor
    nmdb-import-show-inventory
    nmdb-import-show-mac-address-table
    nmdb-import-tshark
    nmdb-import-vyos
  )
  target_as_tool(${ITEM})
endforeach()

# ----------------------------------------------------------------------
# Deb Creation
# ----------------------------------------------------------------------
set(desc "This package provides the Datastore Importers module for the \
Netmeld tool suite.  Included are the library and headers necessary to \
run or expand capabilities through tool development, as well as the \
binaries necessary to import structured data into the Netmeld Datastore.")
nm_add_deb_description(${desc})
set(deps "netmeld-core, netmeld-datastore, \
libboost-iostreams1.67.0, libboost-system1.67.0, \
libpcap0.8 (>= 0.9.8), libpugixml1v5 (>= 1.7)")
nm_add_deb_depends(${deps})
