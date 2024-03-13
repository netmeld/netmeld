DESCRIPTION
===========

Parse and import the output from the `systeminfo` command
on modern Windows systems.

EXAMPLES
========

Process the target data for the device `workstation` from a local file.
```
nmdb-import-systeminfo --device-id workstation systeminfo.txt
```

Assuming `...` is some command chain which retrieves the target data from a
remote host and displays the results locally, then the following would
process it and save the data to a file called `systeminfo.txt` in the current
working directory.
```
... | nmdb-import-systeminfo --pipe --device-id workstation systeminfo.txt
```
TROUBLESHOOTING
========

There is trouble with the actual output from systeminfo. It is unicode 16 little endian with crlf so you can convert it with `iconv -f UTF-16 -t US-ASCII ~/systeminfo.txt -o ~/systeminfoascii.txt` and then I had to convert it using `uni2ascii` and then I had to remove all the '\r' with `cat systeminfoascii2.txt | tr -d '\r' > ~/systeminfoascii3.txt` and then I had to remove the first line which was a /n `cat ~/systeminfoascii.txt | tail -n +2 > systeminfoascii2.txt`
