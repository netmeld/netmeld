DESCRIPTION
===========

Parse and import Juniper's Junos XML output.
Handles the XML output from all of the following commands:

* ``show configuration | display inheritance | display xml | no-more``
* ``show configuration | display xml | no-more``
* ``show configuration groups junos-defaults | display xml | no-more``
* ``show route logical-system all extensive | display xml | no-more``
* ``show route extensive | display xml | no-more``
* ``show ethernet-switching table extensive | display xml | no-more``
* ``show ethernet-switching ... | display xml | no-more``
* ``show arp no-resolve | display xml | no-more``
* ``show ipv6 neighbors | display xml | no-more``
* ``show lldp neighbors | display xml | no-more``

Since `nmdb-import-juniper` is importing information about a Juniper
device's configuration, the `--device-id` and `--data-path` options are required.


EXAMPLES
========
``` 
nmdb-import-juniper-xml --device-id firewall --data-path firewall_config.xml
```
