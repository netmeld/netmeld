DESCRIPTION
===========

Parse and import Cisco switch, router, and firewall configuration files
(the output of `show running-config all`).
This tool also does a reasonable job importing configuration files
from some non-Cisco equipment that use a Cisco-like configuration syntax.


EXAMPLES
========
``` 
nmdb-import-cisco --device-id device cisco.example 

cat cisco.example | nmdb-import-cisco --device --pipe copy.example
```
