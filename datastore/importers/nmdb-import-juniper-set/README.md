DESCRIPTION
===========

Parse and import the output of a Juniper device's `get config` command.
Currently, this tool supports ScreenOS and NetScreen type devices.  However,
there are commands on various Juniper devices to explicitly use or show data in
this format (e.g., in JunOS `show | display set`).


EXAMPLES
========

Process the target data for the device `switch` from a local file.
```
nmdb-import-juniper-set --device-id switch config.txt
```

Assuming `...` is some command chain which retrieves the target data from a
remote host and displays the results locally, then the following would process
it and save the data to a file called `config.txt` in the current working
directory.
```
... | nmdb-import-juniper-set --device-id switch config.txt --pipe
```
