DESCRIPTION
===========

Parse and import PaloAlto XML output.
Handles the XML output from all of the following command sequence:

```
set cli config-output-format xml
configure
show
```

Since `nmdb-import-paloalto-xml` is importing information about a PaloAlto
device's configuration, the `--device-id` and `--data-path` options are required.


EXAMPLES
======== 
``` 
nmdb-import-paloalto-xml --device-id firewall --data-path firewall_config.xml
```
