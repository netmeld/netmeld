DESCRIPTION
===========

Parse and import Nmap's XML output (`.xml` files).
You must run Nmap with either the `-oA` or `-oX` output format options
in order to produce XML output.
If you run Nmap using `clw`, `clw` automatically adds `-oA` to the command
to ensure that XML output is produced.

Currently supported arguments:
+ `scan-origin-ip`: IP address of the device where nmap scan originated


EXAMPLES
========
``` 
nmdb-import-nmap nmap_out.xml 

cat nmap_out.xml | nmdb-import-nmap --pipe copy.xml 
```

See Also: `nmap (1)`
