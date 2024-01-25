// =============================================================================
// Copyright 2021 National Technology & Engineering Solutions of Sandia, LLC
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

#include <netmeld/core/utils/StringUtilities.hpp>

#include "Parser.hpp"

// =============================================================================
// Parser logic
// =============================================================================
Parser::Parser() : Parser::base_type(start)
{
  start =
    +( arp
     | ndp
     | errorMessage
     | qi::eol
     )
    ;

  // =============================================================================
  // ARP Rules
  // =============================================================================

  arp =
    ( arpArista
    | arpCiscoIos
    | arpCiscoNxos
    //| arpCiscoWlc
    | arpJuniperConfig
    )
    ;

  // -----------------------------------------------------------------------------
  // ARP: Arista
  // -----------------------------------------------------------------------------

  arpArista =
    arpHeaderArista >>
    *arpEntryArista
    ;

  arpHeaderArista =
    -(qi::lit("VRF:") >> token >> qi::eol) >>
    qi::lit("Address") >>
    qi::lit("Age") >> token >>
    qi::lit("Hardware Addr") >>
    qi::lit("Interface") >>
    (qi::eol | qi::eoi)
    ;

  arpEntryArista =
    ( ipv4Addr[(qi::_c = qi::_1)] >>
      age >>
      macAddr[(qi::_b = qi::_1)] >>
      iface[(qi::_a = qi::_1)] >>
      -(qi::lit(",") >> +(qi::char_ - qi::eol)) >>
      (qi::eol | qi::eoi)
    )[(pnx::bind(&nmdo::InterfaceNetwork::setName, &qi::_val, qi::_a),
       pnx::bind(&nmdo::MacAddress::addIpAddress, &qi::_b, qi::_c),
       pnx::bind(&nmdo::InterfaceNetwork::addReachableMac, &qi::_val, qi::_b))]
    ;

  // -----------------------------------------------------------------------------
  // ARP: Cisco IOS
  // -----------------------------------------------------------------------------

  arpCiscoIos =
    arpHeaderCiscoIos >>
    *arpEntryCiscoIos
    ;

  arpHeaderCiscoIos =
    qi::lit("Protocol") >>
    qi::lit("Address") >>
    qi::lit("Age (min)") >>
    qi::lit("Hardware Addr") >>
    qi::lit("Type") >>
    qi::lit("Interface") >>
    (qi::eol | qi::eoi)
    ;

  arpEntryCiscoIos =
    ( qi::lit("Internet") >>
      ipv4Addr[(qi::_c = qi::_1)] >>
      age >>
      ( qi::lit("Incomplete")
      | macAddr[(qi::_b = qi::_1)]
      ) >>
      token >>
      -(iface[(qi::_a = qi::_1)]) >>
      (qi::eol | qi::eoi)
    )[(pnx::bind(&nmdo::InterfaceNetwork::setName, &qi::_val, qi::_a),
       pnx::bind(&nmdo::MacAddress::addIpAddress, &qi::_b, qi::_c),
       pnx::bind(&nmdo::InterfaceNetwork::addReachableMac, &qi::_val, qi::_b))]
    ;

  // -----------------------------------------------------------------------------
  // ARP: Cisco NXOS
  // -----------------------------------------------------------------------------

  arpCiscoNxos =
    arpHeaderCiscoNxos >>
    *arpEntryCiscoNxos
    ;

  arpHeaderCiscoNxos =
    *( !qi::lit("IP ARP Table for context") >>
       *(qi::char_ - qi::eol) >>
       qi::eol
     ) >>
    qi::lit("IP ARP Table for context") >>
    token >>
    qi::eol >>
    qi::lit("Total number of entries:") >>
    qi::omit[qi::int_] >>
    qi::eol >>
    qi::lit("Address") >>
    qi::lit("Age") >>
    qi::lit("MAC Address") >>
    qi::lit("Interface") >>
    qi::lit("Flags") >>
    (qi::eol | qi::eoi)
    ;

  arpEntryCiscoNxos =
    ( ipv4Addr[(qi::_c = qi::_1)] >>
      age >>
      ( qi::lit("INCOMPLETE")
      | macAddr[(qi::_b = qi::_1)]
      ) >>
      iface[(qi::_a = qi::_1)] >>
      qi::omit[*(qi::char_ - qi::eol)] >>
      (qi::eol | qi::eoi)
    )[(pnx::bind(&nmdo::InterfaceNetwork::setName, &qi::_val, qi::_a),
       pnx::bind(&nmdo::MacAddress::addIpAddress, &qi::_b, qi::_c),
       pnx::bind(&nmdo::InterfaceNetwork::addReachableMac, &qi::_val, qi::_b))]
    ;

  // -----------------------------------------------------------------------------
  // ARP: Cisco WLC and WISM
  // -----------------------------------------------------------------------------

  arpCiscoWlc =
    arpHeaderCiscoWlc >>
    *arpEntryCiscoWlc
    ;

  arpHeaderCiscoWlc =
    *qi::eol >>
    qi::lit("Number of arp entries") >>
    qi::omit[+(qi::char_ - qi::eol)] >>
    +qi::eol >>
    qi::lit("MAC Address") >>
    qi::lit("IP Address") >>
    qi::lit("Port") >>
    qi::lit("VLAN") >>
    qi::lit("Type") >>
    qi::eol >>
    +(qi::lit("-") | qi::lit(" ")) >>
    (qi::eol | qi::eoi)
    ;

  // Problem is the WLC/WISM ARP output contains Port and VLAN,
  // but does NOT contain the interface name.
  // Also, the VLAN ID can't just be expanded to something like "Vlan N"
  // because those aren't what the interfaces are called in the config.
  arpEntryCiscoWlc =
    ( macAddr[(qi::_b = qi::_1)] >>
      ipv4Addr[(qi::_c = qi::_1)] >>
      token >>
      token[(qi::_a = qi::_1)] >>
      token >>
      (qi::eol | qi::eoi)
    )[(pnx::bind(&nmdo::InterfaceNetwork::setName, &qi::_val, qi::_a),
       pnx::bind(&nmdo::MacAddress::addIpAddress, &qi::_b, qi::_c),
       pnx::bind(&nmdo::InterfaceNetwork::addReachableMac, &qi::_val, qi::_b))]
    ;

  // =============================================================================
  // ARP: Juniper Config
  // =============================================================================

  arpJuniperConfig =
    arpHeaderJuniper >>
    *arpEntryJuniper
    ;

  arpHeaderJuniper =
    qi::lit("MAC Address") >>
    qi::lit("Address") >>
    qi::lit("Name") >>
    qi::lit("Interface") >>
    (qi::eol | qi::eoi)
    ;

  arpEntryJuniper =
    ( macAddr[(qi::_b = qi::_1)] >>
      ipv4Addr[(qi::_c = qi::_1)] >>
      token >>
      iface[(qi::_a = qi::_1)] >>
      (qi::eol | qi::eoi)
    )[(pnx::bind(&nmdo::InterfaceNetwork::setName, &qi::_val, qi::_a),
       pnx::bind(&nmdo::MacAddress::addIpAddress, &qi::_b, qi::_c),
       pnx::bind(&nmdo::InterfaceNetwork::addReachableMac, &qi::_val, qi::_b))]
    ;

  // =============================================================================
  // NDP Rules
  // =============================================================================

  ndp =
    ( ndpArista
    | ndpCiscoIos
    | ndpCiscoIosDetail
    )
    ;

  // -----------------------------------------------------------------------------
  // NDP: Arista
  // -----------------------------------------------------------------------------

  ndpArista =
    ndpHeaderArista >>
    *ndpEntryArista
    ;

  ndpHeaderArista =
    -(qi::lit("VRF:") >> token >> qi::eol) >>
    qi::lit("IPv6 Address") >>
    qi::lit("Age") >>
    qi::lit("Hardware Addr") >>
    qi::lit("State") >>
    qi::lit("Interface") >>
    (qi::eol | qi::eoi)
    ;

  ndpEntryArista =
    ( ipv6Addr[(qi::_c = qi::_1)] >>
      age >>
      macAddr[(qi::_b = qi::_1)] >>
      token >>
      iface[(qi::_a = qi::_1)] >>
      -(qi::lit(",") >> +(qi::char_ - qi::eol)) >>
      (qi::eol | qi::eoi)
    )[(pnx::bind(&nmdo::InterfaceNetwork::setName, &qi::_val, qi::_a),
       pnx::bind(&nmdo::MacAddress::addIpAddress, &qi::_b, qi::_c),
       pnx::bind(&nmdo::InterfaceNetwork::addReachableMac, &qi::_val, qi::_b))]
    ;

  // -----------------------------------------------------------------------------
  // NDP: Cisco
  // -----------------------------------------------------------------------------

  ndpCiscoIos =
    ndpHeaderCiscoIos >>
    *ndpEntryCiscoIos
    ;

  ndpCiscoIosDetail =
    ndpHeaderCiscoIosDetail >>
    *ndpEntryCiscoIosDetail
    ;

  ndpHeaderCiscoIos =
    qi::lit("IPv6 Address") >>
    qi::lit("Age") >>
    qi::lit("Link-layer Addr") >>
    qi::lit("State") >>
    qi::lit("Interface") >>
    (qi::eol | qi::eoi)
    ;

  ndpEntryCiscoIos =
    ( ipv6Addr[(qi::_c = qi::_1)] >>
      age >>
      macAddr[(qi::_b = qi::_1)] >>
      token >>
      iface[(qi::_a = qi::_1)] >>
      (qi::eol | qi::eoi)
    )[(pnx::bind(&nmdo::InterfaceNetwork::setName, &qi::_val, qi::_a),
       pnx::bind(&nmdo::MacAddress::addIpAddress, &qi::_b, qi::_c),
       pnx::bind(&nmdo::InterfaceNetwork::addReachableMac, &qi::_val, qi::_b))]
    ;

  ndpHeaderCiscoIosDetail =
    qi::lit("IPv6 Address") >>
    qi::lit("TRLV") >>
    qi::lit("Age") >>
    qi::lit("Link-layer Addr") >>
    qi::lit("State") >>
    qi::lit("Interface") >>
    (qi::eol | qi::eoi)
    ;

  ndpEntryCiscoIosDetail =
    ( ipv6Addr[(qi::_c = qi::_1)] >>
      token >>
      age >>
      macAddr[(qi::_b = qi::_1)] >>
      token >>
      iface[(qi::_a = qi::_1)] >>
      (qi::eol | qi::eoi)
    )[(pnx::bind(&nmdo::InterfaceNetwork::setName, &qi::_val, qi::_a),
       pnx::bind(&nmdo::MacAddress::addIpAddress, &qi::_b, qi::_c),
       pnx::bind(&nmdo::InterfaceNetwork::addReachableMac, &qi::_val, qi::_b))]
    ;

  // -----------------------------------------------------------------------------

  age =
    +(qi::ascii::graph)
    ;

  iface =
    +(qi::ascii::graph - qi::char_(","))
    ;

  token =
    +(qi::ascii::graph)
    ;

  errorMessage =
    -(qi::lit(">") >> *(qi::char_ - qi::eol) >> qi::eol) >>
    (qi::lit("%") >> *(qi::char_ - qi::eol) >> qi::eol)
    ;


  BOOST_SPIRIT_DEBUG_NODES(
      (start)
      (age)
      (iface)
      (errorMessage)
      // ARP
      (arpArista)         (arpHeaderArista)         (arpEntryArista)
      (arpCiscoIos)       (arpHeaderCiscoIos)       (arpEntryCiscoIos)
      (arpCiscoNxos)      (arpHeaderCiscoNxos)      (arpEntryCiscoNxos)
      (arpCiscoWlc)       (arpHeaderCiscoWlc)       (arpEntryCiscoWlc)
      // NDP
      (ndpArista)         (ndpHeaderArista)         (ndpEntryArista)
      (ndpCiscoIos)       (ndpHeaderCiscoIos)       (ndpEntryCiscoIos)
      (ndpCiscoIosDetail) (ndpHeaderCiscoIosDetail) (ndpEntryCiscoIosDetail)
      );
}


// =============================================================================
// Parser helper methods
// =============================================================================
