DESCRIPTION
===========

The `nmdb-playbook-insert-source` tool is utilized to insert a configuration
to be used during a stage of the playbook execution.  Thus, at a minimum one
must specify the scan type, stage number, interface name, and ip address.
Though not required, it is typically useful to provide a description that
contains the switch and port information being utilized in the scan when known.


EXAMPLES
========

Assume that in stage 1:
* Our computer will be connected to a trunk port on a switch that provides
  access to VLANs 10, 30, and 50.
  * The IP our computer is assigned/assumes per VLAN is 192.168.ID.242, where
    ID is the VLAN ID.
  * The IP subnets are all of size /24.
* We are planning on running both intra-network scans from VLANS 10 and 50 and
  inter-network scans from 30 and 50.
* VLAN 30 requires us to spoof a MAC address of 12:34:56:78:90:12.

Then the following three commands would achieve the desired configuration for
stage 1.
```
nmdb-playbook-insert-source --intra-network --stage 1 --interface eth0 --vlan 10 --ip-addr 192.168.10.242/24
nmdb-playbook-insert-source --inter-network --stage 1 --interface eth0 --vlan 30 --ip-addr 192.168.30.242/24 --mac-addr 12:34:56:78:90:12
nmdb-playbook-insert-source --intra-network --inter-network --stage 1 --interface eth0 --vlan 50 --ip-addr 192.168.50.242/24
```

Note that while the first two commands would not have an alternate form, the
third command can be split in two and achieve the same effect.  It is presented
to show that if a configuration is the same from both the intra-network and
inter-network perspective, then it can be inserted in a single command as well.
