DESCRIPTION
===========

Parse and import Prowler's JSON output.  You must run Prowler with the `-M
json` output format option in order to produce the correct JSON format
(i.e., JSON lines).

As the data file can contain information about multiple hosts, this tool
will not honor usage of the `--device-id` option.  However, the tool still
allows it to be passed, but ignored, to help facilitate automation.


EXAMPLES
========

Process the target data contained in the file `checks.json`.
```
nmdb-import-prowler checks.json
```

Assuming `...` is some command chain which retrieves the target data from a
remote host and displays the results locally, then the following would process
it and save the data to a file called `checks.json` in the current working
directory.
```
... | nmdb-import-nmap checks.json --pipe
```

See Also: `https://github.com/prowler-cloud/prowler`
