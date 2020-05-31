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

#include <netmeld/core/utils/ServiceFactory.hpp>
#include <netmeld/core/utils/StringUtilities.hpp>

namespace nmcu = netmeld::core::utils;

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

      | (qi::lit("spanning-tree portfast") >>
          (  qi::lit("bpduguard")
               [pnx::bind(&Parser::globalBpduGuardEnabled, this) = true]
           | qi::lit("bpdufilter")
               [pnx::bind(&Parser::globalBpduFilterEnabled, this) = true]
          ) > *token > qi::eol)

<<<<<<< HEAD:datastore/importers/nmdb-import-cisco/Parser.cpp
      | (interface)
          [pnx::bind(&Parser::ifaceFinalize, this)]

      | (qi::lit("policy-map") >> policyMap)

      | (qi::lit("class-map") >> classMap)

      | ((qi::lit("ipv6") | qi::lit("ip")) >> qi::lit("route") >>
         (   (ipAddr >> ipAddr >> ipAddr)
               [pnx::bind(&nmdo::IpAddress::setNetmask, &qi::_1, qi::_2),
                pnx::bind(&Parser::addRouteIp, this, qi::_1, qi::_3)]
           | (ipAddr >> ipAddr >> token)
               [pnx::bind(&nmdo::IpAddress::setNetmask, &qi::_1, qi::_2),
                pnx::bind(&Parser::addRouteIface, this, qi::_1, qi::_3)]
           | (ipAddr >> ipAddr)
               [pnx::bind(&Parser::addRouteIp, this, qi::_1, qi::_2)]
           | (ipAddr >> token)
               [pnx::bind(&Parser::addRouteIface, this, qi::_1, qi::_2)]
         ) >> -token >> qi::eol)

=======
>>>>>>> master:importers/nmdb-import-cisco/Parser.cpp
      | (qi::lit("aaa ") >> tokens >> qi::eol)
            [pnx::bind(&Parser::deviceAaaAdd, this, qi::_1)]

      | globalServices

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

<<<<<<< HEAD:datastore/importers/nmdb-import-cisco/Parser.cpp
  vlan =
    ((qi::lit("vlan") >> qi::ushort_ >> qi::eol)
        [pnx::bind(&nmdo::Vlan::setId, &qi::_val, qi::_1)] >>
     *(indent >>
       (
        (qi::lit("name") >> token)
           [pnx::bind(&nmdo::Vlan::setDescription, &qi::_val, qi::_1)]

        // Ignore all other settings
        | (qi::omit[+token])
       ) >> qi::eol
      )
=======
  domainData =
    ( ((qi::lit("switchname") | qi::lit("hostname")) > domainName > qi::eol)
          [pnx::bind([&](const std::string& val)
                     {d.devInfo.setDeviceId(val);}, qi::_1)]
     | (qi::lit("ip domain-name") > domainName > qi::eol)
          [pnx::bind(&Parser::unsup, this, "ip domain-name")]
     | (qi::lit("domain-name") >> domainName >> qi::eol)
          [pnx::bind(&Parser::unsup, this, "domain-name")]
>>>>>>> master:importers/nmdb-import-cisco/Parser.cpp
    )
    ;

  // [ip|ipv6] route dstNet nextHop {administrative_distance} {permanent}
  //   dstNet  == [ip/prefix | ip mask]
  //   nextHop == [ip/prefix | ip | iface]
  route = 
    (qi::lit("ipv6") | qi::lit("ip")) >> qi::lit("route") >>
       // ip_mask ip
    (  (ipMask >> ipAddr)
         [pnx::bind(&Parser::routeAddIp, this, qi::_1, qi::_2)]
       // ip_mask iface
     | (ipMask >> (!(qi::digit | qi::lit("permanent")) >> token))
         [pnx::bind(&Parser::routeAddIface, this, qi::_1, qi::_2)]
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
            [pnx::bind(&nmdo::InterfaceNetwork::setDescription,
                       pnx::bind(&Parser::tgtIface, this), qi::_1)]
       | (qi::lit("shutdown"))
<<<<<<< HEAD:datastore/importers/nmdb-import-cisco/Parser.cpp
            [pnx::bind(&nmdo::InterfaceNetwork::setState,
                       pnx::bind(&Parser::tgtIface, this),
                       pnx::bind(&Parser::isNo, this))]
       | (qi::lit("cdp enable"))
            [pnx::bind(&nmdo::InterfaceNetwork::setDiscoveryProtocol,
                       pnx::bind(&Parser::tgtIface, this),
                       !pnx::bind(&Parser::isNo, this)),
             pnx::bind(&Parser::addManuallySetCdpIface, this)]

       | ((qi::lit("ipv6") | qi::lit("ip")) >> qi::lit("address") >>
          (  (ipAddr >> ipAddr)
            [pnx::bind(&nmdo::IpAddress::setNetmask, &qi::_1, qi::_2),
             pnx::bind(&nmdo::InterfaceNetwork::addIpAddress,
                       pnx::bind(&Parser::tgtIface, this), qi::_1)]
           | (ipAddr)
            [pnx::bind(&nmdo::InterfaceNetwork::addIpAddress,
                       pnx::bind(&Parser::tgtIface, this), qi::_1)]
=======
            [pnx::bind([&](){tgtIface->setState(isNo);})]
       | (qi::lit("cdp enable"))
            [pnx::bind([&](){tgtIface->setDiscoveryProtocol(!isNo);}),
             pnx::bind(&Parser::ifaceSetUpdate, this, &ifaceSpecificCdp)]

       | ((qi::lit("ipv6") | qi::lit("ip")) >> -qi::lit("-")
          >> qi::lit("address") >>
          (  (ipMask)
               [pnx::bind(&nmco::InterfaceNetwork::addIpAddress,
                          pnx::bind(&Parser::tgtIface, this), qi::_1)]
           | (ipAddr)
               [pnx::bind(&nmco::InterfaceNetwork::addIpAddress,
                          pnx::bind(&Parser::tgtIface, this), qi::_1)]
           | (domainName >> ipAddr)
               [pnx::bind(&Parser::ifaceAddAlias, this, qi::_1, qi::_2)]
           | (domainName)
               [pnx::bind(&Parser::ifaceAddAlias, this,
                          qi::_1, nmco::IpAddress())]
>>>>>>> master:importers/nmdb-import-cisco/Parser.cpp
          )
         )

       | (qi::lit("ip helper-address")
          >> -((qi::lit("vrf") > qi::omit[token]) | qi::lit("global"))
          >> ipAddr > -tokens)
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
<<<<<<< HEAD:datastore/importers/nmdb-import-cisco/Parser.cpp
       | (qi::lit("standby")>> qi::int_ >> qi::lit("ip") >> ipAddr)
            [pnx::bind(&nmdo::InterfaceNetwork::addIpAddress,
=======
       | (qi::lit("standby") >> qi::int_ >> qi::lit("ip")
          >> ipAddr >> -qi::lit("secondary"))
            [pnx::bind(&nmco::InterfaceNetwork::addIpAddress,
>>>>>>> master:importers/nmdb-import-cisco/Parser.cpp
                       pnx::bind(&Parser::tgtIface, this), qi::_2)]
       // Capture Cisco N1000V virtual switches
       | (qi::lit("vmware vm mac") >> macAddr)
            [pnx::bind(&nmdo::InterfaceNetwork::addReachableMac,
                       pnx::bind(&Parser::tgtIface, this), qi::_1)]
<<<<<<< HEAD:datastore/importers/nmdb-import-cisco/Parser.cpp
       // Capture ADX configs
       | (qi::lit("ip-address") >> ipAddr)
            [pnx::bind(&nmdo::InterfaceNetwork::addIpAddress,
                       pnx::bind(&Parser::tgtIface, this), qi::_1)]
=======
>>>>>>> master:importers/nmdb-import-cisco/Parser.cpp
       /* END: No examples of these, cannot verify */

       | switchport
       | spanningTree

       // Ignore all other settings
       | (qi::omit[+token | &qi::eol])
      ) >> qi::eol
    )
    ;

  globalServices =
    // NOTE None handle when an alias is used instead of an IP
    (  ((qi::lit("sntp") | qi::lit("ntp")) >> qi::lit("server")
        > -(qi::lit("vrf") > token)
        > (  ipAddr [pnx::bind(&Parser::serviceAddNtp, this, qi::_1)]
           | token
          ) > -tokens > qi::eol)
      | (qi::lit("snmp-server host")
         > ( ipAddr [pnx::bind(&Parser::serviceAddSnmp, this, qi::_1)]
            | token
           ) > -tokens > qi::eol)
      | (qi::lit("radius-server host")
         >> ipAddr [pnx::bind(&Parser::serviceAddRadius, this, qi::_1)]
         > -tokens > qi::eol)
      | (qi::lit("ip name-server") > -(qi::lit("vrf") > token)
         > +ipAddr [pnx::bind(&Parser::serviceAddDns, this, qi::_1)]
         > qi::eol)
      | (qi::lit("logging server")
         > ipAddr [pnx::bind(&Parser::serviceAddSyslog, this, qi::_1)]
         > -tokens > qi::eol)
    )
    ;

  switchport =
    qi::lit("switchport") >>
<<<<<<< HEAD:datastore/importers/nmdb-import-cisco/Parser.cpp
    (  (qi::lit("mode") >> token)
          [pnx::bind(&nmdo::InterfaceNetwork::setSwitchportMode,
                     pnx::bind(&Parser::tgtIface, this), "L2 " + qi::_1)]
     | (qi::lit("nonegotiate"))
          [pnx::bind(&nmdo::InterfaceNetwork::setSwitchportMode,
                     pnx::bind(&Parser::tgtIface, this), "L2 nonegotiate")]

     | (qi::lit("port-security mac-address") >> -qi::lit("sticky") >> macAddr)
          [pnx::bind(&nmdo::InterfaceNetwork::addPortSecurityStickyMac,
                     pnx::bind(&Parser::tgtIface, this), qi::_1)]
     | (qi::lit("port-security mac-address sticky"))
          [pnx::bind(&nmdo::InterfaceNetwork::setPortSecurityStickyMac,
                     pnx::bind(&Parser::tgtIface, this),
                     !pnx::bind(&Parser::isNo, this))]
     | (qi::lit("port-security maximum") >> qi::ushort_)
          [pnx::bind(&nmdo::InterfaceNetwork::setPortSecurityMaxMacAddrs,
                     pnx::bind(&Parser::tgtIface, this), qi::_1)]
     | (qi::lit("port-security violation") >> token)
          [pnx::bind(&nmdo::InterfaceNetwork::setPortSecurityViolationAction,
                     pnx::bind(&Parser::tgtIface, this), qi::_1)]
     | (qi::lit("port-security"))
          [pnx::bind(&nmdo::InterfaceNetwork::setPortSecurity,
                     pnx::bind(&Parser::tgtIface, this),
                     !pnx::bind(&Parser::isNo, this))]

     | (qi::lit("access vlan") /* default, vlan 1 */ >> qi::ushort_)
          [pnx::bind(&nmdo::InterfaceNetwork::addVlan,
                     pnx::bind(&Parser::tgtIface, this), qi::_1)]

     | (qi::lit("trunk native vlan") /* default, vlan 1 */ >> qi::ushort_)
          [pnx::bind(&nmdo::InterfaceNetwork::addVlan,
                     pnx::bind(&Parser::tgtIface, this), qi::_1)]
     // { VLAN-LIST | all | none | [add|except|remove] { VLAN-LIST } }
     | (qi::lit("trunk allowed vlan") /* default is all */ >>
        (  (-qi::lit("add") >>
            ((  (qi::ushort_ >> qi::lit('-') >> qi::ushort_)
                   [pnx::bind(&nmdo::InterfaceNetwork::addVlanRange,
                              pnx::bind(&Parser::tgtIface, this),
                              qi::_1, qi::_2)]
              | (qi::ushort_)
                   [pnx::bind(&nmdo::InterfaceNetwork::addVlan,
                              pnx::bind(&Parser::tgtIface, this), qi::_1)]
             ) % qi::lit(',')))
         | (token >> token)
              [pnx::bind(&Parser::addObservation, this,
                         "VLAN trunk " + qi::_1)]
        )
       )
     // { VLAN-ID | [dot1p|none|untagged] }
     | (qi::lit("voice vlan") >>
        (  qi::ushort_
             [pnx::bind(&nmdo::InterfaceNetwork::addVlan,
                        pnx::bind(&Parser::tgtIface, this), qi::_1)]
         | token
             [pnx::bind(&Parser::addObservation, this,
                        "voice VLAN " + qi::_1)]
        )
       )
    )
    ;

  spanningTree =
    qi::lit("spanning-tree") >
    (  qi::lit("bpduguard")
         [pnx::bind(&nmdo::InterfaceNetwork::setBpduGuard,
                    pnx::bind(&Parser::tgtIface, this),
                    !pnx::bind(&Parser::isNo, this)),
          pnx::bind(&Parser::addManuallySetBpduGuardIface, this)]
     | qi::lit("bpdufilter")
         [pnx::bind(&nmdo::InterfaceNetwork::setBpduFilter,
                    pnx::bind(&Parser::tgtIface, this),
                    !pnx::bind(&Parser::isNo, this)),
          pnx::bind(&Parser::addManuallySetBpduFilterIface, this)]
     | (qi::lit("portfast"))
          [pnx::bind(&nmdo::InterfaceNetwork::setPortfast,
                     pnx::bind(&Parser::tgtIface, this),
                     !pnx::bind(&Parser::isNo, this))]
     | qi::omit[+token]
    )
    ;

  policy =
    ( // "ip access-list standard" NAME
      (qi::lit("standard") >> token >> qi::eol)
        [pnx::bind(&Parser::unsup, this, "ip access-list standard " + qi::_1)]
     |
       // "ip access-list extended" NAME
       //    ACTION PROTOCOL SOURCE [PORTS] [ DEST [PORTS] ] ["log"]
       //   ---
       //    ACTION ( "permit" | "deny" )
       //    PROTOCOL ( name )
       //    SOURCE ( IpAddr *mask | "host" IpAddr | "any" )
       //      - Note: *mask (wildcard mask) takes bit representation of mask and
       //              compares with bits of IpAddr. (1=wild, 0=must match IpAddr)
       //    DEST ( IpAddr *mask | "host" IpAddr | "any" | "object-group" name )
       //    PORTS ( "eq" port | "range" startPort endPort )
       //    "log" (Couldn't find anything about options to this in this context)
      (qi::lit("extended") >> token >> qi::eol) // NAME
        [pnx::bind(&Parser::updateCurRuleBook, this, qi::_1)] >>
      *(indent
         [pnx::bind(&Parser::updateCurRule, this)] >>
        token // ACTION
          [pnx::bind(&Parser::setCurRuleAction, this, qi::_1)] >>
        token // PROTOCOL
          [pnx::bind(&Parser::curRuleProtocol, this) = qi::_1] >>
        addressArgument // SOURCE
          [pnx::bind(&Parser::setCurRuleSrc, this, qi::_1)] >>
        -(ports // SOURCE PORTS
          [pnx::bind(&Parser::curRuleSrcPort, this) = qi::_1]) >>
        -(addressArgument // DESTINATION
            [pnx::bind(&Parser::setCurRuleDst, this, qi::_1)] >>
          -(ports // DESTINATION PORTS
             [pnx::bind(&Parser::curRuleDstPort, this) = qi::_1])) >>
        -qi::lit("log") >>
        qi::eol
       ) [pnx::bind(&Parser::curRuleFinalize, this)]
     )
    ;

  addressArgument =
    ( (ipAddr >> ipAddr)
        [qi::_val = pnx::bind(&Parser::setWildcardMask, this, qi::_1, qi::_2)]
     |
      (qi::lit("host") >> ipAddr)
         [qi::_val = pnx::bind(&nmdo::IpAddress::toString, &qi::_1)]
     |
      (qi::lit("object-group") >> token)
        [qi::_val = qi::_1]
     |
      (qi::string("any"))
        [qi::_val = qi::_1]
    )
    ;

  ports =
    ( (qi::lit("eq") >> token)
        [qi::_val = qi::_1]
     |
      (qi::lit("range") >> token >> token)
        [qi::_val = (qi::_1 + "-" + qi::_2)]
=======
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
            [pnx::bind([&](){tgtIface->setPortSecurityStickyMac(!isNo);})]
        > -macAddr
            [pnx::bind(&nmco::InterfaceNetwork::addPortSecurityStickyMac,
                       pnx::bind(&Parser::tgtIface, this), qi::_1)]
       )
     | ((qi::lit("maximum") >> qi::ushort_)
          [pnx::bind(&nmco::InterfaceNetwork::setPortSecurityMaxMacAddrs,
                     pnx::bind(&Parser::tgtIface, this), qi::_1)]
         > -(qi::lit("vlan") > qi::ushort_)
          [pnx::bind(&Parser::unsup, this,
                     "switchport port-security maximum COUNT vlan ID")]
       )
     | (qi::lit("violation") >> token)
          [pnx::bind(&nmco::InterfaceNetwork::setPortSecurityViolationAction,
                     pnx::bind(&Parser::tgtIface, this), qi::_1)]
     | (&qi::eol)
          [pnx::bind([&](){tgtIface->setPortSecurity(!isNo);})]
    )
    ;
  switchportVlan =
    (qi::string("access ") | qi::string("trunk ") | qi::string("voice "))
    >> (  qi::string("native ")
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
      // NOTE All have "default" values which require knowing edge-port state
    (  (qi::lit("bpduguard")
         [pnx::bind([&](){tgtIface->setBpduGuard(!isNo);}),
          pnx::bind(&Parser::ifaceSetUpdate, this, &ifaceSpecificBpduGuard)] >
        -(  qi::lit("enable")
             [pnx::bind([&](){tgtIface->setBpduGuard(true);})]
         | qi::lit("disable")
             [pnx::bind([&](){tgtIface->setBpduGuard(false);})]
        )
       )
     | (qi::lit("bpdufilter")
         [pnx::bind([&](){tgtIface->setBpduFilter(!isNo);}),
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
          [pnx::bind([&](){tgtIface->setPortfast(!isNo);})]
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
          [pnx::bind(&Parser::createAccessGroup, this, qi::_1, qi::_2, qi::_3)]
>>>>>>> master:importers/nmdb-import-cisco/Parser.cpp
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

  // General Helper(s)
  ipMask =
    (ipAddr >> +qi::blank >> ipAddr)
      [pnx::bind(&nmco::IpAddress::setNetmask, &qi::_1, qi::_3),
       qi::_val = qi::_1]
    ;

  BOOST_SPIRIT_DEBUG_NODES(
      //(start)
      (config)
      (domainData)
      (globalServices)
      (route)
      (vlanDef)
      (interface)
        (switchport)
          (switchportPortSecurity)
            (vlanRange)
            (vlanId)
        (spanningTree)
      (accessPolicyRelated)
      (switchportVlan)
      (policyMap)(classMap)
      (addressArgument)
      (ports)
      (vlan)
      (ipMask)
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
void
Parser::serviceAddDhcp(const nmco::IpAddress& ip)
{
  auto service = nmcu::ServiceFactory::makeDhcp();
  service.setDstAddress(ip);
  service.setServiceReason(d.devInfo.getDeviceId() + "'s config");
  d.services.push_back(service);
}

void
Parser::serviceAddNtp(const nmco::IpAddress& ip)
{
  auto service = nmcu::ServiceFactory::makeNtp();
  service.setDstAddress(ip);
  service.setServiceReason(d.devInfo.getDeviceId() + "'s config");
  d.services.push_back(service);
}

void
Parser::serviceAddSnmp(const nmco::IpAddress& ip)
{
  auto service = nmcu::ServiceFactory::makeSnmp();
  service.setDstAddress(ip);
  service.setServiceReason(d.devInfo.getDeviceId() + "'s config");
  d.services.push_back(service);
}

void
<<<<<<< HEAD:datastore/importers/nmdb-import-cisco/Parser.cpp
Parser::addDhcpService(const nmdo::IpAddress& ip)
{
  nmdo::Service service {"dhcps", ip}; // match nmap output
  service.setProtocol("udp");
  service.addDstPort("67"); // port server uses
  service.addSrcPort("68"); // port client uses
=======
Parser::serviceAddRadius(const nmco::IpAddress& ip)
{
  auto service = nmcu::ServiceFactory::makeRadius();
  service.setDstAddress(ip);
>>>>>>> master:importers/nmdb-import-cisco/Parser.cpp
  service.setServiceReason(d.devInfo.getDeviceId() + "'s config");
  d.services.push_back(service);
}

void
<<<<<<< HEAD:datastore/importers/nmdb-import-cisco/Parser.cpp
Parser::addNtpService(const nmdo::IpAddress& ip)
{
  nmdo::Service service {"ntp", ip};
  service.setProtocol("udp");
  service.addDstPort("123"); // same port used by client and server
  service.addSrcPort("123");
=======
Parser::serviceAddDns(const nmco::IpAddress& ip)
{
  auto service = nmcu::ServiceFactory::makeDns();
  service.setDstAddress(ip);
>>>>>>> master:importers/nmdb-import-cisco/Parser.cpp
  service.setServiceReason(d.devInfo.getDeviceId() + "'s config");
  d.services.push_back(service);
}

void
<<<<<<< HEAD:datastore/importers/nmdb-import-cisco/Parser.cpp
Parser::addSnmpService(const nmdo::IpAddress& ip)
{
  nmdo::Service service {"snmp", ip};
  service.setProtocol("udp");
  service.addDstPort("162"); // port manager receives on
=======
Parser::serviceAddSyslog(const nmco::IpAddress& ip)
{
  auto service = nmcu::ServiceFactory::makeSyslog();
  service.setDstAddress(ip);
>>>>>>> master:importers/nmdb-import-cisco/Parser.cpp
  service.setServiceReason(d.devInfo.getDeviceId() + "'s config");
  d.services.push_back(service);
}


// Route related
void
<<<<<<< HEAD:datastore/importers/nmdb-import-cisco/Parser.cpp
Parser::addRouteIp(const nmdo::IpAddress& dstNet, const nmdo::IpAddress& rtrIp)
=======
Parser::routeAddIp(const nmco::IpAddress& dstNet, const nmco::IpAddress& rtrIp)
>>>>>>> master:importers/nmdb-import-cisco/Parser.cpp
{
  nmdo::Route route;
  route.setDstNet(dstNet);
  route.setRtrIp(rtrIp);

  d.routes.push_back(route);
}

void
<<<<<<< HEAD:datastore/importers/nmdb-import-cisco/Parser.cpp
Parser::addRouteIface(const nmdo::IpAddress& dstNet, const std::string& rtrIface)
=======
Parser::routeAddIface(const nmco::IpAddress& dstNet,
                      const std::string& rtrIface)
>>>>>>> master:importers/nmdb-import-cisco/Parser.cpp
{
  nmdo::Route route;
  route.setDstNet(dstNet);
  route.setIfaceName(rtrIface);

  d.routes.push_back(route);
}


// Interface related
void
Parser::ifaceInit(const std::string& _name)
{
  tgtIface = &d.ifaces[_name];
  tgtIface->setName(_name);
}

void
Parser::ifaceSetUpdate(std::set<std::string>* const set)
{
  set->insert(tgtIface->getName());
}

void
Parser::ifaceAddAlias(const std::string& _alias, const nmco::IpAddress& _mask)
{
  nmco::IpAddress ip;
  if (_mask.isValid()) {
    ip.setMask(_mask);
  }
  postIfaceAliasIpData.push_back(std::make_tuple(tgtIface, _alias, ip));
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
      nmdo::Vlan vlan {id};
      vlan.setIpNet(ipAddr);
      d.vlans.push_back(vlan);
    }
  }
}

<<<<<<< HEAD:datastore/importers/nmdb-import-cisco/Parser.cpp
// Vlan related
void
Parser::addVlan(nmdo::Vlan& vlan)
{
  d.vlans.push_back(vlan);
}

=======
>>>>>>> master:importers/nmdb-import-cisco/Parser.cpp

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
Parser::createServicePolicy(const std::string& direction,
                            const std::string& policyName)
{
  servicePolicies[tgtIface->getName()].insert({policyName, direction});
}

void
Parser::updatePolicyMap(const std::string& policyName,
                        const std::string& className)
{
  policies[policyName].insert(className);
}

void
Parser::updateClassMap(const std::string& className,
                       const std::string& bookName)
{
  classes[className].insert(bookName);
}

void
Parser::aclRuleBookAdd(std::pair<std::string, RuleBook>& _pair)
{
  if (_pair.first.empty()) { return; }

  auto search = d.ruleBooks.find(_pair.first);
  if (search == d.ruleBooks.end()) { // add new
    d.ruleBooks.emplace(_pair);
  } else { // update existing
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
<<<<<<< HEAD:datastore/importers/nmdb-import-cisco/Parser.cpp
Parser::setCurRuleDst(const std::string& addr)
{
  curRule->setDstId(ZONE);
  curRule->addDst(addr);
  d.networkBooks[ZONE][addr].addData(addr);
}

std::string
Parser::setWildcardMask(nmdo::IpAddress& ipAddr, const nmdo::IpAddress& mask)
=======
Parser::unsup(const std::string& val)
>>>>>>> master:importers/nmdb-import-cisco/Parser.cpp
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

  // Resolve interface IP aliases (e.g., someName -> 1.2.3.4)
  if (0 != d.networkBooks.count(ZONE)) {
    const auto& tgtBook {d.networkBooks.at(ZONE)};
    for (auto& [iface, alias, ip] : postIfaceAliasIpData) {
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

  // Ensure AcRule src/dst/service are tracked in the named books
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
