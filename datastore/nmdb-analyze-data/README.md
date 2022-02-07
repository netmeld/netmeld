Description
===========
This tool executes commands from a file which are intended to be ran in a
post-processing sense.  That is, usage of this tool will most usually occur
after data has been processed by other Netmeld tools (e.g., tools in the
Datalake, Datastore, or Playbook modules).

Command File Format
-------------------
The command file is YAML based.  An example document can be generated with
the `--example` option which contains a few examples to help with
understanding the format.  However, in the most simplistic sense, the
file is a set of commands and associated names for execution.

Examples
========
Generate an example of the command file format.
```
nmdb-analyze-data --example
```

Run the example generated from the tool.
```
nmdb-analyze-data --cmds-file <(nmdb-analyze-data --example)
```
