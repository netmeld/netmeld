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

#ifndef INTERFACE_NETWORK_HPP
#define INTERFACE_NETWORK_HPP

#include <set>

#include <netmeld/datastore/objects/AbstractDatastoreObject.hpp>
#include <netmeld/datastore/objects/IpAddress.hpp>
#include <netmeld/datastore/objects/MacAddress.hpp>
#include <netmeld/datastore/objects/Vlan.hpp>


namespace netmeld::datastore::objects {

  class InterfaceNetwork : public AbstractDatastoreObject {
    // =========================================================================
    // Variables
    // =========================================================================
    private: // Variables will probably rarely appear at this scope
    protected: // Variables intended for internal/subclass API
      std::string  name;
      std::string  description;

      bool isPartial {false};

      // Default interface settings
      std::string  mediaType {"ethernet"};
      bool         isUp      {true}; // Assume up, parser can set actual
      bool         isDiscoveryProtocolEnabled {true}; // cisco cdp, junos lldp
      MacAddress   macAddr;

      // Default switchport settings
      std::string           mode {"l3"}; // TODO layer 2/3, mode access/trunk
      bool                  isPortSecurityEnabled {false};
      unsigned short        portSecurityMaxMacAddrs {1};
      std::string           portSecurityViolationAction {"shutdown"};
                            // protect|restrict|shutdown
      bool                  isPortSecurityStickyMac {false};
      std::set<MacAddress>  learnedMacAddrs;
      std::set<MacAddress>  reachableMacAddrs;
      std::set<Vlan>        vlans;

      // Default spanning-tree settings
      bool isBpduGuardEnabled   {false};
      bool isBpduFilterEnabled  {false};
      bool isPortfastEnabled    {false};

    public: // Variables should rarely appear at this scope

    // =========================================================================
    // Constructors
    // =========================================================================
    private: // Constructors which should be hidden from API users
    protected: // Constructors part of subclass API
    public: // Constructors part of public API
      InterfaceNetwork();
      explicit InterfaceNetwork(const std::string&);


    // =========================================================================
    // Methods
    // =========================================================================
    private: // Methods which should be hidden from API users
    protected: // Methods part of subclass API
    public: // Methods part of public API
      void addIpAddress(const IpAddress&);
      void addPortSecurityStickyMac(const MacAddress&);
      void addReachableMac(const MacAddress&);
      void addVlan(const uint16_t);
      void addVlanRange(const uint16_t, const uint16_t);
      void setDiscoveryProtocol(bool);
      void setDescription(const std::string&);
      void setMacAddress(const MacAddress&);
      void setMediaType(const std::string&);
      void setName(const std::string&);
      void setState(bool);
      void setSwitchportMode(const std::string&);
      void setPortSecurity(bool);
      void setPortSecurityMaxMacAddrs(const unsigned short);
      void setPortSecurityViolationAction(const std::string&);
      void setPortSecurityStickyMac(bool);
      void setBpduGuard(bool);
      void setBpduFilter(bool);
      void setPortfast(bool);
      void setPartial(bool);

      std::string getName() const;
      bool getState() const;
      const std::set<IpAddress>& getIpAddresses() const;
      const std::set<Vlan>& getVlans() const;

      // Always overriden from AbstractDatastoreObject
      bool isValid() const override;
      void save(pqxx::transaction_base&,
                const nmco::Uuid&, const std::string&) override;
      std::string toDebugString() const override;
  };
}
#endif // INTERFACE_NETWORK_HPP
