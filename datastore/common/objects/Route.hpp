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

#ifndef ROUTE_HPP
#define ROUTE_HPP

#include <compare>
#include <netmeld/datastore/objects/AbstractDatastoreObject.hpp>
#include <netmeld/datastore/objects/IpAddress.hpp>
#include <netmeld/datastore/objects/IpNetwork.hpp>

#include <map>
#include <string>
#include <vector>

namespace netmeld::datastore::objects {

  class Route : public AbstractDatastoreObject {
    // =========================================================================
    // Variables
    // =========================================================================
    private: // Variables will probably rarely appear at this scope
    protected: // Variables intended for internal/subclass API
      std::string  vrfId;
      std::string  tableId;
      IpNetwork    dstIpNet;
      std::string  nextVrfId;
      std::string  nextTableId;
      IpAddress    nextHopIpAddr;
      std::string  ifaceName;
      std::string  protocol;
      std::string  description;
      size_t adminDistance;
      size_t metric;
      bool isActive;

    public: // Variables should rarely appear at this scope

    // =========================================================================
    // Constructors
    // =========================================================================
    private: // Constructors which should be hidden from API users
    protected: // Constructors part of subclass API
    public: // Constructors part of public API
      Route();

    // =========================================================================
    // Methods
    // =========================================================================
    private: // Methods which should be hidden from API users
    protected: // Methods part of subclass API
      void updateForSave(const bool);

    public: // Methods part of public API
      void setVrfId(const std::string&);
      void setTableId(const std::string&);
      void setDstIpNet(const IpAddress&);
      void setNextVrfId(const std::string&);
      void setNextTableId(const std::string&);
      void setNextHopIpAddr(const IpAddress&);
      void setIfaceName(const std::string&);
      void setProtocol(const std::string&);
      void setDescription(const std::string&);
      void setAdminDistance(size_t);
      void setMetric(size_t);
      void setActive(bool);

      bool isValid() const override;
      bool isV4() const;
      bool isV6() const;
      void save(pqxx::transaction_base&,
                const nmco::Uuid&, const std::string&) override;
      void saveAsMetadata(pqxx::transaction_base&,
                          const nmco::Uuid&) override;

      std::string toDebugString() const override;

      std::partial_ordering operator<=>(const Route&) const;
      bool operator==(const Route&) const;
  };

  typedef std::vector<Route> RoutingTable;
}
#endif // ROUTE_HPP
