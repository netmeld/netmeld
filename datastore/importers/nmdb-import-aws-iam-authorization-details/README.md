DESCRIPTION
===========

Parse and import the JSON output of the AWS
`aws iam` command.

As the data file can contain information about multiple hosts, this tool
will not honor usage of the `--device-id` option.  However, the tool still
allows it to be passed, but ignored, to help facilitate automation.

EXAMPLES
========
```
nmdb-import-aws-iam-authorization-details data.json
```

Assuming `...` is some command chain which retrieves the target data and
displays the results locally, then the following would process it and save
the data to a file called `data.json` in the current working directory.
```
... | nmdb-import-aws-iam-authorization-details data.json --pipe
```