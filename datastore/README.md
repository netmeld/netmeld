DESCRIPTION
===========

This module primarily provides a data store back-end to facilitate storage and
extraction of data.  This module also contains set(s) of tools binned by
general functionality to further aid in this effort for an end-user.  Unlike
the Datalake module, only data which has been identified as being useful from
an assessment perspective is extracted and stored.

![](docs/netmeld-datastore.png)

The binaries called out are part of this module specifically and provide a
consistent way to perform general manipulation of the data store directly
(regardless of actual back-end).  The binaries with an asterisk, `*`, are
part of sub-modules of the same name and are grouped by function, for example:

* The `nmdb-import-*` tools each parse a specific type of data and insert that
  data into the data store.
* The `nmdb-insert-*` tools each provide a way to manually insert a specific
  type of data into the data store.
* The `nmdb-export-*` tools each take data from the data store and export the
  data in a specific textual format.
* The `nmdb-graph-*` tools each take data from the data store and produce a
  graphical view of some property of interest.

See the documentation for the library or sub-modules for more information on it
specifically.


DATASTORE FUNDAMENTALS
======================

TYPES AND HANDLERS
------------------
While there could be multiple ways to implement the data store back-end,
currently it is a PostgreSQL database.  We have considered multiple types
and handlers in the past, but have not found sufficient justification for
adding support directly.  We have made several design decisions to enable
this should the desire be furthered.


TOOL RUNS
---------

We needed to be able to track where every piece of data in an assessment's
data store came from in order to accurately reconstruct which commands were
executed from which network locations, and to deal with (and back out) any
sources of invalid or incorrect data.  The tool run ID is a UUID that
uniquely identifies each tool run and the data produced by that tool run.
The `tool_runs` in the data store stores the tool run ID along with
information about the tool run, such as the command line executed,
path to where the output data is stored, and what time the command was run.

For tool runs during a live assessment, this provides a place to store the
context about where the tools were run (e.g., the assessor's MAC addresses,
IP addresses, routes, etc) and associate this context with the tool run ID.

There is a reserved tool run ID of `32b2fd62-08ff-4d44-8da7-6fbd581a90c6`
that is used for data that is manually entered by a human operator
(i.e., end user manipulation of the backing data store).
All manually entered data should use this tool run ID.
While some fidelity in the data origin is lost by this single tool run ID,
it keeps the human operator's job manageable.

All automated and semi-automated tools should generate and manage
unique tool run IDs so the origin of data can be properly tracked.


RAW VS. NON
-----------

Tool run IDs are necessary for tracking the origin of data and for
performing certain join actions in queries.  However, human analysts
generally don't need or generally want to see tool run IDs when looking
at the data store.

Analysts are generally interested in issues that were identified and
not in the multiple different tool runs that identified the same issue.
However, the ability to dig into the multiple tool runs must be available
whenever an analyst needs that level of detail.

In the Netmeld data store, all of the stores which expose the tool run ID
have a `raw_` prefix (e.g., `raw_mac_addrs`, `raw_ip_addrs`, etc).
All of the stores which discard the tool run ID and merge
duplicate entries from multiple tool runs lack the `raw_` prefix
(e.g., `mac_addrs`, `ip_addrs`, etc).

If you are writing new Netmeld tools or objects, you will be inserting data
into `raw_*` stores and querying both `raw_*` and regular stores.

Note that not every store will have a corresponding `raw_*` store
as many are produced by queries that combine and filter across multiple
`raw_*` and regular stores.  So, the relationship of a `raw_*` store could be
of the one-to-many nature and the stores which lack the prefix could be of the
many-to-many nature.


ADDRESSES AND HOSTNAMES
-----------------------

The Netmeld data store contains various locations for storing
MAC addresses, IP addresses, and hostnames that are not necessarily
associated with any specific device or piece of equipment.
Identified relationships between these addresses and hostnames
(such as MAC-to-IP and IP-to-hostname) are also stored.
The information comes both from data sources
that don't have in-depth information about the associated devices
(such as port scans, DNS queries, and parsed traffic captures)
as well as data sources that have in-depth information about devices
(such as configuration files from and commands executed on devices).

Examples of some of the address and hostname stores include:
`mac_addrs`, `ip_addrs`, `mac_addrs_ip_addrs`, `mac_addr_vendors`,
`hostnames`, `vlans`, `ip_nets`, `vlans_ip_nets`,
and the `raw_*` store versions of most of these.
There are several library objects which handle populating one or more of those
for tool developers:
`Interface`, `MacAddress`, `IpAddress`, `Vlan`, etc.


PORTS AND SERVICES
------------------

The Netmeld data store contains various locations for storing
information about port state (such as open, closed, or filtered),
services detected on those ports, banner grabs, and vulnerability information.
The port information is linked to the appropriate IP addresses
in the address stores.
All of the detected services, banner grabs, and vulnerability information
is linked to the IP address and port to which that information pertains.

It is important to note that "to which that information pertains" may in fact
mean the host which is leveraging the service and not the server itself.  To
help distinguish, `network_services` contains services which are discovered
from a network perspective (e.g., `nmap` scan) where as `device_ip_servers`
contains services which are discovered from a local/device perspective
(e.g., `ss` or `netstat` output) and contains a flag to denote if the
service is local or not.

Examples of some of the port and service stores include:
`ports`, `network_services`, `device_ip_servers`, `operating_systems`,
`ssh_host_algorithms`, `ssh_host_public_keys`, `nse_results`, `nessus_results`,
and the `raw_*` store versions of most of these.
The `Service` object attempts to *correctly* bin services into the
appropriate location (primary determinant being is there a device-id).
For the rest, examples include:
`Port`, `OperatingSystem`, and several objects which are tool specific.



DEVICES
-------

The Netmeld data store contains various locations for storing specific
information about networked devices.  The information about devices usually
comes from configuration files or the output of commands executed on devices.
The device-focused stores contain information about the device name, network
interfaces, and other device configuration properties.  The device's network
interfaces are then linked to the appropriate MAC addresses and IP addresses in
the address stores.

All tools that insert data into the device-focused stores must
have an appropriate value to put in the `device_id` column.
Therefore, all of the device-focused tools require specifying a
`--device-id` option on the command line.
While some tools are able to (and will) automatically extract, potentially
multiple, device-id(s) from a device configuration file, this has proven to
be more confusing to analysts and adds unnecessary complexity to tracking
pedigree.
So tools default to the provided `--device-id` and append the extracted portion
to the provided when appropriate (e.g., virtual router config data).

Examples of some of the device stores include:
`devices`, `device_interfaces`, `device_interface_summaries`,
`device_mac_addrs`, `device_ip_addrs`, `device_mac_addrs_ip_addrs`,
`device_ip_routes`, `device_virtualizations`,
and the `raw_*` store versions of most of these.
Almost every object contains the ability to pass and store a device-id, however
the `DeviceInformation` object primarily enables the usage and logic.


TOOL FUNDAMENTALS
=================

COMMON COMMAND-LINE OPTIONS
---------------------------

All of the tools shall support the same base options as defined in the Netmeld
core library, that is:

* `--help`
* `--version`
* `--verbosity`

All of the `nmdb-*` (Netmeld Database) tools additionally support the following
options:

* `--db-name arg`: The Netmeld data store to connect to. If not specified, this
  option defaults to the `site` data store.  You will typically use the default
  `site` data store while on-site and then specify an assessment-specific data
  store when re-importing data from all of the on-site assessment computers
  after returning home.  This assessment-specific data store is used for
  analysis and report writing.
* `--db-args args`: Additional database connection arguments. If not specified,
  this option defaults to '' (an empty string).  You will only need to specify
  these options if the database is not running on localhost and/or the default
  port, or requires additional options such as a password, etc. to make the
  connection.  Arguments are of the form `keyword=value`, each pair is
  separated by a `space`.  For more information about the format and available
  options see sections 33.1.1 and 33.1.2 of the libpqxx docs at:
  https://www.postgresql.org/docs/current/libpq-connect.html
* `--tool-run-id arg`: The tool run ID to assign to the data.  If not
  specified, a tool run ID will be auto-generated.  Human assessors will almost
  never need to use this option.  This option is used by tool developers when
  one tool (such as `clw`) is calling another tool (such as `nmdb-import-nmap`)
  and the tool run ID needs to propagate between those tools.
