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

namespace nmdo = netmeld::datastore::objects;
namespace nmdp = netmeld::datastore::parsers;

using qi::ascii::blank;

// =====
// Documenation:
// - parser says see RFC 1035, 5395, etc.
// - https://en.wikipedia.org/wiki/Dig_(command)
// =====
class TestParser : public Parser
{
  public:
    using Parser::questionSection;
    using Parser::responseSection;
    using Parser::serverFooter;
    using Parser::receivedFooter;
    using Parser::start;
};

BOOST_AUTO_TEST_CASE(testRuleQuestionSection)
{
  TestParser tp;
  const auto& parserRule {tp.questionSection};
  // tests questionSectionHeader as well
  // tests questionRecord as well

  std::string test {
      ";; QUESTION SECTION:\n"
      ";some.domain.        IN A\n\n"
    };

  nmco::DnsQuestion out, expected;
  expected.setFqdn("some.domain");
  expected.setClass("IN");
  expected.setType("A");
  BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
            , "Parser rule 'questionSection': " << test
            );
  BOOST_TEST(expected == out);
}

BOOST_AUTO_TEST_CASE(testRuleResponseSection)
{
  TestParser tp;
  const auto& parserRule {tp.responseSection};
  // tests responseSectionHeader as well
  // tests responseRecords as well
  // tests responseRecord as well

  {
    std::vector<std::string> testsOk {
        ";; ANSWER SECTION:\n"
        "some.domain.   1 IN A 1.2.3.4\n"
        "some.domain.   1 IN A 1.2.3.4\n\n"
      , ";; ADDITIONAL SECTION:\n"
        "some.domain.   1 IN A 1.2.3.4\n"
        "some.domain.   1 IN A 1.2.3.4\n\n"
      };

    for (const auto& test : testsOk) {
      DnsResponseSection out;
      BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
                , "Parser rule 'responseSection': " << test
                );
      BOOST_TEST((  "ANSWER" == out.first
                 || "ADDITIONAL" == out.first
                 )
                );
      
      nmco::DnsResponse expected;
      expected.setFqdn("some.domain");
      expected.setTtl(1);
      expected.setClass("IN");
      expected.setType("A");
      expected.setData("1.2.3.4");
      BOOST_TEST(2 == out.second.size());
      for (const auto& dnsResponse : out.second) {
        BOOST_TEST(expected == dnsResponse);
      }
    }
  }
  {
    std::string test {
        ";; AUTHORITY SECTION:\n"
        // name ttl class type mname rname serial refresh retry expire minimum
        "domain.   1 IN  SOA dns.domain. e\\.mail.domain. 1 2 3 4 5\n"
        "domain.   1 IN  SOA dns.domain. mail.domain. 1 2 3 4 5\n\n"
      };

    DnsResponseSection out;
    BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
              , "Parser rule 'responseSection': " << test
              );
    BOOST_TEST("AUTHORITY" == out.first);

    nmco::DnsResponse expected;
    expected.setFqdn("domain");
    expected.setTtl(1);
    expected.setClass("IN");
    expected.setType("SOA");
    expected.setData(R"(dns.domain. e\.mail.domain. 1 2 3 4 5)");
    BOOST_TEST(2 == out.second.size());
    for (const auto& dnsResponse : out.second) {
      BOOST_TEST(expected == dnsResponse);
      expected.setData(R"(dns.domain. mail.domain. 1 2 3 4 5)");
    }
  }
  {
    std::string test {
        ";; AUTHORITY SECTION:\n"
        ".   1 IN  SOA dns.domain. mail.domain. 1 2 3 4 5\n\n"
      };

    DnsResponseSection out;
    BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
              , "Parser rule 'responseSection': " << test
              );
    BOOST_TEST("AUTHORITY" == out.first);

    nmco::DnsResponse expected;
    expected.setFqdn("");
    expected.setTtl(1);
    expected.setClass("IN");
    expected.setType("SOA");
    expected.setData(R"(dns.domain. mail.domain. 1 2 3 4 5)");
    for (const auto& dnsResponse : out.second) {
      BOOST_TEST(expected == dnsResponse);
    }
  }
}

BOOST_AUTO_TEST_CASE(testRuleServerOrReceivedFooter)
{
  TestParser tp;
  {
    const auto& parserRule {tp.serverFooter};

    std::string test {
        ";; SERVER: 1.2.3.4#12(1.2.3.4) (UDP)\n"
      };

    nmdo::Port out, expected;
    expected.setIpAddr(nmdo::IpAddress("1.2.3.4"));
    expected.setPort(12);
    BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
              , "Parser rule 'serverFooter': " << test
              );
    BOOST_TEST(expected == out);
  }
  {
    const auto& parserRule {tp.receivedFooter};

    std::string test {
        ";; Received 12 bytes from 1.2.3.4#12(1.2.3.4) in 12 ms\n"
      };

    nmdo::Port out, expected;
    expected.setIpAddr(nmdo::IpAddress("1.2.3.4"));
    expected.setPort(12);
    BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
              , "Parser rule 'receivedFooter': " << test
              );
    BOOST_TEST(expected == out);
  }
}

BOOST_AUTO_TEST_CASE(testRuleWhole)
{
  TestParser tp;
  const auto& parserRule {tp.start};

  std::string test {R"(
; <<>> DiG 1.2.3-4-Linux <<>> a.b.c
;; global options: +cmd
;; Got answer:
;; ->>HEADER<<- opcode: QUERY, status: NOERROR, id: 38438
;; flags: qr rd ra; QUERY: 1, ANSWER: 1, AUTHORITY: 0, ADDITIONAL: 1

;; OPT PSEUDOSECTION:
; EDNS: version: 0, flags:; MBZ: 0x0005, udp: 1232
; COOKIE: e44c48e27464d00a0100004064144cd5e449343b4e444444 (good)
;; QUESTION SECTION:
;a.b.c.		IN	A

;; ANSWER SECTION:
a.b.c.	1	IN	A	1.2.3.4

;; ADDITIONAL SECTION:
a.b.c.	1	IN	A	1.2.3.5

;; AUTHORITY SECTION:
a.b.c.	1	IN	SOA	d.a.b.c.  m.b.c.  1 2 3 4 5

;; Query time: 3 msec
;; SERVER: 10.10.10.10#53(10.10.10.10) (UDP)
;; WHEN: Sun Jan 01 01:01:03 UTC 1234
;; MSG SIZE  rcvd: 90
)"
    };

  Result out;
  BOOST_TEST( nmdp::testAttr(test.c_str(), parserRule, out, blank)
            , "Parser rule 'start': " << test
            );
  for (const auto& o : out) {
    const auto& tStr1 {o.toDebugString()};
    nmdp::testInString(tStr1, "[port: 53,");
    nmdp::testInString(tStr1, "[ipAddress: 10.10.10.10/32,");
    nmdp::testInString(tStr1, "questionFqdn: a.b.c,");
    nmdp::testInString(tStr1, "questionClass: IN,");
    nmdp::testInString(tStr1, "questionType: A]");
    nmdp::testInString(tStr1, "status: NOERROR,");
    nmdp::testInString(tStr1, "{ANSWER, [[responseFqdn: a.b.c,"
                              " responseClass: IN, responseType: A,"
                              " responseTtl: 1, responseData: 1.2.3.4]]}"
                      );
    nmdp::testInString(tStr1, "{ADDITIONAL, [[responseFqdn: a.b.c,"
                              " responseClass: IN, responseType: A,"
                              " responseTtl: 1, responseData: 1.2.3.5]]}"
                      );
    nmdp::testInString(tStr1, "{AUTHORITY, [[responseFqdn: a.b.c,"
                              " responseClass: IN, responseType: SOA,"
                              " responseTtl: 1, responseData: d.a.b.c."
                              "  m.b.c.  1 2 3 4 5]]}"
                      );
  }
}
