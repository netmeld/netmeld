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

#include "ProwlerOCSFData.hpp"

namespace nmdo = netmeld::datastore::objects;

namespace netmeld::datastore::objects::prowler {


    class TestProwlerOCSFData : public ProwlerOCSFData
    {
        public:
            using ProwlerOCSFData::assessmentStartTime;
            using ProwlerOCSFData::findingUniqueId;
            using ProwlerOCSFData::provider;
            using ProwlerOCSFData::profile;
            using ProwlerOCSFData::accountId;
            using ProwlerOCSFData::organizationsInfo;
            using ProwlerOCSFData::region;
            using ProwlerOCSFData::checkId;
            using ProwlerOCSFData::checkTitle;
            using ProwlerOCSFData::checkTypes;
            using ProwlerOCSFData::serviceName;
            using ProwlerOCSFData::subServiceName;
            using ProwlerOCSFData::status;
            using ProwlerOCSFData::statusExtended;
            using ProwlerOCSFData::severity;
            using ProwlerOCSFData::resourceId;
            using ProwlerOCSFData::resourceArn;
            using ProwlerOCSFData::resourceTags;
            using ProwlerOCSFData::resourceType;
            using ProwlerOCSFData::resourceDetails;
            using ProwlerOCSFData::description;
            using ProwlerOCSFData::risk;
            using ProwlerOCSFData::relatedUrl;
            using ProwlerOCSFData::recommendation;
            using ProwlerOCSFData::recommendationUrl;
            using ProwlerOCSFData::remediationCode;
            using ProwlerOCSFData::categories;
            using ProwlerOCSFData::notes;
            using ProwlerOCSFData::compliance;
            using ProwlerOCSFData::ProwlerOCSFData;
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

        // Create an instance of ProwlerOCSFData using the constructor
        TestProwlerOCSFData data(jline);

        // Perform assertions to verify the values are correctly assigned
        BOOST_TEST("2024-04-08T11:33:51" == data.assessmentStartTime.toString());
        BOOST_TEST("prowler-aws-cloudtrail_multi_region_enabled-123456789012-ap-northeast-1-123456789012" == data.findingUniqueId);
        BOOST_TEST("aws" == data.provider);
        //BOOST_TEST(null == data.profile);
        BOOST_TEST("123456789012" == data.accountId);
        BOOST_TEST("ap-northeast-1" == data.region);
        BOOST_TEST("cloudtrail_multi_region_enabled" == data.checkId);
        BOOST_TEST("Ensure CloudTrail is enabled in all regions" == data.checkTitle);
        BOOST_TEST("cloudtrail" == data.serviceName);
        //BOOST_TEST(null == data.subServiceName);
        BOOST_TEST("FAIL" == data.status);
        BOOST_TEST("No CloudTrail trails enabled and logging were found." == data.statusExtended);
        BOOST_TEST("High" == data.severity);
        BOOST_TEST("123456789012" == data.resourceId);
        BOOST_TEST("arn:aws:cloudtrail:ap-northeast-1:123456789012:trail" == data.resourceArn);
        BOOST_TEST("AwsCloudTrailTrail" == data.resourceType);
        BOOST_TEST("More detail here" == data.resourceDetails);
        BOOST_TEST("Ensure CloudTrail is enabled in all regions" == data.description); // Default since no value was given
        BOOST_TEST("" == data.risk); // Default since no value was given
        BOOST_TEST("https://example.com" == data.relatedUrl);
        BOOST_TEST("Example notes" == data.notes);
        BOOST_TEST("Software and Configuration Checks\nIndustry and Regulatory Standards\nCIS AWS Foundations Benchmark" == data.checkTypes);
        BOOST_TEST("forensics-ready" == data.categories);
        BOOST_TEST("account_name: test-account\naccount_org: example-name" == data.organizationsInfo); // Default since no values given
        BOOST_TEST("" == data.resourceTags); // Default since no values given
        //BOOST_TEST("Key1\n- Value1\n- Value2\nKey2\n- Value3" == data.compliance); // TODO: Way too long for now
        BOOST_TEST("Ensure Logging is set to ON on all regions (even if they are not being used at the moment." == data.recommendation);
        BOOST_TEST("https://docs.aws.amazon.com/awscloudtrail/latest/userguide/cloudtrailconcepts.html#cloudtrail-concepts-management-events\n" == data.recommendationUrl);
        BOOST_TEST("aws cloudtrail create-trail --name <trail_name> --bucket-name <s3_bucket_for_cloudtrail> --is-multi-region-trail aws cloudtrail update-trail --name <trail_name> --is-multi-region-trail \n" == data.remediationCode);
    }

    BOOST_AUTO_TEST_CASE(ConstructorTestNoData)
    {
        json jline = json::parse("{}");

        // Create an instance of ProwlerOCSFData using the constructor
        TestProwlerOCSFData emptyData(jline);
        TestProwlerOCSFData defaultData;

        // Perform assertions to verify that the ProwlerOCSFData object constructed from empty JSON
        // is equal to ProwlerOCSFData's default state
        BOOST_TEST(defaultData == emptyData);
    }

    BOOST_AUTO_TEST_CASE(SpaceshipOperatorTest)
    {
        // Create two ProwlerOCSFData objects using the default constructor
        TestProwlerOCSFData data1;
        TestProwlerOCSFData data2;

        // Verify that these objects are equal
        BOOST_TEST((data1 == data2));

        // Change the timestap value for data 1 and verify that the objects are still equal
        data2.assessmentStartTime = nmco::Time();
        BOOST_TEST(data1 == data2);

        // Change account numbider and verify that the two objects are no longer equal
        data1.accountId = "999999999";
        BOOST_TEST(data1 != data2);
    }
}
