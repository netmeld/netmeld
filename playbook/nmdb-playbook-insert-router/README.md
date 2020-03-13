DESCRIPTION
===========

The `nmdb-playbook-insert-router` tool is utilized to configure the routes to
be used during a scan. At a minimum it requires the ip address.


EXAMPLES
========

For the inter-network scans, assume that 192.168.10.1, 192.168.30.1, and
192.168.50.1 are the IP addresses of the routers in VLANs 10, 30, and 50
respectively.
```
nmdb-playbook-insert-router --ip-addr 192.168.10.1
nmdb-playbook-insert-router --ip-addr 192.168.30.1
nmdb-playbook-insert-router --ip-addr 192.168.50.1
```
