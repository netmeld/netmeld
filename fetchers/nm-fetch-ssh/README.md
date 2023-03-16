DESCRIPTION
===========

Fetch data leveraging an SSH connection.

For files containing the command output, the command name has spaces replaced
by an underscore (`_`) and forward slashes replaced by Unicode code point
U+2215 (`∕`).


EXAMPLES
========

Connect to the `linhost` target with the account `user` and execute the `ls`
command.
```
nmdb-fetch-ssh user@linhost ls
```

Similar to above, except use a file with a list of commands to execute
```
nmdb-fetch-ssh --infile commands-linux.txt user@linhost
```
