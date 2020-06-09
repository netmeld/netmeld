DESCRIPTION
===========

Parse and import the output from the `ipconfig /all /allcompartment` command on
modern Windows systems.

Since `nmdb-import-ipconfig` is importing information about a device's
routing tables, the `--device-id` option is required.

EXAMPLES
========
``` 
nmdb-import-ip-config --device-id workstation ipconfig.txt
 
ipconfig /all /allcompartments | nmdb-import-ip-addrconfig --pipe --device-id workstation copy.txt
```
