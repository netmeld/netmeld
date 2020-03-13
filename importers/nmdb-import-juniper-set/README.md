DESCRIPTION
===========

Parse and import Juniper set formatted configuration files.  This is mainly
seen in their ScreenOS and NetScreen type devices.

Since `nmdb-import-juniper-set` is importing information about a Juniper
device's configuration, the `--device-id` option is required.

EXAMPLES
========
``` 
nmdb-import-juniper-set --device-id conf.txt 

cat conf1.txt | nmdb-import-juniper-set --pipe --device-id conf2.txt
```
