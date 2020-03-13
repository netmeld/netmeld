DESCRIPTION
===========

This tool reverses Junos type 9 encoding.

EXAMPLES
========
Process a singular type 9 encoding.
```
junos-type9-decode '$9$2GgZjHkPQ39.PhrvLVb.P5Tz6'
```

Process a file with multiple type 9 encodings, one per line.
```
cat "type-9-encoded" | while read line; do junos-type9-decode $line; done
```
