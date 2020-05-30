DESCRIPTION
===========

Parse and import the `/etc/hosts` file
from Linux, BSD, and other UNIX-like systems.
IP addresses are inserted into the `raw_ip_addrs` table.
IP address and hostname pairs are inserted into the `raw_hostnames` table.

Note that `nmdb-import-hosts` filters out and does not insert
special addresses such as localhost (`127.0.0.1` and `::1`),
broadcast (`255.255.255.255`), and multicast (`ff02::1` and `ff02::2`).

EXAMPLES
========
``` 
nmdb-import-hosts /etc/hosts
cat /etc/hosts | nmdb-import-hosts --pipe ./copy
```

See Also: `hosts (1)`
