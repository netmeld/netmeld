DESCRIPTION
===========

Manually insert device information into the Netmeld database.
Though many of the options can be given independently, providing some within
one command will cause different logic to execute.  So it is best to chain as
many options as possible together for a device insertion.


EXAMPLES
========

Insert a device named `workstation`.
```
nmdb-insert-device --device-id workstation
```

Same, but define an interface, MAC address, IP address, and subnet for the
device as well.  Note, if we do not provide the subnet size it will assume
a default of a `/32`.
```
nmdb-insert-device --device-id workstation --interface eth0 \
    --mac-addr 01:02:03:04:05:06 --ip-addr 1.2.3.4/24
```

Note, that the prior will **NOT** result in the same results as the following
and the following should be considered not preferred.  Effectively, the issue
is there is no way to know to tie the information together when it is provided
in multiple tool runs.
```
nmdb-insert-device --device-id workstation --interface eth0
nmdb-insert-device --device-id workstation --mac-addr 01:02:03:04:05:06
nmdb-insert-device --device-id workstation --ip-addr 1.2.3.4/24
```

Insert a device named `switch` with an interface that, though connected, is
not responding or reachable.
```
nmdb-insert-device --device-id switch --interface 'gi0/0' --responding false
```

Insert a virtual device named `vm01` and associate it with the VM host machine
called `metal01`.
```
nmdb-insert-device --device-id vm01 --vm-host-device-id metal01
```
