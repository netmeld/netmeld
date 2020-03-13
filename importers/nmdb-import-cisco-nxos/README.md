DESCRIPTION
===========

Parse and import Cisco Nexus configuration files (the ouput of
`show running-config`).
This tool is more targeted Nexus class devices and as such extracts more
information than the `nmdb-import-cisco` tool.  It can also be ran against
non-Nexus class configs to attempt to extract more information.

EXAMPLE
=======
``` 
nmdb-import-cisco-nxos tool-data.txt 
command | nmdb-import-cisco-nxos --pipe tool-data.txt
```
