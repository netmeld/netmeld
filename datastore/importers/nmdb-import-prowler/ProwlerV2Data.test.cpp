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

#include "ProwlerV2Data.hpp"

namespace nmdo = netmeld::datastore::objects;
// namespace nmdp = netmeld::datastore::parsers;
namespace netmeld::datastore::objects::prowler {


    class TestProwlerV2Data : public ProwlerV2Data
    {
        public:
            using ProwlerV2Data::accountNumber;
            using ProwlerV2Data::timestamp;
            using ProwlerV2Data::region;
            using ProwlerV2Data::control;
            using ProwlerV2Data::severity;
            using ProwlerV2Data::status;
            using ProwlerV2Data::level;
            using ProwlerV2Data::controlId;
            using ProwlerV2Data::service;
            using ProwlerV2Data::risk;
            using ProwlerV2Data::remediation;
            using ProwlerV2Data::documentationLink;
            using ProwlerV2Data::resourceId;
            using ProwlerV2Data::ProwlerV2Data;
    };

    BOOST_AUTO_TEST_CASE(ConstructorTest)
    {
        // Create a JSON object with the expected structure
        json jline = {
            {"Account Number", "123456789"},
            {"Region", "us-east-1"},
            {"Control", "Some Control"},
            {"Severity", "HIGH"},
            {"Status", "Passed"},
            {"Level", "2"},
            {"Control ID", "C-12345"},
            {"Service", "EC2"},
            {"Risk", "Medium"},
            {"Remediation", "Apply patch"},
            {"Doc link", "http://example.com/doc"},
            {"Resource ID", "i-1234567890abcdef0"},
            {"Timestamp", "2022-01-01T01:01:01Z"}
        };

        // Construct a ProwlerV2Data object using the JSON object
        TestProwlerV2Data data(jline);

        // Verify that the fields are correctly populated using BOOST_TEST
        BOOST_TEST(data.accountNumber == "123456789");
        BOOST_TEST(data.region == "us-east-1");
        BOOST_TEST(data.control == "Some Control");
        BOOST_TEST(data.severity == "high"); // Assuming toLower works as expected
        BOOST_TEST(data.status == "Passed");
        BOOST_TEST(data.level == "2");
        BOOST_TEST(data.controlId == "C-12345");
        BOOST_TEST(data.service == "EC2");
        BOOST_TEST(data.risk == "Medium");
        BOOST_TEST(data.remediation == "Apply patch");
        BOOST_TEST(data.documentationLink == "http://example.com/doc");
        BOOST_TEST(data.resourceId == "i-1234567890abcdef0");
        BOOST_TEST(data.timestamp.toString() == "2022-01-01T01:01:01");
    }
}
