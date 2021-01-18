DESCRIPTION
===========

Parse and import the output of a Cisco device's `show mac address-table`
command.

Note that this tool filters out and does not insert any line which does not
start with a numeric VLAN identifier (excluding white space).


EXAMPLE
=======

Process the target data for the device `switch` from a local file.
```
nmdb-import-show-mac-address-table --device-id switch addresses.txt
```

Assuming `...` is some command chain which retrieves the target data from a
remote host and displays the results locally, then the following would process
it and save the data to a file called `addresses.txt` in the current working
directory.
```
... | nmdb-import-show-mac-address-table --device-id switch addresses.txt --pipe
```
