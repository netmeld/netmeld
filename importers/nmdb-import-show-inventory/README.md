DESCRIPTION
===========

Parse and import the output of a Cisco device's `show inventory` command.

Since `nmdb-import-show-inventory` is importing information about a device's
hardware configuration, the `--device-id` option is required.

EXAMPLE
======= 
``` 
nmdb-import-show-inventory --device-id cisco inventory.txt 
```
