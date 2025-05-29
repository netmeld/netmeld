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

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include "ProwlerData.hpp"

namespace nmdo = netmeld::datastore::objects;

namespace netmeld::datastore::objects::prowler {


    class TestProwlerData : public ProwlerData
    {
        public:
            using ProwlerData::assessmentStartTime;
            using ProwlerData::provider;
            using ProwlerData::accountId;
            using ProwlerData::checkId;
            using ProwlerData::serviceName;
            using ProwlerData::severity;
            using ProwlerData::recommendation;
            using ProwlerData::description;

            using ProwlerData::region;
            using ProwlerData::resourceId;
            using ProwlerData::risk;
            using ProwlerData::status;
            using ProwlerData::extras;
            using ProwlerData::ProwlerData;
    };

    BOOST_AUTO_TEST_CASE(ConstructorTest)
    {
        // Create a sample JSON object
        json jline = json::parse(R"({
            "metadata": {
                "event_code": "cloudtrail_multi_region_enabled",
                "product": {
                    "name": "Prowler",
                    "vendor_name": "Prowler",
                    "version": "4.2.4"
                },
                "version": "1.1.0"
            },
            "severity_id": 4,
            "severity": "High",
            "status": "New",
            "status_code": "FAIL",
            "status_detail": "No CloudTrail trails enabled and logging were found.",
            "status_id": 1,
            "activity_name": "Create",
            "activity_id": 1,
            "finding_info": {
                "created_time": "2024-04-08T11:33:51.870861",
                "desc": "Ensure CloudTrail is enabled in all regions",
                "product_uid": "prowler",
                "title": "Ensure CloudTrail is enabled in all regions",
                "uid": "prowler-aws-cloudtrail_multi_region_enabled-123456789012-ap-northeast-1-123456789012",
                "types": ["Software and Configuration Checks","Industry and Regulatory Standards","CIS AWS Foundations Benchmark"]
            },
            "resources": [
                {
                    "cloud_partition": "aws",
                    "region": "ap-northeast-1",
                    "group": {
                        "name": "cloudtrail"
                    },
                    "labels": [],
                    "name": "123456789012",
                    "type": "AwsCloudTrailTrail",
                    "uid": "arn:aws:cloudtrail:ap-northeast-1:123456789012:trail",
                    "data": {
                        "details": "More detail here"
                    }
                }
            ],
            "category_name": "Findings",
            "category_uid": 2,
            "class_name": "DetectionFinding",
            "class_uid": 2004,
            "cloud": {
                "account": {
                    "name": "test-account",
                    "type": "AWS_Account",
                    "type_id": 10,
                    "uid": "123456789012"
                },
                "org": {
                    "name": "example-name",
                    "uid": ""
                },
                "provider": "aws",
                "region": "ap-northeast-1"
            },
            "event_time": "2024-04-08T11:33:51.870861",
            "remediation": {
                "desc": "Ensure Logging is set to ON on all regions (even if they are not being used at the moment.",
                "references": [
                    "aws cloudtrail create-trail --name <trail_name> --bucket-name <s3_bucket_for_cloudtrail> --is-multi-region-trail aws cloudtrail update-trail --name <trail_name> --is-multi-region-trail ",
                    "https://docs.aws.amazon.com/awscloudtrail/latest/userguide/cloudtrailconcepts.html#cloudtrail-concepts-management-events"
                ]
            },
            "type_uid": 200401,
            "type_name": "Create",
            "unmapped": {
                "related_url": "https://example.com",
                "categories": ["forensics-ready"],
                "depends_on": [],
                "related_to": [],
                "notes": "Example notes",
                "compliance": {
                    "CISA": [
                        "your-systems-3",
                        "your-data-2"
                    ],
                    "SOC2": [
                        "cc_2_1",
                        "cc_7_2",
                        "cc_a_1_2"
                    ],
                    "CIS-1.4": [
                        "3.1"
                    ],
                    "CIS-1.5": [
                        "3.1"
                    ],
                    "GDPR": [
                        "article_25",
                        "article_30"
                    ],
                    "AWS-Foundational-Security-Best-Practices": [
                        "cloudtrail"
                    ],
                    "ISO27001-2013": [
                        "A.12.4"
                    ],
                    "HIPAA": [
                        "164_308_a_1_ii_d",
                        "164_308_a_3_ii_a",
                        "164_308_a_6_ii",
                        "164_312_b",
                        "164_312_e_2_i"
                    ]
                }
            }
        })");
        auto config = YAML::Load(R"(# Required fields
assessmentStartTime: ".event_time"
_timeFormat: "%Y-%m-%dT%H:%M:%S"
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

        // Create an instance of ProwlerData using the constructor
        TestProwlerData data(jline, config);

        // Perform assertions to verify the values are correctly assigned
        BOOST_TEST("2024-04-08T11:33:51" == data.assessmentStartTime.toString());
        BOOST_TEST("aws" == data.provider);
        BOOST_TEST("123456789012" == data.accountId);
        BOOST_TEST("cloudtrail_multi_region_enabled" == data.checkId);
        BOOST_TEST("cloudtrail" == data.serviceName);
        BOOST_TEST("High" == data.severity);
        BOOST_TEST("Ensure Logging is set to ON on all regions (even if they are not being used at the moment." == data.recommendation);
        BOOST_TEST("Ensure CloudTrail is enabled in all regions" == data.description);

        BOOST_TEST("ap-northeast-1" == data.region);
        BOOST_TEST("123456789012" == data.resourceId);
        BOOST_TEST("" == data.risk); // Default since no value was given
        BOOST_TEST("FAIL" == data.status);

        BOOST_TEST("Ensure CloudTrail is enabled in all regions" == data.extras["checkTitle"]);
        BOOST_TEST("No CloudTrail trails enabled and logging were found." == data.extras["statusExtended"]);
        BOOST_TEST("arn:aws:cloudtrail:ap-northeast-1:123456789012:trail" == data.extras["resourceArn"]);
        BOOST_TEST("AwsCloudTrailTrail" == data.extras["resourceType"]);
        BOOST_TEST("More detail here" == data.extras["resourceDetails"]);
        BOOST_TEST("https://example.com" == data.extras["relatedUrl"]);
        BOOST_TEST("Example notes" == data.extras["notes"]);
        json checkTypes {"Software and Configuration Checks","Industry and Regulatory Standards","CIS AWS Foundations Benchmark"};
        BOOST_TEST(checkTypes == data.extras["checkTypes"]);
        json categories {"forensics-ready"};
        BOOST_TEST(categories == data.extras["categories"]);
        BOOST_TEST("Account Name: test-account, Account Org: example-name" == data.extras["organizationsInfo"]); // Default since no values given
        json resourceTags = json::array();
        BOOST_TEST(resourceTags == data.extras["resourceTags"]); // Nothing should be returned
        //BOOST_TEST("Key1\n- Value1\n- Value2\nKey2\n- Value3" == data.extras["compliance"]); // TODO: Way too long for now
        BOOST_TEST("prowler-aws-cloudtrail_multi_region_enabled-123456789012-ap-northeast-1-123456789012" == data.extras["findingUniqueId"]);
        BOOST_TEST("https://docs.aws.amazon.com/awscloudtrail/latest/userguide/cloudtrailconcepts.html#cloudtrail-concepts-management-events" == data.extras["recommendationUrl"]);
        BOOST_TEST("aws cloudtrail create-trail --name <trail_name> --bucket-name <s3_bucket_for_cloudtrail> --is-multi-region-trail aws cloudtrail update-trail --name <trail_name> --is-multi-region-trail " == data.extras["remediationCode"]);
    }

    BOOST_AUTO_TEST_CASE(ConstructorTestNoData)
    {
        json jline = json::parse("{}");
        auto config = YAML::Load("{}");

        // Create an instance of ProwlerData using the constructor
        TestProwlerData emptyData(jline, config);
        TestProwlerData defaultData;

        // Perform assertions to verify that the ProwlerData object constructed from empty JSON
        // is equal to ProwlerData's default state
        BOOST_TEST(defaultData == emptyData);
    }

    BOOST_AUTO_TEST_CASE(SpaceshipOperatorTest)
    {
        // Create two ProwlerData objects using the default constructor
        TestProwlerData data1;
        TestProwlerData data2;

        // Verify that these objects are equal
        BOOST_TEST((data1 == data2));

        // Change the timestap value for data 1 and verify that the objects are still equal
        data2.assessmentStartTime = nmco::Time();
        BOOST_TEST(data1 == data2);

        // Change account numbider and verify that the two objects are no longer equal
        data1.accountId = "999999999";
        BOOST_TEST(data1 != data2);
    }

    BOOST_AUTO_TEST_CASE(SimpleTest)
    {
		json jline = json::parse(R"({
		"event_time": "2025-01-01T00:00:00",
		"cloud": {
			"provider": "test-provider",
			"account_uid": "test-account-id"
		},
		"resources": [
			{
				"group_name": "test-service-name"
			},
			{
				"group_name": "wrong-service-name"
			}
		],
		"event_code": "test-check-id",
                "description": "test-description",
		"severity": "HIGH"
	})");
		// Minimum required fields
		auto config = YAML::Load(R"(assessmentStartTime: ".event_time"
_timeFormat: "%Y-%m-%dT%H:%M:%S"
provider: ".cloud.provider"
accountId: ".cloud.account_uid"
serviceName: ".resources.[0].group_name"
checkId: ".event_code"
severity: ".severity"
recommendation: "test-recommendation"
description: ".description"
)");

		// Create an instance of ProwlerData using the constructor
        TestProwlerData data(jline, config);

        BOOST_TEST("2025-01-01T00:00:00" == data.assessmentStartTime.toString());
        BOOST_TEST("test-provider" == data.provider);
        BOOST_TEST("test-account-id" == data.accountId);
        BOOST_TEST("test-service-name" == data.serviceName);
        BOOST_TEST("test-check-id" == data.checkId);
        BOOST_TEST("HIGH" == data.severity);
        BOOST_TEST("test-recommendation" == data.recommendation);
        BOOST_TEST("test-description" == data.description);
	}

    BOOST_AUTO_TEST_CASE(SpecialTest)
    {
		json jline = json::parse(R"({
		"event_time": "2025-01-01T00:00:00",
		"cloud": {
			"provider": "provider",
			"helper": "helper"
		},
		"service_name": [
			"test",
			"service",
			"name"
		],
		"account_id": [
			"which values do we use?",
			"probably not this one",
			"test",
			"nor this one",
			"account",
			"definitely not this one",
			"id"
		],
		"severity": "HIGH"
	})");
		// Minimum required fields
		auto config = YAML::Load(R"(
assessmentStartTime: ".event_time"
_timeFormat: "%Y-%m-%dT%H:%M:%S"
provider:
  concat:
    - "test-"
    - ".cloud.provider"
    - "-with-"
    - ".cloud.helper"
accountId:
  filter:
    source: ".account_id"
    regex: "^[\\S]+$"
    join_str: "-"
serviceName:
  join:
    source: ".service_name"
    join_str: "-"
checkId:
  join:
    source: ".invalid"
    join_str: "\n"
severity: ".severity"
recommendation:
  concat:
    - .invalid
    - .super.duper.invalid
description: ".description"
)");

		// Create an instance of ProwlerData using the constructor
        TestProwlerData data(jline, config);

        BOOST_TEST("2025-01-01T00:00:00" == data.assessmentStartTime.toString());
        BOOST_TEST("test-provider-with-helper" == data.provider);
        BOOST_TEST("test-account-id" == data.accountId);
        BOOST_TEST("test-service-name" == data.serviceName);
        BOOST_TEST("null" == data.checkId); // Not found should be null
        BOOST_TEST("HIGH" == data.severity);
        BOOST_TEST("nullnull" == data.recommendation); // Multiple not founds should be null
        BOOST_TEST("" == data.description); // Not found should be empty string (in non-special)
	}
}
