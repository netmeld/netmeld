DESCRIPTION
===========

Parse and import the output from the `iptables-save -c`
command on modern Linux systems.

Since `nmdb-import-iptables-save` is importing information about a device's
firewall rules, the `--device-id` option is required.

EXAMPLES
========
``` 
nmdb-iptables-save --device-id linux iptables-save-c.example 
``` 

See Also: `iptables, iptables-save`
