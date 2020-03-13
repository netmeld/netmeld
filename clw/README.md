DESCRIPTION
===========

One of the challenges during an assessment is ensuring that results
of commands are captured and that sufficient context about how and where
the commands were executed is also captured.

During long, stressful days, assessors will forget to save the output
from the commands they execute; or they will forget to update the name
of an output file and will accidentally overwrite previously saved output.

Assessors will also forget to record, or will make mistakes recording,
the necessary context about how and where a command was executed.
Port scan results showing an open management port are fine if the scan
was run from the perspective of an approved workstation on the
management network, but indicate a security risk if the scan
was run from the perspective of a system on the open Internet.
Correctly interpreting test results requires this contextual information.

The `clw` tool helps address this challenge by generating a tool run ID;
capturing all of the context about the command's execution; and capturing
the STDIN, STDOUT, and STDERR from the command's execution.
The contextual information captured by `clw` includes:

* Unique tool run ID for this command
* Original command line
* Modified command line (in case `clw` made any additions or changes)
* Environment variables at the time the command was executed
* Kernel settings (`sysctl -a`) at the time the command was executed
* Network interface settings at the time the command was executed
* Network routes at the time the command was executed
* Host firewall rules at the time the command was executed
* DNS settings and `/etc/hosts` at the time the command was executed
* STDIN, STDOUT, and STDERR during the entire command execution
* Start and end times for the command execution

You can run any command-line command with `clw`:

* Instead of running `nmap`, run `clw nmap`
* Instead of running `ping`, run `clw ping`
* Instead of running `traceroute`, run `clw traceroute`

For certain tools (currently `nmap`, `ping`, `ping6`, and `dumpcap`),
`clw` augments the specified command with arguments that need
to be present and will also automatically call the appropriate
`nmdb-import-*` program when the command completes.
* Explicit Augmentations
  * `dumpcap`: The `-w` option is always appended and points to the file
`results.pcapng` in the tool run result directory.
  * `nmap`: The `--reason`, `--stats-every 60s`, `--min-hostgroup 256`, and
`--min-rate 500` options are appended if not already present. The `-oA` option
is always appended.
  * `ping`/`ping6`: The `-n` option is appended if not already present.

The captured information is stored in the tool run result directory at
`~/.netmeld/clw/` and is of the format `toolname_timestamp_uuid`.  Where
`toolname` is the basename of the tool, `timestamp` is the date time to
the microsecond, and `uuid` is a UUID.  For example,
`nmap_20151209T135930.105725_4a7903b1-1f35-4e18-9a14-65d916e90577`.

EXAMPLES
========
```
clw nmap

clw ping

clw traceroute
```
