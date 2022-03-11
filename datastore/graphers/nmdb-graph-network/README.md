DESCRIPTION
===========

Generate Graphviz `.dot` format output to produce a Layer-3 (network) or
Layer-2 (data link) like network graph based on the information currently in
the Netmeld data store.
While close, these graphs are not exactly the same as what many network
engineers would view as a Layer-3 or Layer-2 graph.  These specifically contain
information which has been determined to be useful from an assessment
point-of-view and to aid discussions with the target system's network
engineers.

The network diagram will be rooted at the device ID or subnet CIDR provided to
the `--device-id` option (which must exist).

A rectangular vertex is a *device* and an oval vertex is a *subnet*.
The *device* vertex will contain information (if it exists) such as:
device ID, vendor, model, interface name, MAC address, IP address, and any name
alias such as hostname or DNS name.
The *subnet* vertex will contain information (if it exists) such as:
subnet, CIDR, VLAN ID, and network description.

A solid edge between two vertices represents they are connected and can
potentially communicate with each other (concepts like ACL rules are not
considered here).  In general, this solid edge represents that the device
has a responding IP address within the connected subnet.  However, when
the `responding-state` option is set to either `true` or `false` then
the this does not hold and simply represents containment as responsive
state is as asked for by the value of `responding-state`.

A dashed edge represents one *device* is virtual and contained on another
*device*, with the host being the vertex pointed to by the arrow.
When the `--show-traceroute-hops` option is enabled, a dashed edge with a
label `hop X to IP` (where `X` is a hop count; `IP` is an IP address)
will be applied to known routing paths as collected from traceroute
activities.

IP or MAC addresses are color coded *green* in the cases where the Datasore
shows them as responsive.


ICONS
=====

Device icons are not enabled by default and can be enabled using the `--icons`
flag.

During build, icons should be placed in the `nmdb-graph-network/images` folder,
which is copied during build installation to the `NETMELD_IMAGE_DIR` directory
specified in CMakeLists.txt.
At run-time, the tool will recursively search the folder specified by the
`icons-folder` option for a graphic file corresponding to the `device-type` and
place the path in the graph.  For example, a `device-type` of Cisco will look
for `images/cisco.*`.  If a match cannot be found, the default, a question
mark, is used.


EXAMPLES
========

Generate `.dot` format output, with Layer-3 information contained in the data
store, and rooted at the subnet `10.0.0.0/8`.
```
nmdb-graph-network --layer 3 --device-id '10.0.0.0/8'
```

Same as prior, but only show devices with a responding IP or MAC.  Note
that, for a device, this will remove any IP and MAC pair which is not
responding from the listing on the device as well.
```
nmdb-graph-network --layer 3 --device-id '10.0.0.0/8' --responding-state true
```

Generate `.dot` format output, with Layer-2 information contained in the data
store, and rooted at the device with ID `core`.
```
nmdb-graph-network --layer 2 --device-id core
```


Directly produce a PDF or PNG version of a graph.
```
nmdb-graph-network --layer 3 --device-id core | dot -Tpdf -o layer3.pdf
nmdb-graph-network --layer 3 --device-id core | dot -Tpng -o layer3.png
```

Some versions of Graphviz have issues loading the plugin for SVG conversion.
If using SVG icons, Inkscape can be used to output the graph in PDF or other
formats as such:

```
inkscape <(nmdb-graph-network -L3 --start-device-id Internet | dot -Tsvg) -A layer3.pdf
```
