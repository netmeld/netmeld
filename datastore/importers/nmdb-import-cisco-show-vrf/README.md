DESCRIPTION
===========

This program is designed to parse and import the output of the
Cisco device command `show vrf`. The command outputs various details
about Virtual Routing and Forwarding (VRF) instances on Cisco devices.

It is important to note that the show vrf command can yield multiple
formats based on the specific Cisco device and the version of IOS running on it.

EXAMPLES
========

Process the target data for the device `switch` from a local file.
```
nmdb-import-cisco-show-vrf --device-id switch vrfs.txt
```
