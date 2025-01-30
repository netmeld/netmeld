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

#include <thread>
#include <atomic>

#include "DataContainerSingleton.hpp"


BOOST_AUTO_TEST_CASE(testSingletonInstance)
{
  DataContainerSingleton& dcs1 = DataContainerSingleton::getInstance();
  DataContainerSingleton& dcs2 = DataContainerSingleton::getInstance();

  BOOST_TEST(&dcs1 == &dcs2);
}

BOOST_AUTO_TEST_CASE(testInsertAndHasData)
{
  DataContainerSingleton& dcs = DataContainerSingleton::getInstance();

  Data d1;
  dcs.insert(d1);

  BOOST_TEST(dcs.hasData());
  // clean-out
  dcs.getData();
  BOOST_TEST(!dcs.hasData());
}

BOOST_AUTO_TEST_CASE(testGetData)
{
  DataContainerSingleton& dcs = DataContainerSingleton::getInstance();

  Data d1, d2;
  dcs.insert(d1);
  dcs.insert(d2);

  Result result = dcs.getData();
  BOOST_TEST(result.size() == 2);
  BOOST_TEST(result[0] == d1);
  BOOST_TEST(result[1] == d2);
  BOOST_TEST(!dcs.hasData());
}

BOOST_AUTO_TEST_CASE(testConcurrency)
{
  std::atomic<size_t> pCnt {0};
  std::atomic<size_t> cCnt {0};
  std::atomic<bool> done(false);
  size_t bCnt {123456};

  auto producer = [&]() {
    DataContainerSingleton& pDcs = DataContainerSingleton::getInstance();
    for (size_t i = 0; i < bCnt; ++i) {
      Data d;
      pDcs.insert(d);
      ++pCnt;
    }
  };

  auto consumer = [&]() {
    DataContainerSingleton& cDcs = DataContainerSingleton::getInstance();
    while (!done.load() || cDcs.hasData()) {
      if (cDcs.hasData()) {
        cCnt += cDcs.getData().size();
      } else {
        std::this_thread::yield();
      }
    }
  };

  std::thread prod1(producer);
  std::thread cons1(consumer);
  std::thread prod2(producer);

  prod1.join();
  prod2.join();
  done.store(true);
  cons1.join();

  DataContainerSingleton& dcs = DataContainerSingleton::getInstance();
  BOOST_TEST(!dcs.hasData());
  size_t expCnt {2*bCnt}; // 2 producers
  BOOST_TEST(expCnt == pCnt.load());
  BOOST_TEST(expCnt == cCnt.load());
}
