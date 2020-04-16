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

#ifndef IP_NETWORK_HPP
#define IP_NETWORK_HPP

#include <boost/asio/ip/address.hpp>

#include <netmeld/core/objects/AbstractObject.hpp>

using IpAddr = boost::asio::ip::address;


namespace netmeld::core::objects {

  class IpNetwork : public AbstractObject {
    // =========================================================================
    // Variables
    // =========================================================================
    private:
    protected:
      IpAddr       address;
      uint8_t      cidr        {UINT8_MAX};
      std::string  reason;
      uint32_t     extraWeight {0};

    public:

    // =========================================================================
    // Constructors
    // =========================================================================
    private:
    protected:
    public:
      IpNetwork();
      explicit IpNetwork(const std::string&, const std::string& x="");
      explicit IpNetwork(const std::vector<uint8_t>&);

    // =========================================================================
    // Methods
    // =========================================================================
    private:
    protected:
      template<size_t n>
        bool setCidrFromMask(const IpNetwork&, const size_t, const char, bool);
      template<size_t n>
        std::string convert() const;

      std::string getNetwork() const;

    public:
      static IpNetwork getIpv4Default();
      static IpNetwork getIpv6Default();

      void setAddress(const std::string&);
      void setCidr(uint8_t);
      void setExtraWeight(const uint32_t);
      void setNetmask(const IpNetwork&);
      bool setWildcardMask(const IpNetwork&);
      void setReason(const std::string&);

      bool isDefault() const;
      bool isValid() const override;
      bool isV4() const;
      bool isV6() const;

      void save(pqxx::transaction_base&, const Uuid&, const std::string&) override;

      std::string toString() const;
      std::string toDebugString() const override;

      friend bool operator<(const IpNetwork&, const IpNetwork&);
      friend bool operator==(const IpNetwork&, const IpNetwork&);
  };
}
#endif // IP_NETWORK_HPP
