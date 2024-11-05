// =============================================================================
// Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC
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

namespace nmdp = netmeld::datastore::parsers;

using qi::ascii::blank;

class TestParser : public Parser {
  public:
    using Parser::start;

    using Parser::vrfLine1;
    using Parser::vrfLine2;
};

BOOST_AUTO_TEST_CASE(testParts)
{
  TestParser tp;

  { // vrfLine1
    const auto& parserRule {tp.vrfLine1};

    std::vector<std::tuple<std::string, std::string, std::vector<std::string>>>
      testsOk {
        {"Internet                         <not set>             ipv4        Gi0/0/2\nGi0/0/3\n"
        , "Internet"
        , { "gi0/0/2"
          , "gi0/0/3"
          }
        }
      , {"mgmt-intf                        12345:123             ipv4,ipv6   Gi0\n"
        , "mgmt-intf"
        , {"gi0"}
        }
      };

    for (const auto& [test, id, ifaces] : testsOk) {
      nmdo::Vrf out;
      BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
                , "Parse rule 'vrfLine1': " << test
                );
      nmdo::Vrf expected {id};
      for (const auto& iface : ifaces) {
        expected.addIface(iface);
      }
      BOOST_TEST(expected == out);
    }
  }

  { // vrfLine2
    const auto& parserRule {tp.vrfLine2};

    std::vector<std::tuple<std::string, std::string>>
      testsOk {
        { "Vrf1                                    3 Up      --\n"
        , "Vrf1"
        }
      , { "default                                 1 Up      --\n"
        , "default"
        }
      , { "mgmt                                    2 Up      --\n"
        , "mgmt"
        }
      , { "vrf2                                    4 Up      --\n"
        , "vrf2"
        }
      };

    for (const auto& [test, id] : testsOk) {
      nmdo::Vrf out;
      BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
                , "Parse rule 'vrfLine2': " << test
                );
      nmdo::Vrf expected {id};
      BOOST_TEST(expected == out);
    }
  }
}

BOOST_AUTO_TEST_CASE(testWholeVrfTable1)
{
  { // Format 1
    TestParser tp;
    const auto& parserRule {tp.start};

    std::string test {
      R"STR(  Name                             Default RD            Protocols   Interfaces
  Internet                         <not set>             ipv4        Gi0/0/2
                                                                     Gi0/0/3
  mgmt-intf                        12345:123             ipv4,ipv6   Gi0
      )STR"
      };

    Result out;
    BOOST_TEST(nmdp::testAttr(test.c_str(), parserRule, out, blank),
              "Parse rule 'testWholeVrfTable1': " << test);
    size_t conCount {2};
    BOOST_TEST(conCount == out.size());

    for (const auto& vrf : out) {
      BOOST_TEST(vrf.isValid());
    }
  }
}

BOOST_AUTO_TEST_CASE(testWholeVrfTable2)
{
  { // Format 2
    TestParser tp;
    const auto& parserRule {tp.start};

    std::string test {
      R"STR(VRF-Name                           VRF-ID State   Reason                       
Vrf1                                    3 Up      --                           
default                                 1 Up      --                           
mgmt                                    2 Up      --                           
vrf2                                    4 Up      --
      )STR"
      };

    Result out;
    BOOST_TEST(nmdp::testAttr(test.c_str(), parserRule, out, blank),
              "Parse rule 'testWholeVrfTable2': " << test);
    size_t conCount {4};
    BOOST_TEST(conCount == out.size());

    for (const auto& vrf : out) {
      BOOST_TEST(vrf.isValid());
    }
  }
}
