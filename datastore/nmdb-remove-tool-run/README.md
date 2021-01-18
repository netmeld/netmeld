DESCRIPTION
===========

This tool allows the removal of a problematic tool run (and all of the
associated data of that tool run) from the data store.

You must specify the tool run ID to be removed with the `--tool-run-id` option.
This tool defaults to the `site` data store unless you specify an alternate
Netmeld data store with the `--db-name` option.

This command removes the respective entry from the `tool_runs` table
and cascades to remove entries in other tables that are associated
with the removed tool run ID.  This command does not delete files from the
file system thus if the files are not removed and the tool run data directory
is imported at a time in the future, the problematic tool run will be added
back to the Netmeld data store.

EXAMPLES
========

Remove manually inserted data (if the reserved UUID is used during insertion).
```
nmdb-remove-tool-run --tool-run-id 32b2fd62-08ff-4d44-8da7-6fbd581a90c6
```
