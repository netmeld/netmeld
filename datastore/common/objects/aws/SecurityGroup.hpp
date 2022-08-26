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

#ifndef AWS_SECURITY_GROUP_HPP
#define AWS_SECURITY_GROUP_HPP

#include <netmeld/datastore/objects/AbstractDatastoreObject.hpp>
#include <netmeld/datastore/objects/IpAddress.hpp>


namespace nmdo = netmeld::datastore::objects;


namespace netmeld::datastore::objects::aws {

  class SecurityGroupRule : public nmdo::AbstractDatastoreObject {
    // ========================================================================
    // Variables
    // ========================================================================
    private: // Variables will probably rarely appear at this scope
    protected: // Variables intended for internal/subclass API
      std::string protocol;
      std::int32_t fromPort {INT32_MIN};
      std::int32_t toPort   {INT32_MIN};

      std::set<nmdo::IpAddress> ipAddrs;

      bool egress {false};

    public: // Variables should rarely appear at this scope


    // ========================================================================
    // Constructors
    // ========================================================================
    private: // Constructors which should be hidden from API users
    protected: // Constructors part of subclass API
    public: // Constructors part of public API
      SecurityGroupRule();


    // ========================================================================
    // Methods
    // ========================================================================
    private: // Methods which should be hidden from API users
    protected: // Methods part of subclass API
    public: // Methods part of public API
      void setProtocol(const std::string&);
      void setFromPort(std::int32_t);
      void setToPort(std::int32_t);
      void setEgress();
      void addIpRange(const nmdo::IpAddress&);

      bool isValid() const override;

      void save(pqxx::transaction_base&,
                const nmco::Uuid&, const std::string&) override;

      std::string toDebugString() const override;

      std::partial_ordering operator<=>(const SecurityGroupRule&) const;
      bool operator==(const SecurityGroupRule&) const;
  };


  // -------------------------------------------------------------------------


  class SecurityGroup : public nmdo::AbstractDatastoreObject {
    // ========================================================================
    // Variables
    // ========================================================================
    private: // Variables will probably rarely appear at this scope
    protected: // Variables intended for internal/subclass API
      std::string sgId;
      std::string name;
      std::string description;
      std::string vpcId;

      std::set<SecurityGroupRule> rules;

    public: // Variables should rarely appear at this scope


    // ========================================================================
    // Constructors
    // ========================================================================
    private: // Constructors which should be hidden from API users
    protected: // Constructors part of subclass API
    public: // Constructors part of public API
      SecurityGroup();


    // ========================================================================
    // Methods
    // ========================================================================
    private: // Methods which should be hidden from API users
    protected: // Methods part of subclass API
    public: // Methods part of public API
      void setId(const std::string&);
      void setName(const std::string&);
      void setDescription(const std::string&);
      void setVpcId(const std::string&);
      void addRule(const SecurityGroupRule&);

      bool isValid() const override;

      void save(pqxx::transaction_base&,
                const nmco::Uuid&, const std::string&) override;

      std::string toDebugString() const override;

      std::partial_ordering operator<=>(const SecurityGroup&) const;
      bool operator==(const SecurityGroup&) const;
  };
}
#endif // AWS_SECURITY_GROUP_HPP
