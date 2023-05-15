DESCRIPTION
===========

Parse and import the output of the
`systeminfo`
command on Windows
systems into the Netmeld framework for later analysis. Import's output
based on device-id.

EXAMPLES
========

Gather package output from target with
```
systeminfo > systeminfoOutput.txt
```

Process the target data for the device workstation from a local file
```
nmdb-import-systeminfo --device-id <workstation> systeminfoOutput.txt
```

Assuming `...` is some command chain which retrieves the target data from a
remote host and displays the results locally, then the following would
process it and save the data to a file called `systeminfoOutput.txt` in the current
working directory.
```
... | nmdb-import-systeminfo --pipe --device-id <workstation> packages.txt
```
