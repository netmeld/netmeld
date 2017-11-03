# Description

System assessments typically yield large quantities of data from disparate
sources for an analyst to scrutinize for issues.  Netmeld is used to parse
input from different file formats, store the data in a common format, allow
users to easily query it, and enable analysts to tie different analysis tools
together using a common back-end.

# Installing Netmeld

See [INSTALL.md](INSTALL.md) for instructions on installing netmeld.


-----

# Core Concepts

## Do One Thing and Do It Well

The Netmeld tools follow a slightly modified version of the UNIX philosophy:
> Write programs that do one thing and do it well.
> Write programs to work together.
> Write programs to handle text streams, because that is a universal interface.

However, instead of text streams and pipes for inter-process communication,
Netmeld tools use a PostgreSQL database as a central communication hub
and store of accumulated data.
Where it makes sense, Netmeld tools support text streams and command chaining
on either their input or output.

Each Netmeld tool does one thing:
* The `nmdb-import-*` tools each parse a specific type of data
and insert that data into the database.
* The `nmdb-insert-*` tools each provide a way to manually insert
a specific type of data into the database.
* The `nmdb-export-*` tools each take data from the database
and export the data in a specific format.
* The `nmdb-graph-*` tools each take data from the database
and produce a graphical view of some property of interest.


## Tool Runs

We needed to be able to track where every piece of data in an assessment's
Netmeld database came from in order to accurately reconstruct
which commands were executed from which network locations,
and to deal with (and back out) any sources of invalid or incorrect data.
The tool run ID is a UUID that uniquely identifies each tool run
and the data produced by that tool run.
The `tool_runs` table in the database stores the tool run ID
along with information about the tool run, such as the command line executed,
path to where the output data is stored, and what time the command was run.

For each tool run during a live assessment, other database tables
store context about where the tools were run (such as the assessor's
MAC addresses, IP addresses, routes, etc) and associate this context
with the tool run ID.

There is a reserved tool run ID of `32b2fd62-08ff-4d44-8da7-6fbd581a90c6`
that is used for data that is manually entered by a human operator.
All manually entered data should use this tool run ID.
While some fidelity in the data origin is lost by this single tool run ID,
it keeps the human operator's job manageable.

All automated and semi-automated tools should generate and manage
unique tool run IDs so the origin of data can be properly tracked.


## Raw vs. Regular Database Tables and Views

Tool run IDs are necessary for tracking the origin of data and for
performing certain `JOIN` actions in database queries and views.
However, human analysts generally don't need or want to see tool run IDs
when looking at predefined database views.

Analysts are generally interested in issues that were identified and
not in the multiple different tool runs that identified the same issue.
However, the ability to dig into the multiple tool runs must be available
whenever an analyst needs that level of detail.

In the Netmeld database schema, all of the tables and views that expose
the tool run ID have a `raw_` prefix
(`raw_mac_addrs`, `raw_ip_addrs`, `raw_vlans`, `raw_ip_nets`,
`raw_ports`, `raw_services`, `raw_devices`, etc).
All of the tables and views that discard the tool run ID and merge
duplicate entries from multiple tool runs lack the `raw_` prefix
(`mac_addrs`, `ip_addrs`, `vlans`, `ip_nets`,
`ports`, `services`, `devices`, etc).

If you are writing new Netmeld tools, you will be inserting data into
`raw_*` tables and querying `raw_*` and regular tables and views.

Note that not every regular view will have a corresponding `raw_*` table
as many views are produced by queries that `JOIN` and filter across
multiple `raw_*` and regular tables and views.


## Addresses and Hostnames

The Netmeld database contains various tables and views for storing
MAC addresses, IP addresses, and hostnames that are not necessarily
associated with any specific device or piece of equipment.
Identified relationships between these addresses and hostnames
(such as MAC-to-IP and IP-to-hostname) are also stored.
The information in these tables and views come both from data sources
that don't have in-depth information about the associated devices
(such as port scans, DNS queries, and parsed traffic captures)
as well as data sources that have in-depth information about devices
(such as configuration files from and commands executed on devices).

Examples of some of the address and hostname views include:
`mac_addrs`, `ip_addrs`, `mac_addrs_ip_addrs`, `mac_addr_vendors`,
`hostnames`, `vlans`, `ip_nets`, `vlans_ip_nets`,
and the `raw_*` table versions of most of these views.


## Ports and Services

The Netmeld database contains various tables and views for storing
information about port state (such as open, closed, or filtered),
services detected on those ports, banner grabs, and vulnerability information.
The port information is linked to the appropriate IP addresses
in the address tables and views.
All of the detected services, banner grabs, and vulnerability information
is linked to the IP address and port to which that information pertains.

Examples of some of the port and service views include:
`ports`, `services`, `ports_services`, `operating_systems`,
`ssh_host_algorithms`, `ssh_host_public_keys`,
`nse_results`, `nessus_results`,
and the `raw_*` table versions of most of these views.


## Devices

The Netmeld database contains various tables and views for storing
specific information about networked devices.
The information in the device tables and views usually comes from
configuration files for or the output of commands executed on devices.
The device-focused tables contain information about the device name,
network interfaces, and other device configuration properties.
The device's network interfaces are then linked to the appropriate
MAC addresses and IP addresses in the address tables and views.

All tools that insert data into the device-focused tables must
have an appropriate value to put in the `device_id` column.
Therefore, most of the device-focused tools require specifying a
`--device-id` option on the comand line.
However, some tools are able to automatically extract the device-id
from the device's configuration files.

Examples of some of the device views include:
`devices`, `device_interfaces`, `device_interface_summaries`,
`device_mac_addrs`, `device_ip_addrs`, `device_mac_addrs_ip_addrs`,
`device_ip_routes`, `device_virtualizations`,
and the `raw_*` table versions of most of these views.


-----

# Common Command-Line Options

All of the tools support the following options:

* `--help`: Display the program usage and options, then exit.
* `--version`: Display the program version information, then exit.

All of the `nmdb-*` (Netmeld Database) tools adddtionally support the following 
options:

* `--db-name arg`: The netmeld database to connect to. If not specified, this
option defaults to the `site` database. You will typically use the default 
`site` database while on-site and then specify an assessment-specific database
when re-importing data from all of the on-site assessment computers after
returning home.  This assessment-specific database is used for analysis and
report writing.
* `--tool-run-id arg`: The tool run ID to assign to the data.  If not specified,
a tool run ID will be auto-generated.  Human assessors will almost never need
to use this option.  This option is used by tool developers when one tool
(such as `clw`) is calling another tool (such as `nmdb-import-nmap`) and the
tool run ID needs to propagate between those tools.


-----

# The Command-Line Wrapper (clw)

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

The `clw` tools helps address this challenge by generating a tool run ID;
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

The format names underneath `~/.clw/` is `toolname_timestamp_uuid`
(such as `nmap_20151209T135930.105725_4a7903b1-1f35-4e18-9a14-65d916e90577`).

-----

# The Fetch Tools

One of the challenges prior to an assessment is ensuring all important configs 
from hosts are gathered for analysis.  Often hosts are missed, additional 
information is needed based on initial analysis results, or requested configs 
simply are not supplied as requested.  While many times it cannot be addressed
early on, when on-site assessment begins the oppurtunity is there but time is
limited and these needs can be easily overlooked.

The `nm-fetch-*` tools help address the challenge by providing a mechanism to
automate data collection.  Each tool establishs a connection to a remote host 
and executes a set of commands on that host.  The results of the commands are 
saved to the localhost (i.e. the host the tool is executed from) each in a
separate file associated with the specific command ran.

The `nm-fetch-*` tools can take a single command or a file containing a list
of commands (newline separated) to execute.  The results of which are saved
under `~/.fetch-*` where `*` equates to the specific tool ran.  The format
of the names under these directories are `hostname_timestamp_uuid` (such as
localhost_20170829T114220.575369140_36e15179-40df-474a-9c1c-9d71332f1211).
Each command supplied results in a separate file for the format `command.log`
(such as ip_addr_show.log).

## `nm-fetch-ssh`
Fetch data leveraging an SSH connection.

For files containing the command output, the command name has spaces replaced 
by an underscore (`_`) and forward slashes replaced by unicode code point 
U+2215 (`∕`).

-----

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
the `ifconfig` command, and import that data into the Netmeld database,
you could use the following command chain:

```
ssh user@host-01.example.com "ifconfig" | nmdb-import-ifconfig --device-id host-01 --pipe host-01_ifconfig.txt
```

## `nmdb-import-cisco`

Parse and import Cisco switch, router, and firewall configuration files
(the output of `show running-config`).
This tool also does a reasonable job importing configuration files
from some non-Cisco equipment that uses a Cisco-like configuration syntax.

The `nmdb-import-cisco` automatically extracts the device-id
from the configuration file.

NOTE: The library which this relies on, `ciscoconfparse`, supports parsing brace
delimited configs as of version 1.2.40.  However, the tool did not seem to do
a sufficient job of parsing brace delimited configs when tested.


## `nmdb-import-clw`

Import the contextual information about a command that was captured by `clw`.
In cases where `clw` was wrapping `nmap` or `ping`,
`nmdb-import-clw` will also call `nmdb-import-nmap` or `nmdb-import-ping`
respectively to import those results.

To import an entire `.clw` directory of tool runs en-mass
(such and importing multiple laptops into a single database
after returning from an assessment):

```
for d in `/bin/ls -1 /path/to/.clw`; do
    nmdb-import-clw --db-name site-name $d
done
```

## `nmdb-import-hosts`

Parse and import the `/etc/hosts` file
from Linux, BSD, and other UNIX-like systems.
IP addresses are inserted into the `raw_ip_addrs` table.
IP address and hostname pairs are inserted into the `raw_hostnames` table.

Note that `nmdb-import-hosts` filters out and does not insert
special addresses such as localhost (`127.0.0.1` and `::1`),
broadcast (`255.255.255.255`), and multicast (`ff02::1` and `ff02::2`).


## `nmdb-import-ifconfig`

Parse and import the output of the `ifconfig`
command on Linux, BSD, and other UNIX-like systems.

Since `nmdb-import-ifconfig` is importing information about a device's
network interfaces, the `--device-id` option is required.


## `nmdb-import-ip-addr-show`

Parse and import the output from the `ip addr show`
command on modern Linux systems.

Since `nmdb-import-ip-addr-show` is importing information about a device's
network interfaces, the `--device-id` option is required.


## `nmdb-import-ip-route-show`

Parse and import the output from the `ip route show`
command on modern Linux systems.

Since `nmdb-import-ip-route-show` is importing information about a device's
routing tables, the `--device-id` option is required.


## `nmdb-import-juniper-junos`

Parse and import Juniper's Junos configuration files.

Since `nmdb-import-juniper-junos` is importing information about a Juniper
device's configuration, the `--device-id` option is required.


## `nmdb-import-juniper-screenos`

Parse and import Juniper's ScreenOS/NetScreen configuration files.

Since `nmdb-import-juniper-screenos` is importing information about a Juniper
device's configuration, the `--device-id` option is required.


## `nmdb-import-nessus`

Parse and import Nessus' XML output (`.nessus` files).


## `nmdb-import-nmap`

Parse and import Nmap's XML output (`.xml` files).
You must run Nmap with either the `-oA` or `-oX` output format options
in order to produce XML output.
If you run Nmap using `clw`, `clw` automatically adds `-oA` to the command
to ensure that XML output is produced.


## `nmdb-import-ping`

Parse and import the output of the `ping` and `ping6` commands.
You must run the ping command with the `-n` option
to output numeric IP addresses instead of DNS hostnames.
If you run ping using `clw`, `clw` automatically adds `-n`
to the command for you.


## `nmdb-import-route`

Parse and import the output of the `route -n` or `netstat -nr`
commands on Linux, BSD, and other UNIX-like systems.

Since `nmdb-import-route` is importing information about a device's
routing tables, the `--device-id` option is required.

## `nmdb-import-show-mac-address-table`

Parse and import the output of the `show mac address-table` command on a Cisco
device.  Note that `nmdb-import-show-mac-address-table` filters out and does
not insert any line which does not start with a numeric VLAN identifier 
(excluding whitespace).

Since `nmdb-import-show-mac-address-table` is importing information about a 
device's CAM table, the `--device-id` option is required.

-----

# The Insert Tools

The `nmdb-insert-*` tools are for manually inserting information
that is obtained from system documentation, interviews with system
designers or owners, or from reviewing configuration files for which
we don't yet have import parsers.

Since the `nmdb-insert-*` tools are for manually inserting information,
all of the tools default to using the human tool run ID
(32b2fd62-08ff-4d44-8da7-6fbd581a90c6).
If you are using these tools as part of a larger automated script,
you should consider generating a UUID for each of your tool runs
and passing in that UUID with the `--tool-run-id` option.

## `nmdb-insert-address`

Manually insert MAC addresses and IP addresses into the Netmeld database.
The inserted information is not tied to any specific device.

* Use the `--mac-addr` option to insert a MAC address
into the `raw_mac_addrs` table.
* Use the `--ip-addr` option to insert an IPv4 or IPv6 address
into the `raw_ip_addrs` table.
* If you specify both the `--mac-addr` and `--ip-addr` options,
it will also associate the MAC address and IP address
in the `raw_mac_addrs_ip_addrs` table.


## `nmdb-insert-network`

Manually insert VLANs and IP networks into the Netmeld database.
The inserted information is not tied to any specific device.

* Use the `--vlan` option to insert an 802.1Q VLAN
into the `raw_vlans` table.
* Use the `--ip-net` option to insert an IPv4 or IPv6 CIDR network
into the `raw_ip_nets` table.
* If you specify both the `--vlan` and `--ip-net` options,
it will associate the VLAN and IP network
in the `raw_vlans_ip_nets` table.


## `nmdb-insert-device`

Manually insert device information into the Netmeld database.

Since `nmdb-insert-device` is importing information about a device's
configuration, the `--device-id` option is required.

* Use the `--device-id` option to specify the ID for the device.
* Use the `--vm-host-device-id` option to specify that the device
is a virtual machine running on the specified host device.
* Use the `--device-color` option to specify the device's color
in network graphs.
* Use the `--interface` option to specify the network interface name
to with the following address options apply:
  * Use the `--mac-addr` option to insert a MAC address
into the `raw_mac_addrs` and `raw_device_mac_addrs` tables.
  * Use the `--ip-addr` option to insert an IPv4 or IPv6 address
into the `raw_ip_addrs` and `raw_device_ip_addrs` tables.
  * If you specify both the `--mac-addr` and `--ip-addr` options,
it will also associate the MAC address and IP address
in the `raw_mac_addrs_ip_addrs` table.


-----

# The Export Tools

## `nmdb-export-port-list`

Generate a list of ports suitable for use with Nmap.
By default, the generated port list is based on the contents
of the configuration file `/usr/local/etc/netmeld/port-list.conf`
and the selected protocol options
(such as `--tcp`, `--tcp-all`, `--udp`, and `--udp-all`).
However, if you specify the `--from-db` option, the generated port list
is based on the responding ports currently recorded in the database,
a short list of ports that we always want to check for,
and the selected protocol options.

Examples of how to use this tool with Nmap:
```
clw nmap -sU -p `nmdb-export-port-list --udp` ...
clw nmap -sS -p `nmdb-export-port-list --tcp-all` ...
clw nmap -sS -sU -p `nmdb-export-port-list --from-db --tcp --udp` ...
```

## `nmdb-export-tool-runs`
Generate a script to rerun all tool\_run commands from the database.
Specifically, the tool joins data from the `tool_runs`, `raw_devices`,
`device_colors`, and `device_types` tables of the database.  The script produced
should be sufficient to reproduce the database again on that same machine as is.
If the data is to be reproduced elsewhere, the data pathing may need to be
updated as Netmeld tools place a file's canonical path in the database.

By default the tool pulls from a default database (e.g. `site`) and exports to 
a file prefixed with `tool-runs-export` and with `_DTS.sh`, where DTS is a date
timestamp, as a suffix.  These settings as well as others can be configured on
the command line.

Examples include:
```
nmdb-export-tool-runs
nmdb-export-tool-runs --ignore-tool-run-id
nmdb-export-tool-runs --db-name testing --outfile testing
```

-----

# The Graph Tools

## `nmdb-graph-network`

Output Graphviz `.dot` format output to produce a Layer-3 or Layer-2
network graph based on the information currently in the Netmeld database.
The `-L3` or `--layer 3` options produce a Layer-3 network graph.
The `-L2` or `--layer 2` options produce a Layer-2 network graph.

To directly produce a graph, variations on either or both of the following
command pipelines can be used:

```
nmdb-graph-network -L3 --start-device-id Internet | dot -Tpng -o layer3.png
nmdb-graph-network -L3 --start-device-id Internet | dot -Tpdf -o layer3.pdf
```

You can use other Graphviz layout programs instead of `dot` as appropriate
(such as `neato`, `fdp`, or `sfdp`).
You can also redirect the output from `nmdb-graph-network` to a file
and use any other graphing tools that can process Graphviz `.dot` format.

### Icons
Device icons are enabled by default and can be disabled using the `-n` or 
`--no-icons` flag.

Icons should be placed in the nmdb-graph-network/images folder, which is copied
during installation to the NETMELD_IMAGE_DIR directory specified in 
CMakeLists.txt. At runtime, the tool will search this folder for an icon
corresponding to the device\_type and place it in the graph. For example, a 
device\_type of cisco will look for `images\cisco.*`.

Some versions of graphviz have issues loading the plugin for svg conversion.
If using svg icons, inkscape can be used to output the graph in pdf or other 
formats:

```
inkscape <(nmdb-graph-network -L3 --start-device-id Internet | dot -Tsvg) -A layer3.pdf
```

-----

# Other Tools

## `nmdb-remove-tool-run`

You can remove a problematic tool run (and all of the data from that tool run)
with the `nmdb-remove-tool-run` command.
You must specify the tool run ID to be removed with the `--tool-run-id` option.
This tool defaults to the `site` database unless you specify
an alternate netmeld database with the `--db-name` option.

This command removes the respective entry from the `tool_runs` table
and CASCADEs to remove entries in other tables that are associated
with the removed tool run ID. This command does not delete files from the 
filesystem thus if the files are not removed and the tool run data directory
is imported at a time in the future, the problematic tool run will be added
back to the netmeld database.

-----