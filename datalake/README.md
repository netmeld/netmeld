DESCRIPTION
===========

The Netmeld Datalake module provides a set of tools primarily focused to aid in
the operations of getting data into and out of a data lake backend to allow
further operation by an analyst or other tools.  Due to the nature of a data
lake, this also provides new oppurtunties to both the analyst and a developer
revolving around the concepts of:
* Further inspection, querying, or processing of raw data
* Analysis of change over time
* Additional capabilites for tool or logic chaining

![](docs/netmeld-datalake-workflow.png)


DATALAKE MODULE FUNDAMENTALS
======================

DATA LAKE TYPE AND HANDLERS
---------------------------

The Datalake module tools interact with an interface which tries to be agnostic
of the data lake backend, so in general the tools provide a common set of
functionality that all data lake backends shall possess and support.
However, ultimately they need to operate on a targeted data lake type and
capabilites of those greatly vary.  Thus an end user may directly manipulate
the data lake as needed.  If there are stipulations for how data shall be
represented in the data lake for the handler to operate and pass meaningful
data back to the tools as expected, the data lake handler shall clearly
outline those requirements.

TARGETED DATE AND TIME
----------------------

Support for tool interactions with the data lake as it exists at present or a
specific date and time in the past shall be supported by all backends.
Manipulation, specifically regarding this topic, of the backend outside of the
tools may lead to unexpected or unintentional outcomes.


TOOL FUNDAMENTALS
=================

COMMON COMMAND-LINE OPTIONS
---------------------------

All of the tools shall support the same base options as defined in the Netmeld
core library, that is:

* `--help`
* `--version`
* `--verbosity`

Additionally, all of the `nmdl-*` (Netmeld Datalake) tools shall support the
following options:
* `--lake-type`: The Netmeld data lake type to take action on.  This currently
defaults to a data lake of type `git`.
