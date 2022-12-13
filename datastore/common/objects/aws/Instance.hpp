// =============================================================================
// Copyright 2022 National Technology & Engineering Solutions of Sandia, LLC
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

#ifndef AWS_INSTANCE_HPP
#define AWS_INSTANCE_HPP

#include <netmeld/datastore/objects/AbstractDatastoreObject.hpp>
#include <netmeld/datastore/objects/aws/NetworkInterface.hpp>


namespace nmdo = netmeld::datastore::objects;


namespace netmeld::datastore::objects::aws {

  class Instance : public nmdo::AbstractDatastoreObject {
    // ========================================================================
    // Variables
    // ========================================================================
    private: // Variables will probably rarely appear at this scope
    protected: // Variables intended for internal/subclass API
      std::string instanceId;

      std::string type;
      std::string imageId;
      std::string architecture;
      std::string platformDetails;
      std::string launchTime;
      std::string availabilityZone;
      std::string stateName;

      uint16_t stateCode  {0};

      std::set<NetworkInterface> interfaces;

    public: // Variables should rarely appear at this scope

    // ========================================================================
    // Constructors
    // ========================================================================
    private: // Constructors which should be hidden from API users
    protected: // Constructors part of subclass API
    public: // Constructors part of public API
      Instance();

    // ========================================================================
    // Methods
    // ========================================================================
    private: // Methods which should be hidden from API users
    protected: // Methods part of subclass API
    public: // Methods part of public API
      void setArchitecture(const std::string&);
      void setAvailabilityZone(const std::string&);
      void setId(const std::string&);
      void setImageId(const std::string&);
      void setLaunchTime(const std::string&);
      void setPlatformDetails(const std::string&);
      void setStateCode(const uint16_t);
      void setStateName(const std::string&);
      void setType(const std::string&);

      void addInterface(const NetworkInterface&);

      bool isValid() const override;

      void save(pqxx::transaction_base&,
                const nmco::Uuid&, const std::string&) override;

      std::string toDebugString() const override;

      std::partial_ordering operator<=>(const Instance&) const;
      bool operator==(const Instance&) const;
  };
}
#endif // AWS_INSTANCE_HPP
