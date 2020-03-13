DESCRIPTION
===========

Parse and import the output of the `show mac address-table` command on a Cisco
device.  Note that `nmdb-import-show-mac-address-table` filters out and does
not insert any line which does not start with a numeric VLAN identifier
(excluding white space).

Since `nmdb-import-show-mac-address-table` is importing information about a
device's CAM table, the `--device-id` option is required.

EXAMPLE
=======
``` 
nmdb-import-show-mac-address-table --device-id cisco mac_address_table.txt 
```
