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

#include <netmeld/core/utils/LoggerSingleton.hpp>
#include "AugmentArgs.hpp"

namespace nmtc = netmeld::tools::clw;

BOOST_AUTO_TEST_CASE(testAugmentArgsDumpcap)
{
  std::vector<std::vector<std::string>> testsOk {
      {"dumpcap"}
    , {"dumpcap", "-some-arg"}
    , {"dumpcap", "-1", "-w", "file", "-2"}
    };

  for (auto test : testsOk) {
    size_t origSize {test.size()};
    //LOG_INFO << "test-pre: " << test << std::endl;
    nmtc::augmentArgs(test, ".");
    //LOG_INFO << "test-post: " << test << std::endl;
    BOOST_TEST((origSize+2) == test.size());
    auto it {test.crbegin()};
    BOOST_TEST("./results.pcapng" == *(it));
    BOOST_TEST("-w" == *(++it));
  }
}

BOOST_AUTO_TEST_CASE(testAugmentArgsNmap)
{
  std::vector<std::string> test;
  size_t origSize {0};
  auto it {test.crbegin()};

  // defaults
  test      = {"nmap"};
  origSize  = test.size();
  //LOG_INFO << "test-pre: " << test << std::endl;
  nmtc::augmentArgs(test, ".");
  //LOG_INFO << "test-post: " << test << std::endl;
  BOOST_TEST((origSize+9) == test.size());
  it = test.crbegin();
  BOOST_TEST("./results" == *(it));
  BOOST_TEST("-oA" == *(++it));
  BOOST_TEST("500" == *(++it));
  BOOST_TEST("--min-rate" == *(++it));
  BOOST_TEST("256" == *(++it));
  BOOST_TEST("--min-hostgroup" == *(++it));
  BOOST_TEST("60s" == *(++it));
  BOOST_TEST("--stats-every" == *(++it));
  BOOST_TEST("--reason" == *(++it));
  // NOTE: No test for `-iL file` argument as it copies a file

  // explicitly set
  test      = { "nmap"
              , "--reason"
              , "--stats-every", "10s"
              , "--min-hostgroup", "10"
              , "--min-rate", "10"
              , "-oA", "./unused"
              };
  origSize  = test.size();
  //LOG_INFO << "test-pre: " << test << std::endl;
  nmtc::augmentArgs(test, ".");
  //LOG_INFO << "test-post: " << test << std::endl;
  BOOST_TEST((origSize+2) == test.size());
  it = test.crbegin();
  BOOST_TEST("./results" == *(it));
  BOOST_TEST("-oA" == *(++it));
  BOOST_TEST("./unused" == *(++it));
  BOOST_TEST("-oA" == *(++it));
  BOOST_TEST("10" == *(++it));
  BOOST_TEST("--min-rate" == *(++it));
  BOOST_TEST("10" == *(++it));
  BOOST_TEST("--min-hostgroup" == *(++it));
  BOOST_TEST("10s" == *(++it));
  BOOST_TEST("--stats-every" == *(++it));
  BOOST_TEST("--reason" == *(++it));
  // NOTE: No test for `-iL file` argument as it copies a file
}

BOOST_AUTO_TEST_CASE(testAugmentArgsPing)
{
  std::vector<std::vector<std::string>> testsOk {
      {"ping"}
    , {"ping6"}
    , {"ping", "-1", "-2"}
    , {"ping6", "-1", "-2"}
    };

  for (auto test : testsOk) {
    size_t origSize {test.size()};
    //LOG_INFO << "test-pre: " << test << std::endl;
    nmtc::augmentArgs(test, ".");
    //LOG_INFO << "test-post: " << test << std::endl;
    BOOST_TEST((origSize+1) == test.size());
    auto it {test.crbegin()};
    BOOST_TEST("-n" == *(it));
  }

  std::vector<std::vector<std::string>> testsNoChange {
      {"ping", "-n"}
    , {"ping6", "-n"}
    , {"ping", "-1", "-n", "-2"}
    , {"ping6", "-1", "-n", "-2"}
    };

  for (auto test : testsNoChange) {
    size_t origSize {test.size()};
    //LOG_INFO << "test-pre: " << test << std::endl;
    nmtc::augmentArgs(test, ".");
    //LOG_INFO << "test-post: " << test << std::endl;
    BOOST_TEST(origSize == test.size());
  }
}
