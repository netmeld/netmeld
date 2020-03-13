DESCRIPTION
===========

Parse and import the output of a pcap file (such as from `dumpcap`).
Currently, data extraction is limited to VLAN tag, MAC address, and IPv4/6
address type information.  For enhanced capability, see the
`nmdb-import-tshark` tool.

EXAMPLES
========
``` 
nmdb-import-pcap capture.pcap 
cat capture.pcap | nmdb-import-pcap --pipe copy.pcap 
``` 

See Also: `pcap (3)`
