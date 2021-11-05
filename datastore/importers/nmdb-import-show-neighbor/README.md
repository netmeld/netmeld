DESCRIPTION
===========

Parse and import the output of a Cisco or Arista device's
`show arp` and `show ipv6 neighbors` command.


EXAMPLES
========

Process the target data for the device `switch` from a local file.
```
nmdb-import-show-neighbor --device-id switch show_arp.txt
nmdb-import-show-neighbor --device-id switch show_ipv6_neighbors.txt
```
