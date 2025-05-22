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
    //using Parser::fromJsonV2;
    //using Parser::fromJsonV3;
};

BOOST_AUTO_TEST_CASE(testFromJsonV2)
{
  auto v2Config = json::parse(R"({
    "assessmentStartTime": ".Timestamp",
    "_timeFormat": "%Y-%m-%dT%H:%M:%SZ",
    "findingUniqueId": null,
    "provider": "aws",
    "profile": null,
    "accountId": ".Account Number",
    "organizationsInfo": null,
    "region": ".Region",
    "checkId": ".Control ID",
    "checkTitle": null,
    "checkTypes": null,
    "serviceName": ".Service",
    "subServiceName": null,
    "status": ".Status",
    "statusExtended": null,
    "severity": ".Severity",
    "resourceId": ".Resource ID",
    "resourceArn": null,
    "resourceTags": null,
    "resourceType": null,
    "resourceDetails": null,
    "description": ".Control",
    "risk": ".Risk",
    "relatedUrl": null,
    "recommendation": ".Remediation",
    "recommendationUrl": ".Doc link",
    "remediationCode": null,
    "categories": null,
    "notes": null,
    "compliance": ".Level"
  })");
  // NOTE: These are primarily for testing "file" logic, not data logic
  TestParser tp;
  std::string test;
  Result out;

  const auto getResult = [&](const std::string& jsonData) {
      std::istringstream is {jsonData};
      tp.fromJson(is, v2Config);
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
  auto v3Config = json::parse(R"({
    "assessmentStartTime": ".AssessmentStartTime",
    "_timeFormat": "%Y-%m-%dT%H:%M:%S",
    "findingUniqueId": ".FindingUniqueId",
    "provider": ".Provider",
    "profile": ".Profile",
    "accountId": ".AccountId",
    "organizationsInfo": ".OrganizationsInfo",
    "region": ".Region",
    "checkId": ".CheckID",
    "checkTitle": ".CheckTitle",
    "checkTypes": ".CheckType",
    "serviceName": ".ServiceName",
    "subServiceName": ".SubServiceName",
    "status": ".Status",
    "statusExtended": ".StatusExtended",
    "severity": ".Severity",
    "resourceId": ".ResourceId",
    "resourceArn": ".ResourceArn",
    "resourceTags": ".ResourceTags",
    "resourceType": ".ResourceType",
    "resourceDetails": ".ResourceDetails",
    "description": ".Description",
    "risk": ".Risk",
    "relatedUrl": ".RelatedUrl",
    "recommendation": ".Remediation.Recommendation.Text",
    "recommendationUrl": ".Remediation.Recommendation.Url",
    "remediationCode": ".Remediation.Code",
    "categories": ".Categories",
    "notes": ".Notes",
    "compliance": ".Compliance"
  })");
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
  auto ocsfConfig = json::parse(R"({
      "assessmentStartTime": ".event_time",
      "_timeFormat": "%Y-%m-%dT%H:%M:%S.%f",
      "findingUniqueId": ".finding_info.uid",
      "provider": ".cloud.provider",
      "profile": null,
      "accountId": ".cloud.account.uid",
      "organizationsInfo": {
          "concat": [
              "Account Name: ",
              ".cloud.account.name",
              ", Account Org: ",
              ".cloud.org.name"
          ]
      },
      "region": ".resources.[0].region",
      "checkId": ".metadata.event_code",
      "checkTitle": ".finding_info.title",
      "checkTypes": {
          "join": {
              "source": ".finding_info.types",
              "join_str": "\n"
          }
      },
      "serviceName": ".resources.[0].group.name",
      "subServiceName": null,
      "status": ".status_code",
      "statusExtended": ".status_detail",
      "severity": ".severity",
      "resourceId": ".resources.[0].name",
      "resourceArn": ".resources.[0].uid",
      "resourceTags": ".resources.[0].labels",
      "resourceType": ".resources.[0].type",
      "resourceDetails": ".resources.[0].data.details",
      "description": ".finding_info.desc",
      "risk": ".risk_details",
      "relatedUrl": ".unmapped.related_url",
      "recommendation": ".remediation.desc",
      "recommendationUrl": {
          "filter": {
              "source": ".remediation.references",
              "regex": "^http.*",
              "join_str": "\n"
          }
      },
      "remediationCode": {
          "filter": {
              "source": ".remediation.references",
              "regex": "^(?!http).*",
              "join_str": "\n"
          }
      },
      "categories": ".unmapped.categories",
      "notes": ".unmapped.notes",
      "compliance": ".unmapped.compliance"
  })");
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
  test = R"([{"cloud":{"account":{"id": "123abc"}}, "finding_info": {"uid":"abc123"}}])";
  out = getResult(test);
  BOOST_TEST_REQUIRE(1 == out.size());
  BOOST_TEST(1 == out[0].data.size());
  tp.r =  Result();

  test = R"([{"cloud":{"account":{"id": "123abc"}}, "finding_info": {"uid":"abc123"}},
             {"cloud":{"account":{"id": "123abc"}}, "finding_info": {"uid":"abc123"}}
            ])";
  out = getResult(test);
  BOOST_TEST_REQUIRE(1 == out.size());
  BOOST_TEST(2 == out[0].data.size());
  tp.r =  Result();
}
