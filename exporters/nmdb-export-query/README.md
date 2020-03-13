DESCRIPTION
===========

Generate data store query into formatted output.  Currently, this tool supports
exporting to a `ConTeXt` compatible format.

The tool supports automatic column width determination (e.g., one divided by
the number of columns) as well as being able to manually specify a value for
each column.  Both cases require the total value to be at most `1.0`.  If
multiple column width values are supplied at one time, they should be space
separated.  The column widths will be applied sequentially.

Current supported arguments:
+ `--query` or `-q`, which takes a query to convert to ConTeXt table
+ `--columnWidth` or `-w` gives the specific column width(s) to use, which must total to 1.0


EXAMPLES
========

The following examples will produce the exact same output:

```
nmdb-export-query -q "select * from ip_addrs"
nmdb-export-query -w .5 -w .5 -q "select * from ip_addrs"
nmdb-export-query -w .5 .5 -q "select * from ip_addrs"
```

