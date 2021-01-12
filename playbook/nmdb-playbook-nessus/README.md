DESCRIPTION
===========

NOTE: This tool probably won't work with newer versions of Nessus since they
changed the way to interact with the tool and this has not been updated since
that change.

This tool is utilized to help automate Nessus scans.  The tool allows
configuration and automation of several of the Nessus interactions via the
command line instead of the Nessus web interface.

Currently the tool is a Bash script and it's configuration is located in the
script itself.
The tool requires five general configuration settings that should be updated
before execution.
```
# Nessusd host ip and port
nessus_ip="https://localhost:8834";

# Scan configuration
# Array entries are associated on a per scan basis for all scan_* variables
scan_name=("First Scan Name"
        "Second Scan Name"
        "Third Scan Name"
        "Fourth Scan Name");

# Use nessus policy name
scan_policy=("Kitchen Sink + VMware Compliance - Linux"
             "Kitchen Sink + VMware Compliance - Linux"
             "Kitchen Sink + VMware Compliance - Windows"
             "Kitchen Sink + VMware Compliance - Windows");

# Uses same formatting as nessus target list
scan_targets=("127.0.0.0/31"
              "127.0.0.0, 127.0.0.1"
              "127.0.0.0-127.0.0.1"
              "localhost");

# Report configuration, used for all scans
report_formats=("nessus" "pdf");
chapters="vuln_hosts_summary;vuln_by_plugin;_vuln_by_host;remediations;compliance_exec;compliance;";
```

Typically, one should only have to edit the configuration entries of `nessus_ip`,
`scan_name`, `scan_policy`, and `scan_targets` as the defaults for the rest are
generally wanted.

In the above sample, there would be four total nessus scans ran sequentially.
Each scan would generate both a `nessus` and `pdf` report to the directory from
where the scan was executed.  The file names are of the form
`nessus-report-SCAN_NAME-DTS.FORMAT`, where `SCAN_NAME` is the name of the
current scan, `DTS` is a date-time-stamp of when the report was downloaded, and
`FORMAT` is the report file format.
