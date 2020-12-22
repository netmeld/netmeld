DESCRIPTION
===========

One of the challenges prior to an assessment is ensuring all important configs
from hosts are gathered for analysis.  Often hosts are missed, additional
information is needed based on initial analysis results, or requested configs
simply are not supplied as requested.  While many times it cannot be addressed
early on, when on-site assessment begins the opportunity is there but time is
limited and these needs can be easily overlooked.

The `nm-fetch-*` tools help address the challenge by providing a mechanism to
automate data collection.  Each tool establishes a connection to a remote host
and executes a set of commands on that host.  The results of the commands are
saved to the localhost (i.e., the host the tool is executed from) each in a
separate file associated with the specific command ran.


FETCHER FUNDAMENTALS
====================

FEATURE RICH OR POOR
--------------------
We generally have two categories of needs in this module.  Either we are doing
the collection or someone else is for us.  As such, the `nm-fetch-ansible`
meets our needs and maybe the others, while the other tools are specifically to
meet the other.

Specifically, the `nm-fetch-ansible` *tool* (loose interpretation) pretty much
leverages Ansible tooling for everything so we can get a LOT of flexibility and
capability out of it.  However in the cases where Ansible is not available or
the end user is not familiar with its usage, we need something else to gather
the data.  Thus, we specifically try to keep the non-Ansible based tooling
light (i.e., feature poor, use what is typically available, etc.) so they are
easily examinable and usable by a more general audience.


DATA STORAGE
------------
In general, the tools take a single command or a list of commands to execute.
The results are saved under the Netmeld root directory (e.g., `~/.netmeld/`) in
a folder similar to the tool name, but without the `nm-`.  For example, the
folder for data collected via the `nm-fetch-ssh` tool is `fetch-ssh` and the
full path may be `~/.netmeld/fetch-ssh`.

The data is further organized as `hostname/timestamp_uuid`.  Where both
`hostname` and `timestamp_uuid` are folders.  Each command executed results in
a separate file of the format `command.log` (such as ip_addr_show.log) and
contains the output (`STDOUT` and `STDERR`) of the command.

Efforts have been made to alter or escape problematic characters for `ext` like
filesystems.  Even such, it was only for commonly encountered characters (i.e.,
space and forward slash).

Note, the `nm-fetch-ansible` tool set is the only one currently which may
produce altered storage logic.  Specifically, if the Datalake module tools are
available it will leverage those to save the data instead of what is described
above.  If they are not, it will use the above logic.  This behavior can be
overridden and for more information see the tool documentation.


TOOL FUNDAMENTALS
=================

COMMON COMMAND-LINE OPTIONS
---------------------------
All of the tools shall support the same base options as defined in the Netmeld
core library except the `--verbosity` option, that is:

* `--help`
* `--version`

The `--verbosity` option is allowed to be excluded as in many cases it adds
unwanted complexity to an otherwise simplistic logic flow and the execution
environment allows for alternative ways to increase verbosity.

Except for the `nm-fetch-ansible` tool, all of the `nm-fetch-*`
(Netmeld Fetcher) tools support the following options:

* `--directory`: The data storage path if an alternate root location is wanted.
* `--infile`: A file containing multiple commands (one per line) to execute
  instead of running one command at a time.

The `nm-fetch-ansible` tool is exempt because this is supported via Ansible
configuration instead.
