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

#include "ProwlerData.hpp"

#include <format>
#include <regex>

#include <netmeld/core/utils/StringUtilities.hpp>
#include <netmeld/core/utils/ContainerUtilities.hpp>

namespace nmcu = netmeld::core::utils;


namespace netmeld::datastore::objects::prowler {

  // ===========================================================================
  // Constructors
  // ===========================================================================
  ProwlerData::ProwlerData(const json& jline, const json& format)
  {
    LOG_DEBUG << "json: " << jline.dump() << std::endl << std::endl;
    LOG_DEBUG << "format: " << format.dump() << std::endl << std::endl;

    std::string timeString;
    smartAssign(format, "assessmentStartTime", jline, &timeString);
    std::string timeFormat = "%Y-%m-%dT%H:%M:%S";
    smartAssign(format, "_timeFormat", jline, &timeFormat);
    assessmentStartTime.readFormatted(timeString, timeFormat);

    smartAssign(format, "findingUniqueId", jline, &findingUniqueId);
    smartAssign(format, "provider", jline, &provider);
    smartAssign(format, "profile", jline, &profile);
    smartAssign(format, "accountId", jline, &accountId);
    smartAssign(format, "organizationsInfo", jline, &organizationsInfo);
    smartAssign(format, "region", jline, &region);
    smartAssign(format, "checkId", jline, &checkId);
    smartAssign(format, "checkTitle", jline, &checkTitle);
    smartAssign(format, "checkTypes", jline, &checkTypes);
    smartAssign(format, "serviceName", jline, &serviceName);
    smartAssign(format, "subServiceName", jline, &subServiceName);
    smartAssign(format, "status", jline, &status);
    smartAssign(format, "statusExtended", jline, &statusExtended);
    smartAssign(format, "severity", jline, &severity);
    smartAssign(format, "resourceId", jline, &resourceId);
    smartAssign(format, "resourceArn", jline, &resourceArn);
    smartAssign(format, "resourceTags", jline, &resourceTags);
    smartAssign(format, "resourceType", jline, &resourceType);
    smartAssign(format, "resourceDetails", jline, &resourceDetails);
    smartAssign(format, "description", jline, &description);
    smartAssign(format, "risk", jline, &risk);
    smartAssign(format, "relatedUrl", jline, &relatedUrl);
    smartAssign(format, "recommendation", jline, &recommendation);
    smartAssign(format, "recommendationUrl", jline, &recommendationUrl);
    smartAssign(format, "remediationCode", jline, &remediationCode);
    smartAssign(format, "categories", jline, &categories);
    smartAssign(format, "notes", jline, &notes);
    smartAssign(format, "compliance", jline, &compliance);
  }

  // ===========================================================================
  // Methods
  // ===========================================================================

  json
  ProwlerData::recursiveSearch(std::vector<std::string> keys, const json& obj)
  {
    LOG_DEBUG << "recursiveSearch: " << keys.size() << " keys remain." << std::endl;
    if(keys.size() == 0) {
      return obj;
    }
    auto pattern = keys[0];
    keys.erase(keys.begin()); // Remove first element
    LOG_DEBUG << "\tcurrent key: " << pattern << std::endl;
    if(pattern[0] == '[') {
        LOG_DEBUG << "\tDetected list indexing" << std::endl;
        if(pattern.size() > 2) {
            // [N] single element
            auto index = std::stoi(pattern.substr(1, pattern.size()-2));
            return recursiveSearch(keys, obj[index]);
        } else {
            // [] whole array
            std::vector<json> r;
            for(const auto& value : obj) {
                r.push_back(recursiveSearch(keys, value));
            }
            json result = r;
            return result;
        }
    } else {
      LOG_DEBUG << "\tChecking if key is in object" << std::endl;
      if(obj.contains(pattern)) {
        return recursiveSearch(keys, obj[pattern]);
      } else {
        LOG_DEBUG << "\tkey was not found in " << obj.dump() << std::endl;
        json result; // null
        return result;
      }
    }
  }

  std::vector<std::string>
  ProwlerData::split(std::string s, const std::string& delimiter) {
    LOG_DEBUG << "splitting: " << s << " on " << delimiter << std::endl;
    std::vector<std::string> tokens;
    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
      token = s.substr(0, pos);
      tokens.push_back(token);
      //s.erase(0, pos + delimiter.length());
      s = s.substr(pos+delimiter.length());
    }
    tokens.push_back(s);
    return tokens;
  }

  json
  ProwlerData::keySearch(const json& key, const json& obj)
  {
    LOG_DEBUG << "keySearch()" << std::endl;
    auto type = key.type();
    if(type == nlohmann::json::value_t::null) {
        LOG_DEBUG << "\tnull value" << std::endl;
        return nullptr;
    }

    if (type == nlohmann::json::value_t::object) {
        LOG_DEBUG << "\tobject" << std::endl;
        // Accepted keys: concat, filter, join, json
        if(key.contains("concat")) {
            LOG_DEBUG << "CONCAT KEY: " << key.dump() << std::endl;
            auto sub = key["concat"];
            std::ostringstream oss;
            for(auto c : sub) {
                auto v = keySearch(c, obj);
                if(v.is_null()) {
                    oss << "null";
                } else {
                    oss << v.template get<std::string>();
                }
            }
            return oss.str();
        } else if(key.contains("filter")) {
            LOG_DEBUG << "FILTER KEY: " << key.dump() << std::endl;
            auto sub = key["filter"];
            auto source = sub["source"];
            auto regExp = std::regex(sub["regex"]);
            auto join_str = sub.value("join_str", "\n");
            auto temp = keySearch(source, obj);
            std::vector<std::string> result;
            for(auto partJ : temp)
            {
                auto part = partJ.template get<std::string>();
                if(std::regex_search(part, regExp))
                {
                    result.push_back(part);
                    LOG_DEBUG << "\t" << part << " matched regex" << std::endl;
                }
                else
                {
                    LOG_DEBUG << "\t" << part << " did not match regex" << std::endl;
                }
            }
            return nmcu::toString(result, join_str);
        } else if(key.contains("join")) {
            LOG_DEBUG << "JOIN KEY: " << key.dump() << std::endl;
            auto sub = key["join"];
            auto source = sub["source"];
            auto join_str = sub["join_str"];
            auto temp = keySearch(source, obj).template get<std::vector<std::string>>();
            return nmcu::toString(temp, join_str);
        } else if(key.contains("json")) {
            LOG_DEBUG << "JSON KEY: " << key.dump() << std::endl;
            return obj;
        }
    } else if (type == nlohmann::json::value_t::array) {
        LOG_DEBUG << "\tarray" << std::endl;
        // List builder
        std::vector<json> r;
        for(const auto& k : key) {
            r.push_back(keySearch(k, obj));
        }
        return r;
    } else if (type == nlohmann::json::value_t::string) {
        LOG_DEBUG << "\tstring: " << std::endl;
        // Convert to string
        auto k = key.template get<std::string>();
        if(k[0] != '.') {
            // Default value
            LOG_DEBUG << "\t\tDefault value: " << k << std::endl;
            return key;
        } else {
            LOG_DEBUG << "\t\tkey: " << key << std::endl;
            auto rest = split(key, ".");
            rest.erase(rest.begin()); // Empty string at the start
            return recursiveSearch(rest, obj);
        }
    } else if (type == nlohmann::json::value_t::number_integer) {
        LOG_DEBUG << "\tinteger" << std::endl;
        // Default value
        return key;
    } else if (type == nlohmann::json::value_t::number_float) {
        LOG_DEBUG << "\tfloat" << std::endl;
        // Default value
        return key;
    } else if (type == nlohmann::json::value_t::boolean) {
        LOG_DEBUG << "\tboolean" << std::endl;
        // Default value
        return key;
    }
    LOG_DEBUG << "\tunclear" << std::endl;
    return nullptr; // How did we get here?
  }

  void
  ProwlerData::smartAssign(const json& format, const std::string& key, const json& obj, std::string* ref)
  {
      LOG_DEBUG << "Working on " << key << std::endl;
      if(!format.contains(key)) return; // Key not provided
      json loc = format.value(key, json::object());
      auto result = keySearch(loc, obj);
      LOG_DEBUG << "Smart assign received: " << result.dump() << std::endl;
      if(result.is_null()) return; // No value found
      if(result.is_array() || result.is_object())
      {
          *ref = result.dump(); // Stringify array
          return;
      }
      *ref = result.template get<std::string>();
  }

  bool
  ProwlerData::isValid() const
  {
    return !( assessmentStartTime.isNull()
           || provider.empty()
           || accountId.empty()
           || serviceName.empty()
           || checkId.empty()
           || severity.empty()
           || recommendation.empty()
           )
      ;
  }

  void
  ProwlerData::save(pqxx::transaction_base& t,
                         const nmco::Uuid& toolRunId, const std::string&)
  {
    if (!isValid()) {
      LOG_DEBUG << "ProwlerData object is not saving: " << toDebugString()
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
  ProwlerData::toDebugString() const
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
  ProwlerData::operator<=>(const ProwlerData& rhs) const
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
  ProwlerData::operator==(const ProwlerData& rhs) const
  {
    return 0 == operator<=>(rhs);
  }
}
