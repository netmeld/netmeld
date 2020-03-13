#!/bin/bash --

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

# Clean DB
psql ${DB_NAME} -c "delete from playbook_roe_ip_nets";
psql ${DB_NAME} -c "delete from playbook_intra_network_sources";
psql ${DB_NAME} -c "delete from playbook_inter_network_sources";
psql ${DB_NAME} -c "delete from playbook_ip_routers";


###
# ROE
###
roeOutScope="\
  10.0.0.0/8 \
  fe80::/10 \
";
roeInScope="\
  192.168.100.0/24 \
  fd00:1:1:1::/119 \
";

function loadRoe()
{
  for ip in $roeOutScope; do
    psql \
      "${DB_NAME}" \
      -c "INSERT INTO playbook_roe_ip_nets VALUES ('${ip}', false)" \
      ;
  done;

  for ip in $roeInScope; do
    psql \
      "${DB_NAME}" \
      -c "INSERT INTO playbook_roe_ip_nets VALUES ('${ip}', true)" \
      ;
  done;
}


###
# Scan Stages
###
arr1=\
( 001 ens9 65535 192.168.100.240/24);
arr2=\
( 001 ens9 65535 fd00:1:1:1::1be/119);
arr3=\
( 002 ens9 1234 192.168.100.240/24);
arr4=\
( 003 ens9 1234 fd00:1:1:1::1be/119);
arr5=\
( 004 ens9 123 192.168.100.240/24 00:11:22:33:44:55);
arr6=\
( 004 ens9 123 fd00:1:1:1::1be/119 00:11:22:33:44:55);
srcList=(arr1 arr2 arr3 arr4 arr5 arr6);

function loadSources()
{
  declare -n src;
  for src in "${srcList[@]}"; do
    stage="${src[0]}";
    iface="${src[1]}";
    vlan="${src[2]}";
    ipAddr="${src[3]}";
    macAddr="${src[4]}";

    optionString="--stage $stage --interface $iface --ip-addr $ipAddr";
    if [[ "" != "${macAddr}" ]]; then
      optionString="${optionString} --mac-addr ${macAddr}";
    fi;
    if [[ "" != "${vlan}" ]]; then
      optionString="${optionString} --vlan ${vlan}";
    fi;

    # NOTE: Do not quote ${optionString} (space has to be interpreted as such)
    nmdb-playbook-insert-source \
      --db-name "${DB_NAME}" \
      --inter-network \
      --intra-network \
      ${optionString} \
    ;

  done;
}


###
# Inter-Network Routing
###
routes="\
  192.168.100.1 \
  fd00:1:1:1::1 \
";

function loadRouters()
{
  for ip in $routes; do
    nmdb-playbook-insert-router \
      --db-name "${DB_NAME}" \
      --ip-addr $ip \
      ;
  done;
}

loadRoe;
loadSources;
loadRouters;
