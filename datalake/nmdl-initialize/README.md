DESCRIPTION
===========

The `nmdl-initialize` tool is utilized to initialize the data lake to an
empty, default state.  Currently this is destructive, meaning any previous
data will be lost.  Additionally, this will define the default data lake
type for all other tools to interact with by creating a configuration file
stored in `${NETMELD_CONF_DIR}/nmdl.conf`


EXAMPLES
========

Initialize a targeted data lake type, `git`, to its default state.
```
nmdl-initialize --lake-type git
```
