DESCRIPTION
===========

This export tool is provided as a convenience to export scan results in a
predefined `ConTeXt` format, typically leveraged during assessment
reporting.

The command `nmdb-playbook-export-scans` currently supports creating reports
of the following type:
* Intra-network scan results
* Inter-network scan results
* SSH enabled algorithm results
* Nessus scan results


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
nmdb-playbook-export-scans --intra-network --nessus --toFile
```
