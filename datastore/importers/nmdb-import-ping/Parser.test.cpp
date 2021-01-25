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

#include "Parser.hpp"

namespace nmdp = netmeld::datastore::parsers;

using qi::ascii::blank;

class TestParser : public Parser {
    public:
      using Parser::linuxHeader;
      using Parser::linuxResponse;
      using Parser::linuxFooter;
      using Parser::windowsHeader;
      using Parser::windowsResponse;
      using Parser::windowsFooter;

      using Parser::pingLinux;
      using Parser::pingWindows;
};

BOOST_AUTO_TEST_CASE(testParts)
{
  TestParser tp;

  { // linuxHeader
    const auto& parserRule {tp.linuxHeader};
    std::vector<std::string> testsOk {
      // 6 then 4: -n, no -n, -I
      "PING host1(1::1) 56 data bytes\n",
      "PING host1(host2 (1::1)) 56 data bytes\n",
      "PING 1::1(1::1) from 1::1 eth0: 56 data bytes\n",
      "PING host1 (1.2.3.4) 56(84) bytes of data.\n",
      "PING host1 (1.2.3.4) 56(84) bytes of data.\n",
      "PING 1.2.3.4 (1.2.3.4) from 1.2.3.4 eth0: 56(84) bytes of data.\n",
    };
    for (const auto& test : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
                 "Parse rule 'linuxHeader': " << test);
    }
  }

  { // linuxResponse
    const auto& parserRule {tp.linuxResponse};
    std::vector<std::string> testsOk {
      // 6 then 4: -n, no -n, -I
      "64 bytes from 1::1: icmp_seq=1 ttl=64 time=0.044 ms\n",
      "64 bytes from host1 (1::1): icmp_seq=1 ttl=64 time=0.047 ms\n",
      "64 bytes from 1::1%eth0: icmp_seq=1 ttl=64 time=0.051 ms\n",
      "64 bytes from host1 (1::1%eth0): icmp_seq=1 ttl=64 time=0.047 ms\n",
      "64 bytes from host1%eth0 (1::1%eth0): icmp_seq=1 ttl=64 time=0.047 ms\n",
      "64 bytes from 1.2.3.4: icmp_seq=1 ttl=64 time=0.031 ms\n",
      "64 bytes from host1 (1.2.3.4): icmp_seq=1 ttl=64 time=0.025 ms\n",
      // "router" alive samples
      "64 bytes from 1::1: icmp_seq=1 Destination unreachable: Beyond scope of source addresss\n",
      "64 bytes from host1 (1::1): icmp_seq=1 Time to live exceeded\n",
      "64 bytes from 1::1%eth0: icmp_seq=1 Destination unreachable: Beyond scope of source addresss\n",
      "64 bytes from host1 (1::1%eth0): icmp_seq=1 Time to live exceeded\n",
      "64 bytes from host1%eth0 (1::1%eth0): icmp_seq=1 Time to live exceeded\n",
      "64 bytes from 1.2.3.4: icmp_seq=1 Destination Host Unreachable\n",
      "64 bytes from host1 (1.2.3.4): icmp_seq=1 Time to live exceeded\n",
      "From host1 (1.2.3.4) icmp_seq=1 Time to live exceeded\n",
      "From 1.2.3.4 icmp_seq=3 Time to live exceeded\n",
    };
    for (const auto& test : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
                 "Parse rule 'linuxResponse':" << test);
    }
  }

  { // linuxFooter
    const auto& parserRule {tp.linuxFooter};
    std::vector<std::string> testsOk {
      R"STR(--- host1 ping statistics ---
      3 packets transmitted, 3 received, 0% packet loss, time 2031ms
      rtt min/avg/max/mdev = 0.031/0.060/0.076/0.021 ms
      )STR",
      R"STR(--- 1.2.3.4 ping statistics ---
      3 packets transmitted, 3 received, 0% packet loss, time 2053ms
      rtt min/avg/max/mdev = 0.024/0.040/0.064/0.017 ms
      )STR",
      R"STR(--- 1::1 ping statistics ---
      3 packets transmitted, 3 received, 0% packet loss, time 2045ms
      rtt min/avg/max/mdev = 0.047/0.090/0.140/0.038 ms)STR",
      R"STR(--- 1::1 ping statistics ---
      3 packets transmitted, 0 received, 100% packet loss, time 2037ms
      )STR",
    };
    for (const auto& test : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
                 "Parse rule 'linuxFooter':" << test);
    }
  }

  { // windowsHeader
    const auto& parserRule {tp.windowsHeader};
    std::vector<std::string> testsOk {
      // 6 then 4:
      "Pinging 1::1 with 32 bytes of data:\n",
      "Pinging host1 [1::1] with 32 bytes of data:\n",
      "Pinging 1.2.3.4 with 32 bytes of data:\n",
      "Pinging host1 [1.2.3.4] with 32 bytes of data:\n",
    };
    for (const auto& test : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
                 "Parse rule 'windowsHeader': " << test);
    }
  }

  { // windowsResponse
    const auto& parserRule {tp.windowsResponse};
    std::vector<std::string> testsOk {
      // 6 then 4:
      "Reply from ::1: time<1ms\n",
      "Reply from 1.2.3.4: bytes=32 time<1ms TTL=64\n",
      "Reply from ::1: Destination host unreachable.\n",
      "Reply from 1.2.3.4: Destination host unreachable.\n",
    };
    for (const auto& test : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
                 "Parse rule 'windowsResponse':" << test);
    }
  }

  { // windowsFooter
    const auto& parserRule {tp.windowsFooter};
    std::vector<std::string> testsOk {
      R"STR(Ping statistics for 1.2.3.4:
          Packets: Sent = 3, Received = 3, Lost = 0 (0% loss),
          Approximate round trip times in milli-seconds:
              Minimum = 0ms, Maximum = 0ms, Average = 0ms)STR",
      R"STR(Ping statistics for 1::1:
          Packets: Sent = 3, Received = 3, Lost = 0 (0% loss),
          Approximate round trip times in milli-seconds:
              Minimum = 0ms, Maximum = 0ms, Average = 0ms)STR",
    };
    for (const auto& test : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
                 "Parse rule 'windowsFooter':" << test);
    }
  }
}

BOOST_AUTO_TEST_CASE(testWhole)
{
  TestParser tp;

  const auto& parserRule {tp};
  std::vector<std::string> testsOk {
    // Linux
    R"STR($ ping -nc3 host1
    PING host1(1::1) 56 data bytes
    64 bytes from 1::1: icmp_seq=1 ttl=64 time=0.044 ms
    64 bytes from 1::1: icmp_seq=2 ttl=64 time=0.159 ms
    64 bytes from 1::1: icmp_seq=3 ttl=64 time=0.156 ms

    --- host1 ping statistics ---
    3 packets transmitted, 3 received, 0% packet loss, time 2045ms
    rtt min/avg/max/mdev = 0.044/0.119/0.159/0.053 ms
    )STR",
    R"STR($ ping -c3 host1
    PING host1(host1 (1::1)) 56 data bytes
    64 bytes from host1 (1::1): icmp_seq=1 ttl=64 time=0.047 ms
    64 bytes from host1 (1::1): icmp_seq=2 ttl=64 time=0.173 ms
    64 bytes from host1 (1::1): icmp_seq=3 ttl=64 time=0.162 ms

    --- host1 ping statistics ---
    3 packets transmitted, 3 received, 0% packet loss, time 2043ms
    rtt min/avg/max/mdev = 0.047/0.127/0.173/0.056 ms
    )STR",
    R"STR($ ping -c4 1.2.3.4
    PING 1.2.3.4 (1.2.3.4) 56(84) bytes of data.
    64 bytes from 1.2.3.4: icmp_seq=1 ttl=64 time=0.044 ms
    64 bytes from 1.2.3.4: icmp_seq=2 ttl=64 time=0.017 ms
    64 bytes from 1.2.3.4: icmp_seq=3 ttl=64 time=0.032 ms
    From host1 (1.2.3.4) icmp_seq=1 Time to live exceeded

    --- 1.2.3.4 ping statistics ---
    3 packets transmitted, 3 received, 0% packet loss, time 2030ms
    rtt min/avg/max/mdev = 0.017/0.031/0.044/0.011 ms
    )STR",
    R"STR($ ping -nc3 test.domain -I eth0
    PING test.domain (1.2.3.4) from 4.3.2.1 eth0: 56(84) bytes of data.

    --- test.domain ping statistics ---
    3 packets transmitted, 0 received, 100% packet loss, time 2027ms
    )STR",
    R"STR(ping test.domain
    PING test.domain (1.2.3.4) 56(84) bytes of data.
    64 bytes from test.domain (1.2.3.4): icmp_seq=1 ttl=64 time=0.026 ms
    From 1.2.3.4 icmp_seq=3 Time to live exceeded
    ^C
    --- test.domain ping statistics ---
    4 packets transmitted, 4 received, 0% packet loss, time 3063ms
    rtt min/avg/max/mdev = 0.026/0.029/0.032/0.002 ms)STR",
    R"STR(ping test.domain
    PING test.domain (1.2.3.4) 56(84) bytes of data.
    From 1.2.3.4 icmp_seq=3 Time to live exceeded
    64 bytes from test.domain (1.2.3.4): icmp_seq=1 ttl=64 time=0.026 ms

    --- test.domain ping statistics ---
    4 packets transmitted, 4 received, 0% packet loss, time 3063ms
    rtt min/avg/max/mdev = 0.026/0.029/0.032/0.002 ms)STR",

    // Windows
    R"STR(ping -n 3 test.domain

    Pinging test.domain [1.2.3.4] with 32 bytes of data:
    Reply from 1.2.3.4: bytes=32 time=19ms TTL=115
    Reply from 1.2.3.4: bytes=32 time=25ms TTL=115
    Reply from 1.2.3.4: bytes=32 time=18ms TTL=115

    Ping statistics for 1.2.3.4:
        Packets: Sent = 3, Received = 3, Lost = 0 (0% loss),
        Approximate round trip times in milli-seconds:
            Minimum = 18ms, Maximum = 25ms, Average = 20ms
    )STR",
    R"STR(ping -n 3 test.domain

    Pinging test.domain [1.2.3.4] with 32 bytes of data:
    Reply from 1.2.3.4: bytes=32 time=19ms TTL=115
    Request timed out.
    Reply from 1.2.3.4: bytes=32 time=25ms TTL=115

    Ping statistics for 1.2.3.4:
        Packets: Sent = 3, Received = 3, Lost = 0 (0% loss),
        Approximate round trip times in milli-seconds:
            Minimum = 18ms, Maximum = 25ms, Average = 20ms
    )STR",
    R"STR(ping -n 3 test.domain

    Pinging test.domain [1.2.3.4] with 32 bytes of data:
    Request timed out.
    Reply from 1.2.3.4: bytes=32 time=19ms TTL=115
    Reply from 1.2.3.4: bytes=32 time=25ms TTL=115

    Ping statistics for 1.2.3.4:
        Packets: Sent = 3, Received = 3, Lost = 0 (0% loss),
        Approximate round trip times in milli-seconds:
            Minimum = 18ms, Maximum = 25ms, Average = 20ms
    )STR",

    // Multi
    R"STR(
    $ ping -nc3 host1
    PING host1(1::1) 56 data bytes
    64 bytes from 1::1: icmp_seq=1 ttl=64 time=0.044 ms
    64 bytes from 1::1: icmp_seq=2 ttl=64 time=0.159 ms
    64 bytes from 1::1: icmp_seq=3 ttl=64 time=0.156 ms

    --- host1 ping statistics ---
    3 packets transmitted, 3 received, 0% packet loss, time 2045ms
    rtt min/avg/max/mdev = 0.044/0.119/0.159/0.053 ms

    > ping -n 3 test.domain

    Pinging test.domain [1.2.3.4] with 32 bytes of data:
    Reply from 1.2.3.4: bytes=32 time=19ms TTL=115
    Reply from 1.2.3.4: bytes=32 time=25ms TTL=115
    Reply from 1.2.3.4: bytes=32 time=18ms TTL=115

    Ping statistics for 1.2.3.4:
        Packets: Sent = 3, Received = 3, Lost = 0 (0% loss),
        Approximate round trip times in milli-seconds:
            Minimum = 18ms, Maximum = 25ms, Average = 20ms
    )STR",

  };
  for (const auto& test : testsOk) {
    BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
               "Parse rule 'start': " << test);
  }
}
