DESCRIPTION
===========

The `nmdl-list` tool is utilized to display the data stored in the data lake in
targeted ways.  The primary focus is to display "binned" data and as may be
needed, for an ingest script geared towards usage with the Netmeld Datastore
module tools.

This tool supports extracting information from the data lake from a
particular instance in time via the `--before` option.  This means any data
added after the passed value will not be reflected in the tools output.  This
value defaults to "infinity", which implies use the latest version of data.
Giving an invalid `--before` value will result in handler defined behavior.


EXAMPLES
========

Listing data as organized by the `device-id` provided when stored is the
default.
```
nmdl-list
```

List by any associated ingest tool.
```
nmdl-list --by-tool
```

List data not associated to an ingest tool.
```
nmdl-list --unbinned
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

Generate a script typical for an ingest into the Netmeld data-store.
```
nmdl-list --ingest-script
```

Generate an ingest script as it would have looked on or before
`January 25, 2001 at 18:30:54`.
```
nmdl-list --ingest-script --before '2001-01-25T18:30:54'
```
