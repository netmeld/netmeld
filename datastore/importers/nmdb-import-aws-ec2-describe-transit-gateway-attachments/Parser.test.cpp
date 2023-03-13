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
    using Parser::processTransitGatewayAttachments;
};

BOOST_AUTO_TEST_CASE(testProcessTransitGatewayAttachments)
{
  {
    TestParser tp;
    json tv1 = json::parse(R"(
        { "TransitGatewayAttachments": [
            { "TransitGatewayAttachmentId": ""
            , "TransitGatewayId": ""
            , "TransitGatewayOwnerId": ""
            , "ResourceOwnerId": ""
            , "ResourceType": ""
            , "ResourceId": ""
            , "State": ""
            , "Association": {
                "State": ""
              }
            }
          ]
        }
        )"
      );
    nmdoa::TransitGatewayAttachment tev1;
    tp.processTransitGatewayAttachments(tv1);
    auto tobj = tp.getData()[0].tgwas;
    BOOST_TEST(1 == tobj.size());
    BOOST_TEST(tev1 == tobj[0]);
  }
  {
    TestParser tp;
    json tv1 = json::parse(R"(
        { "TransitGatewayAttachments": [
            { "TransitGatewayAttachmentId": "aB1-2c3"
            , "TransitGatewayId": "aB1-2c3"
            , "TransitGatewayOwnerId": "aB1-2c3"
            , "ResourceOwnerId": "aB1-2c3"
            , "ResourceType": "aB1-2c3"
            , "ResourceId": "aB1-2c3"
            , "State": "aB1-2c3"
            , "Association": {
                "State": "aB1-2c3"
              }
            }
          ]
        }
        )"
      );
    tp.processTransitGatewayAttachments(tv1);
    auto tobj = tp.getData()[0].tgwas;
    const std::vector<std::string> tevs {
        R"([tgwAttachmentId: aB1-2c3,)"
      , R"(tgwId: aB1-2c3,)"
      , R"(tgwOwnerId: aB1-2c3,)"
      , R"(resourceId: aB1-2c3,)"
      , R"(resourceOwnerId: aB1-2c3,)"
      , R"(resourceType: aB1-2c3,)"
      , R"(state: aB1-2c3,)"
      , R"(associationState: aB1-2c3])"
      };
    for (const auto& tev : tevs) {
      nmdp::testInString(tobj[0].toDebugString(), tev);
    }
  }
}


BOOST_AUTO_TEST_CASE(testFromJson)
{
  {
    TestParser tp;

    json tv1 = json::parse(
        R"({ "TransitGatewayAttachments": [] })"
      );
    tp.fromJson(tv1);
    auto tobj = tp.getData()[0].tgwas;
    BOOST_TEST(0 == tobj.size());
  }
}
