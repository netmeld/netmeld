DESCRIPTION
===========

Manually insert MAC addresses and IP addresses into the Netmeld database.
The inserted information is not tied to any specific device.


EXAMPLES
========

Insert a MAC address.
```
nmdb-insert-address --mac-addr 01:02:03:04:05:06
```

Insert an IP address with subnet (otherwise it defaults to a `/32`).
```
nmdb-insert-address --ip-addr 1.2.3.4/24
```

Insert a MAC address, IP address with subnet, and associate the two together.
```
nmdb-insert-address --mac-addr 01:02:03:04:05:06 --ip-addr 1.2.3.4/24
```

Same as prior, but set it to not be responding.
```
nmdb-insert-address --mac-addr 01:02:03:04:05:06 --ip-addr 1.2.3.4/24 --responding false
```
