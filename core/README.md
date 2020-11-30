DESCRIPTION
===========

This provides most of the core functionality for the Netmeld tool suite
to leverage.  Common functions and capabilities are pooled into this module
to enable consistent behavior for tools and quicker tool development.

![](docs/netmeld-core.png)


TOOL FUNDAMENTALS
=================

COMMON COMMAND-LINE OPTIONS
---------------------------

All of the tools support the following options:

* `--help`: Display the program usage and options, then exit.
* `--version`: Display the program version information, then exit.
* `--verbosity`: Alter the program verbosity, OFF (0) - ALL (10), for
execution.  These roughly map to `syslog` levels, with small variations to
support development needs without impacting the end user.
