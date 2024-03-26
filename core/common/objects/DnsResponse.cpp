// =============================================================================
// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
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

#include <sstream>
#include <boost/algorithm/string.hpp>

#include <netmeld/core/objects/DnsResponse.hpp>
#include <netmeld/core/utils/StringUtilities.hpp>

namespace nmcu = netmeld::core::utils;


namespace netmeld::core::objects {

  DnsResponse::DnsResponse() :
    responseFqdn{},
    responseClass{},
    responseType{},
    responseTtl{0},
    responseData{}
  { }

  void
  DnsResponse::setFqdn(const std::string& _fqdn)
  {
    responseFqdn = nmcu::toLower(_fqdn);
    boost::trim_right_if(responseFqdn, boost::is_any_of("."));
  }

  void
  DnsResponse::setClass(const std::string& _class)
  {
    responseClass = nmcu::toUpper(_class);
  }

  void
  DnsResponse::setType(const std::string& _type)
  {
    responseType = nmcu::toUpper(_type);
  }

  void
  DnsResponse::setTtl(const uint32_t _ttl)
  {
    responseTtl = _ttl;
  }

  void
  DnsResponse::setData(const std::string& _data)
  {
    // Whether the data should be lower-cased or trimmed
    // depends on the TYPE. So store the data as-is here.
    // Manipulation is done in getData() based on responseType.
    responseData = _data;
  }

  std::string
  DnsResponse::getFqdn() const
  {
    return responseFqdn;
  }

  std::string
  DnsResponse::getClass() const
  {
    return responseClass;
  }

  std::string
  DnsResponse::getType() const
  {
    return responseType;
  }

  uint32_t
  DnsResponse::getTtl() const
  {
    return responseTtl;
  }

  std::string
  DnsResponse::getData() const
  {
    std::string data{responseData};

    if (// CNAME data: <domain-name>
        ("CNAME" == responseType) ||
        // DNAME data: <domain-name>
        ("DNAME" == responseType) ||
        // KX data: <preference> <domain-name>
        ("KX" == responseType) ||
        // MX data: <preference> <domain-name>
        ("MX" == responseType) ||
        // NS data: <domain-name>
        ("NS" == responseType) ||
        // PTR data: <domain-name>
        ("PTR" == responseType) ||
        // SRV data: <priority> <weight> <port> <domain-name>
        ("SRV" == responseType)) {
      data = nmcu::toLower(data);
      boost::trim_right_if(data, boost::is_any_of("."));
    }

    return data;
  }

  std::string
  DnsResponse::toDebugString() const
  {
    std::ostringstream oss;

    oss << "["
        << "responseFqdn: " << responseFqdn << ", "
        << "responseClass: " << responseClass << ", "
        << "responseType: " << responseType << ", "
        << "responseTtl: " << responseTtl << ", "
        << "responseData: " << responseData
        << "]";

    return oss.str();
  }

  std::strong_ordering
  DnsResponse::operator<=>(const DnsResponse& rhs) const
  {
    return std::tie( responseFqdn
                   , responseClass
                   , responseType
                   , responseTtl
                   , responseData
                   )
       <=> std::tie( rhs.responseFqdn
                   , rhs.responseClass
                   , rhs.responseType
                   , rhs.responseTtl
                   , rhs.responseData
                   )
      ;
  }

  bool
  DnsResponse::operator==(const DnsResponse& rhs) const
  {
    return 0 == operator<=>(rhs);
  }
}
