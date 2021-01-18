DESCRIPTION
===========

Manually insert VLANs and IP networks into the Netmeld database.
The inserted information is not tied to any specific device.


EXAMPLES
========

Insert a VLAN (in the `raw_vlans` table).
```
nmdb-insert-network --vlan VLAN123
```

Insert an IP subnet (in the `raw_ip_nets` table).
```
nmdb-insert-network --ip-net '1.2.3.4/24'
```

Insert a VLAN, an IP subnet, and associate them together (in the
`raw_vlans_ip_nets` table).
```
nmdb-insert-network --vlan VLAN123 --ip-net '1.2.3.4/24'
```

Same as prior, except give a description for the VLAN and IP subnet
```
nmdb-insert-network --vlan VLAN123 --ip-net '1.2.3.4/24' --description 'Internal Network'
```
