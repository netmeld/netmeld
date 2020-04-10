DESCRIPTION
===========

The `nmdl-remove` tool is utilized to remove data from the data lake.  This
can remove targeted data or data in mass if no specific data name is provided.
Additionally, this tool can be leveraged to permanently remove data from the
data lake which will remove the data an all history of it if needed.

There is currently no ability, via Netmeld datalake module tools, to undo a
removal.  However, the data lake backend may allow reversing of this except
in the case when `--permanent` is provided to the tool in which case the
backend shall remove the data without the possibility to undo.

EXAMPLES
========

Remove the `hosts` data from `device001`.
```
nmdl-remove --device-id device001 hosts
```

Permanently remove everything about `device001`.
```
nmdl-remove --device-id device001 --permanent
```
