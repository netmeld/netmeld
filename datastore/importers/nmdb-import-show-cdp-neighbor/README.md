DESCRIPTION
===========

Parse and import the output of a Cisco device's `show cdp neighbor detail`
command.

Since `nmdb-import-show-cdp-neighbor` is importing information about a device's
neighbors, the `--device-id` option is required.  However, the tool will also
extract the device-id for the neighboring devices.

EXAMPLE
=======
``` 
nmdb-import-show-mac-address-table --device-id cisco cdp_neighbors.txt
```
