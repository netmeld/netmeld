DESCRIPTION
===========

Parse and import the output of a Cisco device's `show running-config all`
command.  Currently, this tool supports IOS, NXOS, and ASA type devices.

This tool has been observed to successfully work for other Cisco-like
configuration syntax from some non-Cisco equipment, however it is not tested
against or guaranteed.


EXAMPLES
========

Process the target data for the device `switch` from a local file.
```
nmdb-import-cisco --device-id switch configuration.txt
```

Assuming `...` is some command chain which retrieves the target data from a
remote host and displays the results locally, then the following would process
it and save the data to a file called `configuration.txt` in the current working
directory.
```
... | nmdb-import-cisco --device-id switch configuration.txt --pipe
```
