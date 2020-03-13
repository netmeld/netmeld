DESCRIPTION
===========

Output Graphviz `.dot` format output to produce an access control related graph
based on the information currently in the Netmeld data store.

Vertices contain information related to the source or target.  A vertex title
is of the format `(set_id) set_name (interface)` where `set_id` is an
identifier which contains multiple sets (e.g., a zone in terms of ACLs),
`set_name` is an identifier for the associated network(s), and `interface`
is the device interface where data is flowing.  Whether the interface is for
ingress or egress depends on the edge.  Vertices further contain a listing of
all associated networks based on the data contained in the configuration file.

The vertices are connected by an edge which contains information related to
a rule's associated action(s) and service(s).  They can have multiple rules
associated per edge.  When multiple rules are identified, order matters.
While the rules should be in order of operations, a numerical value is also
provided which should correspond to the order provided by a configuration
file or the order in which it was processed.

A best effort is attempted in terms of resolving network and service set
identifiers into it's numerical equivalent, however this cannot always be
done (e.g., using device built-ins).

Currently supports the `--device-id` option.

EXAMPLE
=======

``` 
nmdb-graph-ac --device-id workstation
```
