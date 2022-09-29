DESCRIPTION
===========

Parse and import the JSON output of the AWS CLIv2
`aws ec2 describe-instances` command.

As the data file can contain information about multiple hosts, this tool
will not honor usage of the `--device-id` option.  However, the tool still
allows it to be passed, but ignored, to help facilitate automation.


EXAMPLES
========

Process the target data from a local file.
```
nmdb-import-aws-ec2-describe-vpcs data.json
```

Assuming `...` is some command chain which retrieves the target data and
displays the results locally, then the following would process it and save
the data to a file called `data.json` in the current working directory.
```
... | nmdb-import-aws-ec2-describe-vpcs data.json --pipe
```
