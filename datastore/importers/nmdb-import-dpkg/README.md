DESCRIPTION
===========

Parse and import the output of the `dpkg -l` command on modern Linux systems.

TROUBLESHOOT
============

Parser is not designed to parse Unicode characters. If a package contains unicode characters in it's description, you will have to convert your `dpkg -l` output to ascii using a tool like `uni2ascii` which can be installed with `apt`.

EXAMPLES
========
Process the target data for the device workstation from a local file
```
nmdb-import-dpkg --device-id <workstation> packageout.txt
```
Assuming `...` is some command chain which retrieves the target data from a remote host and displays the results locally, then the following would process it and save the data to a file called packageout.txt in the current working directory.
```
... |
nmdb-import-dpkg --pipe --device-id <workstation> packageout.txt
```
