DESCRIPTION
===========

Parse and import the output of the `ping -n` or `ping6 -n` commands.


EXAMPLES
========

Process the target data from a local file.
```
nmdb-import-ping ping.txt
```

Assuming `...` is some command chain which retrieves the target data from a
remote host and displays the results locally, then the following would process
it and save the data to a file called `pipe.txt` in the current working
directory.
```
... | nmdb-import-ping ping.txt --pipe
```
