DESCRIPTION
===========

Generate a list of ports suitable for use with Nmap.
By default, the generated port list is based on the contents
of the configuration file `/usr/local/etc/netmeld/port-list.conf`
and the selected protocol options:
+ `--tcp`
+ `--tcp-all`
+ `--udp`
+ `--udp-all`
+ `--stcp`
+ `--stcp-all`

However, if you specify the `--from-db` (also `-d`) option, the generated port list
is based on the responding ports currently recorded in the database,
a short list of ports that we always want to check for,
and the selected protocol options.

EXAMPLES
========
Examples:

```
nmdb-export-port-list --sctp-all --from-db <db>
```


Examples of how to use this tool with Nmap:

```
clw nmap -sU -p `nmdb-export-port-list --udp` ...
clw nmap -sS -p `nmdb-export-port-list --tcp-all` ...
clw nmap -sS -sU -p `nmdb-export-port-list --from-db <db> --tcp --udp` ...
```
