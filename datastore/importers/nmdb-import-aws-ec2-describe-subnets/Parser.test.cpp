// =============================================================================
// Copyright 2022 National Technology & Engineering Solutions of Sandia, LLC
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

namespace nmdo = netmeld::datastore::objects;
namespace nmdp = netmeld::datastore::parsers;

using qi::ascii::blank;

class TestParser : public Parser
{
  public:
    using Parser::processSubnets;
    using Parser::fromJson;
};

BOOST_AUTO_TEST_CASE(testProcessSubnets)
{
  {
    TestParser tp;

    json tv1 = json::parse(R"(
        { "AssignIpv6AddressOnCreation": false
        , "AvailabilityZone": ""
        , "AvailabilityZoneId": ""
        , "AvailableIpAddressCount": 0
        , "CidrBlock": ""
        , "CustomerOwnedIpv4Pool": ""
        , "DefaultForAz": false
        , "EnableDns64": false
        , "EnableLniAtDeviceIndex": 0
        , "Ipv6CidrBlockAssociationSet": []
        , "Ipv6Native": false
        , "MapCustomerOwnedIpOnLaunch": false
        , "MapPublicIpOnLaunch": false
        , "OutpostArn": ""
        , "OwnerId": ""
        , "PrivateDnsNameOptionsOnLaunch": {}
        , "State": ""
        , "SubnetArn": ""
        , "SubnetId": ""
        , "Tags": []
        , "VpcId": ""
        }
        )"
      );
    tp.processSubnets(tv1);
    auto trv1 = tp.getData()[0].subnets;
    auto trv2 = tp.getData()[0].observations;
    BOOST_TEST(1 == trv1.size());
    BOOST_TEST(!trv1[0].isValid());
    BOOST_TEST(trv2.isValid());
  }
  {
    TestParser tp;

    json tv1 = json::parse(R"(
        { "AssignIpv6AddressOnCreation": false
        , "AvailabilityZone": ""
        , "AvailabilityZoneId": ""
        , "AvailableIpAddressCount": 0
        , "CidrBlock": ""
        , "CustomerOwnedIpv4Pool": ""
        , "DefaultForAz": false
        , "EnableDns64": false
        , "EnableLniAtDeviceIndex": 0
        , "Ipv6CidrBlockAssociationSet": []
        , "Ipv6Native": false
        , "MapCustomerOwnedIpOnLaunch": false
        , "MapPublicIpOnLaunch": false
        , "OutpostArn": ""
        , "OwnerId": ""
        , "PrivateDnsNameOptionsOnLaunch": {}
        , "State": "available"
        , "SubnetArn": ""
        , "SubnetId": ""
        , "Tags": []
        , "VpcId": ""
        }
        )"
      );
    tp.processSubnets(tv1);
    auto trv1 = tp.getData()[0].subnets;
    auto trv2 = tp.getData()[0].observations;
    BOOST_TEST(1 == trv1.size());
    BOOST_TEST(!trv1[0].isValid());
    BOOST_TEST(!trv2.isValid());
  }
  {
    TestParser tp;

    json tv1 = json::parse(R"(
        { "AssignIpv6AddressOnCreation": false
        , "AvailabilityZone": "a1"
        , "AvailabilityZoneId": ""
        , "AvailableIpAddressCount": 0
        , "CidrBlock": "a2"
        , "CustomerOwnedIpv4Pool": ""
        , "DefaultForAz": false
        , "EnableDns64": false
        , "EnableLniAtDeviceIndex": 0
        , "Ipv6CidrBlockAssociationSet":
          [{ "AssociationId": ""
          , "Ipv6CidrBlock": "a3"
          , "Ipv6CidrBlockState":
            { "State": "a8"
            , "StatusMessage": ""
            }
          }]
        , "Ipv6Native": false
        , "MapCustomerOwnedIpOnLaunch": false
        , "MapPublicIpOnLaunch": false
        , "OutpostArn": ""
        , "OwnerId": ""
        , "PrivateDnsNameOptionsOnLaunch": {}
        , "State": "a4"
        , "SubnetArn": "a5"
        , "SubnetId": "a6"
        , "Tags": []
        , "VpcId": "a7"
        }
        )"
      );
    tp.processSubnets(tv1);
    auto trv1 = tp.getData()[0].subnets;
    auto trv2 = tp.getData()[0].observations;
    BOOST_TEST(1 == trv1.size());
    BOOST_TEST(trv1[0].isValid());
    BOOST_TEST(trv2.isValid());
    const std::vector<std::string> tevs {
          R"(subnetId: a6)"
        , R"(vpcId: a7)"
        , R"(availabilityZone: a1)"
        , R"(subnetArn: a5)"
        , R"(cidrBlock: a2,)"
        , R"(cidrBlock: a3,)"
        , R"(state: a8,)"
      };
    for (const auto& tev : tevs) {
      nmdp::testInString(trv1[0].toDebugString(), tev);
    }
  }
}

BOOST_AUTO_TEST_CASE(testFromJson)
{
  {
    TestParser tp;

    json tv1 = json::parse(
        R"({ "Subnets": [] })"
      );
    tp.fromJson(tv1);
    auto trv1 = tp.getData()[0].subnets;
    BOOST_TEST(0 == trv1.size());
  }
}
