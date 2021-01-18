DESCRIPTION
===========

Manually insert device hardware type information into the Netmeld database.
The options can be given independently or all in one command.


EXAMPLES
========

Insert device information for `laptop`.  It was manufactured by `Dell` and has
a serial number of `001`
```
nmdb-insert-device-hardware --device-id laptop --vendor Dell --serial-number 001
```

Same as prior, just in multiple lines.
```
nmdb-insert-device-hardware --device-id laptop --vendor Dell
nmdb-insert-device-hardware --device-id laptop --serial-number 001
```
