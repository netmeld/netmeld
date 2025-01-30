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

#ifndef DATA_CONTAINER_SINGLETON_HPP
#define DATA_CONTAINER_SINGLETON_HPP

#include <netmeld/datastore/objects/InterfaceNetwork.hpp>
#include <netmeld/datastore/objects/IpAddress.hpp>
#include <netmeld/datastore/objects/MacAddress.hpp>
#include <netmeld/datastore/objects/ToolObservations.hpp>
#include <netmeld/datastore/objects/Service.hpp>
#include <netmeld/datastore/objects/Vlan.hpp>

#include <netmeld/core/utils/ThreadSafeQueue.hpp>

namespace nmdo = netmeld::datastore::objects;
namespace nmcu = netmeld::core::utils;


struct Data {
  std::map<std::string, nmdo::IpAddress>         ipAddrs;
  std::map<std::string, nmdo::MacAddress>        macAddrs;
  std::map<std::string, nmdo::Vlan>              vlans;
  std::map<std::string, nmdo::InterfaceNetwork>  ifaces;

  nmdo::ToolObservations observations;

  std::set<nmdo::Service> services;

  auto operator<=>(const Data&) const = default;
  bool operator==(const Data&) const = default;
};
typedef std::vector<Data> Result;

class DataContainerSingleton {
  // ===========================================================================
  // Variables
  // ===========================================================================
  private: // Variables will probably rarely appear at this scope
    nmcu::ThreadSafeQueue<Data> data;

  protected: // Variables intended for internal/subclass API
  public: // Variables should rarely appear at this scope

  // ===========================================================================
  // Constructors
  // ===========================================================================
  private: // Constructors which should be hidden from API users
    DataContainerSingleton();
  protected: // Constructors part of subclass API
  public: // Constructors part of public API
    DataContainerSingleton(const DataContainerSingleton&) = delete;
    void operator=(const DataContainerSingleton&)         = delete;

  // ===========================================================================
  // Methods
  // ===========================================================================
  private: // Methods which should be hidden from API users
  protected: // Methods part of subclass API
  public: // Methods part of public API
    static DataContainerSingleton& getInstance();

    void insert(const Data&);
    [[nodiscard]] bool hasData() const;

    Result getData();
};
#endif // DATA_CONTAINER_SINGLETON_HPP
