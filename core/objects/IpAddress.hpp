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

#ifndef IP_ADDRESS_HPP
#define IP_ADDRESS_HPP

#include <set>

#include <netmeld/core/objects/IpNetwork.hpp>


namespace netmeld::core::objects {

  class IpAddress : public IpNetwork {
    // =========================================================================
    // Variables
    // =========================================================================
    private:
    protected:
      // Inherited from IpNetwork
      //   IpAddr       address;
      //   uint8_t      cidr        {UINT8_MAX};
      //   std::string  reason;
      //   uint32_t     extraWeight {0};

      bool isResponding  {false};

      std::set<std::string> aliases;

    public:

    // =========================================================================
    // Constructors
    // =========================================================================
    private:
    protected:
    public:
      IpAddress();
      explicit IpAddress(const std::string&, const std::string& x="");
      explicit IpAddress(const std::vector<uint8_t>&);

    // =========================================================================
    // Methods
    // =========================================================================
    private:
    protected:
      // Inherited from IpNetwork
      //   template<size_t n>
      //   std::string convert() const;
      //   std::string getNetwork() const;
    public:
      // Inherited from IpNetwork
      //   void setAddress(const std::string&);
      //   void setCidr(uint8_t);
      //   void setExtraWeight(uint32_t);
      //   void setNetmask(const IpNetwork&);
      //   void setReason(const std::string&);
      //   bool isDefault() const;
      //   bool isV4() const;
      //   bool isV6() const;

      static IpAddress getIpv4Default();
      static IpAddress getIpv6Default();

      void addAlias(const std::string&, const std::string&);

      void setResponding(const bool);

      bool isValid() const override;

      void save(pqxx::transaction_base&, const Uuid&, const std::string&) override;

      std::string toString() const;
      std::string toDebugString() const override;

      friend bool operator<(const IpAddress&, const IpAddress&);
      friend bool operator==(const IpAddress&, const IpAddress&);
  };
}
#endif // IP_ADDRESS_HPP
