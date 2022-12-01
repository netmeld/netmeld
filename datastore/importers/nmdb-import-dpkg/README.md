DESCRIPTION
===========

Parse and import the output of the `dpkg -l` command on modern Linux
systems into the Netmeld framework for later analysis. Import's package's
based on device-id.

The tool will not handle Unicode characters.
If the output contains a Unicode character anywhere, the character will
need to be converted to the ASCII equivalent, replace, or otherwise removed
prior to processing.
This could be accomplished manually, by using a tool (e.g., `uni2ascii`),
or similar.

EXAMPLES
========

Gather package output from target with
```
dpkg -l > packages.txt
```
or to strip unicode
```
dpkg -l | uni2ascii > packages.txt
```

Process the target data for the device workstation from a local file
```
nmdb-import-dpkg --device-id <workstation> packages.txt
```

Assuming `...` is some command chain which retrieves the target data from a
remote host and displays the results locally, then the following would
process it and save the data to a file called `packages.txt` in the current
working directory.
```
... | nmdb-import-dpkg --pipe --device-id <workstation> packages.txt
```
