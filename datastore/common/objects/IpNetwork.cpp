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


#include <netmeld/datastore/objects/IpNetwork.hpp>
#include <netmeld/datastore/parsers/ParserHelper.hpp>
#include <netmeld/datastore/parsers/ParserIpAddress.hpp>

#include <boost/math/special_functions/relative_difference.hpp>

namespace nmdp = netmeld::datastore::parsers;
namespace nmcu = netmeld::core::utils;


namespace netmeld::datastore::objects {

  // ===========================================================================
  // Constructors
  // ===========================================================================
  IpNetwork::IpNetwork()
  {}

  IpNetwork::IpNetwork(const std::string& _addr, const std::string& _reason) :
    reason(_reason)
  {
    IpNetwork temp = nmdp::fromString<nmdp::ParserIpAddress, IpAddress>(_addr);
    address        = temp.address;
    prefix         = temp.prefix;
  }

  IpNetwork::IpNetwork(const std::vector<uint8_t>& _addr)
  {
    std::ostringstream oss;
    size_t size {_addr.size()};
    if (4 == size) {
      oss << std::dec;
      for (size_t i {0}; i < size; i++) {
        oss << static_cast<uint16_t>(_addr.at(i));
        if (size > (i+1)) { oss << "."; }
      }
      setAddress(oss.str());
    } else if (16 == size) {
      oss << std::hex;
      for (size_t i {0}; i < size; i+=2) {
        oss << static_cast<uint16_t>(_addr.at(i))
            << static_cast<uint16_t>(_addr.at(i+1));
        if (size > (i+2)) { oss << ":"; }
      }
      setAddress(oss.str());
    } else {
      LOG_WARN << "IP address vector not of appropriate size "
              << "for IPv4 (4) or IPv6 (16), doing nothing"
              << std::endl;
    }
  }

  // ===========================================================================
  // Methods
  // ===========================================================================
  template<size_t n> std::string
  IpNetwork::convert() const
  {
    if (prefix == n) { // Short circuit trivial case
      return address.to_string();
    }

    // Convert prefix to bitset
    std::bitset<n> maskBits;
    for (size_t i {n}; i > n-prefix; i--) {
      maskBits.set(i-1);
    }

    // Convert address to bitset
    std::bitset<n> addrBits;
    if (isV4()) {
      for (auto byte : address.to_v4().to_bytes()) {
        addrBits <<= 8;
        addrBits |= byte;
      }
    } else {
      for (auto byte : address.to_v6().to_bytes()) {
        addrBits <<= 8;
        addrBits |= byte;
      }
    }

    std::bitset<n> networkHostOrder {addrBits & maskBits};
    std::bitset<n> networkNetOrder;
    size_t size {n/8};
    for (size_t i {0}; i < size; i++) {
      std::bitset<n> temp {networkHostOrder};
      // chop off don't care or already added bits
      temp >>= 8*(size-1-i);
      temp <<= 8*(size-1);
      networkNetOrder >>= 8;
      networkNetOrder |= temp;
    }

    char maskStr[INET6_ADDRSTRLEN];
    auto domain {isV4() ? AF_INET : AF_INET6};
    inet_ntop(domain, &networkNetOrder, maskStr, INET6_ADDRSTRLEN);

    return std::string(maskStr);
  }

  std::string
  IpNetwork::getNetwork() const
  {
    if (isDefault()) {
      return address.to_string();
    } else if (isV4()) {
      if (32 < prefix) {
        LOG_ERROR << "IPv4 network with invalid CIDR" << std::endl;
        return "";
      }
      return convert<32>();
    } else if (isV6()) {
      if (128 < prefix) {
        LOG_ERROR << "IPv6 network with invalid CIDR" << std::endl;
        return "";
      }
      return convert<128>();
    } else {
      return "";
    }
  }

  IpNetwork
  IpNetwork::getIpv4Default()
  {
    static const IpNetwork instance {"0.0.0.0/0", "Netmeld IPv4 default used"};
    return instance;
  }

  IpNetwork
  IpNetwork::getIpv6Default()
  {
    static const IpNetwork instance {"::/0", "Netmeld IPv6 default used"};
    return instance;
  }

  void
  IpNetwork::setAddress(const std::string& _addr)
  {
    try {
      address = IpAddr::from_string(_addr);
      setPrefix(prefix);
    } catch (std::exception& e) {
      LOG_ERROR << "IP address malformed for parser: " << _addr << std::endl;
      std::exit(nmcu::Exit::FAILURE);
    }
  }

  void
  IpNetwork::setPrefix(uint8_t _prefix)
  {
    if (UINT8_MAX == _prefix) {
      prefix = (address.is_v4()) ? 32 : 128;
    } else {
      prefix = _prefix;
    }
  }

  void
  IpNetwork::setExtraWeight(const double _extraWeight)
  {
    extraWeight = _extraWeight;
  }

  template<size_t bits> bool
  IpNetwork::setPrefixFromMask(const IpNetwork& _mask, bool flipIt)
  {
    size_t count {0};
    bool isContiguous {true};
    bool seenZero {false};

    std::bitset<bits> bVal = _mask.asBitset<bits>();
    if (flipIt) {bVal.flip();}
    for (size_t i {1}; i <= bits; ++i) {
      bool isOne {bVal.test(bits-i)};
      if (isOne && seenZero) {
        isContiguous = false;
        count = bits;
      } else if (isOne) {
        ++count;
      } else {
        seenZero = true;
      }
    }

    prefix = static_cast<uint8_t>(count);

    return isContiguous;
  }

  bool
  IpNetwork::setNetmask(const IpNetwork& _mask)
  {
    if (_mask.isV4()) {
      return setPrefixFromMask<32>(_mask, false);
    } else {
      return setPrefixFromMask<128>(_mask, false);
    }
  }

  bool
  IpNetwork::setWildcardMask(const IpNetwork& _mask)
  {
    if (_mask.isV4()) {
      return setPrefixFromMask<32>(_mask, true);
    } else {
      return setPrefixFromMask<128>(_mask, true);
    }
  }

  template<size_t n> std::bitset<n>
  IpNetwork::asBitset() const
  {
    std::bitset<n> addrBits;
    if (isV4()) {
      for (auto byte : address.to_v4().to_bytes()) {
        addrBits <<= 8;
        addrBits |= byte;
      }
    } else {
      for (auto byte : address.to_v6().to_bytes()) {
        addrBits <<= 8;
        addrBits |= byte;
      }
    }

    return addrBits;
  }

  bool
  IpNetwork::setMask(const IpNetwork& _mask)
  {
    if (_mask.isV4()) {
      const auto& bVal {_mask.asBitset<32>()};
      if (bVal.test(0)) {
        return setWildcardMask(_mask);
      } else {
        return setNetmask(_mask);
      }
    } else {
      const auto& bVal {_mask.asBitset<128>()};
      if (bVal.test(0)) {
        return setWildcardMask(_mask);
      } else {
        return setNetmask(_mask);
      }
    }
  }

  void
  IpNetwork::setReason(const std::string& _reason)
  {
    reason = _reason;
  }

  uint8_t
  IpNetwork::getPrefix() const
  {
    return prefix;
  }

  bool
  IpNetwork::isDefault() const
  {
    return UINT8_MAX == prefix;
  }

  bool
  IpNetwork::isValid() const
  {
    /* Never insert:
         loopback   , multicast  , unspecified
         127.0.0.1/8, 224.0.0.0/4, 0.0.0.0/?
         ::1/128    , ff00::/8   , ::/?

         singular networks, i.e. /32 or /128
    */
    return !(
               address.is_loopback()
            || address.is_multicast()
            || address.is_unspecified()
            )
        && (
              (isV4() && 32  > prefix)
           || (isV6() && 128 > prefix)
           )
        ;
  }
  bool
  IpNetwork::isV4() const
  {
    return address.is_v4();
  }

  bool
  IpNetwork::isV6() const
  {
    return address.is_v6();
  }

  void
  IpNetwork::save(pqxx::transaction_base& t,
                  const nmco::Uuid& toolRunId, const std::string& deviceId)
  {
    if (!isValid()) {
      LOG_DEBUG << "IpNetwork object is not saving: " << toDebugString()
                << std::endl;
      return;
    }

    std::string fullReason {reason};
    if (!fullReason.empty() && !deviceId.empty()) {
      fullReason = deviceId + "'s " + fullReason;
    }

    t.exec_prepared("insert_raw_ip_net",
      toolRunId,
      toString(),
      fullReason); // insert converts '' to null

    if (0.0 < extraWeight) {
      t.exec_prepared("insert_ip_net_extra_weight",
        toString(),
        extraWeight);
    }
  }

  std::string
  IpNetwork::toString() const
  {
    std::ostringstream oss;
    oss << getNetwork() << '/' << static_cast<uint16_t>(prefix);
    return oss.str();
  }

  std::string
  IpNetwork::toDebugString() const
  {
    std::ostringstream oss;

    oss << "[";

    oss << toString() << ", "
        << reason << ", "
        << extraWeight
        ;

    oss << "]";

    return oss.str();
  }

  std::partial_ordering
  IpNetwork::operator<=>(const IpNetwork& rhs) const
  {
    // boost::asio::ip::address doesn't have operator<=>() yet.
    if (address < rhs.address) {
      return std::partial_ordering::less;
    }
    if (address > rhs.address) {
      return std::partial_ordering::greater;
    }

    if (auto cmp = prefix <=> rhs.prefix; 0 != cmp) {
      return cmp;
    }

    if (auto cmp = reason <=> rhs.reason; 0 != cmp) {
      return cmp;
    }

    if (boost::math::epsilon_difference(extraWeight, rhs.extraWeight) <= 1000.0) {
      return std::partial_ordering::equivalent;
    }
    return extraWeight <=> rhs.extraWeight;
  }

  bool
  IpNetwork::operator==(const IpNetwork& rhs) const
  {
    return 0 == operator<=>(rhs);
  }
}
