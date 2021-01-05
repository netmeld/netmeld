DESCRIPTION
===========

This export tool is provided as a convenience to export scan results in a
predefined `ConTeXt` format, typically leveraged during assessment
reporting.

The command `nmdb-playbook-export-scans` currently has the following options:
+ `--intra-network` exports intra-network scan information
+ `--inter-network` exports inter-network scan information
+ `--nessus` exports Nessus scan information
+ `--ssh` exports SSH algorithm scan information
+ `--toFile` outputs to a specified file instead of stdout; this uses
  pre-defined values

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
