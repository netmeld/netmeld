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
  start =
    config
      [pnx::bind([&](){d.devInfo.setVendor("cisco");}),
       qi::_val = pnx::bind(&Parser::getData, this)]
    ;

  // Unknown where belongs
  // vmware vm mac
  // ip vrrp-extended (vrrrpv3 for nexus?)
  // standby \d+ ip

  config =
    *(   domainData

      | (qi::lit("no cdp") >> (qi::lit("run") | qi::lit("enable")) > qi::eol)
           [pnx::bind(&Parser::globalCdpEnabled, this) = false]

        // TODO this goes away when ASA collapsed
      | ((qi::string("PIX") | qi::string("ASA")) >>
         qi::lit("Version") > *nmdsic::token > qi::eol)
           [pnx::bind(&Parser::unsup, this, "(global) " + qi::_1 + " Version"),
            pnx::bind(&Parser::globalCdpEnabled, this) = false]

        // TODO does it make since to collapse with other calls?
      | (qi::lit("spanning-tree portfast") >>
          (  qi::lit("bpduguard")
               [pnx::bind(&Parser::globalBpduGuardEnabled, this) = true]
           | qi::lit("bpdufilter")
               [pnx::bind(&Parser::globalBpduFilterEnabled, this) = true]
          ) > *nmdsic::token > qi::eol)

      | (interface)
          [pnx::bind(&Parser::vlanAddIfaceData, this)]

      | (qi::lit("policy-map") >> policyMap)

      | (qi::lit("class-map") >> classMap)

        // TODO clean this up
      | ((qi::lit("ipv6") | qi::lit("ip")) >> qi::lit("route") >>
         (   (ipAddr >> ipAddr >> ipAddr)
               [pnx::bind(&nmco::IpAddress::setNetmask, &qi::_1, qi::_2),
                pnx::bind(&Parser::routeAddIp, this, qi::_1, qi::_3)]
           | (ipAddr >> ipAddr >> nmdsic::token)
               [pnx::bind(&nmco::IpAddress::setNetmask, &qi::_1, qi::_2),
                pnx::bind(&Parser::routeAddIface, this, qi::_1, qi::_3)]
           | (ipAddr >> ipAddr)
               [pnx::bind(&Parser::routeAddIp, this, qi::_1, qi::_2)]
           | (ipAddr >> nmdsic::token)
               [pnx::bind(&Parser::routeAddIface, this, qi::_1, qi::_2)]
         ) >> -nmdsic::token >> qi::eol)

      | (qi::lit("aaa ") >> nmdsic::tokens >> qi::eol)
            [pnx::bind(&Parser::deviceAaaAdd, this, qi::_1)]

      | (qi::lit("ip access-list") >> policy)

        // TODO These are more opportunistic right? Or guarenteed?
        // TODO doesn't handle sntp, vrf, or alias
        // TODO s?ntp server [vrf vrf-name] (ip|alias)
      | (qi::lit("ntp server") >> ipAddr > qi::eol)
           [pnx::bind(&Parser::serviceAddNtp, this, qi::_1)]
        // TODO doesn't catch when alias
      | (qi::lit("snmp-server host") >> ipAddr > -nmdsic::tokens > qi::eol)
           [pnx::bind(&Parser::serviceAddSnmp, this, qi::_1)]
        // TODO doesn't catch when it is a block
        // TODO radius-server host source-interface iface
      | (qi::lit("radius-server host") >> ipAddr > -qi::omit[+nmdsic::token] > qi::eol)
           [pnx::bind(&Parser::serviceAddRadius, this, qi::_1)]
        // TODO doesn't handle vrf
        // TODO ip name-server [vrf vrf-name] ip1
      | (qi::lit("ip name-server") > +ipAddr > qi::eol)
           [pnx::bind(&Parser::serviceAddDns, this, qi::_1)]
      | (qi::lit("logging server") > ipAddr > -qi::omit[+nmdsic::token] > qi::eol)
           [pnx::bind(&Parser::serviceAddSyslog, this, qi::_1)]

      | (vlan)
          [pnx::bind(&Parser::vlanAdd, this, qi::_1)]

      // ignore the rest
      | (qi::omit[+nmdsic::token > -qi::eol])
      | (qi::omit[+qi::eol])
    )
    ;
//  config =
//    *(
//      | (qi::lit("name") >> ipAddr >> fqdn
//           [pnx::bind(&Parser::tgtBook, this) = qi::_1] >>
//         -(qi::lit("description") >> nmdsic::tokens) >> qi::eol)
//           [pnx::bind(&Parser::updateNetBookIp, this, qi::_1)]
//      | (qi::lit("name") >> nmdsic::tokens >> qi::eol)
//           [pnx::bind(&Parser::unsup, this, "name")]
//      | (asaIface)
//      | (qi::lit("object-group") > (objGrpNet | objGrpSrv | objGrpProto))
//      | (qi::lit("object") > (objNet | objSrv))
//      | (qi::lit("access-list") >> policy)
//      // access-group ac-list {in|out} interface ifaceName
//      | (qi::lit("access-group") >> nmdsic::token >> nmdsic::token >>
//         qi::lit("interface") >> nmdsic::token)
//           [pnx::bind(&Parser::assignRules, this, qi::_1, qi::_2, qi::_3)]
//      | ignoredLine
//     )
//    ;

  domainData =
    ( ((qi::lit("switchname") | qi::lit("hostname")) > domainName > qi::eol)
        [pnx::bind([&](const std::string& val)
                    {d.devInfo.setDeviceId(val);}, qi::_1)]
    | (qi::lit("ip domain-name") > domainName > qi::eol)
        [pnx::bind(&Parser::unsup, this, "ip domain-name")]
//    | (qi::lit("domain-name") >> fqdn >> qi::eol)
//        [pnx::bind(&Parser::unsup, this, "domain-name")]
    )
    ;

  vlan =
    (qi::lit("vlan") >> qi::ushort_ >> qi::eol)
       [pnx::bind(&nmco::Vlan::setId, &qi::_val, qi::_1)] >>
    *(indent >>
      (  (qi::lit("name") >> nmdsic::token)
            [pnx::bind(&nmco::Vlan::setDescription, &qi::_val, qi::_1)]
       | (qi::omit[+nmdsic::token]) // Ignore all other settings
      ) >> qi::eol
    )
    ;

  interface =
    qi::lit("interface") >> // TODO can this be collapsed?
    (  (nmdsic::token >> nmdsic::token > qi::eol)
          [pnx::bind(&Parser::ifaceInit, this, qi::_1 + " " + qi::_2)]
     | (nmdsic::token > qi::eol)
          [pnx::bind(&Parser::ifaceInit, this, qi::_1)]
    ) >>
    *(indent >>
      qi::matches[qi::lit("no")]
        [pnx::bind(&Parser::isNo, this) = qi::_1] >>
      (  (qi::lit("inherit port-profile"))
            [pnx::bind(&Parser::unsup, this, "port-profile")]
       | (qi::lit("description") >> nmdsic::tokens)
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
          )
         )
//       | qi::lit("ipv6 address") >>
//         (  (ipAddr)
//               [pnx::bind(&nmco::InterfaceNetwork::addIpAddress,
//                          pnx::bind(&Parser::tgtIface, this), qi::_1)]
//          | (fqdn)
//               [pnx::bind(&Parser::unsup, this, "ipv6 address DOMAIN-NAME")]
//         )
//       | qi::lit("ip address") >>
//         (  (ipAddr >> ipAddr)
//               [pnx::bind(&nmco::IpAddress::setNetmask, &qi::_1, qi::_2),
//                pnx::bind(&nmco::InterfaceNetwork::addIpAddress,
//                          pnx::bind(&Parser::tgtIface, this), qi::_1)]
//          | (fqdn >> ipAddr)
//               [qi::_a = pnx::bind(&Parser::getIpFromAlias, this, qi::_1),
//                pnx::bind(&nmco::IpAddress::setNetmask, &qi::_a, qi::_2),
//                pnx::bind(&nmco::InterfaceNetwork::addIpAddress,
//                          pnx::bind(&Parser::tgtIface, this), qi::_a)]
//         )// [pnx::bind(&nmco::IpAddress::setNetmask, &qi::_1, qi::_2),
//          //  pnx::bind(&nmco::InterfaceNetwork::addIpAddress,
//          //            pnx::bind(&Parser::tgtIface, this), qi::_1)]

         // TODO ip helper-address [vrf name | global] ip [redundancy vrg-name]
       | (qi::lit("ip helper-address") >> ipAddr)
            [pnx::bind(&Parser::serviceAddDhcp, this, qi::_1)]
       | (qi::lit("ip dhcp relay address") >> ipAddr)
            [pnx::bind(&Parser::serviceAddDhcp, this, qi::_1)]

       | switchport
       | spanningTree

       | (qi::lit("ip access-group") >> nmdsic::token >> nmdsic::token)
            [pnx::bind(&Parser::createAccessGroup, this, qi::_1, qi::_2)]
       | (qi::lit("service-policy") >> token >> token)
            [pnx::bind(&Parser::createServicePolicy, this, qi::_1, qi::_2)]

//       | (qi::lit("nameif") >> nmdsic::token)
//            [pnx::bind([&](const std::string& val)
//                       {ifaceAliases.emplace(val, *tgtIface);}, qi::_1)]

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


       // Ignore all other settings
       | (qi::omit[+nmdsic::token])
      ) >> qi::eol
    )
    ;

  // TODO migrate
  switchport =
    qi::lit("switchport") >>
    (  (qi::lit("mode") >> nmdsic::token)
          [pnx::bind(&nmco::InterfaceNetwork::setSwitchportMode,
                     pnx::bind(&Parser::tgtIface, this), "L2 " + qi::_1)]
     | (qi::lit("nonegotiate"))
          [pnx::bind(&nmco::InterfaceNetwork::setSwitchportMode,
                     pnx::bind(&Parser::tgtIface, this), "L2 nonegotiate")]

        // TODO Doesn't this not set stick when a mac is present?
     | (qi::lit("port-security mac-address") >> -qi::lit("sticky") >> macAddr)
          [pnx::bind(&nmco::InterfaceNetwork::addPortSecurityStickyMac,
                     pnx::bind(&Parser::tgtIface, this), qi::_1)]
     | (qi::lit("port-security mac-address sticky"))
          [pnx::bind(&nmco::InterfaceNetwork::setPortSecurityStickyMac,
                     pnx::bind(&Parser::tgtIface, this),
                     !pnx::bind(&Parser::isNo, this))]
     | (qi::lit("port-security maximum") >> qi::ushort_ >>
        qi::lit("vlan") > qi::ushort_)
          [pnx::bind(&Parser::unsup, this,
            pnx::bind([&](size_t a, size_t b)
              {return "port-security maximum " + std::to_string(a) +
                      " vlan " + std::to_string(b);}, qi::_1, qi::_2))]
     | (qi::lit("port-security maximum") >> qi::ushort_)
          [pnx::bind(&nmco::InterfaceNetwork::setPortSecurityMaxMacAddrs,
                     pnx::bind(&Parser::tgtIface, this), qi::_1)]
     | (qi::lit("port-security violation") >> nmdsic::token)
          [pnx::bind(&nmco::InterfaceNetwork::setPortSecurityViolationAction,
                     pnx::bind(&Parser::tgtIface, this), qi::_1)]
       // TODO add non-consuming lookahead to ensure ends in eol
     | (qi::lit("port-security"))
          [pnx::bind(&nmco::InterfaceNetwork::setPortSecurity,
                     pnx::bind(&Parser::tgtIface, this),
                     !pnx::bind(&Parser::isNo, this))]

     | (qi::lit("access vlan") /* default, vlan 1 */ >> qi::ushort_)
          [pnx::bind(&nmco::InterfaceNetwork::addVlan,
                     pnx::bind(&Parser::tgtIface, this), qi::_1)]

     | (qi::lit("trunk native vlan") /* default, vlan 1 */ >> qi::ushort_)
          [pnx::bind(&nmco::InterfaceNetwork::addVlan,
                     pnx::bind(&Parser::tgtIface, this), qi::_1)]
     // { VLAN-LIST | all | none | [add|except|remove] { VLAN-LIST } }
     | (qi::lit("trunk allowed vlan") /* default is all */ >>
        (  (-qi::lit("add") >>
            ((  (qi::ushort_ >> qi::lit('-') >> qi::ushort_)
                   [pnx::bind(&nmco::InterfaceNetwork::addVlanRange,
                              pnx::bind(&Parser::tgtIface, this),
                              qi::_1, qi::_2)]
              | (qi::ushort_)
                   [pnx::bind(&nmco::InterfaceNetwork::addVlan,
                              pnx::bind(&Parser::tgtIface, this), qi::_1)]
             ) % qi::lit(',')))
         | (nmdsic::token >> nmdsic::token)
              [pnx::bind(&Parser::addObservation, this,
                         "VLAN trunk " + qi::_1)]
        )
       )
     // { VLAN-ID | [dot1p|none|untagged] }
     | (qi::lit("voice vlan") >>
        (  qi::ushort_
             [pnx::bind(&nmco::InterfaceNetwork::addVlan,
                        pnx::bind(&Parser::tgtIface, this), qi::_1)]
         | nmdsic::token
             [pnx::bind(&Parser::addObservation, this,
                        "voice VLAN " + qi::_1)]
        )
       )
    )
    ;

  spanningTree =
    qi::lit("spanning-tree") >
      // TODO These have "default" values which require to know edge-port state
    (  qi::lit("bpduguard")
         [pnx::bind(&nmco::InterfaceNetwork::setBpduGuard,
                    pnx::bind(&Parser::tgtIface, this),
                    !pnx::bind(&Parser::isNo, this)),
          pnx::bind(&Parser::ifaceSetUpdate, this, &ifaceSpecificBpduGuard)]
//        (  qi::lit("enable")
//             [pnx::bind([&](){tgtIface->setBpduGuard(true);})]
//         | qi::lit("disable")
//             [pnx::bind([&](){tgtIface->setBpduGuard(false);})]
//        )
//       ) [pnx::bind(&Parser::ifaceSetUpdate, this, &ifaceSpecificBpduGuard)]
     | qi::lit("bpdufilter")
         [pnx::bind(&nmco::InterfaceNetwork::setBpduFilter,
                    pnx::bind(&Parser::tgtIface, this),
                    !pnx::bind(&Parser::isNo, this)),
          pnx::bind(&Parser::ifaceSetUpdate, this, &ifaceSpecificBpduFilter)]
//        (  qi::lit("enable")
//             [pnx::bind([&](){tgtIface->setBpduFilter(true);})]
//         | qi::lit("disable")
//             [pnx::bind([&](){tgtIface->setBpduFilter(false);})]
//        )
     | (qi::lit("portfast"))
          [pnx::bind(&nmco::InterfaceNetwork::setPortfast,
                     pnx::bind(&Parser::tgtIface, this),
                     !pnx::bind(&Parser::isNo, this))]
//     | (qi::lit("port type") >
//        (  qi::lit("edge")    // == portfast on
//             [pnx::bind([&](){tgtIface->setPortfast(true);})]
//         | qi::lit("network") // == portfast off
//             [pnx::bind([&](){tgtIface->setPortfast(false);})]
//        )
//       )
     | qi::omit[+nmdsic::token]
    )
    ;

  // TODO double check vs other types
  policy =
    ( // "ip access-list standard" NAME
      (qi::lit("standard") >> nmdsic::token >> qi::eol)
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
      (qi::lit("extended") >> nmdsic::token >> qi::eol) // NAME
        [pnx::bind(&Parser::updateCurRuleBook, this, qi::_1)] >>
      *(indent
         [pnx::bind(&Parser::updateCurRule, this)] >>
        nmdsic::token // ACTION
          [pnx::bind(&Parser::setCurRuleAction, this, qi::_1)] >>
        nmdsic::token // PROTOCOL
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
//// access-list access_list_name
////   standard {deny | permit}
////   {any4 | host ip_address | ip_address mask }
//       (nmdsic::token >> qi::lit("standard"))
//          [pnx::bind(&Parser::unsup, this, "access-list NAME standard")]
//          /* Get AcBook so can add rule after defined */
//          /* AcRule.addAction, AcRule.addDst */
//
//// access-list access_list_name
////   remark {nmdsic::tokens}
//    | (nmdsic::token >> qi::lit("remark") >>
//       nmdsic::tokens [pnx::bind(&Parser::curRuleDescription, this) = qi::_1]
//      )
//
//// access-list access_list_name [line line_number]
////   extended {deny | permit}
////   protocol_argument
////   source_address_argument [port_argument]
////   dest_address_argument [port_argument]
////   [log [[level] [interval secs] | disable | default]]
////   [time-range time_range_name]
////   [inactive]
//// NOTE: port_arguments only seem to appear if protocol_argument is not an
////       object-group of type protocol
//     | (nmdsic::token
//          [qi::_a = qi::_1] >>
//        qi::lit("extended") >> nmdsic::token
//          [pnx::bind(&Parser::updateCurRuleId, this, qi::_a),
//           pnx::bind(&Parser::updateRuleAction, this, qi::_a, qi::_1)] >>
//        // protocol_argument
//        ((  (qi::lit("object-group") >> nmdsic::token)
//          | (qi::lit("object") >> nmdsic::token)
//         ) [pnx::bind(&Parser::updateTgtProto, this, qi::_1, true)]
//         | (nmdsic::token)
//             [pnx::bind(&Parser::updateTgtProto, this, qi::_1, false)]
//        ) >>
//        // source_address_argument
//        (addressArgument)
//          [pnx::bind(&Parser::updateRuleSrc, this, qi::_a, qi::_1)] >>
//        -(portArgument)
//          // NOTE: New logic needed if this can also be an object-group
//          [pnx::bind(&Parser::updateTgtSrcPort, this, qi::_1, false)] >>
//        // dest_address_argument
//        (addressArgument)
//          [pnx::bind(&Parser::updateRuleDst, this, qi::_a, qi::_1)] >>
//        (-(  (portArgument)
//              [pnx::bind(&Parser::updateTgtDstPort, this, qi::_1, false)]
//          | (qi::lit("object-group") >> nmdsic::token)
//              [pnx::bind(&Parser::updateTgtDstPort, this, qi::_1, true)]
//         )
//        ) [pnx::bind(&Parser::updateRuleService, this, qi::_a)] >>
//        // other
//        -(logVal
//            [pnx::bind(&Parser::updateRuleAction, this, qi::_a, qi::_1)]
//         ) >>
//        -(qi::string("time-range") >> nmdsic::token)
//            [pnx::bind(&Parser::updateRuleAction, this,
//                       qi::_a, qi::_1+":"+qi::_2)] >>
//        -(qi::lit("inactive"))
//            [pnx::bind(&Parser::disableRule, this, qi::_a)]
//       )
//    ) >> qi::eol
//    ;
//  logVal =
//    qi::as_string[
//      qi::string("log") >>
//      *(qi::blank >>
//        !&(qi::lit("time-range") | qi::lit("inactive")) >> nmdsic::token
//       )
//    ]
//    ;

  addressArgument =
    ( (ipAddr >> ipAddr)
        [qi::_val = pnx::bind(&Parser::setWildcardMask, this, qi::_1, qi::_2)]
     |
      (qi::lit("host") >> ipAddr)
         [qi::_val = pnx::bind(&nmco::IpAddress::toString, &qi::_1)]
     |
      (qi::lit("object-group") >> nmdsic::token)
        [qi::_val = qi::_1]
     |
      (qi::string("any"))
        [qi::_val = qi::_1]
    )
    ;
//  addressArgument =
//    (  (qi::lit("object-group") >> nmdsic::token)
//     | (qi::lit("object") >> nmdsic::token)
//     | (qi::lit("interface") >> nmdsic::token)
//     | (qi::lit("host") >> ipAddrStr)
//     | (qi::string("any4"))
//     | (qi::string("any6"))
//     | (qi::string("any"))
//     | (ipAddrStr)
//    )
//    ;
//  ipAddrStr =
//    (  (ipAddr >> ipAddr)
//         [pnx::bind(&nmco::IpAddress::setNetmask, &qi::_1, qi::_2),
//          qi::_val = pnx::bind(&nmco::IpAddress::toString, &qi::_1)]
//     | (ipAddr)
//         [qi::_val = pnx::bind(&nmco::IpAddress::toString, &qi::_1)]
//    )
//    ;

  ports =
    ( (qi::lit("eq") >> nmdsic::token)
        [qi::_val = qi::_1]
     |
      (qi::lit("range") >> nmdsic::token >> nmdsic::token)
        [qi::_val = (qi::_1 + "-" + qi::_2)]
    )
    ;

  indent =
    qi::no_skip[+qi::char_(' ')]
    ;

//  nmdsic::tokens =
//    qi::as_string[+(nmdsic::token >> *qi::blank)]
//    ;
//
//  nmdsic::token =
//    +(qi::ascii::graph)
//    ;
//  ignoredLine =
//      (+nmdsic::token >> -qi::eol) // ignored config lines
//    | (+qi::eol)          // ignored empty lines
//    ;

  BOOST_SPIRIT_DEBUG_NODES(
      //(start)
      (config)
      (interface)
      (policy)(addressArgument)(ports)
      (vlan)
      (nmdsic::tokens)(nmdsic::token)
      );
//  // object *
//
//  // object network val_name
//  //  { host val_ip | subnet val_ip val_mask | range val_ip val_ip }
//  objNet =
//    (qi::lit("network") >> nmdsic::token >> qi::eol)
//      [pnx::bind(&Parser::tgtBook, this) = qi::_1] >>
//    // Explicitly check to ensure still in interface scope
//    *(qi::no_skip[+qi::char_(' ')] >>
//      (  (qi::lit("subnet") >> ipAddr >> ipAddr)
//           [pnx::bind(&nmco::IpAddress::setNetmask, &qi::_1, qi::_2),
//            pnx::bind(&Parser::updateNetBookIp, this, qi::_1)]
//       | (qi::lit("subnet") >> ipAddr)
//           [pnx::bind(&Parser::updateNetBookIp, this, qi::_1)]
//       | (qi::lit("host") >> ipAddr)
//           [pnx::bind(&Parser::updateNetBookIp, this, qi::_1)]
//       | (qi::lit("range") >> nmdsic::token >> nmdsic::token)
//           [pnx::bind(&Parser::updateNetBookRange, this, qi::_1, qi::_2)]
//
//       // Ignore all other settings
//       | (qi::omit[+nmdsic::token])
//       | (&qi::eol) // space prefixed blank line
//      ) >> qi::eol
//    )
//    ;
//
//  // object service val_name
//  //  service { val_proto | icmp val_type | icmp6 val_type | { tcp | udp } [ source operator val_port ] [ destination operator val_port ]}
//  // NOTE: operator = { eq | neq | lt | gt | range }
//  objSrv =
//    (qi::lit("service") >> nmdsic::token >> qi::eol)
//      [pnx::bind(&Parser::tgtBook, this) = qi::_1] >>
//    // Explicitly check to ensure still in scope
//    *(qi::no_skip[+qi::char_(' ')] >>
//      (
//       // Capture general settings
//         (qi::lit("service") >> objSrvProto >> objSrvSrc >> objSrvDst)
//           [pnx::bind(&Parser::updateSrvcBook, this,
//                      qi::_1+":"+qi::_2+":"+qi::_3)]
//
//       // Ignore all other settings
//       | (qi::omit[+nmdsic::token])
//       | (&qi::eol) // space prefixed blank line
//      ) >> qi::eol
//    )
//    ;
//
//  objSrvProto =
//    (  ((qi::string("icmp6") | qi::string("icmp")) >> qi::omit[-nmdsic::tokens])
//     | (qi::string("tcp"))
//     | (qi::string("udp"))
//     | (nmdsic::token)
//    )
//    ;
//
//  objSrvSrc =
//    (qi::lit("source") >> portArgument) | qi::attr("")
//    ;
//
//  objSrvDst =
//    (qi::lit("destination") >> portArgument) | qi::attr("")
//    ;
//
//  portArgument =
//    (  (qi::lit("eq") >> nmdsic::token)
//          [qi::_val = qi::_1]
//     | (qi::lit("range") >> srvRngVal)
//          [qi::_val = qi::_1]
//     | (qi::lit("neq") >> qi::omit[nmdsic::token])
//          [pnx::bind(&Parser::unsup, this, "neq PORT")]
//     | (qi::lit("lt") >> qi::omit[nmdsic::token])
//          [pnx::bind(&Parser::unsup, this, "lt PORT")]
//     | (qi::lit("gt") >> qi::omit[nmdsic::token])
//          [pnx::bind(&Parser::unsup, this, "gt PORT")]
//    )
//    ;
//
//  srvRngVal =
//    (nmdsic::token >> nmdsic::token)
//      [qi::_val = qi::_1 + "-" + qi::_2]
//    ;
//
//  // object-group *
//
//  // object-group network val_name
//  //  network-object { object val_name | host val_ip | val_ip val_mask}
//  //  group-object val_name
//  objGrpNet =
//    (qi::lit("network") >> nmdsic::token >> qi::eol)
//      [pnx::bind(&Parser::tgtBook, this) = qi::_1] >>
//    *(qi::no_skip[+qi::char_(' ')] >>
//      (  (qi::lit("network-object object") >> nmdsic::token)
//           [pnx::bind(&Parser::updateNetBookGroup, this, qi::_1)]
//       | (qi::lit("network-object host") >> ipAddr)
//           [pnx::bind(&Parser::updateNetBookIp, this, qi::_1)]
//       | (qi::lit("network-object") >> ipAddr >> ipAddr)
//           [pnx::bind(&nmco::IpAddress::setNetmask, &qi::_1, qi::_2),
//            pnx::bind(&Parser::updateNetBookIp, this, qi::_1)]
//       | (qi::lit("network-object") >> nmdsic::token >> ipAddr)
//           [pnx::bind(&Parser::updateNetBookGroupMask, this, qi::_1, qi::_2)]
//       | (qi::lit("group-object") >> nmdsic::token)
//           [pnx::bind(&Parser::updateNetBookGroup, this, qi::_1)]
//
//       // Ignore all other settings
//       | (qi::omit[+nmdsic::token])
//       | (&qi::eol) // space prefixed blank line
//      ) >> qi::eol
//    )
//    ;
//
//
//  // object-group service val_name { tcp | udp | tcp-udp }
//  //  port-object { eq val_port | range val_start val_end }
//  //  group-object val_name
//  //  service-object { val_proto | icmp val_type | icmp6 val_type | { tcp | udp } [ source operator val_port ] [ destination operator val_port ]}
//  // NOTE: operator = { eq | neq | lt | gt | range }
//  objGrpSrv =
//    (qi::lit("service") >>
//     nmdsic::token [pnx::bind(&Parser::tgtBook, this) = qi::_1] >>
//     -nmdsic::token [qi::_a = qi::_1] >>
//     qi::eol
//    ) >
//    // Explicitly check to ensure still in interface scope
//    *(qi::no_skip[+qi::char_(' ')] >>
//      (  (qi::lit("port-object") >> portArgument)
//           [pnx::bind(&Parser::updateSrvcBook, this, qi::_a+"::"+qi::_1)]
//       | (qi::lit("group-object") >> nmdsic::token)
//           [pnx::bind(&Parser::updateSrvcBookGroup, this, qi::_1)]
//       | (qi::lit("service-object object") >> nmdsic::token)
//           [pnx::bind(&Parser::updateSrvcBookGroup, this, qi::_1)]
//       | (qi::lit("service-object") >> objSrvProto >> objSrvSrc >> objSrvDst)
//           [pnx::bind(&Parser::updateSrvcBook, this,
//                      qi::_1+":"+qi::_2+":"+qi::_3)]
// 
//       // Ignore all other settings
//       | (qi::omit[+nmdsic::token])
//       | (&qi::eol) // space prefixed blank line
//      ) >> qi::eol
//    )
//    ;
//
//
//  // object-group protocol val_name
//  //  protocol-object val_proto
//  //  group-object val_name
//  objGrpProto =
//    (qi::lit("protocol") >> nmdsic::token >> qi::eol)
//       [pnx::bind(&Parser::tgtBook, this) = qi::_1] >>
//    // Explicitly check to ensure still in interface scope
//    *(qi::no_skip[+qi::char_(' ')] >>
//      (  (qi::lit("protocol-object") >> nmdsic::token)
//           [pnx::bind(&Parser::updateSrvcBook, this, qi::_1)]
//       | (qi::lit("group-object") >> nmdsic::token)
//           [pnx::bind(&Parser::updateSrvcBookGroup, this, qi::_1)]
//
//       // Ignore all other settings
//       | (qi::omit[+nmdsic::token])
//       | (&qi::eol) // space prefixed blank line
//      ) >> qi::eol
//    )
//    ;
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
}
//void
//Parser::ifaceInit(const std::string& _name)
//{
//  tgtIface = &d.ifaces[_name];
//  tgtIface->setName(_name);
//  tgtIface->setState(true);
//}

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
Parser::createAccessGroup(const std::string& bookName, const std::string& direction)
{
  appliedRuleSets[bookName] = {tgtIface->getName(), direction};
}

void
Parser::updateCurRuleBook(const std::string& name)
{
  curRuleBook = name;
  curRuleId = 0;
}

void
Parser::updateCurRule()
{
  ++curRuleId;
  curRuleProtocol = "";
  curRuleSrcPort = "";
  curRuleDstPort = "";

  curRule = &(d.ruleBooks[curRuleBook][curRuleId]);

  curRule->setRuleId(curRuleId);
  curRule->setRuleDescription(curRuleBook);
}

void
Parser::setCurRuleAction(const std::string& action)
{
  //TODO: Move this one to setting value direction in symantic action
  curRule->addAction(action);
}

void
Parser::setCurRuleSrc(const std::string& addr)
{
  curRule->setSrcId(ZONE);
  curRule->addSrc(addr);
  d.networkBooks[ZONE][addr].addData(addr);
}

void
Parser::setCurRuleDst(const std::string& addr)
{
  curRule->setDstId(ZONE);
  curRule->addDst(addr);
  d.networkBooks[ZONE][addr].addData(addr);
}

std::string
Parser::setWildcardMask(nmco::IpAddress& ipAddr, const nmco::IpAddress& mask)
{
  bool isContiguous {ipAddr.setWildcardMask(mask)};
  if (!isContiguous) {
    std::ostringstream oss;
    oss << "IpAddress (" << ipAddr
        << ") set with non-contiguous wildcard netmask (" << mask << ")";

    d.observations.addUnsupportedFeature(oss.str());
  }

  return ipAddr.toString();
}

void
Parser::curRuleFinalize()
{
  const std::string serviceString = nmcu::getSrvcString(curRuleProtocol,
                                                        curRuleSrcPort,
                                                        curRuleDstPort);
  curRule->addService(serviceString);
  d.serviceBooks[ZONE][serviceString].addData(serviceString);
}


// Policy Related
void
Parser::createAccessGroup(const std::string& bookName, const std::string& direction)
{
  appliedRuleSets[bookName] = {tgtIface->getName(), direction};
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
Parser::updateCurRuleBook(const std::string& name)
{
  curRuleBook = name;
  curRuleId = 0;
}

void
Parser::updateCurRule()
{
  ++curRuleId;
  curRuleProtocol = "";
  curRuleSrcPort = "";
  curRuleDstPort = "";

  curRule = &(d.ruleBooks[curRuleBook][curRuleId]);

  curRule->setRuleId(curRuleId);
  curRule->setRuleDescription(curRuleBook);
}

void
Parser::setCurRuleAction(const std::string& action)
{
  //TODO: Move this one to setting value direction in symantic action
  curRule->addAction(action);
}

void
Parser::setCurRuleSrc(const std::string& addr)
{
  curRule->setSrcId(ZONE);
  curRule->addSrc(addr);
  d.networkBooks[ZONE][addr].addData(addr);
}

void
Parser::setCurRuleDst(const std::string& addr)
{
  curRule->setDstId(ZONE);
  curRule->addDst(addr);
  d.networkBooks[ZONE][addr].addData(addr);
}

std::string
Parser::setWildcardMask(nmco::IpAddress& ipAddr, const nmco::IpAddress& mask)
{
  bool isContiguous {ipAddr.setWildcardMask(mask)};
  if (!isContiguous) {
    std::ostringstream oss;
    oss << "IpAddress (" << ipAddr
        << ") set with non-contiguous wildcard netmask (" << mask << ")";

    d.observations.addUnsupportedFeature(oss.str());
  }

  return ipAddr.toString();
}

void
Parser::curRuleFinalize()
{
  const std::string serviceString = nmcu::getSrvcString(curRuleProtocol,
                                                        curRuleSrcPort,
                                                        curRuleDstPort);
  curRule->addService(serviceString);
  d.serviceBooks[ZONE][serviceString].addData(serviceString);
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
Result
Parser::getData()
{
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

  // Apply interface apply-groups to affected rules
  for (auto& [bookName, dataPair] : appliedRuleSets) {
    auto& [ifaceName, direction] = dataPair;

    for (auto& [id, rule] : d.ruleBooks[bookName]) {
      if ("in" == direction) {
        // in: filter traffic entering iface, regardless destination
        rule.addSrcIface(ifaceName);
        rule.addDstIface("any");
      } else if ("out" == direction) {
        // out: filter traffic leaving iface, regardless origin
        rule.addSrcIface("any");
        rule.addDstIface(ifaceName);
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
              rule.addSrcIface(ifaceName);
              rule.addDstIface("any");
            } else if ("output" == direction) {
              rule.addSrcIface("any");
              rule.addDstIface(ifaceName);
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
