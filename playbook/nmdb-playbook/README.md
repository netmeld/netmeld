DESCRIPTION
===========

The `nmdb-playbook` tool is utilized to perform the actual playbook runs.
It has several command line options and a configuration file which tune the
behavior of this tool, however most have default values set.
The configuration file explains the configuration file only options (see
`--config-file` for default path).

In general, the playbook is broken into stages, phases, and commands as
described in the Playbook module documentation.
It is important to note, the `--stage` and `--phase` options
specify which to keep, while the `--exclude-command` option
specifies which to exclude.  All three can be used at the same time.


GUI OR HEADLESS
---------------

Though the tool is a CLI tool, during scanning it will attempt to display the
data in another interface to distinguish this tools output apart from other
tooling output.  In the default mode it will attempt to start another `xterm`
and in the headless mode it will open a `tmux` window.  The behavior described
herein with regards to *window* can mean either of these cases as it is simply
a matter of how the tooling is called.


INTERACTIVE OR NOT
------------------

While the majority of the activities are automated, there are points during a
stage where the user is required to provide some acknowledgment before the
playbook will continue.  To date, this is to facilitate running manual,
additional scans concurrently with the automated playbook activities.

Specifically, the tool starts a window with a notice to
"Close this window when manual testing is complete."
The tool will not deconfigure network interfaces until the playbook scans are
complete and the window with the notice is closed.  To ignore this behavior,
the `--no-prompt` option is provided.  It will automatically deconfigure
network interfaces as soon as the relevant playbook scans are complete.
However, if it is used in conjunction with the `--script` option, a script will
be executed in lieu of the manual testing before the network interface is
reconfigured.


EXAMPLES
========

For reference, the following are examples of the various commands being
executed during the various phases of a stage from both an intra and inter
network perspective.

General workflow for an intra network playbook execution.
![](../docs/netmeld-playbook-intra-network-workflow.png)

General workflow for an inter network playbook execution.
![](../docs/netmeld-playbook-inter-network-workflow.png)

Perform a dry-run that displays what the playbook is going to do.  Review this
output as a sanity check.
```
nmdb-playbook --intra-network
nmdb-playbook --inter-network
```

Execute the playbook on the network.
```
nmdb-playbook --intra-network --execute
nmdb-playbook --inter-network --execute
```

Execute a specific stage of the playbook on the network.
```
nmdb-playbook --intra-network --stage 1 --execute
nmdb-playbook --inter-network --stage 1 5 23 --execute
```

Execute a specific phase of stages of the playbook on the network.
```
nmdb-playbook --intra-network --stage 1 8 10 --phase 2 3 --execute
nmdb-playbook --inter-network --phase 1 --execute
```

Exclude specific tests from the playbook on the network.
```
nmdb-playbook --intra-network --exclude-command 1 3 8 9 10
```

From the dry-run output one can identify test ID numbers which can be utilized
to exclude the test from a playbook run.  For example, exclude any test
which uses the `nmap` tool.
```
nmdb-playbook --intra-network \
    --exclude-command $(nmdb-playbook --intra-network | grep nmap \
                        | cut -d ':' -f 1 | paste -sd ' ' -)
```

Execute a script instead of manual testing between stages of the playbook on
the network.
```
nmdb-playbook --inter-network --no-prompt --script post-run-script
```
