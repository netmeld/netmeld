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

#include <netmeld/datastore/parsers/ParserTestHelper.hpp>

#include "Parser.hpp"

namespace nmdo = netmeld::datastore::objects;
namespace nmdp = netmeld::datastore::parsers;

using qi::ascii::blank;

class TestParser : public Parser
{
  public:
    using Parser::processRoutes;
    using Parser::processAssociations;
    using Parser::processRouteTable;
    using Parser::fromJson;
};

BOOST_AUTO_TEST_CASE(testProcessRoutes)
{
  {
    TestParser tp;

    json tv1 = json::parse(
        R"({ "Routes": [] })"
      );
    nmdoa::RouteTable tobj, tev1;
    tp.processRoutes(tv1, tobj);
    BOOST_TEST(tev1 == tobj);
  }
  {
    TestParser tp;

    json tv1 = json::parse(R"(
        { "Routes":
          [{ "CarrierGatewayId": ""
          , "CoreNetworkArn": ""
          , "DestinationCidrBlock": ""
          , "DestinationIpv6CidrBlock": ""
          , "DestinationPrefixListId": ""
          , "EgressOnlyInternetGatewayId": ""
          , "GatewayId": ""
          , "InstanceId": ""
          , "InstanceOwnerId": ""
          , "LocalGatewayId": ""
          , "NatGatewayId": ""
          , "NetworkInterfaceId": ""
          , "Origin": ""
          , "State": ""
          , "TransitGatewayId": ""
          , "VpcPeeringConnectionId": ""
          }]
        }
        )"
      );
    nmdoa::RouteTable tobj, tev1;
    tp.processRoutes(tv1, tobj);
    BOOST_TEST(tev1 == tobj);
  }
  {
    TestParser tp;

    json tv1 = json::parse(R"(
        { "Routes":
          [{ "CarrierGatewayId": "ab11"
          , "CoreNetworkArn": "a2"
          , "DestinationCidrBlock": "1.2.3.4/24"
          , "DestinationIpv6CidrBlock": "1::2/24"
          , "DestinationPrefixListId": "a5"
          , "EgressOnlyInternetGatewayId": "a6"
          , "GatewayId": "a7"
          , "InstanceId": "a8"
          , "InstanceOwnerId": "a9"
          , "LocalGatewayId": "b1"
          , "NatGatewayId": "b2"
          , "NetworkInterfaceId": "b3"
          , "Origin": "b4"
          , "State": "b5"
          , "TransitGatewayId": "b6"
          , "VpcPeeringConnectionId": "b7"
          }]
        }
        )"
      );
    nmdoa::RouteTable tobj;
    tp.processRoutes(tv1, tobj);
    const std::vector<std::string> tevs {
          R"(typeId: ab11)"
        , R"(state: b5)"
        , R"(cidrBlock: 1.2.3.4/24,)"
        , R"(cidrBlock: 1::2/24,)"
        , R"(nonCidrBlocks: [a5])"
      };
    for (const auto& tev : tevs) {
      nmdp::testInString(tobj.toDebugString(), tev);
    }

  }
}

BOOST_AUTO_TEST_CASE(testProcessAssociations)
{
  {
    TestParser tp;

    json tv1 = json::parse(R"(
        { "Associations":
          [{ "AssociationState": { "State": "" , "StatusMessage": "" }
          , "GatewayId": ""
          , "Main": false
          , "RouteTableAssociationId": ""
          , "RouteTableId": ""
          , "SubnetId": ""
          }]
        }
        )"
      );
    nmdoa::RouteTable tobj, tev1;
    tp.processAssociations(tv1, tobj);
    BOOST_TEST(tev1 == tobj);
  }
  {
    TestParser tp;

    json tv1 = json::parse(R"(
        { "Associations":
          [{ "AssociationState":
            { "State": "aB c-1 @3"
            , "StatusMessage": ""
            }
          , "GatewayId": "aB c-1 @3"
          , "Main": false
          , "RouteTableAssociationId": ""
          , "RouteTableId": ""
          , "SubnetId": "aB c-1 @3"
          }]
        }
        )"
      );
    nmdoa::RouteTable tobj, tev1;
    tp.processAssociations(tv1, tobj);
    BOOST_TEST(tev1 == tobj);
  }
  {
    TestParser tp;

    json tv1 = json::parse(R"(
        { "Associations":
          [{ "AssociationState":
            { "State": "associated"
            , "StatusMessage": ""
            }
          , "GatewayId": "aB c-1 @3"
          , "Main": false
          , "RouteTableAssociationId": ""
          , "RouteTableId": ""
          , "SubnetId": "aB c-1 @3d"
          }]
        }
        )"
      );
    nmdoa::RouteTable tobj;
    tp.processAssociations(tv1, tobj);
    const std::string tev1 {
        R"(associations: [aB c-1 @3, aB c-1 @3d])"
      };
    nmdp::testInString(tobj.toDebugString(), tev1);
  }
}

BOOST_AUTO_TEST_CASE(testProcessRouteTable)
{
  {
    TestParser tp;

    json tv1 = json::parse(R"(
        { "Associations": []
        , "OwnerId": ""
        , "PropagatingVgws": []
        , "RouteTableId": ""
        , "Routes": []
        , "Tags": []
        , "VpcId": ""
        }
        )"
      );
    tp.processRouteTable(tv1);
    BOOST_TEST(0 == tp.getData().size());
  }
  {
    TestParser tp;

    json tv1 = json::parse(R"(
        { "Associations": []
        , "OwnerId": ""
        , "PropagatingVgws": []
        , "RouteTableId": "a1"
        , "Routes": []
        , "Tags": []
        , "VpcId": "b2"
        }
        )"
      );
    tp.processRouteTable(tv1);
    BOOST_TEST(1 == tp.getData().size());
    auto tobj = tp.getData()[0].routeTables;
    BOOST_TEST(1 == tobj.size());
    const std::string tev1 {
        R"(routeTableId: a1, vpcId: b2, associations: [], isDefault: 0, routes: [])"
      };
    nmdp::testInString(tobj[0].toDebugString(), tev1);
  }
}

BOOST_AUTO_TEST_CASE(testFromJson)
{
  {
    TestParser tp;

    json tv1 = json::parse(
        R"({ "RouteTables": [] })"
      );
    tp.fromJson(tv1);
    BOOST_TEST(0 == tp.getData().size());
  }
}
