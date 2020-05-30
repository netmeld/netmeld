DESCRIPTION
===========

Parse and import the output from the `ip route show` command on modern Linux
systems.

Since `nmdb-import-ip-route-show` is importing information about a device's
routing tables, the `--device-id` option is required.

EXAMPLES
========
``` 
nmdb-import-ip-route-show --device-id workstation ip_route_show.txt 

ip route show | nmdb-import-ip-route-show --pipe --device-id workstation copy.txt 
``` 

See Also: `ip (8)`
