// =============================================================================
// Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC
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

#include <netmeld/datastore/objects/PortRange.hpp>
#include <boost/format.hpp>
#include <regex>


namespace netmeld::datastore::objects {

  PortRange::PortRange(uint16_t port) :
    std::tuple<uint16_t, uint16_t>(port, port)
  { }

  PortRange::PortRange(uint16_t portFirst, uint16_t portLast) :
    std::tuple<uint16_t, uint16_t>(portFirst, portLast)
  { }

  PortRange::PortRange(const std::string& _portRangeString) :
    std::tuple<uint16_t, uint16_t>(0, 0)
  {
    const auto& portRangeString {
        translateFromTypicalServiceAlias(_portRangeString)
      };
    // "[x,y]", "[x,y)", "(x,y]", or "(x,y)" port range strings
    {
      std::regex r {R"(^([\[\(])\s*(\d{1,5})\s*,\s*(\d{1,5})\s*([\]\)])$)"};
      std::smatch m;
      if (std::regex_match(portRangeString, m, r)) {
        std::get<0>(*this) = static_cast<uint16_t>(std::stoul(m[2]));
        std::get<1>(*this) = static_cast<uint16_t>(std::stoul(m[3]));
        if ("(" == m[1]) { ++std::get<0>(*this); }
        if (")" == m[4]) { --std::get<1>(*this); }
        return;
      }
    }

    // "x-y" or "x--y" port range strings
    {
      std::regex r {R"(^(\d{1,5})\s*-{1,2}\s*(\d{1,5})$)"};
      std::smatch m;
      if (std::regex_match(portRangeString, m, r)) {
        std::get<0>(*this) = static_cast<uint16_t>(std::stoul(m[1]));
        std::get<1>(*this) = static_cast<uint16_t>(std::stoul(m[2]));
        return;
      }
    }

    // ">x" port range strings
    {
      std::regex r {R"(^>(\d{1,5})$)"};
      std::smatch m;
      if (std::regex_match(portRangeString, m, r)) {
        std::get<0>(*this) = static_cast<uint16_t>(std::stoul(m[1]));
        std::get<1>(*this) = UINT16_MAX;
        ++std::get<0>(*this);
        return;
      }
    }

    // "<x" port range strings
    {
      std::regex r {R"(^<(\d{1,5})$)"};
      std::smatch m;
      if (std::regex_match(portRangeString, m, r)) {
        std::get<0>(*this) = 0;
        std::get<1>(*this) = static_cast<uint16_t>(std::stoul(m[1]));
        --std::get<1>(*this);
        return;
      }
    }

    // "x" single port strings
    {
      std::regex r {R"(^(\d{1,5})$)"};
      std::smatch m;
      if (std::regex_match(portRangeString, m, r)) {
        std::get<0>(*this) = static_cast<uint16_t>(std::stoul(m[1]));
        std::get<1>(*this) = static_cast<uint16_t>(std::stoul(m[1]));
        return;
      }
    }
  }

  std::string
  PortRange::translateFromTypicalServiceAlias(const std::string& _data) const
  {
    std::vector<std::tuple<std::string, std::string>> mappings {
          {"any", "0-65535"}
        // reverse sort (sort!) to ensure shorter spellings don't match first
        // hyphenated first (prevents accidental early replacement)
        , {"ptp-general", "320"}
        , {"ptp-event", "319"}
        , {"netbios-ns", "137"}
        , {"multihop-bfd", "4784"}
        , {"micro-bfd", "6784"}
        , {"ftp-data", "20"}
        , {"dhcpv6-server", "547"}
        , {"dhcpv6-client", "546"}
        , {"bfd-echo", "3785"}
        // non-hyphenated second
        , {"telnet", "23"}
        , {"tacacs", "49"}
        , {"syslog", "514"}
        , {"ssh", "22"}
        , {"snmptrap", "162"}
        , {"snmp", "161"}
        , {"smtp", "25"}
        , {"sbfd", "7784"}
        , {"rtsp", "554"}
        , {"rip", "520"}
        , {"pop3", "110"}
        , {"ntp", "123"}
        , {"mlag", "4432"}
        , {"lpd", "515"}
        , {"ldp", "646"}
        , {"ldaps", "636"}
        , {"ldap", "389"}
        , {"kerberos", "88"}
        , {"isakmp", "500"}
        , {"https", "443"}
        , {"http", "80"}
        , {"ftp", "21"}
        , {"echo", "7"}
        , {"domain", "53"}
        , {"cmd", "514"}
        , {"capwap", "5246-5247"}
        , {"bootps", "67"}
        , {"bootpc", "68"}
        , {"bgp", "179"}
        , {"bfd", "3784"}
      };

    std::string serviceData {_data};

    for (const auto& [service, port] : mappings) {
      serviceData = std::regex_replace(serviceData, std::regex(service), port);
    }

    return serviceData;
  }

  std::string
  PortRange::toString() const
  {
    std::ostringstream oss;
    oss << boost::format("[%1%,%2%]") % std::get<0>(*this) % std::get<1>(*this);

    return oss.str();
  }

  std::string
  PortRange::toHumanString() const
  {
    std::ostringstream oss;
    if (std::get<0>(*this) == std::get<1>(*this)) {
      oss << boost::format("%1%") % std::get<0>(*this);
    } else {
      oss << boost::format("%1%-%2%") % std::get<0>(*this) % std::get<1>(*this);
    }

    return oss.str();
  }

  std::string
  PortRange::toDebugString() const
  {
    return toString();
  }

  std::ostream&
  operator<<(std::ostream& os, const PortRange& obj)
  {
    return os << obj.toString();
  }
}
