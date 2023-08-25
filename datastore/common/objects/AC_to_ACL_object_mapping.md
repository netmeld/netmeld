# General
- Background
  - The ACL objects were designed to explicitly support network device
    based access control lists.
  - The AC objects were designed to support multiple access control
    concepts encountered.  This includes host device based firewall rules,
    network device based access control lists, etc.
- Notes
  - It appears the AC objects piece together data known from the config
    files prior to entry to the DB (e.g., rules mapped to inerfaces).  The
    ACL objects appear to keep the data separated and re-stitch the data
    inside the DB.
  - It appears the AC objects ignore explicitly recording what the ACL
    objects define as namespace for rules.
    - The ACL objects appear to keep the data.  This is potentially
      unwanted.  Device specific behaviour should not be present in the DB,
      it should be handled in the ingest tool prior to DB entry.
      - For example, JunOS devices take the most specific namespace that
        matches; ignoring others.
        - Speculation: This may be why artificial values for priority had
          to be added to the rule values.
    - Limited testing was performed during AC developoment to assert the
      values in the namespace (i.e., IP net sets) in question is unique per
      config.
      - This may not be accurate for all devices though.
- Current thoughts
  - Both appear to have a nugget the other doesn't, however it is unclear
    if those "nuggets" are useful.  Rather than get rid of either yet,
    herein provides a mapping between the two that is implemented at an
    object level (i.e., so using either will populate the other's DB tables
    automatically).

# AC Objects
- AcBook
  - std::string      id
  - std::string      name
  - std::set<TData>  data
- AcNetworkBook
  - inherited from AcBook
- AcServiceBook
  - inherited from AcBook
- AcRule
  - size\_t                   id {0}
  - std::string               srcId
  - std::vector<std::string>  srcs
  - std::vector<std::string>  srcIfaces
  - std::string               dstId
  - std::vector<std::string>  dsts
  - std::vector<std::string>  dstIfaces
  - std::vector<std::string>  services
  - std::vector<std::string>  actions
  - std::string               description
  - bool enabled {true}

## Relevant Methods
- AcBook
  - setId(const std::string&)
  - setName(const std::string&)
  - addData(const TData&)
  - addData(const std::set<TData>&)
  - removeData(const TData&)
- AcNetworkBook
  - inherited from AcBook
- AcServiceBook
  - inherited from AcBook
- ACRule
  - setRuleId(const size\_t)
  - setRuleDescription(const std::string&)
  - setSrcId(const std::string&)
  - addSrc(const std::string&)
  - addSrcIface(const std::string&)
  - setDstId(const std::string&)
  - addDst(const std::string&)
  - addDstIface(const std::string&)
  - addAction(const std::string&)
  - addService(const std::string&)
  - enable()
  - disable()

## Object Details
- AcRule: Object containing details of the access control rule.
- AcNetworkBook: Object containing details of network books.  Particularly
  in the case of ACLs, this contains the alias leveraged to reference
  multiple networks.
- AcServiceBook: Object containing details of service books.  Particularly
  in the case of ACLs, this contains the alias leveraged to reference
  multiple protocols and ports.
  - Service data is the format of
    `protocol:source_ports:destination_ports`.

# ACL Objects
- AclIpNetSet
  - std::string ns
  - std::string id
  - std::vector<IpNetwork> ipNets
  - std::vector<std::string> hostnames
  - std::vector<std::tuple<std::string, std::string>> includedIds
- AclPortSet
  - std::string id
  - std::vector<PortRange> portRanges
  - std::vector<std::string> includedIds
- AclService
  - std::string id
  - std::string protocol
  - std::vector<PortRange> srcPortRanges
  - std::vector<PortRange> dstPortRanges
  - std::vector<std::string> includedIds
- AclZone
	- std::string id
	- std::vector<std::string> ifaces
	- std::vector<std::string> includedIds
- AclRule
  - size\_t priority
	- std::string action
	- std::string incomingZoneId
	- std::string outgoingZoneId
	- std::string srcIpNetSetNamespace
	- std::string srcIpNetSetId
	- std::string dstIpNetSetNamespace
	- std::string dstIpNetSetId
	- std::string description
- AclRulePort
  - inherited from AclRule
  - std::string protocol
  - std::string srcPortSetId
  - std::string dstPortSetId
- AclRuleService
  - inherited from AclRule
  - std::string serviceId

# Relevant Methods
- AclIpNetSet
  - setId(const std::string&, const std::string& = "")
  - addIpNet(const IpNetwork&)
  - addHostname(const std::string&)
  - addIncludedId(const std::string&, const std::string& = "")
- AclPortSet
  - setId(const std::string&)
  - addIncludedId(const std::string&)
  - commented out functions; needed to be usable
    - //addPortRange(const PortRange&)
- AclService
  - setId(const std::string&)
  - setProtocol(const std::string&)
  - addSrcPortRange(const PortRange&)
  - addDstPortRange(const PortRange&)
  - addIncludedId(const std::string&)
- AclZone
  - setId(const std::string&)
  - addIface(const std::string&)
  - addIncludedId(const std::string&)
- AclRule
  - setPriority(size\_t)
  - setAction(const std::string&)
  - setIncomingZoneId(const std::string&)
  - setOutgoingZoneId(const std::string&)
  - setSrcIpNetSetId(const std::string&, const std::string& = "")
  - setDstIpNetSetId(const std::string&, const std::string& = "")
  - setDescription(const std::string&)
- AclRulePort
  - inherited from AclRule
  - setProtocol(const std::string&)
  - commented out functions; needed to be usable
    - //setSrcPortSetId(const std::string&)
    - //setDstPortSetId(const std::string&)
- AclRuleService
  - inherited from AclRule
  - setServiceId(const std::string&)

# Map AC to or from ACL
Object alias identifiers are in parentheses next to the object name.

## Create ACL objects from AC objects
- AcRule (ACR)
- AcNetworkBook (ACNB)
- AcServiceBook (ACSB)

- AclIpNetSet (ACLINS)
  - ACLINS.setId(ACNB.name, ACNB.id)
  - ACLINS.addIpNet(ACNB.data)
    - The ACL objects differentiate between a subnet and FQDN.  So the AC
      object data has to be binned between the two.
  - ACLINS.addHostname(ACNB.data)
    - See ACLINS.addIpNet().
  - ACLINS.addIncludedId()
    - The AC objects do not have these.  They are resolved prior to save
      via nmdu::expanded() as it's cheaper/easier to do it pre-DB.  There
      is an associated view with suffix "\_flattened", however it is now
      more a safety net in case someone forgets to use the expanded
      function.
- AclService (ACLS)
  - ACLS.setId(ACSB.name)
  - ACLS.setProtocol(ACSB.data--1st part)
    - The data object is a set of strings.  They are a tuple with values
      separated by a colon.  For example, the data is the format of
      `protocol:source_ports:destination_ports`.
  - ACLS.addSrcPortRange(ACSB.data--2nd part)
    - See ACLS.setProtocol().
  - ACLS.addDstPortRange(ACSB.data--3rd part)
    - See ACLS.setProtocol().
  - ACLS.addIncludedId()
    - See ACLINS.addIncludedId().
- AclPortSet (ACLPS): not used, unclear why
  - ACLPS.setId(ACSB.name)
  - ACLPS.addPortRange(ACSB.data--2nd part)
  - ACLPS.addIncludedId()
    - See ACLINS.addIncludedId().
- AclZone (ACLZ)
  - ACLZ.setId(ACR.srcs|ACR.dsts)
  - ACLZ.addIface(ACR.srcIfaces|ACR.dstIfaces)
  - ACLZ.addIncludedId()
    - The AC objects do not have these.  Unclear if zone nesting is even
      possible (beyond the global zone, of "any" for example).
- AclRule (ACLR): not used, base class
  - See derived classes.
- AclRuleService (ACLRS)
  - ACLRS.setPriority(ACR.id)
  - ACLRS.setAction(ACR.actions)
    - The ACL objects appear to only record allow/block actions.  The AC
      objects record more than just allow/block actions.
  - ACLRS.setIncomingZoneId(ACR.srcId)
  - ACLRS.setOutgoingZoneId(ACR.dstId)
  - ACLRS.setSrcIpNetSetId(ACR.srcs, "global")
    - There is not a namespace value for AcRule.  There is one in
      AcNetworkBook.  From limited testing where this was applicable,
      device set names appeared to be required to be unique for the entire
      config.
  - ACLRS.setDstIpNetSetId(ACR.dsts, "global")
    - See ACLRS.setSrcIpNetSetId().
  - ACLRS.setDescription(ACR.description)
  - ACLRS.setServiceId(ACR.services)
- AclRulePort (ACLRP): not used, unclear why
  - ACLRP.setPriority(ACR.id)
  - ACLRP.setAction(ACR.actions)
    - See ACLRS.setAction().
  - ACLRP.setIncomingZoneId(ACR.srcId)
  - ACLRP.setOutgoingZoneId(ACR.dstId)
  - ACLRP.setSrcIpNetSetId(ACR.srcs)
  - ACLRP.setDstIpNetSetId(ACR.dsts)
  - ACLRP.setDescription(ACR.description)
  - ACLRP.setSrcPortSetId(?)
    - Unclear; yet to see a rule that specifies only ports not referencable
      by a singular service set id.
  - ACLRP.setDstPortSetId(?)
    - See ACLRP.setSrcPortSetId(?)

## Create AC objects from ACL objects
- AcRule (ACR)
  - ACR.setRuleId(ACLR.priority)
  - ACR.setRuleDescription(ACLR.description)
  - ACR.setSrcId(ACLR.incomingZoneId)
  - ACR.addSrc(ACLR.srcIpNetSetId)
  - ACR.addSrcIface(ACLZ.ifaces)
    - This may require querying the DB to find what interface has been
      added for the associated zone id.  This may be impossible to automate
      via objects alone.
  - ACR.setDstId(ACLR.outgoingZoneId)
  - ACR.addDst(ACLR.dstIpNetSetId)
  - ACR.addDstIface(ACLZ.ifaces)
    - See ACR.addSrcIface().
  - ACR.addAction(ACLR.action)
  - ACR.addService(ACLRS.serviceId)
  - ACR.enable(); Always enable
    - The ACL object does not appear to allow for this concept.  The need
      was identified during AC development and usage.  ACL rules can be
      defined but disabled.
  - ACR.disable(); Don't add rule if disabled
    - See ACR.enable().
- AcNetworkBook (ACNB)
  - ACNB.setId(ACLINS.ns)
  - ACNB.setName(ACLINS.id)
  - ACNB.addData(ACLINS.ipNets|ACLINS.hostnames)
    - The ACL objects differentiate between subnets and FQDNs.
- AcServiceBook (ACSB)
  - ACSB.setName(ACLS.id)
  - ACSB.addData(ACLS.protocol + ':' + ACLS.srcPortRanges + ':' + ACLS.dstPortRanges)
    - The AC object store the three values as a tuple with values separated
      by a colon.

- AclIpNetSet (ACLINS)
- AclService (ACLS)
- AclPortSet (ACLPS)
- AclZone (ACLZ)
- AclRule (ACLR)
- AclRuleService (ACLRS)
- AclRulePort (ACLRP)
