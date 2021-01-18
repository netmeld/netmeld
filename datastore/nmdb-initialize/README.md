DESCRIPTION
===========

This tool provides the ability to consistently and quickly initialize the
Netmeld data store.

This will remove any existing data store that matches the required `--db-name`
option and initialize an instance of the data store with the targeted schema
to its default state.  Two methods exist for removing data from the data store.
The default being to completely remove the original, if it exists, and
create from scratch.  This is useful if the data store is large.  The other,
`--delete`, attempts to delete all data from the data store and will prove
faster for less populated data stores.

The options `--mac-prefix-file` and `--schema-dir` are for loading alternate
version of either should special needs occur.  If the Netmeld data store schema
is needed to be expanded on, use the `--extra-schema` option.


EXAMPLES
========

Trivial case of (re)initializing the data store.
```
nmdb-initialize
```

(Re)initialize the data store and load extra schema, `new1.sql` and `new2.sql`,
in addition to the default ones.
```
nmdb-initialize --extra-schema /etc/netmeld/schema/new1.sql ./new2.sql
```
