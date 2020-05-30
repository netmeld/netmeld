DESCRIPTION
===========

Parse and import Cisco ASA configuration files (the ouput of
`show running-config all`).
This tool is more targeted towards the ASA configuration and as such extracts
more information than the `nmdb-import-cisco` tool.  Particularly, it
will extract the access control type rules from the device.


EXAMPLES
========
``` 
nmdb-import-cisco-asa tool-data.txt 

command | nmdb-import-cisco-asa --pipe tool-data.txt
```
