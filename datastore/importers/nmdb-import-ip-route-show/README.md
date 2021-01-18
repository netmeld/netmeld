DESCRIPTION
===========

Parse and import the output from the `ip route show` command on modern Linux
systems.


EXAMPLES
========

Process the target data for the device `workstation` from a local file
```
nmdb-import-ip-route-show --device-id workstation ip_routes.txt
```

Assuming `...` is some command chain which retrieves the target data from a
remote host and displays the results locally, then the following would process
it and save the data to a file called `ip_routes.txt` in the current working
directory.
```
... | nmdb-import-ip-route-show --pipe --device-id workstation ip_routes.txt
```
