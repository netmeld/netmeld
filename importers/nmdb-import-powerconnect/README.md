DESCRIPTION
===========

Parse and import PowerConnect configuration files (the output of
`show running-config`).

The `nmdb-import-powerconnect` tool automatically extracts the device-id from
the configuration file.  The tool currently is limited and extracts very
targeted information.  It will at least parse, hostname, interface data, and
routes.
