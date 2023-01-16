DESCRIPTION
===========

Output Graphviz `.dot` format output to produce an AWS related graph based
on the information currently in the Netmeld datastore.
The graph is similar to a Layer-3 (network) type graph however, given the
constructs in AWS and information required to understand the environment it
is not accurate to say it is a Layer-3 diagram.

Vertices contain information related to the source or target (e.g., AWS
network interface, AWS subnet, AWS routes, AWS VPCs, AWS gateways, etc.).
Were possible, a vertex title contains it's associated AWS identifier.
Cases were that is not possible include when the resource is not owned by
AWS.
Furthermore, in certain cases a vertex may contain additional data (e.g., a
network interfaces security group rules, a subnet's network access control
list rules, etc.) about the particular resource.
These typically will not provide the AWS identifier as the data may be the
result of aggregation of multiple AWS resources and are explicitly
associated with the containing vertex (i.e., can be identified by examining
the parent AWS identifier).

A solid edge between two vertices represents they are connected and can
potentially communicate with each other (concepts like ACL rules are not
considered here).
In general, the solid edge represents how the AWS environment is connected.
However, given the various concepts at play is not in strict alignment with
AWS constructs to allow a more explicit visual representation of the logic
within the AWS environment.
For example, an AWS VPC contains a subnet(s) which has an associated route
table with both local and non-local routing rules.  The diagram will
present this as a subnet connected to the local routing, then to the VPC,
and finaly to the external routing.
While not in strict parent-child hierarchy, it does create graphics which
are simpler to understand applied logic in a visual sense.


EXAMPLE
=======

TBS
