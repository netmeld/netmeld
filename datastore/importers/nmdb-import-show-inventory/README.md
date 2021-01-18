DESCRIPTION
===========

Parse and import the output of a Cisco device's `show inventory` command.


EXAMPLE
=======

Process the target data for the device `switch` from a local file.
```
nmdb-import-show-inventory --device-id switch inventory.txt
```

Assuming `...` is some command chain which retrieves the target data from a
remote host and displays the results locally, then the following would process
it and save the data to a file called `inventory.txt` in the current working
directory.
```
... | nmdb-import-show-inventory --device-id switch inventory.txt --pipe
```
