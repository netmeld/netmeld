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

#include "ThreadSafeQueue.hpp"

#include <thread>
#include <atomic>

namespace nmcu = netmeld::core::utils;

BOOST_AUTO_TEST_CASE(testSingleThread)
{
  nmcu::ThreadSafeQueue<size_t> tsq;
  tsq.push(1);
  tsq.push(2);
  tsq.push(3);

  BOOST_TEST(3 == tsq.size());
  BOOST_TEST(1 == tsq.front());
  tsq.pop();
  BOOST_TEST(2 == tsq.front());
  tsq.pop();
  BOOST_TEST(3 == tsq.front());
  tsq.pop();
  BOOST_TEST(tsq.isEmpty());
}

BOOST_AUTO_TEST_CASE(testConcurrentSimple)
{
  nmcu::ThreadSafeQueue<size_t> tsq;
  std::atomic<size_t> pCnt(0), cCnt(0);
  size_t bCnt {300000};

  auto producer = [&]() {
    for (size_t i = 0; i < bCnt; ++i) {
      tsq.push(i);
      ++pCnt;
    }
  };

  auto consumer = [&]() {
    for (size_t i = 0; i < bCnt; ++i) {
      while (tsq.isEmpty()) {
        std::this_thread::yield();
      }
      tsq.pop();
      ++cCnt;
    }
  };

  std::thread prod1(producer);
  std::thread prod2(producer);
  std::thread cons1(consumer);
  std::thread cons2(consumer);

  prod1.join();
  prod2.join();
  cons1.join();
  cons2.join();

  BOOST_TEST(tsq.isEmpty());
  size_t expCnt {2*bCnt}; // 2 producers
  BOOST_TEST(expCnt == pCnt.load());
  BOOST_TEST(expCnt == cCnt.load());
}

BOOST_AUTO_TEST_CASE(testConcurrentMixed)
{
  nmcu::ThreadSafeQueue<size_t> tsq;
  std::atomic<size_t> pCnt(0), cCnt(0);
  size_t bCnt {400000};

  auto producer = [&]() {
    for (size_t i = 0; i < bCnt; ++i) {
      tsq.push(i);
      ++pCnt;
    }
  };

  auto consumer = [&]() {
    for (size_t i = 0; i < bCnt; ++i) {
      while (tsq.isEmpty()) {
        std::this_thread::yield();
      }
      tsq.pop();
      ++cCnt;
    }
  };

  std::thread prod1(producer);
  std::thread cons1(consumer);
  std::thread prod2(producer);
  std::thread cons2(consumer);

  prod1.join();
  cons1.join();
  prod2.join();
  cons2.join();

  BOOST_TEST(tsq.isEmpty());
  size_t expCnt {2*bCnt}; // 2 producers
  BOOST_TEST(expCnt == pCnt.load());
  BOOST_TEST(expCnt == cCnt.load());
}
