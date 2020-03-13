DESCRIPTION
===========

Import the contextual information about a command that was captured by `clw`.
In cases where `clw` was wrapping `nmap` or `ping`,
`nmdb-import-clw` will also call `nmdb-import-nmap` or `nmdb-import-ping`
respectively to import those results.


Currently supported arguments:
+ `--db-name` 
+ `--verbosity` 
+ `--tool-run-id` 
+ `--tool-run-metadata` 
+ `--device-id` 

EXAMPLES
========
Import toolname_timestamp_uuid context into database.
```
nmdb-import-clw ~/.netmeld/clw/toolname_timestamp_uuid
```

To import an entire `clw` directory of tool runs en-mass (such as importing
multiple laptops into a single database after returning from an assessment):
```
for folder in `/bin/ls -1 ~/.netmeld/clw`; do
    nmdb-import-clw "$folder"
done
```

See Also: `clw (1)`
