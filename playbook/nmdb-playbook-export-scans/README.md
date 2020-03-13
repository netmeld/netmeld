DESCRIPTION
===========

This export tool is provided as a convenience to export predefined `ConTeXt`
formatted scan results typically leveraged during assessments.

The command `nmdb-playbook-export-scans` currently has the following options:
+ `--intra-network` exports intra-network scan information
+ `--inter-network` exports inter-network scan information
+ `--nessus` exports nessus scan information
+ `--ssh` exports SSH algorithm scan information
+ `--toFile` outputs to a specified file instead of stdout

EXAMPLES
========
```
nmdb-playbook-export-scans --intra-network --nessus
nmdb-playbook-export-scans --inter-network --ssh --toFile output-data.txt
```
