DESCRIPTION
===========

Parse and import the output from the `ip addr show`
command on modern Linux systems.

Since `nmdb-import-ip-addr-show` is importing information about a device's
network interfaces, the `--device-id` option is required.

EXAMPLES
========
``` 
nmdb-import-ip-addr-show --device-id workstation ip_addr_show.txt 

ip addr show | nmdb-import-ip-addr-show --pipe --device-id workstation copy.txt 
```

See Also: `ip (8)`
