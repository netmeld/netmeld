DESCRIPTION
===========

Parse and import `tshark` JSON output (e.g., from `tshark -nV -T json`).

This tool can currently be used in two different modes.  The first is an import
of a `.pcap` file processed through tshark.  The other is a live capture via
tshark.  Both methods, currently, add data to the Netmeld data store after the
full parsing process has completed.  Future version will support processing
and storage of the data in real-time.

It is important to note that when piping data to this tool, it does not save
the piped data to disk (unlike other Netmeld tools).  If a live capture is
wanted in both `.pcap` format and processed, one will have to use capabilities
inherent in `tshark` (e.g., `tshark -nV -w file.pcapng -T json`).

Currently supported arguments:

EXAMPLES
========

Parse output from a pre-formatted file, named `data.json`.
``` 
nmdb-import-tshark data.json
```

Process a pcapng file with tshark and pipe the output to the tool.
```
tshark -nV -T json -r capture.pcapng | nmdb-import-tshark --pipe
```

Similiar to prior, except do it for multiple files and leverage GNU parallel
to parallelize (based on the number of cores) the processing.
```
find . -name '*.pcapng' | parallel 'tshark -nV -Tjson -r {} | nmdb-import-tshark --pipe'
```

Perform live capture, saving to `packet_capture` and send JSON output to the
tool.
```
tshark -nV -w packet_capture -T json | nmdb-import-tshark --pipe
```
