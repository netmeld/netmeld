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

#include "InterfaceHelper.hpp"

// ----------------------------------------------------------------------------
// InterfaceWrapper
// ----------------------------------------------------------------------------
InterfaceWrapper::InterfaceWrapper(nmdo::Interface& iface
                                  , const std::string& id
                                  ) : interface(iface), deviceId(id)
{}

std::partial_ordering
InterfaceWrapper::operator<=>(const InterfaceWrapper& rhs) const
{
  return std::tie( interface
                 , deviceId
                 )
     <=> std::tie( rhs.interface
                 , rhs.deviceId
                 )
    ;
}

bool
InterfaceWrapper::operator==(const InterfaceWrapper& rhs) const
{
  return 0 == operator<=>(rhs);
}


// ----------------------------------------------------------------------------
// InterfaceHelper
// ----------------------------------------------------------------------------
void
InterfaceHelper::add(nmdo::Interface& iface, const std::string& deviceId)
{
  const auto& ifaceName {iface.getName()};

  auto it = interfaces.find(ifaceName);
  if (it == interfaces.end()) {
    // Create new
    interfaces.emplace(ifaceName, InterfaceWrapper{iface, deviceId});
  } else {
    // Update existing
    auto& wrapper = it->second;

    auto mac {iface.getMacAddress()};
    if (mac.isValid()) {
      wrapper.interface.setMacAddress(mac);
    }
    for (auto& ip : iface.getIpAddresses()) {
      wrapper.interface.addIpAddress(ip);
    }
  }
}

std::partial_ordering
InterfaceHelper::operator<=>(const InterfaceHelper& rhs) const
{
  return std::tie( interfaces
                 )
     <=> std::tie( rhs.interfaces
                 )
    ;
}

bool
InterfaceHelper::operator==(const InterfaceHelper& rhs) const
{
  return 0 == operator<=>(rhs);
}
