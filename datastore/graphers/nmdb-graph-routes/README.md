DESCRIPTION
===========

Generate Graphviz `.dot` format output to produce a routing graph based on
the information currently in the Netmeld data store.

The network diagram will be rooted at the subnet CIDR specified for the
`--source` option and end at any subnet which is reachable and contained
within the subnet CIDR specified for the `--destination` option.

A rectangular vertex is a *device*.
The *device* vertex contains information to identify the device ID and VRF,
if any.
Using the `--add-route-details` adds the route table data to the vertex as
it is relevant towards the requested source and destination.
The route information is ordered by incoming interface and then destination
subnet.

An oval vertex is a *subnet*.
A *subnet* vertex contains information to identify the subnet used between
the *device* vertices for routing the traffic.
Using the `--add-acl-details` adds ACL rules relevant to the vertex.
The rules are for any *device* vertex connected to the *subnet* vertex, so
the data is labeled and ordered by the device and applicable interfaces, to
and from.
Note that the rules are not explicitly expanded to every protocol or port,
but are instead the as recorded, in the device, rules.
The full details of the rule requires targeted queries to the data store.

An octagonal vertex has a specific meaning depending on its label.
The one labeled as *Null Route* represents a route that explicitly goes
nowhere.
Though it may be connected to multiple *device* vertices, the devices are
not actually connected via this vertex.

A solid edge between two vertices represents that the details of the next
hop path is known within the data store.
For example, routing tables and interface IPs are known for both vertices
and it is an explicit next hop or the destination subnet is the next hop.

A dashed edge between two vertices represents that the details of the next
hop path are unclear in terms of the explicit next hop.
For example, if the routing table states the next hop is whatever is
connected to the interface (regardless of IP) then the actual destination
is not known beyond the device's interface.


EXAMPLES
========

Generate `.dot` format output of all routes from `10.0.0.0/9` to
`10.128.0.0/9`.
```
nmdb-graph-routes --source '10.0/9' --destination '10.128/9'
```

Same as prior, but with a smaller destination subnet.
```
nmdb-graph-routes --source '10.0/9' --destination '10.128.0.0/24'
```

Same as prior, but add routing details.
```
nmdb-graph-routes --source '10.0/9' --destination '10.128.0.0/24' --add-route-details
```

Similar to prior, but add ACL details instead.
```
nmdb-graph-routes --source '10.0/9' --destination '10.128.0.0/24' --add-acl-details
```
