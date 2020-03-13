DESCRIPTION
===========

Generate Graphviz `.dot` format output to produce a Layer-3 or Layer-2 like
network graph based on the information currently in the Netmeld data store.

Currently supported arguments:
+ The `-L3` or `--layer 3` options produce a Layer-3 network graph.
+ The `-L2` or `--layer 2` options produce a Layer-2 network graph.
+ The `--device-id` option is used to provide the device ID or subnet CIDR address to use as the graph's root node.
+ The `--icons` option can be used to enable device icons in the graph.



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

To directly produce a graph, variations on either or both of the following
command pipelines can be used:

```
nmdb-graph-network -L3 --start-device-id Internet | dot -Tpng -o layer3.png
nmdb-graph-network -L3 --start-device-id Internet | dot -Tpdf -o layer3.pdf
```

Some versions of Graphviz have issues loading the plugin for SVG conversion.
If using SVG icons, inkscape can be used to output the graph in pdf or other
formats as such:

```
inkscape <(nmdb-graph-network -L3 --start-device-id Internet | dot -Tsvg) -A layer3.pdf
```
