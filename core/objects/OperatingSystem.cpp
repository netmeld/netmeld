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

#include <netmeld/core/objects/OperatingSystem.hpp>
#include <netmeld/core/utils/StringUtilities.hpp>

namespace nmcu = netmeld::core::utils;


namespace netmeld::core::objects {

  OperatingSystem::OperatingSystem()
  {}

  OperatingSystem::OperatingSystem(const IpAddress& _ipAddr) :
    ipAddr(_ipAddr)
  {}

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
    cpe = nmcu::toLower(_cpe);
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
        && !(
                 vendorName.empty()
              && productName.empty()
              && productVersion.empty()
              && cpe.empty()
            )
        ;
  }

  void
  OperatingSystem::save(pqxx::transaction_base& t,
                        const Uuid& toolRunId, const std::string& deviceId)
  {
    if (!isValid()) {
      LOG_DEBUG << "OperatingSystem object is not saving: " << toDebugString()
                << std::endl;
      return;
    }

    ipAddr.save(t, toolRunId, deviceId);

    t.exec_prepared("insert_raw_operating_system",
        toolRunId,
        ipAddr.toString(),
        vendorName,
        productName,
        productVersion,
        cpe,
        accuracy);
  }

  std::string
  OperatingSystem::toDebugString() const
  {
    std::ostringstream oss;

    oss << "["; // opening bracket
    oss << ipAddr.toDebugString() << ", "
        << vendorName << ", "
        << productName << ", "
        << productVersion << ", "
        << cpe << ", "
        << accuracy;
    oss << "]"; // closing bracket

    return oss.str();
  }

}
