DESCRIPTION
===========

The `nmdl-list` tool is utilized to display the data stored in the data lake in
targeted ways.  The primary focus is to display "binned" data and as may be
needed, for an ingest script geared towards usage with the Netmeld datastore.

This tool supports extracting information from the data lake from a
particular instance in time via the `--before` option.  This means any data
added after the passed value will not be reflected in the tools output.  This
value defaults to "infinity", which implies use the latest version of data.
Giving an invalid `--before` value will result in handler defined behaviour.


EXAMPLES
========

List data as organized by the `device-id` provided when stored is the default.
```
nmdl-list
```

List by any associated ingest tool.
```
nmdl-list --by-tool
```

List data on or before `January 25, 2001 at 18:30:54`.
```
nmdl-list --before '2001-01-25T18:30:54'
```

List data on or before `January 25, 2001 at 00:00:00`, so effectively anything
added before January 25, 2001.
```
nmdl-list --before '2001-01-25'
```

Generate a script typical of for an ingest into the Netmeld datastore.
```
nmdl-list --ingest-script
```
