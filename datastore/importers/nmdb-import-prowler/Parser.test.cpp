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

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include <netmeld/datastore/parsers/ParserTestHelper.hpp>

#include "Parser.hpp"

namespace nmdp = netmeld::datastore::parsers;

using qi::ascii::blank;

class TestParser : public Parser
{
  public:
    using Parser::r;
    using Parser::fromJson;
    using Parser::fromJsonLines;
};

BOOST_AUTO_TEST_CASE(testFromJsonV2)
{
  auto v2Config = YAML::Load(R"(# Required fields
assessmentStartTime: ".Timestamp"
_timeFormat: "%Y-%m-%dT%H:%M:%SZ"
provider: 'aws'
accountId: ".Account Number"
checkId: ".Control ID"
serviceName: ".Service"
severity: ".Severity"
recommendation: ".Remediation"
description: ".Control"
# Non-required fields
region: ".Region"
resourceId: ".Resource ID"
risk: ".Risk"
status: ".Status"
# Extras
extras:
  recommendationUrl: ".Doc link"
  compliance: ".Level"
)");
  // NOTE: These are primarily for testing "file" logic, not data logic
  TestParser tp;
  std::string test;
  Result out;

  const auto getResult = [&](const std::string& jsonData) {
      std::istringstream is {jsonData};
      tp.fromJsonLines(is, v2Config);
      return tp.getData();
    };

  // empty, but should not error
  test = "{}";
  out = getResult(test);
  BOOST_TEST_REQUIRE(0 == out.size());

  test = "{}\n{}\n{}";
  out = getResult(test);
  BOOST_TEST_REQUIRE(0 == out.size());

  // Parsable, but no v2 data
  test = R"({"key1": "value", "key2": 123})";
  out = getResult(test);
  BOOST_TEST_REQUIRE(0 == out.size());

  test = R"({"key1": "v1", "key2": 123}
            {"key3": "v3"})";
  out = getResult(test);
  BOOST_TEST_REQUIRE(0 == out.size());

  // Parsable, some v2 data
  test = R"({"Account Number": "123abc"})";
  out = getResult(test);
  BOOST_TEST_REQUIRE(1 == out.size());
  BOOST_TEST(1 == out[0].data.size());
  tp.r =  Result();

  test = R"({"Account Number": "123abc"}
            {"Account Number": "123abc"})";
  out = getResult(test);
  BOOST_TEST_REQUIRE(1 == out.size());
  BOOST_TEST(2 == out[0].data.size());
  tp.r =  Result();
}

BOOST_AUTO_TEST_CASE(testFromJsonV3)
{
  auto v3Config = YAML::Load(R"(# Required fields
assessmentStartTime: ".AssessmentStartTime"
_timeFormat: "%Y-%m-%dT%H:%M:%S"
provider: ".Provider"
accountId: ".AccountId"
checkId: ".CheckID"
serviceName: ".ServiceName"
severity: ".Severity"
recommendation: ".Remediation.Recommendation.Text"
description: ".Description"
# Non-required field
region: ".Region"
resourceId: ".ResourceId"
risk: ".Risk"
status: ".Status"
# Extras
extras:
  findingUniqueId: ".FindingUniqueId"
  profile: ".Profile"
  organizationsInfo: ".OrganizationsInfo" # Object
  checkTitle: ".CheckTitle"
  checkTypes: ".CheckType" # Array of strings
  subServiceName: ".SubServiceName"
  statusExtended: ".StatusExtended"
  resourceArn: ".ResourceArn"
  resourceTags: ".ResourceTags" # Object
  resourceType: ".ResourceType"
  resourceDetails: ".ResourceDetails"
  relatedUrl: ".RelatedUrl"
  recommendationUrl: ".Remediation.Recommendation.Url"
  remediationCode: ".Remediation.Code" # Object
  categories: ".Categories" # Array
  notes: ".Notes"
  compliance: ".Compliance" # Object
)");
  // NOTE: These are primarily for testing "file" logic, not data logic
  TestParser tp;
  std::string test;
  Result out;

  const auto getResult = [&](const std::string& jsonData) {
      std::istringstream is {jsonData};
      tp.fromJson(is, v3Config);
      return tp.getData();
    };

  // empty, but should not error
  test = "[{}]";
  out = getResult(test);
  BOOST_TEST_REQUIRE(0 == out.size());

  test = "[{},{},{}]";
  out = getResult(test);
  BOOST_TEST_REQUIRE(0 == out.size());

  // Parsable, but no v3 data
  test = R"([{"key1": "value", "key2": 123}])";
  out = getResult(test);
  BOOST_TEST_REQUIRE(0 == out.size());

  test = R"([{"key1": "value", "key2": 123},
             {"key3": "v3"}
            ])";
  out = getResult(test);
  BOOST_TEST_REQUIRE(0 == out.size());

  // Parsable, some v3 data
  test = R"([{"AccountId": "123abc", "FindingUniqueId": "abc123"}])";
  out = getResult(test);
  BOOST_TEST_REQUIRE(1 == out.size());
  BOOST_TEST(1 == out[0].data.size());
  tp.r =  Result();

  test = R"([{"AccountId": "123abc", "FindingUniqueId": "abc123"},
             {"AccountId": "123abc", "FindingUniqueId": "abc123"}
            ])";
  out = getResult(test);
  BOOST_TEST_REQUIRE(1 == out.size());
  BOOST_TEST(2 == out[0].data.size());
  tp.r =  Result();
}

BOOST_AUTO_TEST_CASE(testFromJsonOCSF)
{
  auto ocsfConfig = YAML::Load(R"(# Required fields
assessmentStartTime: ".event_time"
_timeFormat: "%Y-%m-%dT%H:%M:%S.%f"
provider: ".cloud.provider"
accountId: ".cloud.account.uid"
checkId: ".metadata.event_code"
serviceName: ".resources.[0].group.name"
severity: ".severity" # .severity_id could also map
recommendation: ".remediation.desc"
description: ".finding_info.desc"
# Non-required fields
region: ".resources.[0].region"
resourceId: ".resources.[0].name"
risk: ".risk_details"
status: ".status_code"
# Extras
extras:
  findingUniqueId: ".finding_info.uid"
  organizationsInfo:
    concat:
      - "Account Name: "
      - ".cloud.account.name"
      - ", Account Org: "
      - ".cloud.org.name"
  checkTitle: ".finding_info.title"
  checkTypes: ".finding_info.types" # Array
  statusExtended: ".status_detail"
  #resources: ".resources"
  resourceArn: ".resources.[0].uid"
  resourceTags: ".resources.[0].labels" # Object
  resourceType: ".resources.[0].type"
  resourceDetails: ".resources.[0].data.details"
  relatedUrl: ".unmapped.related_url"
  recommendationUrl:
    filter:
      source: ".remediation.references" # Array- Have to filter by http
      regex: "^http.*"
      join_str: "\n"
  remediationCode:
    filter:
      source: ".remediation.references" # Array- Have to filter by !http
      regex: "^(?!http).*"
      join_str: "\n"
  categories: ".unmapped.categories" # Array
  notes: ".unmapped.notes"
  compliance: ".unmapped.compliance" # Object with Array values
)");
  // NOTE: These are primarily for testing "file" logic, not data logic
  TestParser tp;
  std::string test;
  Result out;

  const auto getResult = [&](const std::string& jsonData) {
      std::istringstream is {jsonData};
      tp.fromJson(is, ocsfConfig);
      return tp.getData();
    };

  // empty, but should not error
  test = "[{}]";
  out = getResult(test);
  BOOST_TEST_REQUIRE(0 == out.size());

  test = "[{},{},{}]";
  out = getResult(test);
  BOOST_TEST_REQUIRE(0 == out.size());

  // Parsable, but no ocsf data
  test = R"([{"key1": "value", "key2": 123}])";
  out = getResult(test);
  BOOST_TEST_REQUIRE(0 == out.size());

  test = R"([{"key1": "value", "key2": 123},
             {"key3": "v3"}
            ])";
  out = getResult(test);
  BOOST_TEST_REQUIRE(0 == out.size());

  // Parsable, some ocsf data
  test = R"([{"cloud":{"account":{"id": "123abc"}, "provider":"aws"}, "finding_info": {"desc":"description"}, "metadata":{"event_code":"123"}, "resources":[{"group":{"name":"service name"}}], "severity":"High", "remediation":{"desc":"test"}}])";
  out = getResult(test);
  BOOST_TEST_REQUIRE(1 == out.size());
  BOOST_TEST(1 == out[0].data.size());
  tp.r =  Result();

  test = R"([{"cloud":{"account":{"id": "123abc"}, "provider":"aws"}, "finding_info": {"desc":"description"}, "metadata":{"event_code":"123"}, "resources":[{"group":{"name":"service name"}}], "severity":"High", "remediation":{"desc":"test"}},
             {"cloud":{"account":{"id": "123abc"}, "provider":"aws"}, "finding_info": {"desc":"description"}, "metadata":{"event_code":"123"}, "resources":[{"group":{"name":"service name"}}], "severity":"High", "remediation":{"desc":"test"}}
            ])";
  out = getResult(test);
  BOOST_TEST_REQUIRE(1 == out.size());
  BOOST_TEST(2 == out[0].data.size());
  tp.r =  Result();
}
