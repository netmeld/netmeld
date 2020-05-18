// =============================================================================
// Copyright 2017 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// =============================================================================
// Maintained by Sandia National Laboratories <Netmeld@sandia.gov>
// =============================================================================

#include "Parser.hpp"


// =============================================================================
// Parser logic
// =============================================================================
Parser::Parser() : Parser::base_type(start)
{
  using nmdsic::token;
  using nmdsic::tokens;
  using nmdsic::indent;

  start =
    config
      [pnx::bind([&](){d.devInfo.setVendor("cisco");}),
       qi::_val = pnx::bind(&Parser::getData, this)]
    ;

  // Unknown where belongs
  // ip vrrp-extended (vrrrpv3 for nexus?)

  config =
    *(  (qi::lit("no cdp") >> (qi::lit("run") | qi::lit("enable")) > qi::eol)
           [pnx::bind(&Parser::globalCdpEnabled, this) = false]

      | ((qi::string("PIX") | qi::string("ASA"))
         >> qi::lit("Version") > *token > qi::eol)
          [pnx::bind(&Parser::globalCdpEnabled, this) = false]

        // TODO does it make since to collapse with other calls?
      | (qi::lit("spanning-tree portfast") >>
          (  qi::lit("bpduguard")
               [pnx::bind(&Parser::globalBpduGuardEnabled, this) = true]
           | qi::lit("bpdufilter")
               [pnx::bind(&Parser::globalBpduFilterEnabled, this) = true]
          ) > *token > qi::eol)

      | (qi::lit("aaa ") >> tokens >> qi::eol)
            [pnx::bind(&Parser::deviceAaaAdd, this, qi::_1)]

        // TODO These are more opportunistic right? Or guarenteed?
        // TODO doesn't handle sntp, vrf, or alias
        // TODO s?ntp server [vrf vrf-name] (ip|alias)
      | (qi::lit("ntp server") >> ipAddr > qi::eol)
           [pnx::bind(&Parser::serviceAddNtp, this, qi::_1)]
        // TODO doesn't catch when alias
      | (qi::lit("snmp-server host") >> ipAddr > -tokens > qi::eol)
           [pnx::bind(&Parser::serviceAddSnmp, this, qi::_1)]
        // TODO doesn't catch when it is a block
        // TODO radius-server host source-interface iface
      | (qi::lit("radius-server host") >> ipAddr > -qi::omit[+token] > qi::eol)
           [pnx::bind(&Parser::serviceAddRadius, this, qi::_1)]
        // TODO doesn't handle vrf
        // TODO ip name-server [vrf vrf-name] ip1
      | (qi::lit("ip name-server") > +ipAddr > qi::eol)
           [pnx::bind(&Parser::serviceAddDns, this, qi::_1)]
      | (qi::lit("logging server") > ipAddr > -qi::omit[+token] > qi::eol)
           [pnx::bind(&Parser::serviceAddSyslog, this, qi::_1)]

      | domainData
      | (interface)
          [pnx::bind(&Parser::vlanAddIfaceData, this)]
      | route
      | vlan
          [pnx::bind(&Parser::vlanAdd, this, qi::_1)]
      | networkBooks
      | serviceBooks
      | accessPolicyRelated

      // ignore the rest
      | (qi::omit[+token > -qi::eol])
      | (qi::omit[+qi::eol])
    )
    ;

  domainData =
    ( ((qi::lit("switchname") | qi::lit("hostname")) > domainName > qi::eol)
          [pnx::bind([&](const std::string& val)
                     {d.devInfo.setDeviceId(val);}, qi::_1)]
     | (qi::lit("ip domain-name") > domainName > qi::eol)
          [pnx::bind(&Parser::unsup, this, "ip domain-name")]
     | (qi::lit("domain-name") >> domainName >> qi::eol)
          [pnx::bind(&Parser::unsup, this, "domain-name")]
    )
    ;

  // [ip|ipv6] route dstNet nextHop {administrative_distance} {permanent}
  //   dstNet  == [ip/prefix | ip mask]
  //   nextHop == [ip/prefix | ip | iface]
  route = 
    (qi::lit("ipv6") | qi::lit("ip")) >> qi::lit("route") >>
       // ip_mask ip
    (  (ipAddr >> ipAddr >> ipAddr)
         [pnx::bind(&nmco::IpAddress::setNetmask, &qi::_1, qi::_2),
          pnx::bind(&Parser::routeAddIp, this, qi::_1, qi::_3)]
       // ip_mask iface
     | (ipAddr >> ipAddr >>
        (!(qi::digit | qi::lit("permanent")) >> token))
         [pnx::bind(&nmco::IpAddress::setNetmask, &qi::_1, qi::_2),
          pnx::bind(&Parser::routeAddIface, this, qi::_1, qi::_3)]
       // ip ip
     | (ipAddr >> ipAddr)
         [pnx::bind(&Parser::routeAddIp, this, qi::_1, qi::_2)]
       // ip iface
     | (ipAddr >> token)
         [pnx::bind(&Parser::routeAddIface, this, qi::_1, qi::_2)]
    ) > -(qi::uint_ | qi::lit("permanent")) > qi::eol
    ;

  vlan =
    (qi::lit("vlan") >> qi::ushort_ >> qi::eol)
       [pnx::bind(&nmco::Vlan::setId, &qi::_val, qi::_1)] >>
    *(indent >>
      (  (qi::lit("name") >> token)
            [pnx::bind(&nmco::Vlan::setDescription, &qi::_val, qi::_1)]
       | (qi::omit[+token]) // Ignore all other settings
      ) >> qi::eol
    )
    ;

  // TODO consider moving interface to own parser
  interface =
    qi::no_skip[qi::lit("interface")] >>
    (  (token >> token > qi::eol)
          [pnx::bind(&Parser::ifaceInit, this, qi::_1 + " " + qi::_2)]
     | (token > qi::eol)
          [pnx::bind(&Parser::ifaceInit, this, qi::_1)]
    ) >>
    *(indent >>
      qi::matches[qi::lit("no")]
        [pnx::bind(&Parser::isNo, this) = qi::_1] >>
      (  (qi::lit("inherit port-profile"))
            [pnx::bind(&Parser::unsup, this, "port-profile")]
       | (qi::lit("description") >> tokens)
            [pnx::bind(&nmco::InterfaceNetwork::setDescription,
                       pnx::bind(&Parser::tgtIface, this), qi::_1)]
       | (qi::lit("shutdown"))
            [pnx::bind(&nmco::InterfaceNetwork::setState,
                       pnx::bind(&Parser::tgtIface, this),
                       pnx::bind(&Parser::isNo, this))]
       | (qi::lit("cdp enable"))
            [pnx::bind(&nmco::InterfaceNetwork::setDiscoveryProtocol,
                       pnx::bind(&Parser::tgtIface, this),
                       !pnx::bind(&Parser::isNo, this)),
             pnx::bind(&Parser::ifaceSetUpdate, this, &ifaceSpecificCdp)]

       | ((qi::lit("ipv6") | qi::lit("ip")) >> qi::lit("address") >>
          (  (ipAddr >> ipAddr)
                [pnx::bind(&nmco::IpAddress::setNetmask, &qi::_1, qi::_2),
                 pnx::bind(&nmco::InterfaceNetwork::addIpAddress,
                           pnx::bind(&Parser::tgtIface, this), qi::_1)]
           | (ipAddr)
              [pnx::bind(&nmco::InterfaceNetwork::addIpAddress,
                         pnx::bind(&Parser::tgtIface, this), qi::_1)]
           | (domainName >> ipAddr)
                [pnx::bind([&](const std::string& _alias,
                               const nmco::IpAddress& _mask)
                           {
                            nmco::IpAddress temp;
                            temp.setMask(_mask);
                            postIfaceIpAliasData.push_back(std::make_tuple(
                                  tgtIface, _alias, temp
                                  ));
                           }, qi::_1, qi::_2)]
           | (domainName)
                [pnx::bind([&](const std::string& _alias)
                           {nmco::IpAddress temp;
                            postIfaceIpAliasData.push_back(std::make_tuple(
                                  tgtIface, _alias, temp
                                  ));
                           }, qi::_1)]
          )
         )

         // TODO ip helper-address [vrf name | global] ip [redundancy vrg-name]
       | (qi::lit("ip helper-address") >> ipAddr)
            [pnx::bind(&Parser::serviceAddDhcp, this, qi::_1)]
       | (qi::lit("ip dhcp relay address") >> ipAddr)
            [pnx::bind(&Parser::serviceAddDhcp, this, qi::_1)]

       | (qi::lit("ip access-group") >> token >> token)
            [pnx::bind(&Parser::createAccessGroup, this, qi::_1, qi::_2, "")]
       | (qi::lit("service-policy") >> token >> token)
            [pnx::bind(&Parser::createServicePolicy, this, qi::_1, qi::_2)]

       | (qi::lit("nameif") >> token)
            [pnx::bind([&](const std::string& val)
                       {ifaceAliases.emplace(val, tgtIface);}, qi::_1)]

       /* START: No examples of these, cannot verify */
       // HSRP, virtual IP target for redundant network setup
         // TODO standby [ group-number ] ip [ ip-address [ secondary ]]
       | (qi::lit("standby") >> qi::int_ >> qi::lit("ip") >> ipAddr)
            [pnx::bind(&nmco::InterfaceNetwork::addIpAddress,
                       pnx::bind(&Parser::tgtIface, this), qi::_2)]
       // Capture Cisco N1000V virtual switches
       | (qi::lit("vmware vm mac") >> macAddr)
            [pnx::bind(&nmco::InterfaceNetwork::addReachableMac,
                       pnx::bind(&Parser::tgtIface, this), qi::_1)]
       // Capture ADX configs
         // TODO move with ip capture above?
       | (qi::lit("ip-address") >> ipAddr)
            [pnx::bind(&nmco::InterfaceNetwork::addIpAddress,
                       pnx::bind(&Parser::tgtIface, this), qi::_1)]
       /* END: No examples of these, cannot verify */

       | switchport
       | spanningTree

       // Ignore all other settings
       | (qi::omit[+token | &qi::eol])
      ) >> qi::eol
    )
    ;

  // TODO migrate
  switchport =
    qi::lit("switchport") >>
    (  ((qi::lit("mode") > token) | qi::string("nonegotiate"))
          [pnx::bind(&nmco::InterfaceNetwork::setSwitchportMode,
                     pnx::bind(&Parser::tgtIface, this), "L2 " + qi::_1)]

     | switchportPortSecurity
     | switchportVlan
    )
    ;
  switchportPortSecurity =
    qi::lit("port-security ") >
    (
       (qi::lit("mac-address ")
        > -qi::lit("sticky")
            [pnx::bind(&nmco::InterfaceNetwork::setPortSecurityStickyMac,
                       pnx::bind(&Parser::tgtIface, this),
                       !pnx::bind(&Parser::isNo, this))]
        > -macAddr
            [pnx::bind(&nmco::InterfaceNetwork::addPortSecurityStickyMac,
                       pnx::bind(&Parser::tgtIface, this), qi::_1)]
       )
     | (qi::lit("maximum") >> qi::ushort_ >> qi::lit("vlan") > qi::ushort_)
          [pnx::bind(&Parser::unsup, this,
            pnx::bind([&](size_t a, size_t b)
              {return "port-security maximum " + std::to_string(a) +
                      " vlan " + std::to_string(b);}, qi::_1, qi::_2))]
     | (qi::lit("maximum") >> qi::ushort_)
          [pnx::bind(&nmco::InterfaceNetwork::setPortSecurityMaxMacAddrs,
                     pnx::bind(&Parser::tgtIface, this), qi::_1)]
     | (qi::lit("violation") >> token)
          [pnx::bind(&nmco::InterfaceNetwork::setPortSecurityViolationAction,
                     pnx::bind(&Parser::tgtIface, this), qi::_1)]
     | (&qi::eol)
          [pnx::bind(&nmco::InterfaceNetwork::setPortSecurity,
                     pnx::bind(&Parser::tgtIface, this),
                     !pnx::bind(&Parser::isNo, this))]
    )
    ;
  switchportVlan =
    (  qi::string("access ")
     | qi::string("trunk ")
     | qi::string("voice ")
    ) >> ( qi::string("native ")
         | qi::string("allowed ")
         | qi::as_string[qi::attr(" ")]
        )
    >> qi::lit("vlan") > -qi::lit("add")
    > (  ((vlanRange | vlanId) % qi::lit(','))
       | (tokens)
            [pnx::bind(&Parser::unsup, this,
                       "switchport " + qi::_a + qi::_b + "vlan " + qi::_1)]
      )

    ;
  vlanRange =
    (qi::ushort_ >> qi::lit('-') >> qi::ushort_)
      [pnx::bind(&nmco::InterfaceNetwork::addVlanRange,
                 pnx::bind(&Parser::tgtIface, this),
                 qi::_1, qi::_2)]
    ;
  vlanId =
    qi::ushort_
      [pnx::bind(&nmco::InterfaceNetwork::addVlan,
                 pnx::bind(&Parser::tgtIface, this), qi::_1)]
    ;

  spanningTree =
    qi::lit("spanning-tree") >
      // TODO These have "default" values which require to know edge-port state
    (  (qi::lit("bpduguard")
         [pnx::bind(&nmco::InterfaceNetwork::setBpduGuard,
                    pnx::bind(&Parser::tgtIface, this),
                    !pnx::bind(&Parser::isNo, this)),
          pnx::bind(&Parser::ifaceSetUpdate, this, &ifaceSpecificBpduGuard)] >
        -(  qi::lit("enable")
             [pnx::bind([&](){tgtIface->setBpduGuard(true);})]
         | qi::lit("disable")
             [pnx::bind([&](){tgtIface->setBpduGuard(false);})]
        )
       )
     | (qi::lit("bpdufilter")
         [pnx::bind(&nmco::InterfaceNetwork::setBpduFilter,
                    pnx::bind(&Parser::tgtIface, this),
                    !pnx::bind(&Parser::isNo, this)),
          pnx::bind(&Parser::ifaceSetUpdate, this, &ifaceSpecificBpduFilter)] >
        -(  qi::lit("enable")
             [pnx::bind([&](){tgtIface->setBpduFilter(true);})]
         | qi::lit("disable")
             [pnx::bind([&](){tgtIface->setBpduFilter(false);})]
        )
       )
     | ((qi::lit("port type") | qi::lit("portfast")) >>
        (  qi::lit("edge")    // == portfast on
             [pnx::bind([&](){tgtIface->setPortfast(true);})]
         | qi::lit("network") // == portfast off
             [pnx::bind([&](){tgtIface->setPortfast(false);})]
         | qi::lit("normal")  // == default
        )
       )
     | (qi::lit("portfast"))
//          [pnx::bind([&](){tgtIface->setPortfast(!isNo);})]
          [pnx::bind(&nmco::InterfaceNetwork::setPortfast,
                     pnx::bind(&Parser::tgtIface, this),
                     !pnx::bind(&Parser::isNo, this))]
     | tokens
         [pnx::bind(&Parser::unsup, this, "spanning-tree " + qi::_1)]
    )
    ;

  accessPolicyRelated =
    (  (qi::lit("policy-map") >> policyMap)
     | (qi::lit("class-map") >> classMap)
     | aclRuleBook [pnx::bind(&Parser::aclRuleBookAdd, this, qi::_1)]
       // access-group ac-list {in|out} interface ifaceName
     | (qi::lit("access-group") >> token >> token >>
        qi::lit("interface") >> token)
          [//pnx::bind(&Parser::ifaceInit, this, qi::_3),
           pnx::bind(&Parser::createAccessGroup, this, qi::_1, qi::_2, qi::_3)]
    )
    ;

  policyMap =
    token [qi::_a = qi::_1] >> qi::eol >>
    *(  (indent >> qi::lit("class") >> token >> qi::eol)
          [pnx::bind(&Parser::updatePolicyMap, this, qi::_a, qi::_1)]
      | qi::omit[(+indent >> tokens >> qi::eol)]
    )
    ;

  classMap =
    qi::omit[token] >> token [qi::_a = qi::_1] >> qi::eol >>
    *(  (indent >> qi::lit("match access-group name") >> token >> qi::eol)
          [pnx::bind(&Parser::updateClassMap, this, qi::_a, qi::_1)]
      | qi::omit[(+indent >> tokens >> qi::eol)]
    )
    ;

  BOOST_SPIRIT_DEBUG_NODES(
      //(start)
      (config)
      (domainData)(route)(vlanDef)
      (policyMap)(classMap)(addressArgument)(ports)
      (interface)(switchport)(spanningTree)
      (vlan)
      (tokens)(token)
      );
}

// =============================================================================
// Parser helper methods
// =============================================================================

// Device related
void
Parser::deviceAaaAdd(const std::string& aaa)
{
  d.aaas.push_back(nmcu::trim("aaa " + aaa));
}

// Service related
// TODO can we make these more statically generated?
void
Parser::serviceAddDhcp(const nmco::IpAddress& ip)
{
  nmco::Service service {"dhcps", ip}; // match nmap output
  service.setProtocol("udp");
  service.addDstPort("67"); // port server uses
  service.addSrcPort("68"); // port client uses
  service.setServiceReason(d.devInfo.getDeviceId() + "'s config");
  // TODO technically wrong...but the Service object does the right thing
  service.setInterfaceName(tgtIface->getName());
  d.services.push_back(service);
}

void
Parser::serviceAddNtp(const nmco::IpAddress& ip)
{
  nmco::Service service {"ntp", ip};
  service.setProtocol("udp");
  service.addDstPort("123"); // same port used by client and server
  service.addSrcPort("123");
  service.setServiceReason(d.devInfo.getDeviceId() + "'s config");
  d.services.push_back(service);
}

void
Parser::serviceAddSnmp(const nmco::IpAddress& ip)
{
  nmco::Service service {"snmp", ip};
  service.setProtocol("udp");
  service.addDstPort("162"); // port manager receives on
  service.setServiceReason(d.devInfo.getDeviceId() + "'s config");
  d.services.push_back(service);
}

void
Parser::serviceAddRadius(const nmco::IpAddress& ip)
{
  nmco::Service service {"radius", ip};
  service.setProtocol("udp");
  service.addDstPort("1812"); // authentication and authorization
  service.addDstPort("1813"); // accounting
  service.setServiceReason(d.devInfo.getDeviceId() + "'s config");
  d.services.push_back(service);
}

void
Parser::serviceAddDns(const std::vector<nmco::IpAddress>& ips)
{
  for (auto& ip : ips) {
    nmco::Service service {"dns", ip};
    service.setProtocol("udp");
    service.addDstPort("53");
    service.setServiceReason(d.devInfo.getDeviceId() + "'s config");
    d.services.push_back(service);
  }
}

void
Parser::serviceAddSyslog(const nmco::IpAddress& ip)
{
  nmco::Service service {"syslog", ip};
  service.setProtocol("udp");
  service.addDstPort("514");
  service.setServiceReason(d.devInfo.getDeviceId() + "'s config");
  d.services.push_back(service);
}


// Route related
// TODO These two are more correct than the third, but we can probably collapse logic
void
Parser::routeAddIp(const nmco::IpAddress& dstNet, const nmco::IpAddress& rtrIp)
{
  nmco::Route route;
  route.setDstNet(dstNet);
  route.setRtrIp(rtrIp);

  d.routes.push_back(route);
}

void
Parser::routeAddIface(const nmco::IpAddress& dstNet, const std::string& rtrIface)
{
  nmco::Route route;
  route.setDstNet(dstNet);
  route.setIfaceName(rtrIface);

  d.routes.push_back(route);
}
//void
//Parser::routeAdd(const nmco::IpAddress& dstNet, const std::string& rtrIpStr)
//{
//  nmco::IpAddress rtrIp;
//  if ("Null0" != rtrIpStr) {
//    rtrIp = nmco::IpAddress {rtrIpStr};
//  }
//
//  nmco::Route route;
//  route.setDstNet(dstNet);
//  route.setRtrIp(rtrIp);
//
//  d.routes.push_back(route);
//}


// Interface related
void
Parser::ifaceInit(const std::string& _name)
{
  tgtIface = &d.ifaces[_name];
  tgtIface->setName(_name);
//  tgtIface->setState(true);
}

void
Parser::ifaceSetUpdate(std::set<std::string>* const set)
{
  set->insert(tgtIface->getName());
}


// Vlan related
void
Parser::vlanAdd(nmco::Vlan& vlan)
{
  d.vlans.push_back(vlan);
}

void
Parser::vlanAddIfaceData()
{
  std::string vlanPrefix {"vlan"};
  std::string ifaceName {tgtIface->getName()};
  std::string ifacePrefix {ifaceName.substr(0, vlanPrefix.size())};
  if (ifacePrefix == vlanPrefix) {
    std::istringstream iss {ifaceName.erase(0, vlanPrefix.size())};
    unsigned short id;
    iss >> id;
    for (auto ipAddr : tgtIface->getIpAddresses()) {
      nmco::Vlan vlan {id};
      vlan.setIpNet(ipAddr);
      d.vlans.push_back(vlan);
    }
  }
}


// Policy Related
void
Parser::createAccessGroup(const std::string& bookName,
                          const std::string& direction,
                          const std::string& ifaceName)
{
  auto tgtName {ifaceName};
  if (tgtName.empty()) {
    tgtName = tgtIface->getName();
  }
  appliedRuleSets[bookName] = {tgtName, direction};
}

void
Parser::createServicePolicy(const std::string& direction, const std::string& policyName)
{
  servicePolicies[tgtIface->getName()].insert({policyName, direction});
}

void
Parser::updatePolicyMap(const std::string& policyName, const std::string& className)
{
  policies[policyName].insert(className);
}

void
Parser::updateClassMap(const std::string& className, const std::string& bookName)
{
  classes[className].insert(bookName);
}

void
Parser::aclRuleBookAdd(std::pair<std::string, RuleBook>& _pair)
{
  if (_pair.first.empty()) { return; }

  auto search = d.ruleBooks.find(_pair.first);
  if (search == d.ruleBooks.end()) {
    d.ruleBooks.emplace(_pair);
  } else {
    auto* book = &(search->second);
    size_t count {book->size()};
    for (auto& [_, rule] : _pair.second) {
      rule.setRuleId(++count);
      book->emplace(count, rule);
    }
  }
}


// Named Books Related
void
Parser::finalizeNamedBooks()
{
  d.networkBooks = networkBooks.getFinalVersion();
  d.serviceBooks = serviceBooks.getFinalVersion();
}


// Unsupported
void
Parser::unsup(const std::string& val)
{
  d.observations.addUnsupportedFeature(nmcu::trim(val));
}
void
Parser::addObservation(const std::string& obs)
{
  d.observations.addNotable(nmcu::trim(obs));
}

// Object return
void
Parser::setRuleTargetIface(nmco::AcRule& _rule, const std::string& _name,
                           void (nmco::AcRule::*x)(const std::string&))
{
  if (0 == ifaceAliases.count(_name)) {
    (_rule.*x)(_name);
  } else {
    (_rule.*x)(ifaceAliases[_name]->getName());
  }
}

Result
Parser::getData()
{
  finalizeNamedBooks();

  if (globalCdpEnabled) {
    d.observations.addNotable("CDP is enabled at global scope.");
  }

  if (!globalBpduGuardEnabled && !globalBpduFilterEnabled) {
    d.observations.addNotable(
      "BPDU guard/filter is not enabled at global scope.");
  }

  // Apply global settings to those interfaces that did not manually set them
  for (auto& [name, iface] : d.ifaces) {
    if (!ifaceSpecificCdp.count(iface.getName())){
      iface.setDiscoveryProtocol(globalCdpEnabled);
    }
    if (!ifaceSpecificBpduGuard.count(iface.getName())){
      iface.setBpduGuard(globalBpduGuardEnabled);
    }
    if (!ifaceSpecificBpduFilter.count(iface.getName())){
      iface.setBpduFilter(globalBpduFilterEnabled);
    }
  }

  if (0 != d.networkBooks.count(ZONE)) {
    const auto& tgtBook {d.networkBooks.at(ZONE)};
    for (auto& [iface, alias, ip] : postIfaceIpAliasData) {
      if (0 == tgtBook.count(alias)) { continue; } // no alias-ip known
      for (const auto& ipStr : tgtBook.at(alias).getData()) {
        auto temp {ipStr};
        temp.erase(temp.find("/"));
        ip.setAddress(temp);
        ip.addAlias(alias, "config");
        iface->addIpAddress(ip);
      }
    }
  }

  // TODO is there a better way?
  for (auto& [name, book] : d.ruleBooks) {
    for (auto& [id, rule] : book) {
      std::string zone;
      zone = rule.getSrcId();
      for (auto& strVal : rule.getSrcs()) {
        if (0 == d.networkBooks[zone].count(strVal)) {
          d.networkBooks[zone][strVal].addData(strVal);
        }
      }
      zone = rule.getDstId();
      for (auto& strVal : rule.getDsts()) {
        if (0 == d.networkBooks[zone].count(strVal)) {
          d.networkBooks[zone][strVal].addData(strVal);
        }
      }
      // TODO is this right?
      for (auto& strVal : rule.getServices()) {
        if (0 == d.serviceBooks[zone].count(strVal)) {
          d.serviceBooks[zone][strVal].addData(strVal);
        }
      }
    }
  }

  // Apply interface apply-groups to affected rules
  for (auto& [bookName, dataPair] : appliedRuleSets) {
    auto& [ifaceName, direction] = dataPair;

    for (auto& [id, rule] : d.ruleBooks[bookName]) {
      if ("in" == direction) {
        // in: filter traffic entering iface, regardless destination
        setRuleTargetIface(rule, ifaceName, &nmco::AcRule::addSrcIface);
        rule.addDstIface("any");
      } else if ("out" == direction) {
        // out: filter traffic leaving iface, regardless origin
        rule.addSrcIface("any");
        setRuleTargetIface(rule, ifaceName, &nmco::AcRule::addDstIface);
      } else {
        LOG_ERROR << "(apply-groups) Unknown rule direction parsed: "
                  << direction << std::endl;
      }
    }
  }

  // Apply rules from interface->service-policy->policy-map->class-map
  for (const auto& [ifaceName, policyPairs] : servicePolicies) {
    for (const auto& [policyName, direction] : policyPairs) {
      for (const auto& className : policies[policyName]) {
        for (const auto& bookName : classes[className]) {
          for (auto& [id, rule] : d.ruleBooks[bookName]) {
            if ("input" == direction) {
              setRuleTargetIface(rule, ifaceName, &nmco::AcRule::addSrcIface);
              rule.addDstIface("any");
            } else if ("output" == direction) {
              rule.addSrcIface("any");
              setRuleTargetIface(rule, ifaceName, &nmco::AcRule::addDstIface);
            } else {
              LOG_ERROR << "(service-policy) Unknown rule direction parsed: "
                        << direction << std::endl;
            }
          }
        }
      }
    }
  }

  Result r;
  r.push_back(d);
  return r;
}
