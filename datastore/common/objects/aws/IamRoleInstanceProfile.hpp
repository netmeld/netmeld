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

#ifndef AWS_IAM_ROLE_INSTANCE_PROFILE_HPP
#define AWS_IAM_ROLE_INSTANCE_PROFILE_HPP

#include <nlohmann/json.hpp>
#include <netmeld/datastore/objects/aws/IamBase.hpp>

namespace nmdo = netmeld::datastore::objects;

using json = nlohmann::json;

namespace netmeld::datastore::objects::aws {

  class IamRoleInstanceProfile : public nmdo::AbstractDatastoreObject {
    // =========================================================================
    // Variables
    // =========================================================================
    private: // Variables will probably rarely appear at this scope
    protected: // Variables intended for internal/subclass API
        std::string parentRoleId;
        std::string profileId;
        std::string profileName;
        std::string arn;
        std::string createDate; // Timestamp?
        std::string path;
        std::string childRoleId;
    public: // Variables should rarely appear at this scope

    // =========================================================================
    // Constructors
    // =========================================================================
    private: // Constructors which should be hidden from API users
    protected: // Constructors part of subclass API
    public: // Constructors part of public API
      IamRoleInstanceProfile();
      IamRoleInstanceProfile(const std::string&, const std::string&, const std::string&, const std::string&, const std::string&, const std::string&, const std::string&);

    // =========================================================================
    // Methods
    // =========================================================================
    private: // Methods which should be hidden from API users
    protected: // Methods part of subclass API
    public: // Methods part of public API
      void setParentRoleId(const std::string&);
      void setProfileId(const std::string&);
      void setProfileName(const std::string&);
      void setArn(const std::string&);
      void setCreateDate(const std::string&);
      void setPath(const std::string&);
      void setChildRoleId(const std::string&);

      std::string getParentRoleId() const;
      std::string getProfileId() const;
      std::string getProfileName() const;
      std::string getArn() const;
      std::string getCreateDate() const;
      std::string getPath() const;
      std::string getChildRoleId() const;

      bool isValid() const override;

      std::string toDebugString() const override;

      auto operator<=>(const IamRoleInstanceProfile&) const;
      bool operator==(const IamRoleInstanceProfile&) const;

      void save(pqxx::transaction_base&,
                const nmco::Uuid&, const std::string&) override;
  };
}
#endif // AWS_IAM_ROLE_INSTANCE_PROFILE_HPP
