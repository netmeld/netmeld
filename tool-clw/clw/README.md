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
capturing known important context about the command's execution environment;
and capturing the STDIN, STDOUT, and STDERR from the command's execution.


EXECUTION CONTEXT
-----------------

While some of this important context requires privileged access, the tool
can run from a non-privileged user context and may output several warnings
when it fails to collect certain context data.
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

You can run any command-line command with `clw`.
However, the context is captured when the command is invoked.
So while the `clw` tool will work on interactive sessions, it will not capture
execution environment changes which occur during the interactive session.
However, the `clw` tool also works in a nested environment.
So, if an interactive session is started via `clw bash` then running `clw nmap`
during that session will correctly capture the context for the `nmap` command
separately from the `bash` context.


COMMAND AUGMENTATION
--------------------

For certain tools (currently `nmap`, `ping`, `ping6`, and `dumpcap`),
`clw` augments the specified command with arguments we identify as needing
to be present and will also automatically call the appropriate
`nmdb-import-*` program (if installed) when the command completes.
* Explicit augmentations
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

Wrap a nmap scan of localhost:
```
clw nmap localhost
```
![](/docs/term/clwnmap.svg)

Wrap a ping of localhost:
```
clw ping localhost
```
![](/docs/term/clwping.svg)

Wrap a traceroute to localhost:
```
clw traceroute localhost
```
![](/docs/term/clwtr.svg)

Wrap a netcat session listening on port 8080:
```
clw nc -nvlp 8080
```
![](/docs/term/clwnc.svg)

Wrap a bash session:
```
clw bash
```
