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

#include "CmdExec.hpp"

namespace nmcu = netmeld::core::utils;

BOOST_AUTO_TEST_CASE(testCmdExecVariations)
{
  // isCmdAvailable
  BOOST_TEST(nmcu::isCmdAvailable(""));

  // cmdExec
  BOOST_TEST(0 == nmcu::cmdExec(""));
  BOOST_TEST_REQUIRE(nmcu::isCmdAvailable("exit"));
  BOOST_TEST(0  == nmcu::cmdExec("exit 0"));
  BOOST_TEST(1  == nmcu::cmdExec("exit 1"));
  BOOST_TEST(-1 == nmcu::cmdExec("exit 255"));
  BOOST_TEST(0  == nmcu::cmdExec("exit 256"));
  BOOST_TEST(1  == nmcu::cmdExec("exit 257"));

  // cmdExecOut
  BOOST_TEST(nmcu::cmdExecOut("").empty());
  BOOST_TEST_REQUIRE(nmcu::isCmdAvailable("echo"));
  BOOST_TEST("test\n" == nmcu::cmdExecOut("echo test"));

  // cmdExecOrExit
  BOOST_TEST(0 == nmcu::cmdExecOrExit(""));
  // can't test exit at this level unless re-write logic
}
