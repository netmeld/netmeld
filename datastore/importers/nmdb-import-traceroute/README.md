DESCRIPTION
===========

Parses and imports the output from the `traceroute [-n]` command on Linux
systems and the `tracert [-d]` command on Windows systems.

EXAMPLES
========

Parse the contents of a traceroute from a local text file.
```
nmdb-import-traceroute --device-id server tool-data.txt
```



Assuming `...` is some command chain which retrieves the target data from a
remote host and displays the results locally, then the following would process
it and save the data to a file called `traceroute.txt` in the current working
directory.
```
... | nmdb-import-traceroute --pipe --device-id server traceroute.txt
```
