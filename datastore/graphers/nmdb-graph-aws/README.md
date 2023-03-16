DESCRIPTION
===========

Output Graphviz `.dot` format output to produce an AWS related graph based
on the information currently in the Netmeld datastore.
The graph is similar to a Layer-3 (network) type graph however, given the
constructs in AWS and information required to understand the environment it
is not accurate to say it is a Layer-3 diagram.
Furthermore, current logic generates a graph focused on providing an AWS
VPC representation more so than all known AWS resources and connectivity.
For instance, constructs such as AWS S3, AWS RDS, etc. are not explicitly
depicted on a graph.
However, network interfaces or prefix lists supporting those resources may
appear on the graph.

Vertices contain information related to the source or target (e.g., AWS
network interface, AWS subnet, AWS routes, AWS VPCs, AWS gateways, etc.).
Were possible, a vertex title contains it's associated AWS identifier.
Cases were that is not possible include when the resource is not contained
within AWS.
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
For example, route tables are depicted as a vertex and an edge appears
between it, it's associated AWS subnet vertex, and the defined next-hop
vertices.
If the next-hop is a blackhole route, then that next-hop vertex also
contains an edge connecting it to a vertex called blackhole.
This, for example, is to aid in visual understanding that either the AWS
resource has been deleted yet a route table is still leveraging it or the
resource is being used differently depending on the route table.


EXAMPLE
=======
All examples assume sufficient data has been imported for visual depiction.

Generate a graph depicting, AWS network interfaces, subnets, route tables,
gateways, etc. known and connected within the AWS account(s).
```
nmdb-graph-aws
```

Similar to prior, however add known AWS instances.
```
nmdb-graph-aws --graph-instances
```

Similar to the first example, however remove extra details on the vertices.
```
nmdb-graph-aws --no-details
```

Generate a graph with the minimum details possible.  This is particularly
useful when there are a large number of known AWS resources.
```
nmdb-graph-aws --no-details --no-network-interfaces
```
