DESCRIPTION
===========

The playbook provides a set of tools to aid an analyst in various
aspects of network scanning type activities for a system assessment.
The behavior of the majority of these tools is around configuration and
orchestration rather than doing the actual work of scanning.

![](docs/netmeld-playbook.png)


PLAYBOOK FUNDAMENTALS
=====================

INTRA-NETWORK VS INTER-NETWORK
------------------------------

Intra-network scans are constrained to the broadcast domains
of directly connected networks (physical or VLAN).
Routers are not used during intra-network scans.
Intra-network scans typically provide the best data about
active hosts (ARP is almost never blocked) and
network reachable services (only host-based firewall rules are encountered).
Intra-network scans provide one good set of data fairly quickly.

Inter-network scans will try to route through various routers or firewalls
attempting to reach devices and services in other networks.
Inter-network scans frequently must content with restrictive firewalls,
load-balancers, and other network devices that block or shape traffic.
Inter-network scans provide another good set of data more slowly
that can then be compared to intra-network scans to better validate
and characterize the performance of network security devices.


STAGES
------

A stage is a physical configuration that requires a human assessor to change
the physical network connections or location of the assessment computer.
Within a single stage, the assessment computer can run scans in parallel
or automatically start the next scan in a series.

For example:

* **In stage 1**, assume that the assessment computer is connected
  to a trunk port on switch 1.  The playbook can automatically run scans,
  in parallel or in series, on each VLAN in that switch 1 trunk.
* Between stage 1 and stage 2, the assessor needs to physically disconnect
  the assessment computer from switch 1, move it to another equipment rack,
  and physically connect it to a trunk port on switch 2.
* **In stage 2**, the playbook can automatically run scans,
  in parallel or in series, on each VLAN in that switch 2 trunk.
* Between stage 2 and stage 3, the assessor needs to physically disconnect
  the assessment computer from switch 2, move it to the location of server 1,
  disconnect server 1 from the network, and connect the assessment computer
  in place of server 1.
* **In stage 3**, the playbook can automatically run scans,
  in parallel or in series, from the network perspective of server 1.


PHASES
------

A phase is a logical grouping of commands which will execute in series
or parallel within a stage.  Activities within a phase may feed data,
via the data store, into other phases for that stage.  Phases are currently
limited to being used for containing optional units of work within a stage.
As such, not all commands are located under a phase (e.g. interface
set up and tear down).  Intensity and complexity of activities
typically increases through phase progression.


COMMANDS
--------

A command is a singular unit of work within the playbook.


PLAYBOOK WORKFLOW
=================

The following is an overview of the workflow and tools involved to
leverage the playbook for scanning activities.

![](docs/netmeld-playbook-workflow.png)


PREPARING THE PLAYBOOK
----------------------

Typically, a script is created which contains the logic for preparing
the playbook.  The aim is to make it simple and understandable.
It is suggested to create a singular script per stage.  See
[](playbook-script.sh) for an example and the individual tools for
more details.


RULES OF ENGAGEMENT
-------------------
Currently, the `psql` tool is utilized to manipulated the Rules of
Engagement (ROE) table within the data store.

Assume that the ROE authorizes the assessment team to target the IP address
space 10.1.1.0/24 and 192.168.0.0/16 at the customer site;
however, the subnet 192.168.2.0/24 is off-limits for any number of reasons.
```
#!/bin/bash

DB_NAME=site

psql ${DB_NAME} -c "INSERT INTO playbook_roe_ip_nets VALUES ('10.1.1.0/24', true)"
psql ${DB_NAME} -c "INSERT INTO playbook_roe_ip_nets VALUES ('192.168.0.0/16', true)"
psql ${DB_NAME} -c "INSERT INTO playbook_roe_ip_nets VALUES ('192.168.2.0/24', false)"
```
A value of `true` in the second column means that the addresses are in-scope,
while a value of `false` means that the addresses are out-of-scope.
The values in the `playbook_roe_ip_nets` database table are used to generate
target lists and exclude lists for tools such as Nmap.
