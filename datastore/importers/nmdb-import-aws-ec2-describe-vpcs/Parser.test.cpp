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
    using Parser::processCidrBlockAssociationSet;
    using Parser::processVpcs;
    using Parser::fromJson;
};

BOOST_AUTO_TEST_CASE(testProcessCidrBlockAssociationSet)
{
  {
    TestParser tp;

    json tv1 = json::parse(R"(
        { "CidrBlockAssociationSet":
          [{ "AssociationId": ""
          , "CidrBlock": ""
          , "CidrBlockState":
            { "State": ""
            , "StatusMessage": ""
            }
          }]
        , "Ipv6CidrBlockAssociationSet":
          [{ "AssociationId": ""
          , "Ipv6CidrBlock": ""
          , "Ipv6CidrBlockState":
            { "State": ""
            , "StatusMessage": ""
            }
          , "Ipv6Pool": ""
          , "NetworkBorderGroup": ""
          }]
        }
        )"
      );
    nmdoa::Vpc tobj, tev1;
    tp.processCidrBlockAssociationSet(tv1, tobj);
    BOOST_TEST(tev1 == tobj);
  }
  {
    TestParser tp;

    json tv1 = json::parse(R"(
        { "CidrBlockAssociationSet":
          [{ "AssociationId": ""
          , "CidrBlock": "a1"
          , "CidrBlockState":
            { "State": "a2"
            , "StatusMessage": ""
            }
          }]
        , "Ipv6CidrBlockAssociationSet":
          [{ "AssociationId": ""
          , "Ipv6CidrBlock": "a3"
          , "Ipv6CidrBlockState":
            { "State": "a4"
            , "StatusMessage": ""
            }
          , "Ipv6Pool": ""
          , "NetworkBorderGroup": ""
          }]
        }
        )"
      );
    nmdoa::Vpc tobj, tev1;
    tp.processCidrBlockAssociationSet(tv1, tobj);
    const std::vector<std::string> tevs {
          R"(cidrBlock: a1,)"
        , R"(cidrBlock: a3,)"
        , R"(state: a2,)"
        , R"(state: a4,)"
      };
    for (const auto& tev : tevs) {
      nmdp::testInString(tobj.toDebugString(), tev);
    }
  }
}

BOOST_AUTO_TEST_CASE(testProcessVpcs)
{
  {
    TestParser tp;

    json tv1 = json::parse(R"(
        { "CidrBlock": ""
        , "CidrBlockAssociationSet": []
        , "DhcpOptionsId": ""
        , "InstanceTenancy": ""
        , "Ipv6CidrBlockAssociationSet": []
        , "IsDefault": false
        , "OwnerId": ""
        , "State": ""
        , "Tags": []
        , "VpcId": ""
        }
        )"
      );
    tp.processVpcs(tv1);
    auto trv1 = tp.getData()[0].vpcs;
    BOOST_TEST(1 == trv1.size());
    BOOST_TEST(!trv1[0].isValid());
  }
  {
    TestParser tp;

    json tv1 = json::parse(R"(
        { "CidrBlock": ""
        , "CidrBlockAssociationSet": []
        , "DhcpOptionsId": ""
        , "InstanceTenancy": ""
        , "Ipv6CidrBlockAssociationSet": []
        , "IsDefault": false
        , "OwnerId": ""
        , "State": "a1"
        , "Tags": []
        , "VpcId": "a2"
        }
        )"
      );
    tp.processVpcs(tv1);
    auto trv1 = tp.getData()[0].vpcs;
    BOOST_TEST(1 == trv1.size());
    BOOST_TEST(trv1[0].isValid());

    const std::vector<std::string> tevs {
          R"(vpcId: a2)"
        , R"(state: a1)"
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
        R"({ "Vpcs": [] })"
      );
    tp.fromJson(tv1);
    auto trv1 = tp.getData()[0].vpcs;
    BOOST_TEST(0 == trv1.size());
  }
}
