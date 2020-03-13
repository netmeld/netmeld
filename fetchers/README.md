# The Fetch Tools

One of the challenges prior to an assessment is ensuring all important configs
from hosts are gathered for analysis.  Often hosts are missed, additional
information is needed based on initial analysis results, or requested configs
simply are not supplied as requested.  While many times it cannot be addressed
early on, when on-site assessment begins the opportunity is there but time is
limited and these needs can be easily overlooked.

The `nm-fetch-*` tools help address the challenge by providing a mechanism to
automate data collection.  Each tool establishes a connection to a remote host
and executes a set of commands on that host.  The results of the commands are
saved to the localhost (i.e. the host the tool is executed from) each in a
separate file associated with the specific command ran.

The `nm-fetch-*` tools can take a single command or a file containing a list
of commands (newline separated) to execute.  The results of which are saved
under `~/.netmeld/fetch-*` where `*` equates to the specific tool ran.  The
format of the names under these directories are `hostname_timestamp_uuid`
(such as
localhost_20170829T114220.575369140_36e15179-40df-474a-9c1c-9d71332f1211).
Each command supplied results in a separate file for the format `command.log`
(such as ip_addr_show.log).
