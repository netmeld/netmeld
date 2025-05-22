DESCRIPTION
===========

Parse and import Prowler's JSON output.  The format is as if Prowler is ran
with the `-M json` output format option.  Note that the format between
Prowler version 2 and 3 changed significantly, thus there is a option for
this tool to specify which format is being imported.

As the data file can contain information about multiple hosts, this tool
will not honor usage of the `--device-id` option.  However, the tool still
allows it to be passed, but ignored, to help facilitate automation.

PROWLER CONFIG FILE FORMAT
========
A config must have each supported prowler version as a top level key.
Currnetly these include:
- v2
- v3
- ocsf

For each supported prowler version, the required keys are:
- assessmentStartTime
- provider
- accountId
- serviceName
- checkId
- severity
- recommendation

For each key, the supported values are as follows:
- `null` which results in a null value
- a string that starts with a `.` (e.g. `.finding_info`) which defines a path into the JSON object
- a string that does not start with a `.` which will be used as a default value
- an object which will be handled as a special case

A special case object must have one of the following keys
- concat
- join
- filter
- json

The `concat` key expects an array as its associated value. Each value will be concatenated.
Any of the valid values are supported (index strings, raw strings, or more special cases).

The `join` key expects an object as its associated value. It requires the following keys:
- *source-* a valid index value that returns an array
- *join_str-* The string to join on

The `filter` key expects an object as its associated value. It requires the following keys:
- *source-* A valid index value that returns an array
- *regex-* A regular expression to attempt to match each string, on success, the result will be included and joined
- *join_str-* The string to join on

The `json` key returns a json object instead of a string.
This currently is not supported in the database, but at some point a `JSONB` field may be added
to the table that allows any additional information to be assembled.


EXAMPLES
========

Process the target data contained in the file `checks.json` (defaults to
Prowler v3 JSON formatting).
```
nmdb-import-prowler checks.json
```

Assuming `...` is some command chain which retrieves the target data from a
remote host and displays the results locally, then the following would process
it and save the data to a file called `checks.json` in the current working
directory.
```
... | nmdb-import-prowler checks.json --pipe
```

See Also: `https://github.com/prowler-cloud/prowler`
