DESCRIPTION
===========

Parse and import the output from the `ipconfig /allcompartments /all` command
on modern Windows systems.
While the tool will also correctly work if the command happens to be `ipconfig
/all`, it will not (currently) work as expected if the command is `ipconfig`.


EXAMPLES
========

Process the target data for the device `workstation` from a local file.
```
nmdb-import-ip-config --device-id workstation ipconfig.txt
```

Assuming `...` is some command chain which retrieves the target data from a
remote host and displays the results locally, then the following would process
it and save the data to a file called `ipconfig` in the current working
directory.
```
... | nmdb-import-ipconfig --device-id workstation ipconfig.txt --pipe
```
