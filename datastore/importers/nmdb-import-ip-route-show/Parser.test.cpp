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
#include <netmeld/datastore/objects/Route.hpp>

#include "Parser.hpp"


namespace nmdo = netmeld::datastore::objects;
namespace nmdp = netmeld::datastore::parsers;

using qi::ascii::blank;


class TestParser : public Parser
{
  public:
    using Parser::IP_REASON;
    // parts
    using Parser::dstIpNet;
    using Parser::nextHopIp;
    using Parser::ifaceName;
    // whole
    using Parser::defaultRoute;
    using Parser::route;
    using Parser::nullRoute;
};

BOOST_AUTO_TEST_CASE(testParts)
{
  {
    TestParser tp;
    const auto& parserRule {tp.ifaceName};
    // OK
    std::vector<std::tuple<std::string, std::string>> testsOk {
          {"dev lo", "lo"}
        , {"dev dummy0", "dummy0"}
        , {"dev special-_.@chars4", "special-_.@chars4"}
      };
    for (const auto& [test, expected] : testsOk) {
      std::string out;
      BOOST_TEST(nmdp::testAttr(test.c_str(), parserRule, out, blank)
                , "Parse rule 'ifaceName': " << test
                );
      BOOST_TEST(expected == out);
    }
    // Bad
    std::vector<std::string> testsBad {
          {"lo"}
        , {"devlo"}
      };
    for (const auto& test : testsBad) {
      std::string out;
      BOOST_TEST(!nmdp::testAttr(test.c_str(), parserRule, out, blank)
                , "Parse rule 'ifaceName': " << test
                );
    }
  }

  {
    TestParser tp;
    const auto& parserRule {tp.dstIpNet};
    // OK
    std::vector<std::tuple<std::string, nmdo::IpAddress>> testsOk {
          {"default", nmdo::IpAddress("0.0.0.0/0")}
        , {"1.2.3.4/5", nmdo::IpAddress("1.2.3.4/5")}
        , {"1::2/34", nmdo::IpAddress("1::2/34")}
      };
    for (auto& [test, expected] : testsOk) {
      nmdo::IpAddress out;
      BOOST_TEST(nmdp::testAttr(test.c_str(), parserRule, out, blank)
                , "Parser rule 'dstIpNet': " << test
                );
      expected.setReason(tp.IP_REASON); // parser sets reason
      BOOST_TEST(expected == out);
    }
  }

  {
    TestParser tp;
    const auto& parserRule {tp.nextHopIp};
    // OK
    std::vector<std::tuple<std::string, nmdo::IpAddress>> testsOk {
          {"1.2.3.4/5", nmdo::IpAddress("1.2.3.4/5")}
        , {"1::2/34"  , nmdo::IpAddress("1::2/34")}
      };
    for (auto& [test, expected] : testsOk) {
      nmdo::IpAddress out;
      BOOST_TEST(nmdp::testAttr(test.c_str(), parserRule, out, blank)
                , "Parser rule 'dstIpNet': " << test
                );
      expected.setReason(tp.IP_REASON); // parser sets reason
      BOOST_TEST(expected == out);
    }
  }
}

BOOST_AUTO_TEST_CASE(testWhole)
{
  {
    TestParser tp;
    const std::string desc {tp.IP_REASON};
    const auto& parserRule {tp.nullRoute};
    // OK
    std::vector<std::tuple<std::string, nmdo::IpAddress>> testsOk {
          {"unreachable 1.2.3.4\n"  , nmdo::IpAddress("1.2.3.4/32", desc)}
        , {"blackhole 1.2.3.4/24\n" , nmdo::IpAddress("1.2.3.4/24", desc)}
        , {"prohibit 1.2.3.4/16\n"  , nmdo::IpAddress("1.2.3.4/16", desc)}
        , {"prohibit 1.2.3.0/4\n"   , nmdo::IpAddress("1.2.3.0/4" , desc)}
        , {"unreachable 1::2/64\n"  , nmdo::IpAddress("1::2/64", desc)}
        , {"blackhole 1::2/64\n"    , nmdo::IpAddress("1::2/64", desc)}
        , {"prohibit 1::2/64\n"     , nmdo::IpAddress("1::2/64", desc)}
      };

    nmdo::Route expected;
    expected.setNullRoute(true);
    for (const auto& [test, dstNet] : testsOk) {
      expected.setDstIpNet(dstNet);
      nmdo::Route out;
      BOOST_TEST(nmdp::testAttr(test.c_str(), parserRule, out, blank)
                , "Parser rule 'nullRoute': " << test
                );
      BOOST_TEST(out.getNextHopIpAddrString().empty());
      BOOST_TEST(expected == out);
    }
  }

  {
    TestParser tp;
    const auto& parserRule {tp.defaultRoute};
    { // v4
      std::vector<std::string> testsOkv4 {
            "default via 1.2.3.4 dev eth0\n"
          , "default via 1.2.3.4/32 dev eth0 metric 1 mtu 1 advmss 1\n"
        };

      nmdo::IpAddress nextHop {"1.2.3.4/32"};
      nextHop.setReason(tp.IP_REASON);
      nmdo::IpAddress dstIpNet {"0.0.0.0/0"};
      dstIpNet.setReason(tp.IP_REASON);
      nmdo::Route expected;
      expected.setDstIpNet(dstIpNet);
      expected.setNextHopIpAddr(nextHop);
      expected.setIfaceName("eth0");
      for (const auto& test : testsOkv4) {
        nmdo::Route out;
        BOOST_TEST(nmdp::testAttr(test.c_str(), parserRule, out, blank)
                  , "Parser rule 'defaultRoute': " << test
                  );
        BOOST_TEST(expected == out);
      }
    }
    { // v6
      std::vector<std::string> testsOkv6 {
            "default via 1::2 dev eth0 metric 1 mtu 1 advmss 1\n"
          , "default via 1::2 dev eth0 proto static metric 1 mtu 1 advmss 1\n"
          , "default via 1::2 dev eth0 proto kernel metric 1 mtu 1 advmss 1\n"
        };

      nmdo::IpAddress nextHop {"1::2/128"};
      nextHop.setReason(tp.IP_REASON);
      nmdo::IpAddress dstIpNet {"::/0"};
      dstIpNet.setReason(tp.IP_REASON);
      nmdo::Route expected;
      expected.setDstIpNet(dstIpNet);
      expected.setNextHopIpAddr(nextHop);
      expected.setIfaceName("eth0");
      for (const auto& test : testsOkv6) {
        nmdo::Route out;
        BOOST_TEST(nmdp::testAttr(test.c_str(), parserRule, out, blank)
                  , "Parser rule 'defaultRoute': " << test
                  );
        BOOST_TEST(expected == out);
      }
    }
  }

  {
    TestParser tp;
    const auto& parserRule {tp.route};
    { // v4
      std::vector<std::string> testsOkv4 {
            "1.2.3.0/24 dev eth0 proto kernel scope link src 1.2.3.4\n"
          , "1.2.3.0/24 dev eth0 proto kernel scope link src 1.2.3.4 linkdown\n"
        };

      nmdo::IpAddress dstIp {"1.2.3.0/24"};
      dstIp.setReason(tp.IP_REASON);
      nmdo::IpAddress nextHop {"1.2.3.4/32"};
      nextHop.setReason(tp.IP_REASON);
      nmdo::Route expected;
      expected.setDstIpNet(dstIp);
      expected.setNextHopIpAddr(nextHop);
      expected.setIfaceName("eth0");
      for (const auto& test : testsOkv4) {
        nmdo::Route out;
        BOOST_TEST(nmdp::testAttr(test.c_str(), parserRule, out, blank)
                  , "Parser rule 'defaultRoute': " << test
                  );
        BOOST_TEST(expected == out);
      }
    }
    { // v6
      std::vector<std::string> testsOkv6 {
            "1::2/64 dev eth0 metric 256 mtu 1500 advmss 1440\n"
          , "1::2/64 dev eth0 proto kernel metric 256 mtu 1500 advmss 1440\n"
        };

      //nmdo::IpAddress nextHop {"1::2/64"};
      //nextHop.setReason(tp.IP_REASON);
      nmdo::IpAddress dstIpNet {"1::2/64"};
      dstIpNet.setReason(tp.IP_REASON);
      nmdo::Route expected;
      expected.setDstIpNet(dstIpNet);
      //expected.setNextHopIpAddr(nextHop);
      expected.setIfaceName("eth0");
      for (const auto& test : testsOkv6) {
        nmdo::Route out;
        BOOST_TEST(nmdp::testAttr(test.c_str(), parserRule, out, blank)
                  , "Parser rule 'defaultRoute': " << test
                  );
        BOOST_TEST(expected == out);
      }
    }
  }
}
