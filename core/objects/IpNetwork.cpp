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


#include <netmeld/core/objects/IpNetwork.hpp>
#include <netmeld/core/parsers/ParserHelper.hpp>
#include <netmeld/core/parsers/ParserIpAddress.hpp>

namespace nmcp = netmeld::core::parsers;
namespace nmcu = netmeld::core::utils;


namespace netmeld::core::objects {

  // ===========================================================================
  // Constructors
  // ===========================================================================
  IpNetwork::IpNetwork()
  {}

  IpNetwork::IpNetwork(const std::string& _addr, const std::string& _reason) :
    reason(_reason)
  {
    IpNetwork temp = nmcp::fromString<nmcp::ParserIpAddress, IpAddress>(_addr);
    address        = temp.address;
    cidr           = temp.cidr;
  }

  IpNetwork::IpNetwork(const std::vector<uint8_t>& _addr)
  {
    std::ostringstream oss;
    size_t size {_addr.size()};
    if (4 == size) {
      oss << std::dec;
      for (size_t i {0}; i < size; i++) {
        oss << static_cast<unsigned int>(_addr.at(i));
        if (size > (i+1)) { oss << "."; }
      }
      setAddress(oss.str());
    } else if (16 == size) {
      oss << std::hex;
      for (size_t i {0}; i < size; i+=2) {
        oss << static_cast<unsigned int>(_addr.at(i))
            << static_cast<unsigned int>(_addr.at(i+1));
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
    if (cidr == n) { // Short circuit trivial case
      return address.to_string();
    }

    // Convert cidr to bitset
    std::bitset<n> maskBits;
    for (size_t i {n}; i > n-cidr; i--) {
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
      if (32 < cidr) {
        LOG_ERROR << "IPv4 network with invalid CIDR" << std::endl;
        return "";
      }
      return convert<32>();
    } else if (isV6()) {
      if (128 < cidr) {
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
      setCidr(cidr);
    } catch (std::exception& e) {
      LOG_ERROR << "IP address malformed for parser: " << _addr << std::endl;
      std::exit(nmcu::Exit::FAILURE);
    }
  }

  void
  IpNetwork::setCidr(uint8_t _cidr)
  {
    if (UINT8_MAX == _cidr) {
      cidr = (address.is_v4()) ? 32 : 128;
    } else {
      cidr = _cidr;
    }
  }

  void
  IpNetwork::setExtraWeight(const uint32_t _extraWeight)
  {
    extraWeight = _extraWeight;
  }

  void
  IpNetwork::setNetmask(const IpNetwork& _mask)
  {
    size_t count {0};
    size_t base;
    char   sep;

    if (_mask.isV4()) {
      base = 10;
      sep = '.';
    } else {
      base = 16;
      sep = ':';
    }

    std::istringstream octets(_mask.address.to_string());
    for (std::string octet;
         std::getline(octets, octet, sep) && !octet.empty();
        ) {
      size_t end;
      unsigned long val = std::stoul(octet, &end, base);

      while (val != 0) {
        if (val & 0x1) { count++; }
        val >>= 1;
      }
    }

    cidr = count;
  }

  void
  IpNetwork::setWildcardNetmask(const IpNetwork& _mask)
  {
    //TODO: Implement this functionality
    //TODO: Write a test for this functionality
  }

  void
  IpNetwork::setReason(const std::string& _reason)
  {
    reason = _reason;
  }

  bool
  IpNetwork::isDefault() const
  {
    return UINT8_MAX == cidr;
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
              (isV4() && 32  > cidr)
           || (isV6() && 128 > cidr)
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
                  const Uuid& toolRunId, const std::string& deviceId)
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

    if (0 < extraWeight) {
      t.exec_prepared("insert_ip_net_extra_weight",
        toString(),
        extraWeight);
    }
  }

  std::string
  IpNetwork::toString() const
  {
    std::ostringstream oss;
    oss << getNetwork() << '/' << static_cast<unsigned int>(cidr);
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

  bool
  operator<(const IpNetwork& first, const IpNetwork& second)
  {
    return first.address < second.address;
  }

  bool
  operator==(const IpNetwork& first, const IpNetwork& second)
  {
    return first.address == second.address
        && first.cidr == second.cidr
        && first.reason == second.reason
        && first.extraWeight == second.extraWeight
        ;
  }
}
