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

#ifndef PROWLER_V2_DATA_HPP
#define PROWLER_V2_DATA_HPP

#include <set>
#include <nlohmann/json.hpp>

#include <netmeld/core/objects/Time.hpp>
#include <netmeld/datastore/objects/AbstractDatastoreObject.hpp>

using json = nlohmann::json;
namespace nmco = netmeld::core::objects;

namespace netmeld::datastore::objects::prowler {

  class ProwlerV2Data : public AbstractDatastoreObject {
    // =========================================================================
    // Variables
    // =========================================================================
    private: // Variables will probably rarely appear at this scope
    protected: // Variables intended for internal/subclass API
      std::string accountNumber;
      nmco::Time  timestamp;
      std::string region;
      std::string control;
      std::string severity;
      std::string status;
      std::string level;
      std::string controlId;
      std::string service;
      std::string risk;
      std::string remediation;
      std::string documentationLink;
      std::string resourceId;

    public: // Variables should rarely appear at this scope

    // =========================================================================
    // Constructors
    // =========================================================================
    private: // Constructors which should be hidden from API users
    protected: // Constructors part of subclass API
    public: // Constructors part of public API
      ProwlerV2Data() = default;
      explicit ProwlerV2Data(const json&);

    // =========================================================================
    // Methods
    // =========================================================================
    private: // Methods which should be hidden from API users
    protected: // Methods part of subclass API
    public: // Methods part of public API
      bool isValid() const override;
      void save(pqxx::transaction_base&,
                const nmco::Uuid&, const std::string&) override;

      // Utilized for full object data dump, for debug purposes
      std::string toDebugString() const override;

      std::strong_ordering operator<=>(const ProwlerV2Data&) const;
      bool operator==(const ProwlerV2Data&) const;
  };
}
#endif // PROWLER_V2_DATA_HPP
