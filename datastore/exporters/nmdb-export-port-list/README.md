DESCRIPTION
===========

Generate a list of ports suitable for use with Nmap.
By default, the generated port list is based on the contents of the
configuration file `/usr/local/etc/netmeld/port-list.conf` and the selected
protocol options are one of:
* TCP
* UDP
* STCP

However, if you specify the `--from-db` (also `-d`) option, the generated port
list is based on the responding ports currently recorded in the database, a
short list of ports that we always want to check for, and the selected protocol
options.

The options of form `*-all` will always produce the full possible range of
ports.

EXAMPLES
========

Generate a list of select TCP ports.
```
nmdb-export-port-list --tcp
```

Generate a list of all TCP ports.
```
nmdb-export-port-list --tcp-all
```

Generate a list of select TCP ports based on what is already known in the
data store.
```
nmdb-export-port-list --tcp --from-db
```


Perform a scan with Nmap and use this tool to provide the UDP ports to scan.
```
nmap -sU -p `nmdb-export-port-list --udp` localhost
```

Perform a scan with Nmap and use this tool to provide both the TCP and UDP
ports to scan.  For TCP ports, it will provide/scan all and for UDP ports
it will be a smaller, targeted list.
```
nmap -sS -sU -p `nmdb-export-port-list --from-db --tcp-all --udp` localhost
```
