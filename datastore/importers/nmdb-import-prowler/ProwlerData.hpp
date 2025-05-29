// =============================================================================
// Copyright 2025 National Technology & Engineering Solutions of Sandia, LLC
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

#ifndef PROWLER_DATA_HPP
#define PROWLER_DATA_HPP

#include <set>
#include <nlohmann/json.hpp>
#include <yaml-cpp/yaml.h>

#include <netmeld/core/objects/Time.hpp>
#include <netmeld/datastore/objects/AbstractDatastoreObject.hpp>

using json = nlohmann::json;
namespace nmco = netmeld::core::objects;

namespace netmeld::datastore::objects::prowler {

  class ProwlerData : public AbstractDatastoreObject {
    // =========================================================================
    // Variables
    // =========================================================================
    private: // Variables will probably rarely appear at this scope
    protected: // Variables intended for internal/subclass API
      // minimum required fields: based on nmdb-export-scan needs
      nmco::Time assessmentStartTime;
      std::string provider;
      std::string accountId;
      std::string checkId;
      std::string serviceName;
      std::string severity;
      std::string recommendation;
      std::string description;
      // non required fields
      std::string region;
      std::string resourceId;
      std::string risk;
      std::string status;
      // Extras
      json extras;

    public: // Variables should rarely appear at this scope

    // =========================================================================
    // Constructors
    // =========================================================================
    private: // Constructors which should be hidden from API users
    protected: // Constructors part of subclass API
    public: // Constructors part of public API
      ProwlerData() = default;
      explicit ProwlerData(const json&, const YAML::Node&);

    // =========================================================================
    // Methods
    // =========================================================================
    private: // Methods which should be hidden from API users
      void assertRequiredKey(const YAML::Node&, const std::string&);
      std::string dump(const YAML::Node&);
    protected: // Methods part of subclass API
      std::vector<std::string> split(std::string, const std::string&);
    public: // Methods part of public API
      json keySearch(const YAML::Node&, const json&);
      json recursiveSearch(std::vector<std::string>, const json&);
      void smartAssign(const YAML::Node&, const std::string&, const json&, std::string*);

      bool isValid() const override;
      void save(pqxx::transaction_base&,
                const nmco::Uuid&, const std::string&) override;

      // Utilized for full object data dump, for debug purposes
      std::string toDebugString() const override;

      std::strong_ordering operator<=>(const ProwlerData&) const;
      bool operator==(const ProwlerData&) const;
  };
}
#endif // PROWLER_DATA_HPP
