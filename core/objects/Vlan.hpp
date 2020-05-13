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

#ifndef VLAN_HPP
#define VLAN_HPP

#include <netmeld/core/objects/AbstractDatastoreObject.hpp>
#include <netmeld/core/objects/IpNetwork.hpp>


namespace netmeld::core::objects {

  class Vlan : public AbstractDatastoreObject {
    // =========================================================================
    // Variables
    // =========================================================================
    private:
    protected:
      uint16_t     vlanId {UINT16_MAX};
      std::string  description;
      IpNetwork    ipNet;

    public:

    // =========================================================================
    // Constructors
    // =========================================================================
    private:
    protected:
    public:
      Vlan();
      explicit Vlan(const uint16_t, const std::string& x="");

    // =========================================================================
    // Methods
    // =========================================================================
    private:
    protected:
    public:
      uint16_t getVlanId() const;

      void setDescription(const std::string&);
      void setId(const uint16_t);
      void setIpNet(const IpNetwork&);

      bool isValid() const override;

      void save(pqxx::transaction_base&, const Uuid&, const std::string&) override;

      std::string toDebugString() const override;

      friend bool operator<(const Vlan&, const Vlan&);
  };
}
#endif // VLAN_HPP
