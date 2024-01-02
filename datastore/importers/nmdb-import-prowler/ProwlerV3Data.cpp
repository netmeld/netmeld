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

#include "ProwlerV3Data.hpp"

#include <format>

#include <netmeld/core/utils/StringUtilities.hpp>

namespace nmcu = netmeld::core::utils;


namespace netmeld::datastore::objects::prowler {

  // ===========================================================================
  // Constructors
  // ===========================================================================
  /* V3 -- {
      "AssessmentStartTime":"", "FindingUniqueId": "", "Provider": ""
    , "Profile": "", "AccountId": "", "OrganizationsInfo": [], "Region": ""
    , "CheckID": "", "CheckTitle": "", "CheckType": [], "ServiceName": ""
    , "SubServiceName": "", "Status": "", "StatusExtended": "", "Severity": ""
    , "ResourceId": "", "ResourceArn": "", "ResourceTags": {}
    , "ResourceType": "", "ResourceDetails": "", "Description": "", "Risk": ""
    , "RelatedUrl": "" , "Remediation": {
      "Code": { "NativeIaC": "", "Terraform": "", "CLI": "", "Other": "" }
      , "Recommendation": { "Text": "", "Url": "" }
    }, "Categories": [] , "Notes": "", "Compliance": {}
  */
  ProwlerV3Data::ProwlerV3Data(const json& jline)
  {
    // string values
    assessmentStartTime.readFormatted(jline.value("AssessmentStartTime", "")
                                     , "%Y-%m-%dT%H:%M:%S");
    findingUniqueId = jline.value("FindingUniqueId", "");
    provider = jline.value("Provider", "");
    profile = jline.value("Profile", "");
    accountId = jline.value("AccountId", "");
    region = jline.value("Region", "");
    checkId = jline.value("CheckID", "");
    checkTitle = jline.value("CheckTitle", "");
    serviceName = jline.value("ServiceName", "");
    subServiceName = jline.value("SubServiceName", "");
    status = jline.value("Status", "");
    statusExtended = jline.value("StatusExtended", "");
    severity = jline.value("Severity", "");
    resourceId = jline.value("ResourceId", "");
    resourceArn = jline.value("ResourceArn", "");
    resourceType = jline.value("ResourceType", "");
    resourceDetails = jline.value("ResourceDetails", "");
    description = jline.value("Description", "");
    risk = jline.value("Risk", "");
    relatedUrl = jline.value("RelatedUrl", "");
    notes = jline.value("Notes", "");

    // array values
    if (jline.contains("CheckType")) {
      std::vector<std::string> temp;
      for (const auto& value : jline["CheckType"]) {
        if (static_cast<std::string>(value).empty()) {continue;}
        temp.push_back(value);
      }
      checkTypes = nmcu::toString(temp, '\n');
    }

    if (jline.contains("Categories")) {
      std::vector<std::string> temp;
      for (const auto& value : jline["Categories"]) {
        if (static_cast<std::string>(value).empty()) {continue;}
        temp.push_back(value);
      }
      categories = nmcu::toString(temp, '\n');
    }

    // array of, generally, key/value pairs
    if (jline.contains("OrganizationsInfo")) {
      std::vector<std::string> temp;
      for (const auto& [key, value] : jline["OrganizationsInfo"].items()) {
        if (static_cast<std::string>(value).empty()) {continue;}
        std::ostringstream oss;
        oss << key << ": " << value;
        temp.push_back(oss.str());
      }
      organizationsInfo = nmcu::toString(temp, '\n');
    }

    if (jline.contains("ResourceTags")) {
      std::vector<std::string> temp;
      for (const auto& [key, value] : jline["ResourceTags"].items()) {
        if (static_cast<std::string>(value).empty()) {continue;}
        std::ostringstream oss;
        oss << key << ": " << value;
        temp.push_back(oss.str());
      }
      resourceTags = nmcu::toString(temp, '\n');
    }

    if (jline.contains("Compliance")) {
      std::ostringstream oss;
      std::string sep;
      for (const auto& [key, values] : jline["Compliance"].items()) {
        oss << sep << key;
        for (const auto& value : values) {
          oss << "\n- " << static_cast<std::string>(value);
        }
        sep = '\n';
      }
      compliance = oss.str();
    }

    // more complex constructs
    if (jline.contains("Remediation")) {
      const auto& jRemedi = jline["Remediation"];
      if (jRemedi.contains("Recommendation")) {
        const auto& jRecom = jRemedi["Recommendation"];
        recommendation = jRecom.value("Text", "");
        recommendationUrl = jRecom.value("Url", "");
      }
      if (jRemedi.contains("Code")) {
        std::vector<std::string> temp;
        for (const auto& [key, value] : jRemedi["Code"].items()) {
          if (static_cast<std::string>(value).empty()) {continue;}
          std::ostringstream oss;
          oss << std::string(key)
              << ": " << std::string(value)
              ;
          temp.push_back(oss.str());
        }
        remediationCode = nmcu::toString(temp, '\n');
      }
    }
  }

  // ===========================================================================
  // Methods
  // ===========================================================================

  bool
  ProwlerV3Data::isValid() const
  {
    return !( assessmentStartTime.isNull()
           || findingUniqueId.empty()
           )
      ;
  }

  void
  ProwlerV3Data::save(pqxx::transaction_base& t,
                         const nmco::Uuid& toolRunId, const std::string&)
  {
    if (!isValid()) {
      LOG_DEBUG << "ProwlerV3Data object is not saving: " << toDebugString()
                << std::endl;
      return; // Always short circuit if invalid object
    }

    // NOTE: The following are the suspected minimum for unique:
    //         timestamp, findingUniqueId
    t.exec_prepared("insert_raw_prowler_v3_check",
          toolRunId
        , assessmentStartTime
        , findingUniqueId
        , provider
        , profile
        , accountId
        , organizationsInfo
        , region
        , checkId
        , checkTitle
        , checkTypes
        , serviceName
        , subServiceName
        , status
        , statusExtended
        , severity
        , resourceId
        , resourceArn
        , resourceTags
        , resourceType
        , resourceDetails
        , description
        , risk
        , relatedUrl
        , recommendation
        , recommendationUrl
        , remediationCode
        , categories
        , notes
        , compliance
      );
  }

  std::string
  ProwlerV3Data::toDebugString() const
  {
    std::ostringstream oss;

    oss << R"([)"
        << R"("assessmentStartTime": ")" << assessmentStartTime
        << R"(", "findingUniqueId": ")" << findingUniqueId
        << R"(", "provider": ")" << provider
        << R"(", "profile": ")" << profile
        << R"(", "accountId": ")" << accountId
        << R"(", "organizationsInfo": ")" << organizationsInfo
        << R"(", "region": ")" << region
        << R"(", "checkId": ")" << checkId
        << R"(", "checkTitle": ")" << checkTitle
        << R"(", "checkTypes": ")" << checkTypes
        << R"(", "serviceName": ")" << serviceName
        << R"(", "subServiceName": ")" << subServiceName
        << R"(", "status": ")" << status
        << R"(", "statusExtended": ")" << statusExtended
        << R"(", "severity": ")" << severity
        << R"(", "resourceId": ")" << resourceId
        << R"(", "resourceArn": ")" << resourceArn
        << R"(", "resourceTags": ")" << resourceTags
        << R"(", "resourceType": ")" << resourceType
        << R"(", "resourceDetails": ")" << resourceDetails
        << R"(", "description": ")" << description
        << R"(", "risk": ")" << risk
        << R"(", "relatedUrl": ")" << relatedUrl
        << R"(", "recommendation": ")" << recommendation
        << R"(", "recommendationUrl": ")" << recommendationUrl
        << R"(", "remediationCode": ")" << remediationCode
        << R"(", "categories": ")" << categories
        << R"(", "notes": ")" << notes
        << R"(", "compliance": ")" << compliance
        << R"("])"
        ;

    return oss.str();
  }

  std::strong_ordering
  ProwlerV3Data::operator<=>(const ProwlerV3Data& rhs) const
  {
    return std::tie( assessmentStartTime
                   , findingUniqueId
                   , provider
                   , profile
                   , accountId
                   , organizationsInfo
                   , region
                   , checkId
                   , checkTitle
                   , checkTypes
                   , serviceName
                   , subServiceName
                   , status
                   , statusExtended
                   , severity
                   , resourceId
                   , resourceArn
                   , resourceTags
                   , resourceType
                   , resourceDetails
                   , description
                   , risk
                   , relatedUrl
                   , recommendation
                   , recommendationUrl
                   , remediationCode
                   , categories
                   , notes
                   , compliance
                   )
       <=> std::tie( rhs.assessmentStartTime
                   , rhs.findingUniqueId
                   , rhs.provider
                   , rhs.profile
                   , rhs.accountId
                   , rhs.organizationsInfo
                   , rhs.region
                   , rhs.checkId
                   , rhs.checkTitle
                   , rhs.checkTypes
                   , rhs.serviceName
                   , rhs.subServiceName
                   , rhs.status
                   , rhs.statusExtended
                   , rhs.severity
                   , rhs.resourceId
                   , rhs.resourceArn
                   , rhs.resourceTags
                   , rhs.resourceType
                   , rhs.resourceDetails
                   , rhs.description
                   , rhs.risk
                   , rhs.relatedUrl
                   , rhs.recommendation
                   , rhs.recommendationUrl
                   , rhs.remediationCode
                   , rhs.categories
                   , rhs.notes
                   , rhs.compliance
                   )
      ;
  }

  bool
  ProwlerV3Data::operator==(const ProwlerV3Data& rhs) const
  {
    return 0 == operator<=>(rhs);
  }
}
