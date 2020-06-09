DESCRIPTION
===========

Parse and import Brocade configuration files (the output of `configshow -all`).

The `nmdb-import-brocade` tool automatically extracts the device-id from the
configuration file.  The tool is limited in capability at this time and only
extracts very targeted information.

EXAMPLES
========
```
nmdb-import-brocade tool-data.txt
nmdb-import-brocade --pipe tool-data.txt
```
