NAME
====

Netmeld - A tool suite for use during system assessments.


DESCRIPTION
===========

System assessments typically yield large quantities of data from disparate
sources for an analyst to scrutinize for issues.  Netmeld is used to parse
input from different file formats, store the data in a common format, allow
users to easily query it, and enable analysts to tie different analysis tools
together using a common back-end.


INSTALLATION
------------------

We primarily target Kali Rolling and Debian Testing, so we package deb
[releases](https://github.com/netmeld/netmeld/releases)
on the GitHub page.
To compile from source, see the [INSTALL.md](docs/INSTALL.md) for instructions.


DO ONE THING AND DO IT WELL
---------------------------

The Netmeld tools follow a slightly modified version of the UNIX philosophy:
> Write programs that do one thing and do it well.
> Write programs to work together.
> Write programs to handle text streams, because that is a universal interface.

However, instead of text streams and pipes for inter-process communication,
Netmeld tools primarily use a data store as a central communication hub and
store of accumulated data.  Where it makes sense, Netmeld tools support text
streams and command chaining on either their input or output.

Following this, the Netmeld tool suite is divided into several modules which
focus on a specific area with regard to data collection and processing.
Furthermore, the tools in these modules are focused on performing one specific
task within the purview of the module.

A generalized work and data flow for the Netmeld tool suite is depicted in
the following diagram.

![](docs/netmeld-overview.png)

In general:
* The `Core` module is a library to supply the functionality common to all
  modules within this tool suite.
* The `Datalake` module provides a repository for raw data collection and the
  tools to import, export, or otherwise query the data stored.
* The `Datastore` module provides a repository for the processed data and the
  tools to import, export, or otherwise query the data stored.
* The `Fetchers` module provides tools to automate the collection of data
  from hosts within the targeted system.
* The `Playbook` modules provides tools to automate the collection of data
  from a network perspective within the targeted system.
* The `Tool-*` modules are targeted tools which resolve a specific need across
  multiple modules (potentially even external to Netmeld).  Generally, the
  desire to keep these as loosely coupled to other Netmeld tools as possible is
  high.

See the individual module documentation for more detailed information on it
and its tooling.  Note that in the modules documentation, the term `End User`
is used instead of identifying all the possible data sources for simplicity and
may be a person or other tool.


AUTHOR
======
Written by Michael Berg (2013-2015, pre v1.0). Currently
maintained (2016-present) by the Netmeld development team at
Sandia National Laboratories.


REPORTING BUGS
==============
Report bugs to <Netmeld@sandia.gov> or on the
[issue tracker](https://github.com/netmeld/netmeld/issues)
of the projects GitHub page.
