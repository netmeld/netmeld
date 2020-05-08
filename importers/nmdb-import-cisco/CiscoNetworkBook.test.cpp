// =============================================================================
// Copyright 2017 National Technology & Engineering Solutions of Sandia, LLC
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

#include <netmeld/core/parsers/ParserTestHelper.hpp>

#include "CiscoNetworkBook.hpp"

namespace nmco = netmeld::core::objects;
namespace nmcp = netmeld::core::parsers;
namespace nmdsic = netmeld::datastore::importers::cisco;

using qi::ascii::blank;
using nmdsic::NetworkBook;

class TestCiscoNetworkBook : public nmdsic::CiscoNetworkBook {

  public:

  public:
    const NetworkBook& getNetworkBooks()
    { return networkBooks; }
    const nmco::AcNetworkBook& getCurBook()
    { return curBook; }
    const std::string& getZone()
    { return ZONE; }
//    const std::string& getRuleBookName()
//    { return ruleBookName; }
//    const nmdsic::RuleBook& getRuleBook()
//    { return ruleBook; }
//    size_t getCurRuleId()
//    { return curRuleId; }
//    const std::string& getCurRuleProtocol()
//    { return curRuleProtocol; }
//    const std::string& getCurRuleSrcPort()
//    { return curRuleSrcPort; }
//    const std::string& getCurRuleDstPort()
//    { return curRuleDstPort; }
//    const std::set<std::string>& getIgnoredRuleData()
//    { return ignoredRuleData; }

};

BOOST_AUTO_TEST_CASE(testCiscoNetworkBookRules)
{
  TestCiscoNetworkBook tcnb;

  {
    const auto& parserRule = tcnb.bookName;
    // OK
    std::vector<std::tuple<std::string, std::string>> testsOk {
      // {test, expected format}
      {"some-name", "some-name"},
      {" Other_Name ", "Other_Name"},
    };
    for (const auto& [test, format] : testsOk) {
      BOOST_TEST(nmcp::test(test.c_str(), parserRule, blank));
      const auto& temp = tcnb.getCurBook();
      BOOST_TEST(format == temp.getName());
    }
    // BAD
    // -- The parser is a 'token', so no checkable bad case?
  }
}

/*
   name IP LIST [description TEXT]

   - IP   -- host address
   - LIST -- [-_a-zA-Z0-9]{0,63} (but we don't care about validating it)

See: https://www.cisco.com/c/en/us/td/docs/security/asa/asa-command-reference/I-R/cmdref2/n.html
*/
BOOST_AUTO_TEST_CASE(testCiscoNetworkBookName)
{
  TestCiscoNetworkBook tcnb;
  {
    const auto& parserRule = tcnb.nameLine;
    // OK
    std::vector<std::string> testsOk {
      // {test}
      {"name 1.2.3.4 SomeName description Some rando description\n"},
      {"name 1.2.3.4 SomeName description Some\n"},
      {"name 1.2.3.4 SomeName\n"},
    };
    for (const auto& test : testsOk) {
      BOOST_TEST(nmcp::test(test.c_str(), parserRule, blank));
      const auto& temp = tcnb.getCurBook();
      BOOST_TEST("SomeName" == temp.getName());
      const auto& data = temp.getData();
      BOOST_TEST(1 == data.size());
      BOOST_TEST("1.2.3.4/32" == *(data.begin()));
    }
    // BAD
  }
}

/*
   object network LIST
    description TEXT
    fqdn [v4|v6] D.OMAIN-NAME.0
    host IP
    nat IGNORE
    range IP IP
    subnet ( IP MASK | IP/PREFIX )

See: https://www.cisco.com/c/en/us/td/docs/security/asa/asa-command-reference/I-R/cmdref2/o.html
  - nat [\(R_IFACE,M_IFACE\)] dynamic \
    ( M_IP [interface [ipv6]] | [M_OBJ] \
    [pat-pool M_OBJ [round-robin] [extended] [flat [include-reserve]] [block-allocation]] \
    [interface [ipv6]]) [dns]
  - nat [\(R_IFACE,M_IFACE\)] static \
    ( M_IP | M_OBJ | interface [ipv6] ) \
    [net-to-net] [dns|service ( tcp | udp | sctp ) R_PORT M_PORT] \
    [no-proxy-arp] [route-lookup]
*/
BOOST_AUTO_TEST_CASE(testCiscoNetworkBookObjectNetwork)
{}

/*
*/
BOOST_AUTO_TEST_CASE(testCiscoNetworkBookObjectGroupNetwork)
{}
