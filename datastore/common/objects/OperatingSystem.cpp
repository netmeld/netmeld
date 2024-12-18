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

#include <boost/math/special_functions/relative_difference.hpp>

#include <netmeld/datastore/objects/OperatingSystem.hpp>
#include <netmeld/core/utils/StringUtilities.hpp>

namespace nmcu = netmeld::core::utils;


namespace netmeld::datastore::objects
{
  OperatingSystem::OperatingSystem()
  {}

  OperatingSystem::OperatingSystem(const IpAddress& _ipAddr) :
    ipAddr(_ipAddr)
  {}

  void
  OperatingSystem::setIpAddr(const IpAddress& _ipAddr)
  {
    ipAddr = _ipAddr;
  }

  void
  OperatingSystem::setVendorName(const std::string& _name)
  {
    vendorName = nmcu::toLower(_name);
  }

  void
  OperatingSystem::setProductName(const std::string& _name)
  {
    productName = nmcu::toLower(_name);
  }

  void
  OperatingSystem::setProductVersion(const std::string& _version)
  {
    productVersion = nmcu::toLower(_version);
  }

  void
  OperatingSystem::setCpe(const std::string& _cpe)
  {
    if (_cpe.empty()) {
      // Following NMAP limited CPE format
      //   cpe:/{part}:{vendor}:{product}:{version}
      cpe = std::format( "cpe:/o:{}:{}:{}"
                       , nmcu::toLower(vendorName)
                       , nmcu::toLower(productName)
                       , nmcu::toLower(productVersion)
                       );
      if ("cpe:/o:::" == cpe) { // nothing set
        cpe.clear();
      }
    } else {
      cpe = nmcu::toLower(_cpe);
    }

    // swap spaces to underscores
    std::replace(cpe.begin(), cpe.end(), ' ', '_');
  }

  void
  OperatingSystem::setAccuracy(const double _accuracy)
  {
    accuracy = _accuracy;
  }

  bool
  OperatingSystem::isValid() const
  {
    return ipAddr.isValid()
        && !( vendorName.empty()
           && productName.empty()
           && productVersion.empty()
           && cpe.empty()
           )
        ;
  }

  void
  OperatingSystem::save( pqxx::transaction_base& t
                       , const nmco::Uuid& toolRunId
                       , const std::string& deviceId
                       )
  {
    if (!isValid()) {
      LOG_DEBUG << "OperatingSystem object is not saving: " << toDebugString()
                << std::endl;
      return;
    }

    ipAddr.save(t, toolRunId, deviceId);

    t.exec_prepared( "insert_raw_operating_system"
                   , toolRunId
                   , ipAddr.toString()
                   , vendorName
                   , productName
                   , productVersion
                   , toCpeString()
                   , accuracy
                   );
  }

  std::string
  OperatingSystem::toDebugString() const
  {
    std::ostringstream oss;

    oss << "["
        << "ipAddr: " << ipAddr.toDebugString()
        << ", vendorName: " << vendorName
        << ", productName: " << productName
        << ", productVersion: " << productVersion
        << ", cpe: " << cpe
        << ", accuracy: " << accuracy
        << "]"
        ;

    return oss.str();
  }

  std::string
  OperatingSystem::toCpeString()
  {
    if (cpe.empty()) {
      setCpe();
    }

    return cpe;
  }

  std::partial_ordering
  OperatingSystem::operator<=>(const OperatingSystem& rhs) const
  {
    if (!(boost::math::epsilon_difference(accuracy, rhs.accuracy) <= 1000.0)) {
      return accuracy <=> rhs.accuracy;
    }
    return std::tie( ipAddr
                   , vendorName
                   , productName
                   , productVersion
                   , cpe
                   )
       <=> std::tie( rhs.ipAddr
                   , rhs.vendorName
                   , rhs.productName
                   , rhs.productVersion
                   , rhs.cpe
                   )
      ;
  }

  bool
  OperatingSystem::operator==(const OperatingSystem& rhs) const
  {
    return 0 == operator<=>(rhs);
  }
}
