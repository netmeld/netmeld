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

#include <netmeld/core/objects/DnsQuestion.hpp>

namespace nmco = netmeld::core::objects;


BOOST_AUTO_TEST_CASE(testConstructors)
{
  {
    nmco::DnsQuestion dnsQuestion;

    BOOST_TEST(dnsQuestion.getFqdn().empty());
    BOOST_TEST(dnsQuestion.getClass().empty());
    BOOST_TEST(dnsQuestion.getType().empty());
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    nmco::DnsQuestion dnsQuestion;
    dnsQuestion.setFqdn("Www.Example.Com");
    dnsQuestion.setClass("in");
    dnsQuestion.setType("aaaa");

    BOOST_TEST(dnsQuestion.getFqdn() == "www.example.com");
    BOOST_TEST(dnsQuestion.getClass() == "IN");
    BOOST_TEST(dnsQuestion.getType() == "AAAA");
  }
}

