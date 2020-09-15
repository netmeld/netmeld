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

#include <netmeld/datastore/parsers/ParserTestHelper.hpp>

#include "CiscoNetworkBook.hpp"

namespace nmdo = netmeld::datastore::objects;
namespace nmdp = netmeld::datastore::parsers;
namespace nmdsic = netmeld::datastore::importers::cisco;

using qi::ascii::blank;
using nmdsic::NetworkBook;

class TestCiscoNetworkBook : public nmdsic::CiscoNetworkBook {
  public:
    using nmdsic::CiscoNetworkBook::curBook;
};

BOOST_AUTO_TEST_CASE(testCiscoNetworkBookRules)
{
  {
    TestCiscoNetworkBook tcnb;
    const auto& parserRule = tcnb.bookName;
    // OK
    std::vector<std::tuple<std::string, std::string>> testsOk {
      // {test, expected format}
      {"some-name", "some-name"},
      {" Other_Name ", "Other_Name"},
    };
    for (const auto& [test, format] : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank));
      const auto& temp = tcnb.curBook;
      BOOST_TEST(format == temp.getName());
    }
    // BAD
    // -- The parser is a 'token', so no checkable bad case?
  }

  {
    TestCiscoNetworkBook tcnb;
    const auto& parserRule = tcnb.description;
    // OK
    std::vector<std::string> testsOk {
      // {test, expected format}
      {"description some descript text\n"},
      {" description some \n"},
    };
    for (const auto& test : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank));
    }
    // BAD
    // -- The parser is a 'tokens', so no checkable bad case?
  }

  {
    TestCiscoNetworkBook tcnb;
    const auto& parserRule = tcnb.objectNetworkHostLine;
    // OK
    std::vector<std::tuple<std::string, std::string>> testsOk {
      // {test, expected format}
      {"host 1.2.3.4\n", "1.2.3.4/32"},
      {" host some.fqdn \n", "some.fqdn"},
    };
    for (const auto& [test, format] : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank));
      const auto& temp = tcnb.curBook;
      BOOST_TEST(1 == temp.getData().count(format));
    }
    // BAD
  }

  {
    TestCiscoNetworkBook tcnb;
    const auto& parserRule = tcnb.objectNetworkSubnetLine;
    // OK
    std::vector<std::tuple<std::string, std::string>> testsOk {
      // {test, expected format}
      {"subnet 1.2.3.4 0.0.0.255\n", "1.2.3.4/24"},
      {" subnet 1.2.3.4/31 \n", "1.2.3.4/31"},
    };
    for (const auto& [test, format] : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank));
      const auto& temp = tcnb.curBook;
      BOOST_TEST(1 == temp.getData().count(format));
    }
    // BAD
  }

  {
    TestCiscoNetworkBook tcnb;
    const auto& parserRule = tcnb.objectNetworkRangeLine;
    // OK
    std::vector<std::tuple<std::string, std::string>> testsOk {
      // {test, expected format}
      {"range 1.2.3.4 4.3.2.1\n", "1.2.3.4/32-4.3.2.1/32"},
    };
    for (const auto& [test, format] : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank));
      const auto& temp = tcnb.curBook;
      BOOST_TEST(1 == temp.getData().count(format));
    }
    // BAD
  }

  {
    TestCiscoNetworkBook tcnb;
    const auto& parserRule = tcnb.objectNetworkNatLine;
    // OK
    std::vector<std::string> testsOk {
      // {test, expected format}
      {"nat (not,yet) dynamic valid\n"},
    };
    for (const auto& test : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank));
    }
    // BAD
  }

  {
    TestCiscoNetworkBook tcnb;
    const auto& parserRule = tcnb.objectNetworkFqdnLine;
    // OK
    std::vector<std::tuple<std::string, std::string>> testsOk {
      // {test, expected format}
      {"fqdn some.domain\n", "some.domain"},
      {" fqdn v4 some.v4.domain \n", "some.v4.domain"},
      {"fqdn v6 some.v6.domain\n", "some.v6.domain"},
    };
    for (const auto& [test, format] : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank));
      const auto& temp = tcnb.curBook;
      BOOST_TEST(1 == temp.getData().count(format));
    }
    // BAD
  }

  {
    TestCiscoNetworkBook tcnb;
    const auto& parserRule = tcnb.networkObjectLine;
    // OK
    std::vector<std::tuple<std::string, std::string>> testsOk {
      // {test, expected format}
      {"network-object object Some_Object\n", "Some_Object"},
      {" network-object host 1.2.3.4 \n", "1.2.3.4/32"},
//      {"network-object SomeNetworkObject 0.0.0.255\n", ""},
      {"network-object 1.2.3.4 0.0.0.255\n", "1.2.3.4/24"},
      {"network-object 1.2.3.4/31\n", "1.2.3.4/31"},
    };
    for (const auto& [test, format] : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank));
      const auto& temp = tcnb.curBook;
      BOOST_TEST(1 == temp.getData().count(format));
    }
    // BAD
  }

  {
    TestCiscoNetworkBook tcnb;
    const auto& parserRule = tcnb.groupObjectLine;
    // OK
    std::vector<std::tuple<std::string, std::string>> testsOk {
      // {test, expected format}
      {"group-object Some_Object\n", "Some_Object"},
      {" group-object Some_Object \n", "Some_Object"},
    };
    for (const auto& [test, format] : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank));
      const auto& temp = tcnb.curBook;
      BOOST_TEST(1 == temp.getData().count(format));
    }
    // BAD
  }
}

/*
See: https://www.cisco.com/c/en/us/td/docs/security/asa/asa-command-reference/I-R/cmdref2/n.html
---
   name IP LIST [description TEXT]

   - IP   -- host address
   - LIST -- [-_a-zA-Z0-9]{0,63} (but we don't care about validating it)

*/
BOOST_AUTO_TEST_CASE(testCiscoNetworkBookName)
{
  TestCiscoNetworkBook tcnb;
  {
    const auto& parserRule = tcnb.nameLine;
    // OK
    std::vector<std::string> testsOk {
      // {test}
      {"name 1.2.3.4 SomeName description some descript text\n"},
      {"name 1.2.3.4 SomeName\n"},
    };
    for (const auto& test : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank));
      const auto& temp = tcnb.curBook;
      BOOST_TEST("SomeName" == temp.getName());
      const auto& data = temp.getData();
      BOOST_TEST(1 == data.size());
      BOOST_TEST("1.2.3.4/32" == *(data.begin()));
    }
    // BAD
  }
}

/*
See: https://www.cisco.com/c/en/us/td/docs/security/asa/asa-command-reference/I-R/cmdref2/o.html
  - nat [\(R_IFACE,M_IFACE\)] dynamic \
    ( M_IP [interface [ipv6]] | [M_OBJ] \
    [pat-pool M_OBJ [round-robin] [extended] [flat [include-reserve]] [block-allocation]] \
    [interface [ipv6]]) [dns]
  - nat [\(R_IFACE,M_IFACE\)] static \
    ( M_IP | M_OBJ | interface [ipv6] ) \
    [net-to-net] [dns|service ( tcp | udp | sctp ) R_PORT M_PORT] \
    [no-proxy-arp] [route-lookup]
---    
   object network LIST
    description TEXT
    fqdn [v4|v6] D.OMAIN-NAME.0
    host IP
    nat IGNORE
    range IP IP
    subnet ( IP MASK | IP/PREFIX )

*/
BOOST_AUTO_TEST_CASE(testCiscoNetworkBookObjectNetwork)
{
  TestCiscoNetworkBook tcnb;
  {
    const auto& parserRule = tcnb.objectNetwork;
    // OK
    std::string fullText {
      "object network TEST\n"
      "  \n"
      "  description some descript text\n"
      "  fqdn v4 some.00.domain.name\n" // +1
      "  fqdn v6 some.00.domain.name\n"
      "  fqdn final.1\n" // +1
      "  host 1.2.3.4\n" // +1
      "  nat (too,complex) dynamic ignore\n"
      "  range 1.2.3.4 4.3.2.1\n" // +1
      "  subnet 1.2.3.4 0.0.0.255\n" // +1
      "  subnet 1.2.3.4/31\n" // +1
    };
    BOOST_TEST(nmdp::test(fullText.c_str(), parserRule, blank));
    const auto& temp = tcnb.curBook;
    BOOST_TEST("TEST" == temp.getName());
    BOOST_TEST(6 == temp.getData().size());
  }
}

/*
*/
BOOST_AUTO_TEST_CASE(testCiscoNetworkBookObjectGroupNetwork)
{
  TestCiscoNetworkBook tcnb;
  {
    const auto& parserRule = tcnb.objectGroupNetwork;
    // OK
    std::string fullText {
      "object-group network TEST\n"
      "  description some descript text\n"
      "  group-object Group_Object\n" // +1
      "  network-object object Network_Object\n" // +1
      "  network-object host 1.2.3.4\n" // +1
      "  network-object 1.2.3.4 0.0.0.255\n" // +1
      "  network-object 1.2.3.4/31\n" // +1
    };
    BOOST_TEST(nmdp::test(fullText.c_str(), parserRule, blank));
    const auto& temp = tcnb.curBook;
    BOOST_TEST("TEST" == temp.getName());
    BOOST_TEST(5 == temp.getData().size());
  }
}
