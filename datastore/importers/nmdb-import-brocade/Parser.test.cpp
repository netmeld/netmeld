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

class TestParser : public Parser
{
  public:
    using Parser::d;

    using Parser::start;
    using Parser::config;
    using Parser::bootIface;
};

// NOTE: This parser does very limited processing
// Primarily, this parser is towards Fabric OS, see:
// - http://h10032.www1.hp.com/ctg/Manual/c00657482.pdf
// - https://techdocs.broadcom.com/content/dam/broadcom/techdocs/us/en/pdf/fc-networking/software-fabric-os/fos-91x-command.pdf

BOOST_AUTO_TEST_CASE(testBootIface)
{
  TestParser tp;
  const auto& parserRule {tp.bootIface};

  std::string test {
    R"(boot.other1
      boot.ipa:1.2.3.4
      boot.mac:10:00:11:22:33:44:55:66
      boot.device:eth0/1
      boot.gateway.ipa:1.2.3.1
      boot.other2:abc
    )"
    };

  nmdo::InterfaceNetwork  out;
  BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
            , "Parse rule 'bootIface': " << test
            );
  BOOST_TEST(out.isValid());

  std::string dbgStr {out.toDebugString()};
  nmdp::testInString(dbgStr, "ipAddress: 1.2.3.4/32,");
  nmdp::testInString(dbgStr, "macAddress: 11:22:33:44:55:66,");
  nmdp::testInString(dbgStr, "name: eth0/1,");
}

BOOST_AUTO_TEST_CASE(testConfig)
{
  TestParser tp;
  const auto& parserRule {tp.config};

  std::string test {
    R"([Switch Configuration Begin : 0]
      SwitchName = abc.123
      [Possible Header, or Not]
      fc4.fcp.vendorId:BROCADE
      fc4.fcp.productId:FC Switch
      ts.clockServer:1.2.3.4
      ts.clockServerList:1.2.3.1;1.2.3.2;1.2.3.3
      [Boot Parameters]
      boot.ipa:1.2.3.4
      boot.gateway.ipa:1.2.3.1
    )"
    };

  std::string dbgStr;
  BOOST_TEST( nmdp::test(test.c_str(), parserRule, blank)
            , "Parse rule 'config': " << test
            );

  dbgStr = tp.d.devInfo.toDebugString();
  nmdp::testInString(dbgStr, "id: abc.123,");
  nmdp::testInString(dbgStr, "vendor: brocade,");
  nmdp::testInString(dbgStr, "type: fc switch,");

  BOOST_TEST(1 == tp.d.ifaces.size());

  BOOST_TEST_REQUIRE(4 == tp.d.services.size());
  for (const auto& srvc : tp.d.services) {
    dbgStr = srvc.toDebugString();
    nmdp::testInString(dbgStr, "serviceName: ntp,");
    nmdp::testInString(dbgStr, "dstAddress: [ipAddress: 1.2.3.");
    nmdp::testInString(dbgStr, "serviceReason: abc.123's config");
  }
}
