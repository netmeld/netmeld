DESCRIPTION
===========

This tool is provided as a convenience to export common scan results
in formats, typically leveraged during assessment reporting.
The tool will generate, by default, the output to STDOUT and place a
header between data blobs (if multiple).  However, the tool also accepts
the `--to-file` flag which will force it to put the data into file(s)
with pre-defined names.
In the case no data resides in the data store for the selected type, the
tool will generate an example (no actual values) template instead.

The tool currently supports creating reports of the following type:
* Intra-network scan results
* Inter-network scan results
* SSH enabled algorithm results
* Nessus scan results

The tool currently supports creating reports in the following formats:
* `ConTeXt`
* CSV

EXAMPLES
========

Generate `ConTeXt` page information to STDOUT for any intra-network scan
information contained in the data store.
```
nmdb-playbook-export-scans --intra-network
```

Generate `ConTeXt` page information to STDOUT for any intra-network and Nessus
scan information contained in the data store.
```
nmdb-playbook-export-scans --intra-network --nessus
```

Generate `ConTeXt` page information to files for any intra-network and Nessus
scan information contained in the data store (pre-defined file names).
```
nmdb-playbook-export-scans --intra-network --nessus --to-file
```

Generate the same as prior, but to CSV format.
```
nmdb-playbook-export-scans --intra-network --nessus --to-file --out-format csv
```
