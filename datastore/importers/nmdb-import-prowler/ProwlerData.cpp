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
  ProwlerData::ProwlerData(const json& jline, const YAML::Node& format)
  {
    LOG_DEBUG << "json: " << jline.dump() << std::endl << std::endl;
    LOG_DEBUG << "format: " << dump(format) << std::endl << std::endl;

    // Assert required keys exist
    assertRequiredKey(format, "assessmentStartTime");
    assertRequiredKey(format, "provider");
    assertRequiredKey(format, "accountId");
    assertRequiredKey(format, "serviceName");
    assertRequiredKey(format, "checkId");
    assertRequiredKey(format, "severity");
    assertRequiredKey(format, "description");
    assertRequiredKey(format, "recommendation");

    // Time field
    std::string timeString;
    smartAssign(format, "assessmentStartTime", jline, &timeString);
    std::string timeFormat = "%Y-%m-%dT%H:%M:%S";
    smartAssign(format, "_timeFormat", jline, &timeFormat);
    assessmentStartTime.readFormatted(timeString, timeFormat);

    // Requried string fields
    smartAssign(format, "provider", jline, &provider);
    smartAssign(format, "accountId", jline, &accountId);
    smartAssign(format, "checkId", jline, &checkId);
    smartAssign(format, "serviceName", jline, &serviceName);
    smartAssign(format, "severity", jline, &severity);
    smartAssign(format, "recommendation", jline, &recommendation);
    smartAssign(format, "description", jline, &description);
    // Non-required fields
    smartAssign(format, "region", jline, &region);
    smartAssign(format, "resourceId", jline, &resourceId);
    smartAssign(format, "risk", jline, &risk);
    smartAssign(format, "status", jline, &status);

    LOG_DEBUG << "Starting extras" << std::endl;
    for(auto pair : format["extras"]) {
      LOG_DEBUG << "\tKey = " << pair.first << std::endl;
      auto result = keySearch(pair.second, jline);
      if(!result.is_null())
      {
          extras[pair.first.as<std::string>()] = result;
      }
    }
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

  std::string
  ProwlerData::dump(const YAML::Node& node)
  {
      YAML::Emitter out;
      out << node;
      return out.c_str();
  }

  json
  ProwlerData::keySearch(const YAML::Node& key, const json& obj)
  {
    LOG_DEBUG << "keySearch()" << std::endl;
    if(key.IsNull()) {
        LOG_DEBUG << "\tnull value" << std::endl;
        return nullptr;
    }

    if (key.IsMap()) {
        LOG_DEBUG << "\tobject" << std::endl;
        // Accepted keys: concat, filter, join, json
        auto concat = key["concat"];
        auto filter = key["filter"];
        auto join = key["join"];
        auto _json = key["json"];
        if(concat.IsDefined()) {
            LOG_DEBUG << "CONCAT KEY: " << dump(key) << std::endl;
            std::ostringstream oss;
            for(auto c : concat) {
                auto v = keySearch(c, obj);
                if(v.is_null()) {
                    oss << "null";
                } else {
                    oss << v.template get<std::string>();
                }
            }
            return oss.str();
        } else if(filter.IsDefined()) {
            LOG_DEBUG << "FILTER KEY: " << dump(key) << std::endl;
            auto source = filter["source"];
            auto regExp = std::regex(filter["regex"].as<std::string>());
            auto join_str = filter["join_str"].as<std::string>();
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
        } else if(join.IsDefined()) {
            LOG_DEBUG << "JOIN KEY: " << dump(key) << std::endl;
            auto source = join["source"];
            auto join_str = join["join_str"].as<std::string>();
            auto r = keySearch(source, obj);
            if(r.is_null())
            {
                return "null"; // Different from empty string
            }
            auto temp = r.template get<std::vector<std::string>>();
            return nmcu::toString(temp, join_str);
        } else if(_json.IsDefined()) {
            LOG_DEBUG << "JSON KEY: " << dump(key) << std::endl;
            return obj;
        }
    } else if (key.IsSequence()) {
        LOG_DEBUG << "\tarray" << std::endl;
        // List builder
        std::vector<json> r;
        for(const auto& k : key) {
            r.push_back(keySearch(k, obj));
        }
        return r;
    } else if (key.IsScalar()) {
        // Convert to string
        auto k = key.as<std::string>();
        if(k.empty()) {
            // Might be an int, float, or boolean
            LOG_DEBUG << "\tint, float, or boolean" << std::endl;
            // TODO might have to do more conversion here. Not sure how YAML works
            return k;
        }
        LOG_DEBUG << "\tstring: " << std::endl;
        if(k[0] != '.') {
            // Default value
            LOG_DEBUG << "\t\tDefault value: " << k << std::endl;
            return k;
        } else {
            LOG_DEBUG << "\t\tkey: " << key << std::endl;
            auto rest = split(k, ".");
            rest.erase(rest.begin()); // Empty string at the start
            return recursiveSearch(rest, obj);
        }
    }
    LOG_DEBUG << "\tunclear" << std::endl;
    return nullptr; // How did we get here?
  }

  void
  ProwlerData::smartAssign(const YAML::Node& format, const std::string& key, const json& obj, std::string* ref)
  {
      LOG_DEBUG << "Working on " << key << std::endl;
      auto sub = format[key];
      if(!sub.IsDefined()) return; // Key not provided
      YAML::Node loc = format[key];
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

  void
  ProwlerData::assertRequiredKey(const YAML::Node& format, const std::string& key)
  {
      auto sub = format[key];
      if(!sub.IsDefined())
      {
          LOG_WARN << "Config is missing required key: " << key << std::endl;
      }
  }

  bool
  ProwlerData::isValid() const
  {
    return !( assessmentStartTime.isNull()
           || provider.empty()
           || accountId.empty()
           || checkId.empty()
           || serviceName.empty()
           || severity.empty()
           || recommendation.empty()
           || description.empty()
           || extras.is_null()
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
    //         timestamp, provider, accountId, serviceName, checkId, severity, description, recommendation
    t.exec_prepared("insert_raw_prowler_check",
          toolRunId
        , assessmentStartTime
        , provider
        , accountId
        , checkId
        , serviceName
        , nmcu::toLower(severity)
        , recommendation
        , description
        , region
        , resourceId
        , risk
        , status
      );

      t.exec_prepared("insert_raw_prowler_check_extras",
          toolRunId
        , assessmentStartTime
        , provider
        , accountId
        , checkId
        , serviceName
        , extras.dump()
      );
  }

  std::string
  ProwlerData::toDebugString() const
  {
    std::ostringstream oss;

    oss << R"([)"
        << R"("assessmentStartTime": ")" << assessmentStartTime
        << R"(", "provider": ")" << provider
        << R"(", "accountId": ")" << accountId
        << R"(", "checkId": ")" << checkId
        << R"(", "serviceName": ")" << serviceName
        << R"(", "severity": ")" << severity
        << R"(", "recommendation": ")" << recommendation
        << R"(", "description": ")" << description
        << R"(", "region": ")" << region
        << R"(", "resourceId": ")" << resourceId
        << R"(", "risk": ")" << risk
        << R"(", "status": ")" << status
        << R"(", "extras": )" << extras.dump()
        << R"("])"
        ;

    return oss.str();
  }

  std::strong_ordering
  ProwlerData::operator<=>(const ProwlerData& rhs) const
  {
    return std::tie( // assessmentStartTime,
                     provider
                   , accountId
                   , checkId
                   , serviceName
                   , severity
                   , recommendation
                   , description
                   , region
                   , resourceId
                   , risk
                   , status
                   )
       <=> std::tie( // rhs.assessmentStartTime,
                     rhs.provider
                   , rhs.accountId
                   , rhs.checkId
                   , rhs.serviceName
                   , rhs.severity
                   , rhs.recommendation
                   , rhs.description
                   , rhs.region
                   , rhs.resourceId
                   , rhs.risk
                   , rhs.status
                   )
      ;
  }

  bool
  ProwlerData::operator==(const ProwlerData& rhs) const
  {
    return 0 == operator<=>(rhs);
  }
}
