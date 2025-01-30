DESCRIPTION
===========

Parse and import the output from the `systeminfo` command
on modern Windows systems.

The output from `systeminfo` may be in Unicode 16 little endian format with
CRLF line endings. As such, the following these steps may be needed to
convert it:

1. Convert the encoding from UTF-16 to US-ASCII: 
    `iconv -f UTF-16 -t US-ASCII ~/systeminfo.txt -o ~/systeminfoascii.txt`
2. Convert any remaining non-ASCII characters with `uni2ascii`.
3. Remove all '\r' characters: 
    `tr -d '\r' < ~/systeminfoascii.txt > ~/systeminfoascii2.txt`
4. Remove the first line (which is a newline character): 
    `tail -n +2 ~/systeminfoascii2.txt > ~/systeminfoascii3.txt`

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
