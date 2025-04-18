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

#include "ProwlerOCSFData.hpp"

#include <format>

#include <netmeld/core/utils/StringUtilities.hpp>
#include <netmeld/core/utils/ContainerUtilities.hpp>

namespace nmcu = netmeld::core::utils;


namespace netmeld::datastore::objects::prowler {

  // ===========================================================================
  // Constructors
  // ===========================================================================
  ProwlerOCSFData::ProwlerOCSFData(const json& jline)
  {

    std::vector<std::string> orgInfoBuilder;
    // top level
    assessmentStartTime.readFormatted(jline.value("event_time", "")
                                     , "%Y-%m-%dT%H:%M:%S");
    //profile = jline.value("Profile", ""); // Not mapped yet
    //subServiceName = jline.value("SubServiceName", ""); // Not mapped yet
    status = jline.value("status_code", "");
    statusExtended = jline.value("status_detail", "");
    risk = jline.value("risk_details", "");

    // finding_info
    auto jFindingInfo = jline.value("finding_info", json::object());
    findingUniqueId = jFindingInfo.value("uid", "");
    checkTitle = jFindingInfo.value("title", "");
    description = jFindingInfo.value("desc", "");

    // cloud
    auto jCloud = jline.value("cloud", json::object());
    provider = jCloud.value("provider", "");
    // jCloud["org"].value("uid", ""); // Should map to the same as jMetadata.value("tenant_uid", "");
    auto jCloudOrg = jCloud.value("org", json::object());

    // account
    auto jAccount = jCloud.value("account", json::object());
    accountId = jAccount.value("uid", "");

    if(jAccount.contains("name"))
    {
      orgInfoBuilder.push_back(makeKeyValuePair("account_name", jAccount["name"]));
    }
    if(jCloudOrg.contains("name"))
    {
        orgInfoBuilder.push_back(makeKeyValuePair("account_org", jCloudOrg["name"]));
    }

    std::ostringstream labelsOss;
    for(const auto& element : jAccount.value("labels", json::array()))
    {
      labelsOss << element.get<std::string>() << '\n';
    }
    std::string labelsStr = labelsOss.str();
    if(labelsStr.size() > 0)
    {
      orgInfoBuilder.push_back(makeKeyValuePair("account_tags", labelsOss.str()));
    }

    // resources
    auto jResourceArray = jline.value("resources", json::array());
    auto jResources = jResourceArray.size() > 0 ? jResourceArray[0] : json::object();
    region = jResources.value("region", "");
    auto jResGroup = jResources.value("group", json::object());
    serviceName = jResGroup.value("name", "");
    resourceId = jResources.value("name", "");
    resourceArn = jResources.value("uid", "");
    resourceType = jResources.value("type", "");
    auto jResData = jResources.value("data", json::object());
    resourceDetails = jResData.value("details", "");

    // metadata
    auto jMetadata = jline.value("metadata", json::object());
    checkId = jMetadata.value("event_code", "");
    // jMetadata.value("tenant_uid", ""); // Azure had a tenant_uid that we might want

    // unmapped
    auto jUnmapped = jline.value("unmapped", json::object());
    relatedUrl = jUnmapped.value("related_url", "");
    notes = jUnmapped.value("notes", "");
    // jUnmapped.value("depends_on", ""); // Wasn't used in v3?
    // jUnmapped.value("related_to", ""); // Wasn't used in v3?

    // There's an enum that may not map all values
    if(jline.contains("severity")) {
        severity = jline["severity"];
    } else if(jline.contains("severity_id")) {
        LOG_WARN << "Key: severity not found. Assuming severity from severity_id";
        int severity_id = jline["severity_id"];
        switch(severity_id)
        {
            case 0:
                severity = "Unknown";
                break;
            case 1:
                severity = "Informational";
                break;
            case 2:
                severity = "Low";
                break;
            case 3:
                severity = "Medium";
                break;
            case 4:
                severity = "High";
                break;
            case 5:
                severity = "Critical";
                break;
            case 6:
                severity = "Fatal";
                break;
            default:
                severity = "Other";
        }
    } else {
        LOG_ERROR << "Neither severity or severity_id found. At least one of these fields is required";
        //std::exit(nmcu::Exit::FAILURE);
        // TODO Once we resolve #181, uncomment the hard fail
    }

    // array values
    if (jFindingInfo.contains("types")) {
      std::vector<std::string> temp;
      for (const auto& value : jFindingInfo["types"]) {
        if (static_cast<std::string>(value).empty()) {continue;}
        temp.push_back(value);
      }
      checkTypes = nmcu::toString(temp, '\n');
    }

    if (jUnmapped.contains("categories")) {
      std::vector<std::string> temp;
      for (const auto& value : jUnmapped["categories"]) {
        if (static_cast<std::string>(value).empty()) {continue;}
        temp.push_back(value);
      }
      categories = nmcu::toString(temp, '\n');
    }

    // array of, generally, key/value pairs
    if (jAccount.contains("labels")) {
      std::vector<std::string> temp;
      for (const auto& [key, value] : jAccount["labels"].items()) {
        if (static_cast<std::string>(value).empty()) {continue;}
        std::ostringstream oss;
        oss << key << ": " << value;
        temp.push_back(oss.str());
      }
      orgInfoBuilder.push_back(makeKeyValuePair("account_labels", nmcu::toString(temp, '\n')));
    }

    if (jResources.contains("labels")) {
      std::vector<std::string> temp;
      for (const auto& [key, value] : jResources["labels"].items()) {
        if (static_cast<std::string>(value).empty()) {continue;}
        std::ostringstream oss;
        oss << key << ": " << value;
        temp.push_back(oss.str());
      }
      resourceTags = nmcu::toString(temp, '\n');
    }

    if (jUnmapped.contains("compliance")) {
      std::ostringstream oss;
      std::string sep;
      for (const auto& [key, values] : jUnmapped["compliance"].items()) {
        oss << sep << key;
        for (const auto& value : values) {
          oss << "\n- " << static_cast<std::string>(value);
        }
        sep = '\n';
      }
      compliance = oss.str();
    }

    // more complex constructs
    if (jline.contains("remediation")) {
      const auto& jRemedi = jline["remediation"];
      recommendation = jRemedi.value("desc", "");
      if (jRemedi.contains("references")) {
        std::ostringstream url;
        std::ostringstream oss;
        const auto& jRef = jRemedi["references"];
        for (const auto& value : jRef) {
          std::string svalue(value);
          // Have to test for url, CLI, Terraform, NativelaC, or Other (If we want the key)
          if(svalue.starts_with("http"))
          {
            url << svalue << '\n';
          }
          else
          {
            oss << svalue << '\n';
          }
        }
        recommendationUrl = url.str();
        remediationCode = oss.str();
      }
    }

    organizationsInfo = nmcu::toString(orgInfoBuilder, '\n');
  }

  // ===========================================================================
  // Methods
  // ===========================================================================

  std::string
  ProwlerOCSFData::makeKeyValuePair(const std::string& key, const std::string& value) const
  {
    std::ostringstream oss;
    oss << key << ": " << value;
    return oss.str();
  }

  bool
  ProwlerOCSFData::isValid() const
  {
    return !( assessmentStartTime.isNull()
           || findingUniqueId.empty()
           )
      ;
  }

  void
  ProwlerOCSFData::save(pqxx::transaction_base& t,
                         const nmco::Uuid& toolRunId, const std::string&)
  {
    if (!isValid()) {
      LOG_DEBUG << "ProwlerOCSFData object is not saving: " << toDebugString()
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
        , nmcu::toLower(severity)
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
  ProwlerOCSFData::toDebugString() const
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
  ProwlerOCSFData::operator<=>(const ProwlerOCSFData& rhs) const
  {
    return std::tie( // assessmentStartTime,
                     findingUniqueId
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
       <=> std::tie( // rhs.assessmentStartTime,
                     rhs.findingUniqueId
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
  ProwlerOCSFData::operator==(const ProwlerOCSFData& rhs) const
  {
    return 0 == operator<=>(rhs);
  }
}
