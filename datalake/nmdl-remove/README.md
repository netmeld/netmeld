DESCRIPTION
===========

The `nmdl-remove` tool is utilized to remove data from the data lake.  This can
remove targeted data by providing the name as stored or data in mass if only a
`--device-id` is provided.  Additionally, this tool can be leveraged to
permanently remove data from the data lake which will remove the data and all
history of it if needed.

There is currently no ability, via Netmeld Datalake module tools, to undo a
removal.  However, the data lake backend may allow reversing of this except
in the case when `--permanent` is provided to the tool in which case the
backend shall remove the data without the possibility to undo.

EXAMPLES
========

Assuming the `nmdl-list` shows a data entry like `device001->hosts`, to remove
the `hosts` data from `device001`.
```
nmdl-remove --device-id device001 hosts
```

Permanently remove everything about `device001`.
```
nmdl-remove --device-id device001 --permanent
```
