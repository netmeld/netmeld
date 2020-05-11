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

#include "CiscoServiceBook.hpp"

namespace nmco = netmeld::core::objects;
namespace nmcp = netmeld::core::parsers;
namespace nmdsic = netmeld::datastore::importers::cisco;

using qi::ascii::blank;
using nmdsic::ServiceBook;

class TestCiscoServiceBook : public nmdsic::CiscoServiceBook {

  public:

  public:
    const ServiceBook& getServiceBooks()
    { return serviceBooks; }
    const nmco::AcServiceBook& getCurBook()
    { return curBook; }
    const std::string& getZone()
    { return ZONE; }
//    const std::string& getRuleBookName()
//    { return ruleBookName; }
//    const nmdsic::RuleBook& getRuleBook()
//    { return ruleBook; }
//    size_t getCurRuleId()
//    { return curRuleId; }
    const std::string& getCurProtocol()
    { return curProtocol; }
    const std::string& getCurSrcPort()
    { return curSrcPort; }
    const std::string& getCurDstPort()
    { return curDstPort; }
//    const std::set<std::string>& getIgnoredRuleData()
//    { return ignoredRuleData; }

};

BOOST_AUTO_TEST_CASE(testCiscoServiceBookRules)
{
  {
    TestCiscoServiceBook tcnb;
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

/*
description TEXT
*/
  {
    TestCiscoServiceBook tcnb;
    const auto& parserRule = tcnb.description;
    // OK
    std::vector<std::string> testsOk {
      // {test, expected format}
      {"description some descript text\n"},
      {" description some \n"},
    };
    for (const auto& test : testsOk) {
      BOOST_TEST(nmcp::test(test.c_str(), parserRule, blank));
    }
    // BAD
    // -- The parser is a 'tokens', so no checkable bad case?
  }

/*
service (  PROTOCOL
         | (tcp|udp|sctp) [source OP NUM] [destination OP NUM]
         | (icmp|icmpv6) [TYPE [CODE] | MESSAGE]
        )
*/
  {
    TestCiscoServiceBook tcnb;
    const auto& parserRule = tcnb.objectServiceLine;
    // OK
    std::vector<std::tuple<std::string, std::string>> testsOk {
      // {test, expected format}
      {"service tcp\n", "tcp::"},
      {" service 1 \n", "1::"},
      {"service udp source neq 67\n", "udp:!67:"},
      {"service sctp destination range 123 456\n", "sctp::123-456"},
      {"service tcp source gt 123 destination eq 456\n", "tcp:>123:456"},
      {"service icmp 6 255\n", "icmp:6:255"},
      {"service icmp\n", "icmp::"},
      {"service icmpv6 255\n", "icmpv6:255:"},
      {"service icmp echo-reply\n", "icmp:echo-reply:"},
    };
    for (const auto& [test, format] : testsOk) {
      BOOST_TEST(nmcp::test(test.c_str(), parserRule, blank),
                 "parse: " << test);
      const auto& temp = tcnb.getCurBook();
      BOOST_TEST(1 == temp.getData().count(format),
                 "one of: " << format);
    }
    // BAD
  }
/*
port-object ( eq PORT | range START END )
*/
  {
    TestCiscoServiceBook tcnb;
    const auto& parserRule = tcnb.portObjectArgumentLine;
    // OK
    std::vector<std::tuple<std::string, std::string>> testsOk {
      // {test, expected format}
      {"port-object eq telnet\n", ":telnet:telnet"},
      {"port-object eq 10\n", ":10:10"},
      {"port-object range 1 9999\n", ":1-9999:1-9999"},
    };
    for (const auto& [test, format] : testsOk) {
      BOOST_TEST(nmcp::test(test.c_str(), parserRule, blank),
                 "parse: " << test);
      const auto& temp = tcnb.getCurBook();
      BOOST_TEST(1 == temp.getData().count(format),
                 "one of: " << format);
    }
    // BAD
  }

/*
service-object (  PROTOCOL
                | (tcp|udp|tcp-udp|sctp) [source OP NUM] [destination OP NUM]
                | (icmp|icmpv6) [TYPE [CODE]]
                | object LIST
               )
*/
  {
    TestCiscoServiceBook tcnb;
    const auto& parserRule = tcnb.serviceObjectLine;
    // OK
    std::vector<std::tuple<std::string, std::string>> testsOk {
      // {test, expected format}
      {"service-object tcp\n", "tcp::"},
      {" service-object 1 \n", "1::"},
      {"service-object udp source neq 67\n", "udp:!67:"},
      {"service-object sctp destination range 123 456\n", "sctp::123-456"},
      {"service-object tcp source lt 123 destination eq 456\n", "tcp:<123:456"},
      {"service-object icmp 6 255\n", "icmp:6:255"},
      {"service-object icmpv6 255 255\n", "icmpv6:255:255"},
      {"service-object object Some_Object\n", "Some_Object"},
    };
    for (const auto& [test, format] : testsOk) {
      BOOST_TEST(nmcp::test(test.c_str(), parserRule, blank),
                 "parse: " << test);
      const auto& temp = tcnb.getCurBook();
      BOOST_TEST(1 == temp.getData().count(format),
                 "one of: " << format);
    }
    // BAD
  }

/*
protocol-object PROTOCOL
*/
  {
    TestCiscoServiceBook tcnb;
    const auto& parserRule = tcnb.protocolObjectLine;
    // OK
    std::vector<std::tuple<std::string, std::string>> testsOk {
      // {test, expected format}
      {"protocol-object icmp\n", "icmp::"},
      {"protocol-object 138\n", "138::"},
    };
    for (const auto& [test, format] : testsOk) {
      BOOST_TEST(nmcp::test(test.c_str(), parserRule, blank),
                 "parse: " << test);
      const auto& temp = tcnb.getCurBook();
      BOOST_TEST(1 == temp.getData().count(format),
                 "one of: " << format);
    }
    // BAD
  }

/*
group-object LIST
*/
  {
    TestCiscoServiceBook tcnb;
    const auto& parserRule = tcnb.groupObjectLine;
    // OK
    std::vector<std::tuple<std::string, std::string>> testsOk {
      // {test, expected format}
      {"group-object Some_Object\n", "Some_Object"},
      {" group-object Some_Object \n", "Some_Object"},
    };
    for (const auto& [test, format] : testsOk) {
      BOOST_TEST(nmcp::test(test.c_str(), parserRule, blank),
                 "parse: " << test);
      const auto& temp = tcnb.getCurBook();
      BOOST_TEST(1 == temp.getData().count(format),
                 "one of: " << format);
    }
    // BAD
  }
}

/*
See: https://www.cisco.com/c/en/us/td/docs/security/asa/asa-command-reference/I-R/cmdref2/o.html
---    
   object service LIST
    description TEXT
    service SERVICE_LINE
*/
BOOST_AUTO_TEST_CASE(testCiscoServiceBookObjectService)
{
  TestCiscoServiceBook tcnb;
  {
    const auto& parserRule = tcnb.objectService;
    // OK
    std::string fullText {
      "object service TEST\n"
      "  description some descript text\n"
      "  service tcp source eq www destination eq ssh\n" // +1
    };
    BOOST_TEST(nmcp::test(fullText.c_str(), parserRule, blank));
    const auto& temp = tcnb.getCurBook();
    BOOST_TEST("TEST" == temp.getName());
    BOOST_TEST(1 == temp.getData().size());
  }
}

/*
See: https://www.cisco.com/c/en/us/td/docs/security/asa/asa-command-reference/I-R/cmdref2/o.html
---
  object-group service LIST (tcp|udp|tcp-udp)
    description TEXT
    service-object SERVICE_OBJECT_LINE
    group-object GROUP_OBJECT_LINE
    port-object PORT_OBJECT_LINE
*/
BOOST_AUTO_TEST_CASE(testCiscoServiceBookObjectGroupService)
{
  TestCiscoServiceBook tcnb;
  {
    const auto& parserRule = tcnb.objectGroupService;
    // OK
    std::string fullText {
      "object-group service TEST tcp-udp\n"
      "  description some descript text\n"
      "  service-object object Service_Object\n" // +1
      "  service-object tcp-udp\n" // +1
      "  service-object icmp 123 123\n" // +1
      "  group-object Group_Object\n" // +1
      "  port-object eq 123\n" // +1
    };
    BOOST_TEST(nmcp::test(fullText.c_str(), parserRule, blank));
    const auto& temp = tcnb.getCurBook();
    BOOST_TEST("TEST" == temp.getName());
    BOOST_TEST(5 == temp.getData().size());
  }
}

/*
See: https://www.cisco.com/c/en/us/td/docs/security/asa/asa-command-reference/I-R/cmdref2/o.html
---
  object-group protocol LIST
    description TEXT
    protocol-object PROTOCOL_OBJECT_LINE
    group-object GROUP_OBJECT_LINE
*/
BOOST_AUTO_TEST_CASE(testCiscoServiceBookObjectGroupProtocol)
{
  TestCiscoServiceBook tcnb;
  {
    const auto& parserRule = tcnb.objectGroupProtocol;
    // OK
    std::string fullText {
      "object-group protocol TEST\n"
      "  description some descript text\n"
      "  protocol-object tcp\n" // +1
      "  protocol-object tcp-udp\n" // +1
      "  group-object Group_Object\n" // +1
    };
    BOOST_TEST(nmcp::test(fullText.c_str(), parserRule, blank));
    const auto& temp = tcnb.getCurBook();
    BOOST_TEST("TEST" == temp.getName());
    BOOST_TEST(3 == temp.getData().size());
  }
}
