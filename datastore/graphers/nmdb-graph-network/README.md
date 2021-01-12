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
potentially communicate with each other (things like ACL rules are not
considered here).
A dashed edge represents one *device* is virtual and contained on another
*device*, with the host being the vertex pointed to by the arrow.


ICONS
=====

Device icons are not enabled by default and can be enabled using the `--icons`
flag.

Icons should be placed in the `nmdb-graph-network/images` folder, which is copied
during build installation to the `NETMELD_IMAGE_DIR` directory specified in
CMakeLists.txt. At runtime, the tool will search this folder for an icon
corresponding to the `device-type` and place it in the graph. For example, a
`device-type` of cisco will look for `images/cisco.*`.


EXAMPLES
========

Generate `.dot` format output, with Layer-3 information contained in the data
store, and rooted at the subnet `10.0.0.0/8`.
```
nmdb-graph-network --layer 3 --device-id '10.0.0.0/8'
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
If using SVG icons, inkscape can be used to output the graph in pdf or other
formats as such:

```
inkscape <(nmdb-graph-network -L3 --start-device-id Internet | dot -Tsvg) -A layer3.pdf
```
