DESCRIPTION
===========

Parse and import Nmap's XML output.  You must run Nmap with either the `-oA` or
`-oX` output format options in order to produce XML output.

As the data file can contain information about multiple hosts, this tool will
not honor usage of the `--device-id` option.  However, the tool still allows
it to be passed, but ignored, to help facilitate automation.


EXAMPLES
========

Process the target data contained in the file `result.xml`.
```
nmdb-import-nmap result.xml
```

Assuming `...` is some command chain which retrieves the target data from a
remote host and displays the results locally, then the following would process
it and save the data to a file called `result.xml` in the current working
directory.
```
... | nmdb-import-nmap result.xml --pipe
```

Process the target data contained in the file `result.xml` and add some
pedigree information to certain stored data by passing in the IP of the host
which instigated the scan.
```
nmdb-import-nmap result.xml --scan-origin-ip "1.2.3.4/24"
```
