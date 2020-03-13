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
``` 
nmdb-import-tshark --db-name site

nmdb-import-tshark --db-name site path.txt
nmdb-import-tshark --db-name site --data-path path.txt
```

