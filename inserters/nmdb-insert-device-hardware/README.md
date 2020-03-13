DESCRIPTION
===========

Manually insert device hardware type information into the Netmeld database.

Since `nmdb-insert-device-hardware` is importing information about a device's
hardware, the `--device-id` option is required.

* Use the `--vendor` option to specify the name of device hardware vendor.
* Use the `--model` option to specify the model of device hardware.
* Use the `--hardware-revision` option to specify the hardware revision of
device hardware.
* Use the `--serial-number` option to specify the serial number of device
hardware.
* Use the `--description` option to specify a description of the device
hardware.

EXAMPLES 
======== 
``` 
nmdb-insert-device-hardware --device-id workstation --vendor Dell
```
