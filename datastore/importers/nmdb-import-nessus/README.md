DESCRIPTION
===========

Parse and import Nessus' XML output (`.nessus` files).  Specifically, the file
needs to conform to Nessus' `NessusClientData_v2` format.

As the data file can contain information about multiple hosts, this tool will
not honor usage of the `--device-id` option.  However, the tool still allows
it to be passed, but ignored, to help facilitate automation.

Unlike most of the other import tools, this tool will attempt to use the
timestamps contained in the target data for tool execution time information
instead of using ones it generates.


EXAMPLES
========

Process the target data contained in the file `result.nessus`.
```
nmdb-import-nessus result.nessus
```

Assuming `...` is some command chain which retrieves the target data from a
remote host and displays the results locally, then the following would process
it and save the data to a file called `result.nessus` in the current working
directory.
```
... | nmdb-import-nessus result.nessus --pipe
```
