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

#include <netmeld/datastore/objects/IpAddress.hpp>
#include <netmeld/datastore/parsers/ParserHelper.hpp>
#include <netmeld/datastore/parsers/ParserIpAddress.hpp>

namespace nmdo = netmeld::datastore::objects;
namespace nmdp = netmeld::datastore::parsers;

BOOST_AUTO_TEST_CASE(testWellformedWithPrefix)
{
  std::vector<std::string> ips =
  {
    "1.2.3.4/1",
    "255.255.255.255/32",
    "1.12.255.0/24",
    "12.1.4.255/10",
    "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff/128",
    "ffff::ffff/64",
    "ffff::/8",
    "::ffff/15",
    "2001:db8:1::23:456:789/48"
  };

  for (const auto& ip : ips) {
    auto temp = nmdp::fromString<nmdp::ParserIpAddress, nmdo::IpAddress>(ip);
    BOOST_CHECK_EQUAL(ip, temp.toString());
  }
}

BOOST_AUTO_TEST_CASE(testWellformedNoPrefixV4)
{
  std::vector<std::string> ips =
  {
    "1.2.3.4",
  };

  for (const auto& ip : ips) {
    auto temp = nmdp::fromString<nmdp::ParserIpAddress, nmdo::IpAddress>(ip);
    auto tempIp = ip + "/32";
    BOOST_CHECK_EQUAL(tempIp, temp.toString());
  }
}

BOOST_AUTO_TEST_CASE(testWellformedNoPrefixV6)
{
  std::vector<std::string> ips =
  {
    "::",
    "::1",
    "2001:db8:1:a:b:23:456:789"
  };

  for (const auto& ip : ips) {
    auto temp = nmdp::fromString<nmdp::ParserIpAddress, nmdo::IpAddress>(ip);
    auto tempIp = ip + "/128";
    BOOST_CHECK_EQUAL(tempIp, temp.toString());
  }
}

BOOST_AUTO_TEST_CASE(testMalformedNotFullyParsed)
{
  std::vector<std::string> ips =
  {
    "::1:",
    "1.2.3.1001",
    "255.255.255.255/5.255.255.255",
    "1.1.1.1/g",
    //"ffff:ffff:ffff:ffff::ffff:ffff:ffff:ffff/128" will pass, but std::exit()
    "ffff:ffff:ffff:ffff:ffff:ffff:ffff:fffg/128"
  };

  for (const auto& ip : ips) {
    nmdo::IpAddress result;
    std::istringstream dataStream(ip);
    dataStream.unsetf(std::ios::skipws); // disable skipping whitespace
    nmdp::IstreamIter i(dataStream), e;
    bool const success = qi::parse(i, e, nmdp::ParserIpAddress(), result);

    BOOST_CHECK(success);
    BOOST_CHECK(i != e);
  }
}

BOOST_AUTO_TEST_CASE(testMalformedParseFail)
{
  std::vector<std::string> ips =
  {
    "2001:db8:1:a:b",
    "192.168 .1.10",
    //"1.1.1.256", will pass, but std::exit()
    "ffff:ffff:ffff:ffff:ffff:ffff:ffff:gfff/128"
  };

  for (const auto& ip : ips) {
    nmdo::IpAddress result;
    std::istringstream dataStream(ip);
    dataStream.unsetf(std::ios::skipws); // disable skipping whitespace
    nmdp::IstreamIter i(dataStream), e;
    bool const success = qi::parse(i, e, nmdp::ParserIpAddress(), result);

    BOOST_CHECK(!success);
    BOOST_CHECK(i != e);
    std::cout << result << std::endl;
  }
}
