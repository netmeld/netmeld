// =============================================================================
// Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC
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

#include <netmeld/datastore/objects/InterfaceNetwork.hpp>
#include <netmeld/core/utils/StringUtilities.hpp>

namespace nmcu = netmeld::core::utils;


namespace netmeld::datastore::objects {

  // ===========================================================================
  // Constructors
  // ===========================================================================
  InterfaceNetwork::InterfaceNetwork()
  {}

  InterfaceNetwork::InterfaceNetwork(const std::string& _name) :
    name(nmcu::toLower(_name))
  {}


  // ===========================================================================
  // Methods
  // ===========================================================================
  void
  InterfaceNetwork::addIpAddress(const IpAddress& _ipAddr)
  {
    if (_ipAddr == IpAddress()) { return; } // don't add defaults
    macAddr.addIpAddress(_ipAddr);
  }

  void
  InterfaceNetwork::addPortSecurityStickyMac(const MacAddress& _macAddr)
  {
    if (_macAddr == MacAddress()) { return; } // don't add defaults
    learnedMacAddrs.insert(_macAddr);
  }

  void
  InterfaceNetwork::addReachableMac(const MacAddress& _macAddr)
  {
    if (_macAddr == MacAddress()) { return; } // don't add defaults
    reachableMacAddrs.insert(_macAddr);
  }

  void
  InterfaceNetwork::addVlan(const uint16_t _vlan)
  {
    vlans.emplace(_vlan);
  }

  void
  InterfaceNetwork::addVlanRange(const uint16_t first, const uint16_t last)
  {
    for (auto i {first}; i <= last; ++i) {
      addVlan(i);
    }
  }

  void
  InterfaceNetwork::setDiscoveryProtocol(bool _state)
  {
    isDiscoveryProtocolEnabled = _state;
  }
  void
  InterfaceNetwork::setDescription(const std::string& _description)
  {
    description = _description;
  }

  void
  InterfaceNetwork::setMacAddress(const MacAddress& _macAddr)
  {
    macAddr.setMac(_macAddr);
  }

  void
  InterfaceNetwork::setMediaType(const std::string& _mediaType)
  {
    mediaType = nmcu::toLower(_mediaType);
  }

  void
  InterfaceNetwork::setName(const std::string& _name)
  {
    name = nmcu::toLower(_name);
  }

  void
  InterfaceNetwork::setState(bool _state)
  {
    isUp = _state;
  }

  void
  InterfaceNetwork::setSwitchportMode(const std::string& _mode)
  {
    mode = nmcu::toLower(_mode);
  }

  void
  InterfaceNetwork::setPortSecurity(bool _state)
  {
    isPortSecurityEnabled = _state;
  }

  void
  InterfaceNetwork::setPortSecurityMaxMacAddrs(const unsigned short _value)
  {
    portSecurityMaxMacAddrs = _value;
  }

  void
  InterfaceNetwork::setPortSecurityViolationAction(const std::string& _action)
  {
    portSecurityViolationAction = nmcu::toLower(_action);
  }

  void
  InterfaceNetwork::setPortSecurityStickyMac(bool _state)
  {
    isPortSecurityStickyMac = _state;
  }

  void
  InterfaceNetwork::setBpduGuard(bool _state)
  {
    isBpduGuardEnabled = _state;
  }

  void
  InterfaceNetwork::setBpduFilter(bool _state)
  {
    isBpduFilterEnabled = _state;
  }

  void
  InterfaceNetwork::setPortfast(bool _state)
  {
    isPortfastEnabled = _state;
  }

  void
  InterfaceNetwork::setPartial(bool _state)
  {
    isPartial = _state;
  }

  std::string
  InterfaceNetwork::getName() const
  {
    return name;
  }

  bool
  InterfaceNetwork::getState() const
  {
    return isUp;
  }

  const std::set<IpAddress>&
  InterfaceNetwork::getIpAddresses() const
  {
    return macAddr.getIpAddresses();
  }

  const std::set<Vlan>&
  InterfaceNetwork::getVlans() const
  {
    return vlans;
  }

  bool
  InterfaceNetwork::isValid() const
  {
    return !name.empty()
        && !mediaType.empty()
        && !mode.empty()
        ;
  }

  const std::set<MacAddress>& 
  InterfaceNetwork::getPortSecurityStickyMacs() const {
    return learnedMacAddrs;
  }

  const std::set<MacAddress>& 
  InterfaceNetwork::getReachableMacs() const {
    return reachableMacAddrs;
  }

  bool
  InterfaceNetwork::getDiscoveryProtocol() const {
    return isDiscoveryProtocolEnabled;
  }

  std::string 
  InterfaceNetwork::getDescription(const std::string&) const {
    return description;
  }

  const MacAddress& 
  InterfaceNetwork::getMacAddress() const {
    return macAddr;
  }

  std::string
  InterfaceNetwork::getMediaType() const {
    return mediaType;
  }

  std::string 
  InterfaceNetwork::getSwitchportMode() const {
    return mode;
  }

  bool 
  InterfaceNetwork::getPortSecurity() const {
    return isPortSecurityEnabled;
  }

  unsigned short 
  InterfaceNetwork::getPortSecurityMaxMacAddrs() const {
    return portSecurityMaxMacAddrs;
  }

  std::string 
  InterfaceNetwork::getPortSecurityViolationAction() const {
    return portSecurityViolationAction;
  }

  bool 
  InterfaceNetwork::getPortSecurityStickyMac() const {
    return isPortSecurityStickyMac;
  }

  bool 
  InterfaceNetwork::getBpduGuard() const {
    return isBpduGuardEnabled;
  }

  bool 
  InterfaceNetwork::getBpduFilter() const {
    return isBpduFilterEnabled;
  }

  bool 
  InterfaceNetwork::getPortfast() const {
    return isPortfastEnabled;
  }

  bool 
  InterfaceNetwork::getPartial() const {
    return isPartial;
  }

  void
  InterfaceNetwork::save( pqxx::transaction_base& t
                        , const nmco::Uuid& toolRunId
                        , const std::string& deviceId
                        )
  {
    if (!isValid() && !deviceId.empty()) {
      LOG_DEBUG << "InterfaceNetwork object is not saving: " << toDebugString()
                << std::endl;
      return;
    }

    t.exec_prepared( "insert_raw_device_interface"
                   , toolRunId
                   , deviceId
                   , name
                   , mediaType
                   , isUp
                   , description
                   );

    if (!isPartial) {
      t.exec_prepared( "insert_raw_device_interfaces_cdp"
                     , toolRunId
                     , deviceId
                     , name
                     , isDiscoveryProtocolEnabled
                     );

      t.exec_prepared( "insert_raw_device_interfaces_bpdu"
                     , toolRunId
                     , deviceId
                     , name
                     , isBpduGuardEnabled
                     , isBpduFilterEnabled
                     );

      t.exec_prepared( "insert_raw_device_interfaces_portfast"
                     , toolRunId
                     , deviceId
                     , name
                     , isPortfastEnabled
                     );

      t.exec_prepared( "insert_raw_device_interfaces_mode"
                     , toolRunId
                     , deviceId
                     , name
                     , mode
                     );

      t.exec_prepared( "insert_raw_device_interfaces_port_security"
                     , toolRunId
                     , deviceId
                     , name
                     , isPortSecurityEnabled
                     , isPortSecurityStickyMac
                     , portSecurityMaxMacAddrs
                     , portSecurityViolationAction
                     );
    }

    for (auto mac : learnedMacAddrs) {
      mac.save(t, toolRunId, "");

      if (!mac.isValid()) { continue; }

      t.exec_prepared( "insert_raw_device_link_connection"
                     , toolRunId
                     , deviceId
                     , name
                     , mac.toString()
                     );

      t.exec_prepared( "insert_raw_device_interfaces_port_security_mac_addr"
                     , toolRunId
                     , deviceId
                     , name
                     , mac.toString()
                     );
    }

    for (auto mac : reachableMacAddrs) {
      mac.save(t, toolRunId, "");

      if (!mac.isValid()) { continue; }

      t.exec_prepared( "insert_raw_device_link_connection"
                     , toolRunId
                     , deviceId
                     , name
                     , mac.toString()
                     );
    }

    macAddr.setResponding(isUp);
    macAddr.save(t, toolRunId, deviceId);

    if (macAddr.isValid()) {
      t.exec_prepared( "insert_raw_device_mac_addr"
                     , toolRunId
                     , deviceId
                     , name
                     , macAddr.toString()
                     );
    }

    for (const auto& ipAddr : macAddr.getIpAddresses()) {
      if (!ipAddr.isValid()) { continue; }
      t.exec_prepared( "insert_raw_device_ip_addr"
                     , toolRunId
                     , deviceId
                     , name
                     , ipAddr.toString()
                     );
    }

    for (auto vlan : vlans) {
      vlan.save(t, toolRunId, deviceId);

      if (vlan.isValid()) {
        t.exec_prepared( "insert_raw_device_interfaces_vlan"
                       , toolRunId
                       , deviceId
                       , name
                       , vlan.getVlanId()
                       );
      }
    }
  }

  std::string
  InterfaceNetwork::toDebugString() const
  {
    std::ostringstream oss;

    oss << "["; // opening bracket
    oss << "name: " << name
        << ", description: " << description
        << ", isPartial: " << std::boolalpha << isPartial
        << ", mediaType: " << mediaType
        << ", isUp: " << isUp
        << ", isDiscoveryProtocolEnabled: " << isDiscoveryProtocolEnabled
        << ", macAddr: " << macAddr.toDebugString()
        << ", mode: " << mode
        << ", isPortSecurityEnabled: " << isPortSecurityEnabled
        << ", portSecurityMaxMacAddrs: " << portSecurityMaxMacAddrs
        << ", portSecurityViolationAction: " << portSecurityViolationAction
        << ", isPortSecurityStickyMac: " << isPortSecurityStickyMac
        << ", learnedMacAddrs: "  << learnedMacAddrs
        << ", reachableMacs: " << reachableMacAddrs
        << ", vlans: " << vlans
        << ", isPortfastEnabled: " << isPortfastEnabled
        << ", isBpduGuardEnabled: " << isBpduGuardEnabled
        << ", isBpduFilterEnabled: " << isBpduFilterEnabled
        ;
    oss << "]"; // closing bracket

    return oss.str();
  }

  std::partial_ordering
  InterfaceNetwork::operator<=>(const InterfaceNetwork& rhs) const
  {
    return std::tie( name
                   , description
                   , isPartial
                   , mediaType
                   , isUp
                   , isDiscoveryProtocolEnabled
                   , macAddr
                   , mode
                   , isPortSecurityEnabled
                   , portSecurityMaxMacAddrs
                   , portSecurityViolationAction
                   , isPortSecurityStickyMac
                   , learnedMacAddrs
                   , reachableMacAddrs
                   , vlans
                   , isBpduGuardEnabled
                   , isBpduFilterEnabled
                   , isPortfastEnabled
                   )
       <=> std::tie( rhs.name
                   , rhs.description
                   , rhs.isPartial
                   , rhs.mediaType
                   , rhs.isUp
                   , rhs.isDiscoveryProtocolEnabled
                   , rhs.macAddr
                   , rhs.mode
                   , rhs.isPortSecurityEnabled
                   , rhs.portSecurityMaxMacAddrs
                   , rhs.portSecurityViolationAction
                   , rhs.isPortSecurityStickyMac
                   , rhs.learnedMacAddrs
                   , rhs.reachableMacAddrs
                   , rhs.vlans
                   , rhs.isBpduGuardEnabled
                   , rhs.isBpduFilterEnabled
                   , rhs.isPortfastEnabled
                   )
      ;
  }

  bool
  InterfaceNetwork::operator==(const InterfaceNetwork& rhs) const
  {
    return 0 == operator<=>(rhs);
  }

  // ===========================================================================
  // Friends
  // ===========================================================================
}
