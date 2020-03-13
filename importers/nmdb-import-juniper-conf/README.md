DESCRIPTION
===========

Parse and import Juniper's JunOS configuration files.

Since `nmdb-import-juniper-junos` is importing information about a Juniper
device's configuration, the `--device-id` and `--data-path` options are required.


EXAMPLES
======== 
``` 
nmdb-import-juniper-conf --device-id conf.txt --data-path conf.txt

cat conf1.txt | nmdb-import-juniper-conf --pipe --device-id conf2.txt
```
