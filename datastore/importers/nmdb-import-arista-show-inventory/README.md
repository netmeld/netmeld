DESCRIPTION
===========

Parse and import the output of an Arista device's `show inventory` command.


EXAMPLE
=======

Process the target data for the device `switch` from a local file.
```
nmdb-import-arista-show-inventory --device-id switch inventory.txt
```

Assuming `...` is some command chain which retrieves the target data from a
remote host and displays the results locally, then the following would process
it and save the data to a file called `inventory.txt` in the current working
directory.
```
... | nmdb-import-arista-show-inventory --device-id switch inventory.txt --pipe
```
