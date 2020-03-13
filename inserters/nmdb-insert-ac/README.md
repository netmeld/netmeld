DESCRIPTION
===========

Manually insert access control related information into the Netmeld data store.

Since `nmdb-insert-ac` is importing information about a device's access control
rules, the `--device-id` option is required at a minimum. 

Options are split into three categories: Access control rules, Network book,
and Service book.  The options needed for each are split and documented in the
tool's `--help` output.
While options for all three can be specified simultaneously, the tool will
*not* process all at the same time.  It will only process the first successful
match.


EXAMPLES
========
Network book insertion:
```
nmdb-insert-ac --device-id ac-dev \
	--nb-id trust --nb-name admin-nets --nb-data 10.0.9.0/24
nmdb-insert-ac --device-id ac-dev \
	--nb-name internet-edge --nb-data 10.0.0.0/24
```

Service book insertion:
```
nmdb-insert-ac --device-id ac-dev \
	--sb-name web --sb-data tcp:any:80,443
```

Rule insertion:
```
nmdb-insert-ac --device-id ac-dev \
	--src-id trust --src admin-nets \
	--dst internet-edge \
	--service web --action deny,log \
  --description "Prevent and log control plane web access"
```
