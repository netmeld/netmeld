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
      using Parser::windowsHeader;
      using Parser::windowsHop;
      using Parser::linuxHeader;
      using Parser::linuxHop;
};

BOOST_AUTO_TEST_CASE(testParts)
{
  TestParser tp;

  { // windowsHeader
    const auto& parserRule {tp.windowsHeader};
    std::vector<std::string> testsOk {

      R"STR(Tracing route to google.com [142.250.72.46]
over a maximum of 30 hops:)STR",
      R"STR(Tracing route to 142.250.72.46
over a maximum of 30 hops:)STR"
    };
    for (const auto& test : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
                 "Parse rule 'windowsHeader': " << test);
    }
  }

  { // windowsHop
    const auto& parserRule {tp.windowsHop};
    std::vector<std::string> testsOk {

      R"STR(  1    <1 ms    10 ms    100 ms  192.168.1.2 
)STR",
      R"STR(  2    <1 ms    1 ms    100 ms  192.168.1.3 
)STR",
      R"STR(  1    <1 ms    10 ms    100 ms  domain.domain.tld [192.168.1.2] 
)STR",
      R"STR(  2    <1 ms    1 ms    100 ms  domain.domain.tld [192.168.1.3] 
)STR"
    };
    for (const auto& test : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
                 "Parse rule 'windowsHop': " << test);
    }
  }

  { // linuxHeader
    const auto& parserRule {tp.linuxHeader};
    std::vector<std::string> testsOk {

      R"STR(traceroute to google.com (142.250.69.238), 30 hops max, 60 byte packets)STR",
      R"STR(traceroute to 142.250.69.238 (142.250.69.238), 30 hops max, 60 byte packets)STR"
    };
    for (const auto& test : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
                 "Parse rule 'linuxHeader': " << test);
    }
  }

  { // linuxHop
    const auto& parserRule {tp.linuxHop};
    std::vector<std::string> testsOk {

      R"STR( 1  192.168.2.3 (192.168.2.3)  0.138 ms  0.061 ms  0.190 ms
)STR",
      R"STR( 10  domain.domain.tld (192.168.2.3)  0.138 ms  0.061 ms  0.190 ms 
)STR",
      R"STR( 2  * domain.domain.tld (192.168.2.3)  2 ms domain2.domain.tld (192.168.2.5)  3 ms
)STR",
      R"STR( 5  * * 192.168.2.3 (192.168.2.3)  0.061 ms
)STR",
      R"STR( 7  * * *
)STR"
    };
    for (const auto& test : testsOk) {
      BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
                 "Parse rule 'linuxHop': " << test);
    }
  }

}

BOOST_AUTO_TEST_CASE(testWhole)
{
  TestParser tp;

  const auto& parserRule {tp};
  std::vector<std::string> testsOk {

    R"STR(
Tracing route to google.com [142.250.72.46]
over a maximum of 30 hops:

  1    <1 ms    <1 ms    <1 ms  192.168.92.68 
  2     4 ms     6 ms     3 ms  10.0.0.1 
  3    13 ms    12 ms    13 ms  99.99.99.99 
  4    12 ms    13 ms    11 ms  domain.test.tld [123.213.11.22] 
  5    17 ms    10 ms    14 ms  domain.test2.tld [226.155.44.55] 
  6    16 ms    12 ms    12 ms  111.111.111.111 
  7    19 ms    18 ms    20 ms  domain.test3.tld [77.177.178.1] 
  8    29 ms    19 ms    18 ms  domain.test4.tld [50.60.70.80] 
  9    26 ms    18 ms    18 ms  domain.test5.tld [25.85.90.150] 
 10    29 ms    25 ms    19 ms  13.26.42.84 
 11    25 ms    20 ms    19 ms  1.2.3.4 
 12    20 ms    18 ms    23 ms  domain.test6.tld [142.250.72.46] 

Trace complete.
)STR",

R"STR(
Tracing route to 142.250.72.46 over a maximum of 30 hops:

  1    <1 ms    <1 ms    <1 ms  192.168.92.68 
  2     4 ms     6 ms     3 ms  10.0.0.1 
  3    13 ms    12 ms    13 ms  99.99.99.99 
  4    12 ms    13 ms    11 ms  123.213.11.22 
  5    17 ms    10 ms    14 ms  226.155.44.55 
  6    16 ms    12 ms    12 ms  111.111.111.111 
  7    19 ms    18 ms    20 ms  77.177.178.1 
  8    29 ms    19 ms    18 ms  50.60.70.80 
  9    26 ms    18 ms    18 ms  25.85.90.150 
 10    29 ms    25 ms    19 ms  13.26.42.84 
 11    25 ms    20 ms    19 ms  1.2.3.4 
 12    20 ms    18 ms    23 ms  142.250.72.46 

Trace complete.
)STR",

R"STR(traceroute to google.com (142.250.69.238), 30 hops max, 60 byte packets
 1  192.168.92.68 (192.168.92.68)  0.138 ms  0.061 ms  0.190 ms
 2  10.0.0.1 (10.0.0.1)  2.762 ms  2.655 ms  2.607 ms
 3  99.99.99.99 (99.99.99.99)  15.195 ms  25.852 ms  25.791 ms
 4  domain.test.tld (100.100.100.100)  22.055 ms  23.609 ms  24.829 ms
 5  domain.test2.tld (123.123.132.132)  25.496 ms  25.431 ms  25.385 ms
 6  50.1.2.3 (50.1.2.3)  25.328 ms  17.604 ms  20.639 ms
 7  domain.test3.tld (90.95.90.95)  40.095 ms domain.test4.tld (91.96.92.97)  36.439 ms domain.test5.tld (92.97.91.96)  46.414 ms
 8  domain.test6.tld (95.90.95.90)  46.300 ms domain.test7.tld (33.55.44.66)  46.249 ms domain.test8.tld (18.28.38.48)  46.145 ms
 9  71.17.28.82 (71.17.28.82)  43.604 ms domain.test9.tld (23.30.206.42)  46.033 ms 71.17.28.82 (71.17.28.82)  40.445 ms
10  * * *
11  44.99.44.99 (44.99.44.99)  40.217 ms 55.56.57.58 (55.56.57.58)  47.051 ms 141.241.41.1 (141.241.41.1)  34.579 ms
12  89.90.91.92 (89.90.91.92)  42.392 ms 121.212.121.212 (121.212.121.212)  40.149 ms 2.22.221.222 (2.22.221.222)  34.577 ms
13  * 1.12.123.12 (1.12.123.12)  49.110 ms 8.89.98.8 (8.89.98.8)  42.313 ms
14  * * *
15  4.3.3.4 (4.3.3.4)  68.355 ms 48.96.144.192 (48.96.144.192)  58.998 ms 4.3.3.4 (4.3.3.4)  68.215 ms
16  91.19.92.29 (91.19.92.29)  62.436 ms  63.339 ms 58.85.85.58 (58.85.85.58)  61.086 ms
17  33.99.133.199 (33.99.133.199)  61.693 ms  54.472 ms 221.221.221.221 (221.221.221.221)  53.461 ms
18  142.251.61.183 (142.251.61.183)  61.280 ms  58.059 ms  58.304 ms
19  domain.test10.tld (142.250.69.238)  57.871 ms  56.099 ms  59.306 ms
)STR",

R"STR(traceroute to google.com (142.250.72.46), 30 hops max, 60 byte packets
 1  192.168.92.68  0.236 ms  0.264 ms  0.155 ms
 2  10.0.0.1  2.113 ms  2.009 ms  3.800 ms
 3  99.99.99.99  14.065 ms  24.768 ms  24.636 ms
 4  25.89.52.52  19.867 ms  21.243 ms  22.064 ms
 5  168.168.168.5  25.708 ms  25.626 ms  25.552 ms
 6  41.41.41.41  25.454 ms  25.196 ms  25.123 ms
 7  91.11.11.19  31.697 ms 21.75.57.12  40.216 ms 91.11.11.19  36.916 ms
 8  47.41.47.4  32.466 ms 53.85.8.45  24.717 ms 47.41.47.4  23.588 ms
 9  85.31.91.85  30.329 ms 54.15.54.15  29.474 ms 85.31.91.85  26.939 ms
10  83.84.5.86  26.980 ms * 21.55.99.128  32.185 ms
11  44.38.33.48  30.477 ms 42.99.11.1  30.402 ms 53.85.33.115  33.578 ms
12  142.250.72.46  27.107 ms 18.19.20.21  28.758 ms 142.250.72.46  33.128 ms
)STR"
  };
  
  for (const auto& test : testsOk) {
    BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank),
               "Parse rule 'start': " << test);
  }
  
}
