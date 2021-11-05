FIREWALL ACCESS CONTROL LISTS (ACLs)
====================================

Due to the different approaches to firewall rules across devices and vendors,
a more complex, but consistent, approach was needed to the ACL schemas.

For each of the object types in a firewall rule, the general structure is:

* A ``_bases`` table that contains the core primary key
  information needed for foreign keys in other related tables.
* An ``_includes`` table that supports nested sets
  (such as a list that contains another named list).
* One or more data type tables that contain the leaf data for elements of that set.


ZONES
-----

Zones define named sets of network interfaces
(such as ``trusted``, ``untrusted``, and ``dmz``).

This is a natural fit for zone-based firewalls, such as Juniper devices.
For interface-based firewall rules, such as can be written on Linux,
a trivial mapping is the create a zone for each interface
(such as a zone ``eth0`` that contains interface ``eth0``).
For firewall rules with no zone or interface component,
``any`` zones can be added that include all interfaces on the device.

The ``device_acl_zones`` view combines data from:

* ``raw_device_acl_zones_bases``
* ``raw_device_acl_zones_interfaces``: Network interfaces in a zone.
* ``raw_device_acl_zones_includes``: Other zones included in a zone.


IP NETWORKS
-----------

IP networks define named sets of IP addresses, IP CIDRs, or hostnames.

The ``device_acl_ip_nets`` view combines data from:

* ``raw_device_acl_ip_nets_bases``
* ``raw_device_acl_ip_nets_ip_nets``: IP addresses or CIDRs in a network set.
* ``raw_device_acl_ip_nets_hostnames``: FQDNs or hostnames in a network set.
* ``raw_device_acl_ip_nets_includes``: Other network sets included in a network set.


PORTS
-----

Ports define named sets of port ranges (such as ``[1024,65535]``).
Single ports can be represented as a one port range (such as ``[22,22]``).
In cases where an unnamed port or port range is used in firewall rules,
a trivial mapping is to create a port set for the port or range
(such as a port set named ``1-1024`` that contains port range ``[1,1024]``).

The ``device_acl_ports`` view combines data from:

* ``raw_device_acl_ports_bases``
* ``raw_device_acl_ports_ports``: Port ranges in a port set.
* ``raw_device_acl_ports_includes``: Other port sets included in a port set.


SERVICES
--------

Services define named sets that bind together protocols (such as ``tcp`` or ``udp``)
with source and destination port ranges (such as ``[1024,65535]``) for that service.

The ``device_acl_services`` view combines data from:

* ``raw_device_acl_services_bases``
* ``raw_device_acl_services_protocols``: Protocols in a service.
* ``raw_device_acl_services_ports``: Port ranges in a service.
* ``raw_device_acl_services_includes``: Other services included in a service.


RULES
-----

There are two firewall rule schemas than can accommodate the currently-known
rule types: port-based rules and service-based rules.
Both rule schemas contain incoming and outgoing zones and IP networks.
The differences between the rule tables are for different styles
of representing ports and services in the rules.

Port-based rules specify a protocol, source port set,
and destination port set in the rule.
The port-based rule schema tables are appropriate for devices
that use named port sets or explicit port numbers or ranges
in the source and destination port fields of firewall rules.

Service-based rules specify a service set.
The service-based rule schema tables are appropriate for devices
that use named service objects in firewall rules.

* ``device_acl_rules_ports`` (view for ``raw_device_acl_rules_ports``)
  contains references to tables or views for:
    + Zones (both incoming and outgoing)
    + IP Networks (both source and destination)
    + Ports (both source and destination)
* ``device_acl_rules_services`` (view for ``raw_device_acl_rules_services``)
  contains references to:
    + Zones (both incoming and outgoing)
    + IP Networks (both source and destination)
    + Services
* ``device_acl_rules`` expands and aggregates ``device_acl_rules_ports`` and
  ``device_acl_rules_services`` to a normalized set of fields.

The final ``device_acl_rules`` view expands zones to interfaces,
networks to IP addresses and CIDRs, and service and port sets to port ranges.
The expanded fields are intended for then comparing to route paths
through a network in order to match a network packet against
incoming and outgoing interfaces on a firewall, protocol,
source and destination IP addresses, and source and destination port numbers
against matching firewall rules.

