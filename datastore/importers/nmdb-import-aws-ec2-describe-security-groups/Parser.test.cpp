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
    using Parser::getRule;
    using Parser::processPermissions;
    using Parser::processSecurityGroup;
    using Parser::fromJson;
};

BOOST_AUTO_TEST_CASE(testGetRule)
{
  {
    TestParser tp;

    json tv1 = json::parse(R"(
          { "FromPort": 15
          , "IpProtocol": "a1"
          , "IpRanges":
            [{ "CidrIp": "a2"
            , "Description": "a3"
            }]
          , "Ipv6Ranges":
            [{ "CidrIpv6": "a4"
            , "Description": "a5"
            }]
          , "PrefixListIds": []
          , "ToPort": 999
          , "UserIdGroupPairs": []
          }
        )"
      );
    auto trv1 = tp.getRule(tv1);
    const std::vector<std::string> tevs {
          R"(protocol: a1)"
        , R"(fromOrType: 15)"
        , R"(toOrCode: 999)"
        , R"(cidrBlock: a2,)"
        , R"(cidrBlock: a4,)"
      };
    for (const auto& tev : tevs) {
      nmdp::testInString(trv1.toDebugString(), tev);
    }
  }
  // TODO add tests for addNonCidr logic
  {
    TestParser tp;

    json tv1 = json::parse(R"(
          { "FromPort": -1
          , "IpProtocol": ""
          , "IpRanges": []
          , "Ipv6Ranges": []
          , "PrefixListIds":
            [{ "Description": ""
            , "PrefixListId": ""
            }]
          , "ToPort": -1
          , "UserIdGroupPairs": []
          }
        )"
      );
    auto trv1 = tp.getRule(tv1);
    const std::string tev1 {
        R"(details: [{"Description":"","PrefixListId":""}])"
      };
    nmdp::testInString(trv1.toDebugString(), tev1);
  }
  {
    TestParser tp;

    json tv1 = json::parse(R"(
          { "FromPort": -1
          , "IpProtocol": ""
          , "IpRanges": []
          , "Ipv6Ranges": []
          , "PrefixListIds": []
          , "ToPort": -1
          , "UserIdGroupPairs":
            [{ "Description": ""
            , "GroupId": ""
            , "GroupName": ""
            , "PeeringStatus": ""
            , "UserId": ""
            , "VpcId": ""
            , "VpcPeeringConnectionId": ""
            }]
          }
        )"
      );
    auto trv1 = tp.getRule(tv1);
    const std::string tev1 {
        R"(details: [{"Description":"","GroupId":"")"
        R"(,"GroupName":"","PeeringStatus":"","UserId":"")"
        R"(,"VpcId":"","VpcPeeringConnectionId":""}])"
      };
    nmdp::testInString(trv1.toDebugString(), tev1);
  }
}

BOOST_AUTO_TEST_CASE(testProcessPermissions)
{
  {
    TestParser tp;

    json tv1 = json::parse(R"(
        { "IpPermissions": []
        , "IpPermissionsEgress": []
        }
        )"
      );
    nmdoa::SecurityGroup tobj, tev1;
    tp.processPermissions(tv1, tobj);
    BOOST_TEST(tev1 == tobj);
  }
  {
    TestParser tp;

    json tv1 = json::parse(R"(
        { "IpPermissions":
          [{ "FromPort": -2147483648
          , "IpProtocol": ""
          , "IpRanges": []
          , "Ipv6Ranges": []
          , "PrefixListIds": []
          , "ToPort": -2147483648
          , "UserIdGroupPairs": []
          }]
        , "IpPermissionsEgress":
          [{ "FromPort": -2147483648
          , "IpProtocol": ""
          , "IpRanges": []
          , "Ipv6Ranges": []
          , "PrefixListIds": []
          , "ToPort": -2147483648
          , "UserIdGroupPairs": []
          }]
        }
        )"
      );
    nmdoa::SecurityGroup tobj, tev1;
    tp.processPermissions(tv1, tobj);

    nmdoa::SecurityGroupRule tev2;
    tev2.setEgress();
    tev1.addRule(tev2);

    BOOST_TEST(tev1 == tobj);
  }
}

BOOST_AUTO_TEST_CASE(testProcessSecurityGroup)
{
  {
    TestParser tp;

    json tv1 = json::parse(R"(
        { "Description": ""
        , "GroupId": ""
        , "GroupName": ""
        , "IpPermissions": []
        , "IpPermissionsEgress": []
        , "OwnerId": ""
        , "Tags": []
        , "VpcId": ""
        }
        )"
      );
    tp.processSecurityGroup(tv1);
    auto trv1 = tp.getData()[0].securityGroups;
    BOOST_TEST(1 == trv1.size());
    nmdoa::SecurityGroup tev1;
    BOOST_TEST(tev1 == trv1[0]);
  }
  {
    TestParser tp;

    json tv1 = json::parse(R"(
        { "Description": "a"
        , "GroupId": "b"
        , "GroupName": "c"
        , "IpPermissions": []
        , "IpPermissionsEgress": []
        , "OwnerId": ""
        , "Tags": []
        , "VpcId": "d"
        }
        )"
      );
    tp.processSecurityGroup(tv1);
    auto trv1 = tp.getData()[0].securityGroups;
    BOOST_TEST(1 == trv1.size());
    const std::string tev1 {
        R"(sgId: b, name: c, description: a, vpcId: d, rules: [])"
      };
    nmdp::testInString(trv1[0].toDebugString(), tev1);
  }
}

BOOST_AUTO_TEST_CASE(testFromJson)
{
  {
    TestParser tp;

    json tv1 = json::parse(
        R"({ "SecurityGroups": [] })"
      );
    tp.fromJson(tv1);
    auto trv1 = tp.getData()[0].securityGroups;
    BOOST_TEST(0 == trv1.size());
  }
}
