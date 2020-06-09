DESCRIPTION
===========

Parse and import the output of the `ping` and `ping6` commands.
You must run the ping command with the `-n` option
to output numeric IP addresses instead of DNS hostnames.
If you run ping using `clw`, `clw` automatically adds `-n`
to the command for you.

EXAMPLES
========
``` 
nmdb-import-ping ping_out.txt 

cat ping_out.txt | nmdb-import-ping --pipe copy.txt 
```

See Also: `ping (8)`
