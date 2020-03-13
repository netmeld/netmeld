DESCRIPTION
===========

Manually insert VLANs and IP networks into the Netmeld database.
The inserted information is not tied to any specific device.

* Use the `--vlan` option to insert an 802.1Q VLAN
into the `raw_vlans` table.
* Use the `--ip-net` option to insert an IPv4 or IPv6 CIDR network
into the `raw_ip_nets` table.
* If you specify both the `--vlan` and `--ip-net` options,
it will associate the VLAN and IP network
in the `raw_vlans_ip_nets` table.
* Use the `--description` option to provide a description of the network.
* Use the `--low-graph-priority` option to ensure the network is out-of-the-way in network graphs (which should be used in management networks).



EXAMPLES 
======== 
``` 
nmdb-insert-network --vlan VLAN20-1 --ip-net 1.2.3.4/24
```
