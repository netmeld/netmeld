DESCRIPTION
===========

Parse and import the `/etc/hosts` file from Linux, BSD, and other UNIX-like
systems.

Note that this tool filters out and does not insert special addresses such as
localhost (`127.0.0.1` and `::1`), broadcast (`255.255.255.255`), and multicast
(`ff02::1` and `ff02::2`).


EXAMPLES
========

Process the target data from a local file.
```
nmdb-import-hosts --device-id workstation hosts.txt
```

Assuming `...` is some command chain which retrieves the target data from a
remote host and displays the results locally, then the following would process
it and save the data to a file called `hosts.txt` in the current working
directory.
```
... | nmdb-import-hosts --device-id workstation hosts.txt --pipe
```
