DESCRIPTION
===========

Manually insert access control (ACLs, firewall rules, etc.) related information
for a device into the data store.

Options are split into the following categories:
* Access control rules: these are the rules themselves
* Network book: defines a set of IP(s) or network(s) which rules can reference
* Service book: defines a set of service(s) which rules can reference

The options needed for each are split and documented in the tool's `--help`
output.  While options for all three can be specified simultaneously, the tool
will **NOT** process all at the same time.  It will only process the first
successful match.  Also, any insertion, except for an exact match, are treated
as new so edits to rules require either manually editing the rule in the
data store or deleting the rule and adding the new version.  So scripting
these inserts may save a lot of effort.

Note that currently most of this data is added as a plain text string, so it is
fairly flexible in terms of ability to describe various access control related
topics without being completely explicit.


EXAMPLES
========

The following describes a scenario where some access control mechanism is
learned and then it is back filled with details.  While this is not ideal, it
shows the loose coupling of the different option sets.  Graphical
representations can be generated after each step via `nmdb-graph-ac --device-id
ac-dev` and will show how addition of data changes the representation.

We have identified a rule which prevents and logs certain access from an admin
network, in some `trust` zone, to the internet, in the global zone.  Note that
we have not already defined any of this, but the tooling allows it and we can
generate a graph as describe above after inserting the data.
```
nmdb-insert-ac --device-id ac-dev \
	--src-id trust --src admin-nets \
	--dst internet-edge \
	--service web --action deny,log \
  --description "Prevent and log control plane web access"
```

Suppose later we learn the service `web` really means TCP communications from
`any` source port to destination ports `80` or `443`.
```
nmdb-insert-ac --device-id ac-dev \
	--sb-name web --sb-data tcp:any:80,443
```

Finally, suppose we learn the network `admin-nets` in the `trust` zone refers
to `10.0.9.0/24` and `10.1.9.0/24`.  While the network `internet-edge` in the
`global` zone refers to `10.0.0.0/24`.

Network book insertion:
```
nmdb-insert-ac --device-id ac-dev \
	--nb-id trust --nb-name admin-nets --nb-data 10.0.9.0/24
nmdb-insert-ac --device-id ac-dev \
	--nb-id trust --nb-name admin-nets --nb-data 10.1.9.0/24
nmdb-insert-ac --device-id ac-dev \
	--nb-name internet-edge --nb-data 10.0.0.0/24
```
