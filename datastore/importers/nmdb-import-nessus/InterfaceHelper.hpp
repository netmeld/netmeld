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

#ifndef INTERFACE_HELPER
#define INTERFACE_HELPER

#include <netmeld/datastore/objects/Interface.hpp>

namespace nmdo = netmeld::datastore::objects;

struct InterfaceWrapper
{
  nmdo::Interface interface;
  std::string deviceId;

  InterfaceWrapper(nmdo::Interface&, const std::string&);

  std::partial_ordering operator<=>(const InterfaceWrapper&) const;
  bool operator==(const InterfaceWrapper&) const;
};

struct InterfaceHelper
{
  // ifaceName to Interface mapping
  std::map<std::string, InterfaceWrapper> interfaces;

  void add(nmdo::Interface&, const std::string&);

  std::partial_ordering operator<=>(const InterfaceHelper&) const;
  bool operator==(const InterfaceHelper&) const;
};

#endif //INTERFACE_HELPER
