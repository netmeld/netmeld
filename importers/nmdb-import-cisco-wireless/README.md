DESCRIPTION
===========

Parse and import Cisco WiSM and WLC configuration files (the ouput of
`show run-config startup-commands`).
This tool is more targeted towards the Cisco wireless device configuration
and as such extracts more information than the `nmdb-import-cisco` tool.

EXAMPLES
======== 
``` 
nmdb-import-cisco-wireless tool-data.txt 

command | nmdb-import-cisco-wireless --pipe tool-data.txt
```
