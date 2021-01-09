DESCRIPTION
===========

DEPRECATED: This tool will more than likely be removed, in the near future.

Fetch data leveraging a RPC connection.

For files containing the command output, the command name has spaces replaced
by an underscore (`_`) and forward slashes replaced by Unicode code point
U+2215 (`∕`).

Use the `-h` flag to find the help message with additional options.

EXAMPLES 
======== 

Connect to the `winhost` target with the account `user` and execute the
`enumprivs` rpcclient command.
``` 
nmdb-fetch-rpcclient user@winhost enumprivs
```

See Also: `rpcclient (1)`
