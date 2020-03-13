# The Import Tools

The `nmdb-import-*` tools each parse a specific data format
and insert the extracted data into the Netmeld database.

If an `nmdb-import-*` tool requires specifying the `--device-id` option,
that requirement will be noted in the tool's description below.

Several of the `nmdb-import-*` tools support a `--pipe` option
that saves a copy of STDIN to the specified input file.
The `--pipe` features is meant to support dynamically grabbing
and importing data from a live system.
For example, to ssh to a host, grab a copy of the output from
the `ip addr show` command, and import that data into the Netmeld database,
you could use the following command chain:

```
ssh user@host-01.example.com "ip addr show" \
  | nmdb-import-ip-addr-show --device-id host-01 --pipe host-01_ip-addr-show.txt
```
