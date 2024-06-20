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
        BOOST_TEST("123456789" == data.accountNumber);
        BOOST_TEST("us-east-1" == data.region);
        BOOST_TEST("Some Control" == data.control);
        BOOST_TEST("high" == data.severity); // Assuming toLower works as expected
        BOOST_TEST("Passed" == data.status);
        BOOST_TEST("2" == data.level);
        BOOST_TEST("C-12345" == data.controlId);
        BOOST_TEST("EC2" == data.service);
        BOOST_TEST("Medium" == data.risk);
        BOOST_TEST("Apply patch" == data.remediation);
        BOOST_TEST("http://example.com/doc" == data.documentationLink);
        BOOST_TEST("i-1234567890abcdef0" == data.resourceId);
        BOOST_TEST("2022-01-01T01:01:01" == data.timestamp.toString());
    }

    BOOST_AUTO_TEST_CASE(ConstructorTestNoData)
    {
        json jline = json::parse("{}");

        // Create an instance of ProwlerV3Data using the constructor
        TestProwlerV2Data emptyData(jline);
        TestProwlerV2Data defaultData;

        // Perform assertions to verify that the ProwlerV3Data object constructed from empty JSON
        // is equal to ProwlerV3Data's default state
        BOOST_TEST(defaultData == emptyData);
    }

    BOOST_AUTO_TEST_CASE(SpaceshipOperatorTest)
    {
        // Create two ProwlerV2Data objects using the default constructor
        TestProwlerV2Data data1;
        TestProwlerV2Data data2;

        // Verify that these objects are equal
        BOOST_TEST((data1 == data2));

        // Change the timestap value for data 1 and verify that the objects are still equal
        data2.timestamp = nmco::Time();
        BOOST_TEST(data1 == data2);

        // Change account number and verify that the two objects are no longer equal
        data1.accountNumber = "999999999";
        BOOST_TEST(data1 != data2);
    }
}
