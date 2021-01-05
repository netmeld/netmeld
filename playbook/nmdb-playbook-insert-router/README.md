DESCRIPTION
===========

The `nmdb-playbook-insert-router` tool is utilized to configure the routes to
be used during a scan.  At a minimum it requires the ip address of the device
acting as a router.

It is important to realize these routes will be attempted to be leveraged for
all inter-network scanning activities, while they are defined, regardless of
the stage.  There are some validity checks made (in other playbook tooling) to
ensure a route can actually be used, so the cost of defining multiple possible
routes is minimized.


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
