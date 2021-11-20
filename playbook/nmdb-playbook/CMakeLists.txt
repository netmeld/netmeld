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

add_executable(${TGT_TOOL}
    CommandRunnerSingleton.cpp
    RaiiCommon.cpp
    RaiiIpAddr.cpp
    RaiiIpLink.cpp
    RaiiIpRoute.cpp
    RaiiMacAddr.cpp
    RaiiVlan.cpp
    ${TGT_TOOL}.cpp
  )

target_link_libraries(${TGT_TOOL}
    netmeld-datastore
    netmeld-playbook
    pthread
  )

nm_install_bin(${TGT_TOOL})
nm_install_conf(nmdb-playbook.conf "playbook")
nm_install_conf(plays.yaml "playbook")
nm_install_conf("sysctl.d/40-nmdb-playbook.conf" "sysctl.d")

install(
    CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \
          ${CMAKE_INSTALL_PREFIX}/${NETMELD_CONF_DIR}/sysctl.d/40-nmdb-playbook.conf \
          /etc/sysctl.d/40-nmdb-playbook.conf \
          )"
  )
