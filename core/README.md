DESCRIPTION
===========

This module provides core functionality for the Netmeld tool suite to leverage.
Common functions and capabilities are pooled into this module to enable
consistent behavior for tools and quicker tool development.  This is a library
only module and is a required dependency by most, but not all, Netmeld modules.

![](docs/netmeld-core.png)


LIBRARY FUNDAMENTALS
====================

BASIC CAPABILITIES
------------------

The module aims to be the minimal set of commonly useful capabilities across
all other modules.  Its singular goal is to help simplify design and
development of other modules and tools.

While the capability names aim to be self evident on purpose, some key
components are:
* `AbstractTool`: A base class for usage by all tools.  This handles the logic
  for consistent behavior of the common command-line options, adds logging
  support and graceful error handling, and provides hook points for common
  module or tool customizations.
* `AbstractObject`: A base class for usage by, primarily, data objects.  It
  does not provide much besides a default debug and out stream print function.
* `*Exec`: These provide a standardize way to perform, log, and interact with
  system level commands.  While there are a couple currently, functionality
  will eventually be consolidated to `CmdExec`.
* `FileManager`: This handles many common scenarios regarding file interaction
  while abstracting the actual backing from the user.  It also handles
  creation of a common storage root for many tools.
* `LoggerSingleton`: This is the primary *logging* capability used throughout
  the Netmeld tool suite.  It is closer to being an interface than an actual
  logging system and logs go standard out or error.
* `ProgramOptions`: Simply put, this handles tool option configuration and
  getting any associated values.  It enables configuration either from
  command-line, config files, or both; addition or removal of options into
  various pre-defined or custom *bins*; and option requirement(s) validation.


COMMON COMMAND-LINE OPTIONS
---------------------------

This module provides a basis for all other tools to support the following
common command-line options:

* `--help`: Display the program usage and options, then exit.
* `--version`: Display the program version information, then exit.
* `--verbosity`: Alter the program verbosity, OFF (0) - ALL (10), for
  execution.  These roughly map to `syslog` levels, with small variations to
  support development needs without impacting the end user.
