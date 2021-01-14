DESCRIPTION
===========

Parse and import the contextual information about a command that was captured
by the Netmeld tool `clw`.

In cases where the `clw` tool was used to wrap `nmap` or `ping`, this tool will
also call their respective import tools on the results as well.

As the data can contain information about multiple hosts, this tool will
not honor usage of the `--device-id` option.  However, the tool still allows
it to be passed, but ignored, to help facilitate automation.


EXAMPLES
========

Process the target data from a folder as stored by the `clw` tool.  For
simplicity, we use `toolname_timestamp_uuid` instead of an actual toolname,
timestamp, and UUID value.
```
nmdb-import-clw ~/.netmeld/clw/toolname_timestamp_uuid
```

Process and import the entire `clw` save directory.
```
for folder in `/bin/ls -1 ~/.netmeld/clw`; do
    nmdb-import-clw "$folder"
done
```
