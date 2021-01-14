DESCRIPTION
===========

Parse and import the output of a packet capture (such as from `dumpcap`).

Currently, data extraction is limited to VLAN tag, MAC address, and IPv4/6
address type information.  For enhanced capability, see the
`nmdb-import-tshark` tool.


EXAMPLES
========

Process the target data from a local file.
```
nmdb-import-pcap capture.pcap
```

Assuming `...` is some command chain which retrieves the target data from a
remote host and displays the results locally, then the following would process
it and save the data to a file called `capture.pcap` in the current working
directory.
```
... | nmdb-import-pcap --pipe copy.pcap
```

Find all `pcap` files in the current directory (recursively) and parallelize
the processing of the data with GNU parallel.
```
find . -name '*.pcap' | parallel 'nmdb-import-pcap {}'
```
