DESCRIPTION
===========

Parse and import the output of a Cisco device's `show cdp neighbor detail`
command.

Because this data contains *other* device information, the device ID provided
on the command line will only be used for data explicitly associated with the
device the data was taken from.  For the other data, the tool will extract
the `Device-ID` value for those neighboring devices and associate that value
to the `device-id` in the data store.


EXAMPLE
=======

Process the target data for the device `switch` from a local file.
```
nmdb-import-show-mac-address-table --device-id switch neighbors.txt
```

Assuming `...` is some command chain which retrieves the target data from a
remote host and displays the results locally, then the following would process
it and save the data to a file called `neighbors.txt` in the current working
directory.
```
... | nmdb-import-show-mac-address-table --device-id switch neighbors.txt
```
