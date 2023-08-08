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
    using Parser::processVpcPeeringConnection;
    using Parser::processAccepter;
    using Parser::processRequester;
    using Parser::processCidrBlockSets;
};

BOOST_AUTO_TEST_CASE(testProcessVpcPeeringConnections)
{
  {
    TestParser tp;

    json tv1 = json::parse(R"(
        { "VpcPeeringConnections":
          [ { "VpcPeeringConnectionId": ""
            , "Status":
              { "Code": ""
              , "Message": ""
              }
            }
          ]
        }
        )"
      );
    tp.processVpcPeeringConnection(tv1);
    auto tobj = tp.getData()[0].pcxs;
    BOOST_TEST(1 == tobj.size());
    nmdoa::VpcPeeringConnection tev1;
    BOOST_TEST(tev1 == tobj[0]);
  }
  {
    TestParser tp;

    json tv1 = json::parse(R"(
          { "VpcPeeringConnectionId": "aB1-2c3"
          , "Status":
            { "Code": "aB1-2c3"
            , "Message": "aB1-2c3"
            }
          }
        )"
      );
    tp.processVpcPeeringConnection(tv1);
    auto tobj = tp.getData()[0].pcxs;
    const std::vector<std::string> tevs {
          R"([pcxId: aB1-2c3,)"
        , R"(statusCode: aB1-2c3,)"
        , R"(statusMessage: aB1-2c3,)"
      };
    for (const auto& tev : tevs) {
      nmdp::testInString(tobj[0].toDebugString(), tev);
    }
  }
}

BOOST_AUTO_TEST_CASE(testProcessAccepter)
{
  {
    TestParser tp;

    json tv1 = json::parse(R"(
        { "AccepterVpcInfo":
          { "OwnerId": "aB1-2c3"
          , "VpcId": "aB1-2c3"
          , "CidrBlock": "aB1-2c3"
          }
        }
        )"
      );
    nmdoa::VpcPeeringConnection tobj;
    tp.processAccepter(tv1, tobj);
    const std::vector<std::string> tevs {
          R"(accepter: [vpcId: aB1-2c3,)"
        , R"(ownerId: aB1-2c3,)"
        , R"(cidrBlocks: [[cidrBlock: aB1-2c3,)"
      };
    for (const auto& tev : tevs) {
      nmdp::testInString(tobj.toDebugString(), tev);
    }
  }
}

BOOST_AUTO_TEST_CASE(testProcessRequester)
{
  {
    TestParser tp;

    json tv1 = json::parse(R"(
        { "RequesterVpcInfo":
          { "OwnerId": "aB1-2c3"
          , "VpcId": "aB1-2c3"
          , "CidrBlock": "aB1-2c3"
          }
        }
        )"
      );
    nmdoa::VpcPeeringConnection tobj;
    tp.processRequester(tv1, tobj);
    const std::vector<std::string> tevs {
          R"(requester: [vpcId: aB1-2c3,)"
        , R"(ownerId: aB1-2c3,)"
        , R"(cidrBlocks: [[cidrBlock: aB1-2c3,)"
      };
    for (const auto& tev : tevs) {
      nmdp::testInString(tobj.toDebugString(), tev);
    }
  }
}

BOOST_AUTO_TEST_CASE(testProcessCidrBlockSets)
{
  {
    TestParser tp;

    json tv1 = json::parse(R"(
        { "CidrBlockSet":
          [ { "CidrBlock": "ipv4-aB1-2c3"
            }
          ]
        , "Ipv6CidrBlockSet":
          [ { "Ipv6CidrBlock": "ipv6-aB1-2c3"
            }
          ]
        }
        )"
      );
    nmdoa::Vpc tobj;
    tp.processCidrBlockSets(tv1, tobj);
    const std::vector<std::string> tevs {
          R"([cidrBlock: ipv4-aB1-2c3,)"
        , R"([cidrBlock: ipv6-aB1-2c3,)"
      };
    for (const auto& tev : tevs) {
      nmdp::testInString(tobj.toDebugString(), tev);
    }
  }
}

BOOST_AUTO_TEST_CASE(testFromJson)
{
  {
    TestParser tp;

    json tv1 = json::parse(
        R"({ "VpcPeeringConnections": [] })"
      );
    tp.fromJson(tv1);
    auto tobj = tp.getData()[0].pcxs;
    BOOST_TEST(0 == tobj.size());
  }
}
