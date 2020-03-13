DESCRIPTION
===========

The `nmdb-playbook-insert-source` tool is utilized to configure the stages of
the playbook.  At a minimum you must specify the scan type, stage number,
interface name, and ip address.  Though not required, typically it is useful to
provide a description that contains the switch and port information being
utilized in the scan.


EXAMPLES
========

Assume that in stage 1, your computer will be connected to a trunk port
on a switch that provides access to VLANs 10, 30, and 50. You are planning on
running both intra-network scans from VLANS 10 and 50 and inter-network scans
from 30 and 50. Additionally, for VLAN 30 you need to spoof a MAC address
of 12:34:56:78:90:12.
```
nmdb-playbook-insert-source --intra-network --stage 1 --interface eth0 --vlan 10 --ip-addr 192.168.10.242/24
nmdb-playbook-insert-source --inter-network --stage 1 --interface eth0 --vlan 30 --ip-addr 192.168.30.242/24 --mac-addr 12:34:56:78:90:12
nmdb-playbook-insert-source --intra-network --inter-network --stage 1 --interface eth0 --vlan 50 --ip-addr 192.168.50.242/24
```
