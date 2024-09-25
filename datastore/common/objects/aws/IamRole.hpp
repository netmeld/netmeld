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

#ifndef AWS_IAM_ROLE_HPP
#define AWS_IAM_ROLE_HPP

#include <nlohmann/json.hpp>
#include <netmeld/datastore/objects/aws/IamBase.hpp>

using json = nlohmann::json;

namespace netmeld::datastore::objects::aws {

  class IamRole : public IamBase{
    // =========================================================================
    // Variables
    // =========================================================================
    private: // Variables will probably rarely appear at this scope
    protected: // Variables intended for internal/subclass API
        std::string lastUsed;
        uint16_t permissionsBoundaryUsageCount;
    public: // Variables should rarely appear at this scope

    // =========================================================================
    // Constructors
    // =========================================================================
    private: // Constructors which should be hidden from API users
    protected: // Constructors part of subclass API
    public: // Constructors part of public API
      IamRole();
      IamRole(const std::string&, const std::string&, const std::string&, const std::string&, const std::string&, const std::string&, uint16_t, const json&);

    // =========================================================================
    // Methods
    // =========================================================================
    private: // Methods which should be hidden from API users
    protected: // Methods part of subclass API
    public: // Methods part of public API
      void setLastUsed(const std::string&);
      void setPermissionsBoundaryUsageCount(const uint16_t);

      std::string getLastUsed() const;
      uint16_t getPermissionsBoundaryUsageCount() const;

      void save(pqxx::transaction_base&,
                const nmco::Uuid&, const std::string&) override;

      std::string toDebugString() const override;

      auto operator<=>(const IamRole&) const;
      bool operator==(const IamRole&) const;
  };
}
#endif // AWS_IAM_ROLE_HPP
