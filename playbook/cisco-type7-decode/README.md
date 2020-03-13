DESCRIPTION
===========

This tool reverses Cisco type 7 encoding.

EXAMPLES
========
Process a singular type 7 encoding.
```
cisco-type7-decode 021605481811003348
```

Process a file with multiple type 7 encodings, one per line.
```
cat "type-7-encoded" | while read line; do cisco-type7-decode $line; done
```

