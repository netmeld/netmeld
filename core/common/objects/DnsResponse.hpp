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

#ifndef DNS_RESPONSE_HPP
#define DNS_RESPONSE_HPP

#include <compare>
#include <string>
#include <vector>

#include <netmeld/core/objects/AbstractObject.hpp>


namespace netmeld::core::objects {

  class DnsResponse : public AbstractObject
  {
    // =========================================================================
    // Variables
    // =========================================================================
    private:
    protected:
      std::string responseFqdn;
      std::string responseClass;
      std::string responseType;
      uint32_t    responseTtl;
      std::string responseData;

    public:

    // =========================================================================
    // Constructors
    // =========================================================================
    private:
    protected:
    public:
      DnsResponse();

    // =========================================================================
    // Methods
    // =========================================================================
    private:
    protected:
    public:
			void setFqdn(const std::string&);
			void setClass(const std::string&);
      void setType(const std::string&);
      void setTtl(const uint32_t);
      void setData(const std::string&);

      std::string getFqdn() const;
      std::string getClass() const;
      std::string getType() const;
      uint32_t    getTtl() const;
      std::string getData() const;

      std::string toDebugString() const override;

      std::strong_ordering operator<=>(const DnsResponse&) const = default;
      bool operator==(const DnsResponse&) const = default;
  };

  typedef std::vector<DnsResponse> DnsResponses;
}

#endif // DNS_RESPONSE_HPP
