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
    using Parser::processEntries;
    using Parser::processNetworkAcl;
    using Parser::fromJson;
};

BOOST_AUTO_TEST_CASE(testProcessEntries)
{
  {
    TestParser tp;

    json tv1 = json::parse(
        R"({"Entries": []})"
      );
    nmdoa::NetworkAcl tobj, tev1;
    tp.processEntries(tv1, tobj);
    BOOST_TEST(tev1 == tobj);
  }
  {
    TestParser tp;

    json tv1 = json::parse(R"(
        {"Entries":
          [{ "CidrBlock": ""
          , "Egress": false
          , "IcmpTypeCode": {"Code": -1, "Type": -1}
          , "Ipv6CidrBlock": ""
          , "PortRange": {"From": -1, "To": -1}
          , "Protocol": ""
          , "RuleAction": ""
          , "RuleNumber": -1
          }]
        }
        )"
      );
    nmdoa::NetworkAcl tobj;
    tp.processEntries(tv1, tobj);
    const std::vector<std::string> tevs {
          R"(number: -1)"
        , R"(portRange: 1)"
        , R"(typeCode: 1)"
        , R"(fromOrType: -1)"
        , R"(toOrCode: -1)"
      };
    for (const auto& tev : tevs) {
      nmdp::testInString(tobj.toDebugString(), tev);
    }
  }
  {
    TestParser tp;

    json tv1 = json::parse(R"(
        {"Entries":
          [{ "CidrBlock": ""
          , "Egress": false
          , "Ipv6CidrBlock": ""
          , "PortRange": {"From": -1, "To": -1}
          , "Protocol": ""
          , "RuleAction": ""
          , "RuleNumber": -1
          }]
        }
        )"
      );
    nmdoa::NetworkAcl tobj;
    tp.processEntries(tv1, tobj);
    const std::vector<std::string> tevs {
          R"(number: -1)"
        , R"(portRange: 1)"
        , R"(typeCode: 0)"
        , R"(fromOrType: -1)"
        , R"(toOrCode: -1)"
      };
    for (const auto& tev : tevs) {
      nmdp::testInString(tobj.toDebugString(), tev);
    }
  }
  {
    TestParser tp;

    json tv1 = json::parse(R"(
        {"Entries":
          [{ "CidrBlock": ""
          , "Egress": false
          , "IcmpTypeCode": {"Code": -1, "Type": -1}
          , "Ipv6CidrBlock": ""
          , "Protocol": ""
          , "RuleAction": ""
          , "RuleNumber": -1
          }]
        }
        )"
      );
    nmdoa::NetworkAcl tobj;
    tp.processEntries(tv1, tobj);
    const std::vector<std::string> tevs {
          R"(number: -1)"
        , R"(portRange: 0)"
        , R"(typeCode: 1)"
        , R"(fromOrType: -1)"
        , R"(toOrCode: -1)"
      };
    for (const auto& tev : tevs) {
      nmdp::testInString(tobj.toDebugString(), tev);
    }
  }
  {
    TestParser tp;

    json tv1 = json::parse(R"(
        {"Entries":
          [{ "CidrBlock": "1.2.3.4/24"
          , "Egress": true
          , "IcmpTypeCode": {"Code": 123, "Type": 123}
          , "Ipv6CidrBlock": "1::2/24"
          , "PortRange": {"From": 123, "To": 123}
          , "Protocol": "aB c-1 @3"
          , "RuleAction": "aB c-1 @3"
          , "RuleNumber": 123
          }]
        }
        )"
      );
    nmdoa::NetworkAcl tobj;
    tp.processEntries(tv1, tobj);
    const std::vector<std::string> tevs {
          R"(number: 123,)"
        , R"(action: aB c-1 @3,)"
        , R"(protocol: aB c-1 @3,)"
        , R"(portRange: 1,)"
        , R"(typeCode: 1,)"
        , R"(fromOrType: 123,)"
        , R"(toOrCode: 123,)"
        , R"(egress: 1,)"
        , R"(cidrBlock: 1.2.3.4/24,)"
        , R"(cidrBlock: 1::2/24,)"
      };
    for (const auto& tev : tevs) {
      nmdp::testInString(tobj.toDebugString(), tev);
    }
  }
}

BOOST_AUTO_TEST_CASE(testProcessNetworkAcl)
{
  {
    TestParser tp;

    json tv1 = json::parse(R"(
          { "Associations": []
          , "Entries": []
          , "IsDefault": false
          , "NetworkAclId": ""
          , "OwnerId": ""
          , "Tags": []
          , "VpcId": ""
          }
        )"
      );
    tp.processNetworkAcl(tv1);
    BOOST_TEST(0 == tp.getData().size());
  }
  {
    TestParser tp;

    json tv1 = json::parse(R"(
          { "Associations":
            [{ "NetworkAclAssociationId": ""
             , "NetworkAclId": ""
             , "SubnetId":"a1"
            }]
          , "Entries": []
          , "IsDefault": false
          , "NetworkAclId": "a2"
          , "OwnerId": ""
          , "Tags": []
          , "VpcId": "a3"
          }
        )"
      );
    tp.processNetworkAcl(tv1);
    BOOST_TEST(1 == tp.getData().size());
    auto tobj = tp.getData()[0].networkAcls;
    BOOST_TEST(1 == tobj.size());
    const std::string tev1 {
        R"(naclId: a2, vpcId: a3, subnetIds: [a1], rules: []])"
      };
    nmdp::testInString(tobj[0].toDebugString(), tev1);
  }
}

BOOST_AUTO_TEST_CASE(testFromJson)
{
  {
    TestParser tp;

    json tv1 = json::parse(
        R"({ "NetworkAcls": [] })"
      );
    tp.fromJson(tv1);
    BOOST_TEST(0 == tp.getData().size());
  }
}
