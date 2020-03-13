DESCRIPTION
===========

Manually insert MAC addresses and IP addresses into the Netmeld database.
The inserted information is not tied to any specific device.

Currently supported arguments:
* `--mac-addr` inserts a MAC address into the `raw_mac_addrs` table.
* `--ip-addr` inserts an IPv4 or IPv6 address into the `raw_ip_addrs` table.
* `--device-id`
* `--device-color`
* `--device-type`
* `--responding`

Note: If you specify both the `--mac-addr` and `--ip-addr` options, it will
also associate the MAC address and IP address in the `raw_mac_addrs_ip_addrs`
table.

EXAMPLES 
======== 
``` 
nmdb-inset-address --mac-addr 01:02:03:04:05:06 --ip-addr 192.168.1.2
```
