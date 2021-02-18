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

#include <netmeld/datastore/objects/IpAddress.hpp>
#include <netmeld/core/utils/StringUtilities.hpp>

#include <boost/math/special_functions/relative_difference.hpp>

namespace nmcu = netmeld::core::utils;


namespace netmeld::datastore::objects {

  // ===========================================================================
  // Constructors
  // ===========================================================================
  IpAddress::IpAddress()
  {}

  IpAddress::IpAddress(const std::string& _addr, const std::string& _reason) :
    IpNetwork(_addr, _reason)
  {}

  IpAddress::IpAddress(const std::vector<uint8_t>& _addr) :
    IpNetwork(_addr)
  {}

  // ===========================================================================
  // Methods
  // ===========================================================================
  IpAddress
  IpAddress::getIpv4Default()
  {
    static const IpAddress instance {"0.0.0.0/0", "Netmeld IPv4 default used"};
    return instance;
  }
  IpAddress
  IpAddress::getIpv6Default()
  {
    static const IpAddress instance {"::/0", "Netmeld IPv6 default used"};
    return instance;
  }

  void
  IpAddress::addAlias(const std::string& _alias, const std::string& _reason)
  {
    if (_alias.empty()) return;

    aliases.insert(nmcu::toLower(_alias));
    reason = nmcu::toLower(_reason);
  }

  std::set<std::string>
  IpAddress::getAliases() const
  {
    return aliases;
  }

  void
  IpAddress::setResponding(const bool _isUp)
  {
    isResponding = _isUp;
  }

  bool
  IpAddress::isValid() const
  {
    /* Never insert:
       loopback   , multicast  , unspecified
       127.0.0.1/8, 224.0.0.0/4, 0.0.0.0/?
       ::1/128    , ff00::/8   , ::/?
    */
    return !(
               address.is_loopback()
            || address.is_multicast()
            || address.is_unspecified()
            )
        && (
              (isV4() && 32  >= prefix)
           || (isV6() && 128 >= prefix)
           )
        ;
  }

  void
  IpAddress::save(pqxx::transaction_base& t,
                  const nmco::Uuid& toolRunId, const std::string& deviceId)
  {
    if (!isValid()) {
      LOG_DEBUG << "IpAddress object is not saving: " << toDebugString()
                << std::endl;
      return;
    }

    // Upcast this object and attempt to save it separately
    // NOTE: Calling IpNetwork::save() is wrong as isValid is overridden
    IpNetwork ipNet = *this;
    if (ipNet.isValid()) { // so no debug when this is valid but ipNet is not
      ipNet.save(t, toolRunId, deviceId);
    }

    std::string fullReason {reason};
    if (!fullReason.empty() && !deviceId.empty()) {
      fullReason = deviceId + "'s " + fullReason;
    }

    t.exec_prepared("insert_raw_ip_addr",
      toolRunId,
      toString(),
      isResponding);

    for (const auto& alias : aliases) {
      t.exec_prepared("insert_raw_hostname",
        toolRunId,
        toString(),
        alias,
        fullReason);
    }
  }

  std::string
  IpAddress::toString() const
  {
    std::ostringstream oss;
    oss << address << '/' << static_cast<unsigned int>(prefix);
    return oss.str();
  }

  std::string
  IpAddress::toDebugString() const
  {
    std::ostringstream oss;

    oss << "[";

    oss << toString() << ", "
        << isResponding << ", "
        << reason << ", "
        << extraWeight << ", "
        << aliases
        ;

    oss << "]";

    return oss.str();
  }

  bool
  operator<(const IpAddress& first, const IpAddress& second)
  {
    return first.address < second.address;
  }

  bool
  operator==(const IpAddress& first, const IpAddress& second)
  {
    return first.address == second.address
        && first.prefix == second.prefix
        && first.reason == second.reason
        && boost::math::epsilon_difference(first.extraWeight, second.extraWeight) <= 1000.0
        && first.isResponding == second.isResponding
        && first.aliases == second.aliases;
  }
}
