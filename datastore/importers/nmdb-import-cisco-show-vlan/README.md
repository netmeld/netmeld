DESCRIPTION
===========

This program is designed to parse and import the output of the Cisco device command show vlan. The command provides details about VLAN configurations, including VLAN IDs, names, and interfaces on Cisco devices.

It is important to note that the show vlan command can yield multiple formats based on the specific Cisco device and the version of IOS running on it.

EXAMPLES
========
Process the target data for the device switch from a local file.

`nmdb-import-cisco-show-vlan --device-id switch vlans.txt`
