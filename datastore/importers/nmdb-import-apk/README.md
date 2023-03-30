DESCRIPTION
===========

Parse and import the output of the `apk list -I -v` command on modern Linux
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
apk list -I -v > packages.txt
```
or to strip unicode
```
apk list -I -v | uni2ascii > packages.txt
```

Process the target data for the device workstation from a local file
```
nmdb-import-apk --device-id <workstation> packages.txt
```

Assuming `...` is some command chain which retrieves the target data from a
remote host and displays the results locally, then the following would
process it and save the data to a file called `packages.txt` in the current
working directory.
```
... | nmdb-import-apk --pipe --device-id <workstation> packages.txt
```
