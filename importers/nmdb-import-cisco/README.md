DESCRIPTION
===========

Parse and import Cisco switch, router, and firewall configuration files
(the output of `show running-config`).
This tool also does a reasonable job importing configuration files
from some non-Cisco equipment that uses a Cisco-like configuration syntax.

The `nmdb-import-cisco` automatically extracts the device-id
from the configuration file.


EXAMPLES
========
``` 
nmdb-import-cisco cisco.example 

cat cisco.example | nmdb-import-cisco --pipe copy.example
```
