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

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include "ProwlerV3Data.hpp"

namespace nmdo = netmeld::datastore::objects;

namespace netmeld::datastore::objects::prowler {


    class TestProwlerV3Data : public ProwlerV3Data
    {
        public:
            using ProwlerV3Data::assessmentStartTime;
            using ProwlerV3Data::findingUniqueId;
            using ProwlerV3Data::provider;
            using ProwlerV3Data::profile;
            using ProwlerV3Data::accountId;
            using ProwlerV3Data::organizationsInfo;
            using ProwlerV3Data::region;
            using ProwlerV3Data::checkId;
            using ProwlerV3Data::checkTitle;
            using ProwlerV3Data::checkTypes;
            using ProwlerV3Data::serviceName;
            using ProwlerV3Data::subServiceName;
            using ProwlerV3Data::status;
            using ProwlerV3Data::statusExtended;
            using ProwlerV3Data::severity;
            using ProwlerV3Data::resourceId;
            using ProwlerV3Data::resourceArn;
            using ProwlerV3Data::resourceTags;
            using ProwlerV3Data::resourceType;
            using ProwlerV3Data::resourceDetails;
            using ProwlerV3Data::description;
            using ProwlerV3Data::risk;
            using ProwlerV3Data::relatedUrl;
            using ProwlerV3Data::recommendation;
            using ProwlerV3Data::recommendationUrl;
            using ProwlerV3Data::remediationCode;
            using ProwlerV3Data::categories;
            using ProwlerV3Data::notes;
            using ProwlerV3Data::compliance;
            using ProwlerV3Data::ProwlerV3Data;
    };

    BOOST_AUTO_TEST_CASE(ConstructorTest)
    {
        // Create a sample JSON object
        json jline = {
            {"AssessmentStartTime", "2024-05-30T10:00:00"},
            {"FindingUniqueId", "123456"},
            {"Provider", "ACME"},
            {"Profile", "Default"},
            {"AccountId", "1234567890"},
            {"Region", "us-west-2"},
            {"CheckID", "CHECK-001"},
            {"CheckTitle", "Example Check"},
            {"ServiceName", "Example Service"},
            {"SubServiceName", "Example Subservice"},
            {"Status", "Open"},
            {"StatusExtended", "In Progress"},
            {"Severity", "High"},
            {"ResourceId", "resource-123"},
            {"ResourceArn", "arn:aws:example"},
            {"ResourceType", "Example Resource"},
            {"ResourceDetails", "Example details"},
            {"Description", "Example description"},
            {"Risk", "High"},
            {"RelatedUrl", "https://example.com"},
            {"Notes", "Example notes"},
            {"CheckType", {"Type1", "Type2"}},
            {"Categories", {"Category1", "Category2"}},
            {"OrganizationsInfo", {
                {"Key1", "Value1"},
                {"Key2", "Value2"}
            }},
            {"ResourceTags", {
                {"Tag1", "Value1"},
                {"Tag2", "Value2"}
            }},
            {"Compliance", {
                {"Key1", {"Value1", "Value2"}},
                {"Key2", {"Value3"}}
            }},
            {"Remediation", {
                {"Recommendation", {
                    {"Text", "Example recommendation"},
                    {"Url", "https://example.com"}
                }},
                {"Code", {
                    {"Code1", "Value1"},
                    {"Code2", "Value2"}
                }}
            }}
        };

        // Create an instance of ProwlerV3Data using the constructor
        TestProwlerV3Data data(jline);

        // Perform assertions to verify the values are correctly assigned
        BOOST_TEST(data.assessmentStartTime.toString() == "2024-05-30T10:00:00");
        BOOST_TEST(data.findingUniqueId == "123456");
        BOOST_TEST(data.provider == "ACME");
        BOOST_TEST(data.profile == "Default");
        BOOST_TEST(data.accountId == "1234567890");
        BOOST_TEST(data.region == "us-west-2");
        BOOST_TEST(data.checkId == "CHECK-001");
        BOOST_TEST(data.checkTitle == "Example Check");
        BOOST_TEST(data.serviceName == "Example Service");
        BOOST_TEST(data.subServiceName == "Example Subservice");
        BOOST_TEST(data.status == "Open");
        BOOST_TEST(data.statusExtended == "In Progress");
        BOOST_TEST(data.severity == "High");
        BOOST_TEST(data.resourceId == "resource-123");
        BOOST_TEST(data.resourceArn == "arn:aws:example");
        BOOST_TEST(data.resourceType == "Example Resource");
        BOOST_TEST(data.resourceDetails == "Example details");
        BOOST_TEST(data.description == "Example description");
        BOOST_TEST(data.risk == "High");
        BOOST_TEST(data.relatedUrl == "https://example.com");
        BOOST_TEST(data.notes == "Example notes");
        BOOST_TEST(data.checkTypes == "Type1\nType2");
        BOOST_TEST(data.categories == "Category1\nCategory2");
        BOOST_TEST(data.organizationsInfo == "Key1: \"Value1\"\nKey2: \"Value2\"");
        BOOST_TEST(data.resourceTags == "Tag1: \"Value1\"\nTag2: \"Value2\"");
        BOOST_TEST(data.compliance == "Key1\n- Value1\n- Value2\nKey2\n- Value3");
        BOOST_TEST(data.recommendation == "Example recommendation");
        BOOST_TEST(data.recommendationUrl == "https://example.com");
        BOOST_TEST(data.remediationCode == "Code1: Value1\nCode2: Value2");
    }

    BOOST_AUTO_TEST_CASE(ConstructorTestNoData)
    {
        json jline = json::parse("{}");

        // Create an instance of ProwlerV3Data using the constructor
        TestProwlerV3Data emptyData(jline);
        TestProwlerV3Data defaultData;

        // Perform assertions to verify that the ProwlerV3Data object constructed from empty JSON
        // is equal to ProwlerV3Data's default state
        // BOOST_TEST(defaultData.assessmentStartTime.toString() == emptyData.assessmentStartTime.toString());
        BOOST_TEST(defaultData.findingUniqueId == emptyData.findingUniqueId);
        BOOST_TEST(defaultData.provider == emptyData.provider);
        BOOST_TEST(defaultData.profile == emptyData.profile);
        BOOST_TEST(defaultData.accountId == emptyData.accountId);
        BOOST_TEST(defaultData.region == emptyData.region);
        BOOST_TEST(defaultData.checkId == emptyData.checkId);
        BOOST_TEST(defaultData.checkTitle == emptyData.checkTitle);
        BOOST_TEST(defaultData.serviceName == emptyData.serviceName);
        BOOST_TEST(defaultData.subServiceName == emptyData.subServiceName);
        BOOST_TEST(defaultData.status == emptyData.status);
        BOOST_TEST(defaultData.statusExtended == emptyData.statusExtended);
        BOOST_TEST(defaultData.severity == emptyData.severity);
        BOOST_TEST(defaultData.resourceId == emptyData.resourceId);
        BOOST_TEST(defaultData.resourceArn == emptyData.resourceArn);
        BOOST_TEST(defaultData.resourceType == emptyData.resourceType);
        BOOST_TEST(defaultData.resourceDetails == emptyData.resourceDetails);
        BOOST_TEST(defaultData.description == emptyData.description);
        BOOST_TEST(defaultData.risk == emptyData.risk);
        BOOST_TEST(defaultData.relatedUrl == emptyData.relatedUrl);
        BOOST_TEST(defaultData.notes == emptyData.notes);
        BOOST_TEST(defaultData.checkTypes == emptyData.checkTypes);
        BOOST_TEST(defaultData.categories == emptyData.categories);
        BOOST_TEST(defaultData.organizationsInfo == emptyData.organizationsInfo);
        BOOST_TEST(defaultData.resourceTags == emptyData.resourceTags);
        BOOST_TEST(defaultData.compliance == emptyData.compliance);
        BOOST_TEST(defaultData.recommendation == emptyData.recommendation);
        BOOST_TEST(defaultData.recommendationUrl == emptyData.recommendationUrl);
        BOOST_TEST(defaultData.remediationCode == emptyData.remediationCode);
    }
}
