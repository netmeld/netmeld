![](https://github.com/netmeld/netmeld/workflows/CI/badge.svg)

NAME
====

Netmeld - A tool suite for use during system assessments


SYNOPSIS
========

See specific tools.


DESCRIPTION
===========

System assessments typically yield large quantities of data from disparate
sources for an analyst to scrutinize for issues.  Netmeld is used to parse
input from different file formats, store the data in a common format, allow
users to easily query it, and enable analysts to tie different analysis tools
together using a common back-end.

INSTALLING NETMELD
------------------

See [INSTALL.md](docs/INSTALL.md) for instructions on installing Netmeld
from source.

DO ONE THING AND DO IT WELL
---------------------------

The Netmeld tools follow a slightly modified version of the UNIX philosophy:
> Write programs that do one thing and do it well.
> Write programs to work together.
> Write programs to handle text streams, because that is a universal interface.

However, instead of text streams and pipes for inter-process communication,
Netmeld tools use a data store as a central communication hub and store of
accumulated data.  The data store is currently a PostgreSQL database.
Where it makes sense, Netmeld tools support text streams and command chaining
on either their input or output.

Each Netmeld tool does one thing, for example:
* The `nmdb-import-*` tools each parse a specific type of data
and insert that data into the data store.
* The `nmdb-insert-*` tools each provide a way to manually insert
a specific type of data into the data store.
* The `nmdb-export-*` tools each take data from the data store
and export the data in a specific format.
* The `nmdb-graph-*` tools each take data from the data store
and produce a graphical view of some property of interest.

Generalized work and data flow of data artifacts and Netmeld modules:

![](docs/netmeld-overview.png)

See the individual modules for more information on the specific module.


AUTHOR
======
Written by the Netmeld team at Sandia National Laboratories.  Netmeld
(pre v1.0) originally written by Michael Berg (2013-2015).


REPORTING BUGS
==============
Report bugs to <Netmeld@sandia.gov>.
