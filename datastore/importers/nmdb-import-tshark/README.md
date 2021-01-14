DESCRIPTION
===========

Parse and import `tshark` JSON output (e.g., from `tshark -nV -T json`).

This tool can currently be used in two different modes, however both require
the tshark tool to output to JSON format.  The first is an import of a packet
capture and the other is from a live capture.  Data processing happens in a
real-time manner in that as a complete packet is seen by the tool, it will
process and store the data to the data store.  While this can minimize on disk
storage needs, it can increase compute and memory needs in certain scenarios.

It is important to note that when piping data to this tool, it does not save
the piped data to disk (unlike other Datastore tools).  If a live capture is
wanted in both packet capture format and processed, one will have to use
capabilities inherent in `tshark` (e.g., `tshark -nV -w file.pcapng -T json`).


EXAMPLES
========

Parse the target data from a local file.  This file is already in the JSON
format produced by `tshark`.
```
nmdb-import-tshark capture.json
```

Process a packet capture with `tshark` and pipe the output to the tool for
processing.
```
tshark -nV -T json -r capture.pcapng | nmdb-import-tshark --pipe
```

Find all `pcapng` files in the current directory (recursively) and parallelize
the processing of the data with GNU parallel.
```
find . -name '*.pcapng' | parallel 'tshark -nV -T json -r {} | nmdb-import-tshark --pipe'
```

Perform live capture, saving to `packet_capture` and send JSON output to the
tool.
```
tshark -nV -w packet_capture -T json | nmdb-import-tshark --pipe
```
