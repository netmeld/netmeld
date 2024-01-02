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

#include <netmeld/datastore/objects/DnsResolver.hpp>
#include <netmeld/core/utils/StringUtilities.hpp>

namespace nmcu = netmeld::core::utils;


namespace netmeld::datastore::objects {

  // ===========================================================================
  // Constructors
  // ===========================================================================
  DnsResolver::DnsResolver() :
    dstPort(53)
  {}

  // ===========================================================================
  // Methods
  // ===========================================================================
  /*
  void
  DnsResolver::setIface(const std::string& _ifaceName)
  {
    ifaceName = _ifaceName;
  }
  */

  /*
  void
  DnsResolver::setScopeDomain(const std::string& _scopeDomain)
  {
    scopeDomain = _scopeDomain;
  }
  */

  void
  DnsResolver::setSrcAddress(const IpAddress& _srcIpAddr)
  {
    srcIpAddr = _srcIpAddr;
  }

  void
  DnsResolver::setDstAddress(const IpAddress& _dstIpAddr)
  {
    dstIpAddr = _dstIpAddr;
  }

  /*
  void
  DnsResolver::setDstPort(const uint16_t _dstPort)
  {
    dstPort = _dstPort;
  }
  */

  void
  DnsResolver::save(pqxx::transaction_base& t,
            const nmco::Uuid& toolRunId, const std::string& deviceId)
  {
    t.exec_prepared("insert_raw_device_dns_resolver",
        toolRunId,
        deviceId,
        ifaceName,
        scopeDomain,
        srcIpAddr.toString(),
        dstIpAddr.toString(),
        dstPort
        );
  }

  std::string
  DnsResolver::toDebugString() const
  {
    std::ostringstream oss;
    oss << "[" // opening bracket
        << "ifaceName: " << ifaceName  << ", "
        << "scopeDomain: " << scopeDomain  << ", "
        << "srcIpAddr: " << srcIpAddr  << ", "
        << "dstIpAddr: " << dstIpAddr  << ", "
        << "dstPort: " << dstPort
        << "]"; // closing bracket

    return oss.str();
  }

  std::partial_ordering
  DnsResolver::operator<=>(const DnsResolver& rhs) const
  {
    return std::tie( ifaceName
                   , scopeDomain
                   , srcIpAddr
                   , dstIpAddr
                   , dstPort
                   )
       <=> std::tie( rhs.ifaceName
                   , rhs.scopeDomain
                   , rhs.srcIpAddr
                   , rhs.dstIpAddr
                   , rhs.dstPort
                   )
      ;
  }

  bool
  DnsResolver::operator==(const DnsResolver& rhs) const
  {
    return 0 == operator<=>(rhs);
  }
}
