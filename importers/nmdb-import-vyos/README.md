DESCRIPTION
===========

Parse and import VyOS configuration files (i.e., the output of
`show configuration`).

The tool has not had a chance to go through robust testing against multiple
VyOS configurations.  Configurations tested against may have special cases
as they all were from the same vendor.  In general, this behaves and processes
in the same manner as the Cisco, JunOS, etc. import tools.


EXAMPLES
======== 
``` 
nmdb-import-vyos --device-id router router-config.txt
``` 
