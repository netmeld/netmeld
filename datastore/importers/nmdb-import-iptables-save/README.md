DESCRIPTION
===========

Parse and import the output from the `iptables-save -c` or `iptables-save`
command on modern Linux systems.


EXAMPLES
========

Process the target data for the device `workstation` from a local file
```
nmdb-iptables-save --device-id workstation iptables.save
```

Assuming `...` is some command chain which retrieves the target data from a
remote host and displays the results locally, then the following would process
it and save the data to a file called `iptables.save` in the current working
directory.
```
... | nmdb-iptables-save --device-id workstation iptables.save --pipe
```
