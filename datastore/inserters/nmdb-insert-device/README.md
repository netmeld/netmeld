DESCRIPTION
===========

Manually insert device information into the Netmeld database.

Since `nmdb-insert-device` is importing information about a device's
configuration, the `--device-id` option is required.

* Use the `--device-id` option to specify the ID for the device.
* Use the `--vm-host-device-id` option to specify that the device
is a virtual machine running on the specified host device.
* Use the `--device-color` option to specify the device's color
in network graphs.
* Use the `--interface` option to specify the network interface name
to with the following address options apply:
* Use the `--mac-addr` option to insert a MAC address
into the `raw_mac_addrs` and `raw_device_mac_addrs` tables.
* Use the `--ip-addr` option to insert an IPv4 or IPv6 address
into the `raw_ip_addrs` and `raw_device_ip_addrs` tables.
* If you specify both the `--mac-addr` and `--ip-addr` options,
it will also associate the MAC address and IP address
in the `raw_mac_addrs_ip_addrs` table.
* The `--mediaType` option will specify the interface media type.
* Use the `--responding` option to flag a device as responding or not.
* If the `--low-graph-priority` flag is set, the device will be out-of-the-way in network graphs.


EXAMPLES 
======== 
``` 
nmdb-insert-device --device-id workstation --interface eth0 \
    --ip-addr 192.168.1.2 --mac-addr 01:02:03:04:05:06
```
