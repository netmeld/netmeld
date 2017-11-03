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

#include <netmeld/common/parser_networking.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/adapted.hpp>

#include <string>
#include <tuple>


using std::get;
using std::string;

namespace qi = boost::spirit::qi;


BOOST_AUTO_TEST_CASE(test_parser_mac_addr_wellformed)
{
  Parser_MAC_Addr<string::const_iterator> const parser_mac_addr;

  // MAC address format used by Linux/BSD
  if (true) {
    string const mac_addr_string{"00:11:22:33:44:55"};
    string::const_iterator
      i = mac_addr_string.cbegin(),
      e = mac_addr_string.cend();

    MAC_Addr mac_addr;

    bool const parse_success =
      qi::phrase_parse(i, e, parser_mac_addr, qi::ascii::blank, mac_addr);

    BOOST_CHECK(parse_success);
    BOOST_CHECK(i == e);

    BOOST_CHECK_EQUAL(0x00, mac_addr.at(0));
    BOOST_CHECK_EQUAL(0x11, mac_addr.at(1));
    BOOST_CHECK_EQUAL(0x22, mac_addr.at(2));
    BOOST_CHECK_EQUAL(0x33, mac_addr.at(3));
    BOOST_CHECK_EQUAL(0x44, mac_addr.at(4));
    BOOST_CHECK_EQUAL(0x55, mac_addr.at(5));
  }

  // MAC address format used by Windows
  if (true) {
    string const mac_addr_string{"00-11-22-33-44-55"};
    string::const_iterator
      i = mac_addr_string.cbegin(),
      e = mac_addr_string.cend();

    MAC_Addr mac_addr;

    bool const parse_success =
      qi::phrase_parse(i, e, parser_mac_addr, qi::ascii::blank, mac_addr);

    BOOST_CHECK(parse_success);
    BOOST_CHECK(i == e);

    BOOST_CHECK_EQUAL(0x00, mac_addr.at(0));
    BOOST_CHECK_EQUAL(0x11, mac_addr.at(1));
    BOOST_CHECK_EQUAL(0x22, mac_addr.at(2));
    BOOST_CHECK_EQUAL(0x33, mac_addr.at(3));
    BOOST_CHECK_EQUAL(0x44, mac_addr.at(4));
    BOOST_CHECK_EQUAL(0x55, mac_addr.at(5));
  }

  // MAC address format used by Cisco
  if (true) {
    string const mac_addr_string{"0011.2233.4455"};
    string::const_iterator
      i = mac_addr_string.cbegin(),
      e = mac_addr_string.cend();

    MAC_Addr mac_addr;

    bool const parse_success =
      qi::phrase_parse(i, e, parser_mac_addr, qi::ascii::blank, mac_addr);

    BOOST_CHECK(parse_success);
    BOOST_CHECK(i == e);

    BOOST_CHECK_EQUAL(0x00, mac_addr.at(0));
    BOOST_CHECK_EQUAL(0x11, mac_addr.at(1));
    BOOST_CHECK_EQUAL(0x22, mac_addr.at(2));
    BOOST_CHECK_EQUAL(0x33, mac_addr.at(3));
    BOOST_CHECK_EQUAL(0x44, mac_addr.at(4));
    BOOST_CHECK_EQUAL(0x55, mac_addr.at(5));
  }
}


BOOST_AUTO_TEST_CASE(test_parser_ipv4_addr_wellformed)
{
  Parser_IPv4_Addr<string::const_iterator> const parser_ipv4_addr;

  // Unspecified address
  if (true) {
    string const ipv4_addr_string{"0.0.0.0"};
    string::const_iterator
      i = ipv4_addr_string.cbegin(),
      e = ipv4_addr_string.cend();

    IPv4_Addr ipv4_addr;

    bool const parse_success =
      qi::phrase_parse(i, e, parser_ipv4_addr, qi::ascii::blank, ipv4_addr);

    BOOST_CHECK(parse_success);
    BOOST_CHECK(i == e);

    BOOST_CHECK(ipv4_addr.is_unspecified());
  }

  // "Network" addresses
  if (true) {
    string const ipv4_addr_string{"192.0.2.0"};
    string::const_iterator
      i = ipv4_addr_string.cbegin(),
      e = ipv4_addr_string.cend();

    IPv4_Addr ipv4_addr;

    bool const parse_success =
      qi::phrase_parse(i, e, parser_ipv4_addr, qi::ascii::blank, ipv4_addr);

    BOOST_CHECK(parse_success);
    BOOST_CHECK(i == e);

    BOOST_CHECK_EQUAL(ipv4_addr, IPv4_Addr::from_string(ipv4_addr_string));
  }

  // Unicast addresses
  if (true) {
    string const ipv4_addr_string{"192.0.2.1"};
    string::const_iterator
      i = ipv4_addr_string.cbegin(),
      e = ipv4_addr_string.cend();

    IPv4_Addr ipv4_addr;

    bool const parse_success =
      qi::phrase_parse(i, e, parser_ipv4_addr, qi::ascii::blank, ipv4_addr);

    BOOST_CHECK(parse_success);
    BOOST_CHECK(i == e);

    BOOST_CHECK_EQUAL(ipv4_addr, IPv4_Addr::from_string(ipv4_addr_string));
  }

  // Broadcast addresses
  if (true) {
    string const ipv4_addr_string{"192.0.2.255"};
    string::const_iterator
      i = ipv4_addr_string.cbegin(),
      e = ipv4_addr_string.cend();

    IPv4_Addr ipv4_addr;

    bool const parse_success =
      qi::phrase_parse(i, e, parser_ipv4_addr, qi::ascii::blank, ipv4_addr);

    BOOST_CHECK(parse_success);
    BOOST_CHECK(i == e);

    BOOST_CHECK_EQUAL(ipv4_addr, IPv4_Addr::from_string(ipv4_addr_string));
  }
}


BOOST_AUTO_TEST_CASE(test_parser_ipv4_addr_malformed)
{
  Parser_IPv4_Addr<string::const_iterator> const parser_ipv4_addr;

  if (true) {
    string const ipv4_addr_string{"192.168.1.1001"};
    string::const_iterator
      i = ipv4_addr_string.cbegin(),
      e = ipv4_addr_string.cend();

    IPv4_Addr ipv4_addr;

    bool const parse_success =
      qi::phrase_parse(i, e, parser_ipv4_addr, qi::ascii::blank, ipv4_addr);

    BOOST_CHECK(parse_success);  // Parses the "192.168.1.100" part,
    BOOST_CHECK(i != e);         // but not the trailing "1".
  }

  if (true) {
    string const ipv4_addr_string{"192.168 .1.10"};
    string::const_iterator
      i = ipv4_addr_string.cbegin(),
      e = ipv4_addr_string.cend();

    IPv4_Addr ipv4_addr;

    bool const parse_success =
      qi::phrase_parse(i, e, parser_ipv4_addr, qi::ascii::blank, ipv4_addr);

    BOOST_CHECK(!parse_success);
    BOOST_CHECK(i != e);
  }
}


BOOST_AUTO_TEST_CASE(test_parser_ipv6_addr_wellformed)
{
  Parser_IPv6_Addr<string::const_iterator> const parser_ipv6_addr;

  // Unspecified address
  if (true) {
    string const ipv6_addr_string{"::"};
    string::const_iterator
      i = ipv6_addr_string.cbegin(),
      e = ipv6_addr_string.cend();

    IPv6_Addr ipv6_addr;

    bool const parse_success =
      qi::phrase_parse(i, e, parser_ipv6_addr, qi::ascii::blank, ipv6_addr);

    BOOST_CHECK(parse_success);
    BOOST_CHECK(i == e);

    BOOST_CHECK(ipv6_addr.is_unspecified());
  }

  // Loopback address (starts with "::")
  if (true) {
    string const ipv6_addr_string{"::1"};
    string::const_iterator
      i = ipv6_addr_string.cbegin(),
      e = ipv6_addr_string.cend();

    IPv6_Addr ipv6_addr;

    bool const parse_success =
      qi::phrase_parse(i, e, parser_ipv6_addr, qi::ascii::blank, ipv6_addr);

    BOOST_CHECK(parse_success);
    BOOST_CHECK(i == e);

    BOOST_CHECK_EQUAL(ipv6_addr, IPv6_Addr::from_string(ipv6_addr_string));
  }

  // Link-local "network" address (ends with "::")
  if (true) {
    string const ipv6_addr_string{"fe80::"};
    string::const_iterator
      i = ipv6_addr_string.cbegin(),
      e = ipv6_addr_string.cend();

    IPv6_Addr ipv6_addr;

    bool const parse_success =
      qi::phrase_parse(i, e, parser_ipv6_addr, qi::ascii::blank, ipv6_addr);

    BOOST_CHECK(parse_success);
    BOOST_CHECK(i == e);

    BOOST_CHECK_EQUAL(ipv6_addr, IPv6_Addr::from_string(ipv6_addr_string));
  }

  // Normal unicast address (contains "::")
  if (true) {
    string const ipv6_addr_string{"2001:DB8:1::23:456:789"};
    string::const_iterator
      i = ipv6_addr_string.cbegin(),
      e = ipv6_addr_string.cend();

    IPv6_Addr ipv6_addr;

    bool const parse_success =
      qi::phrase_parse(i, e, parser_ipv6_addr, qi::ascii::blank, ipv6_addr);

    BOOST_CHECK(parse_success);
    BOOST_CHECK(i == e);

    BOOST_CHECK_EQUAL(ipv6_addr, IPv6_Addr::from_string(ipv6_addr_string));
  }

  if (true) {
    string const ipv6_addr_string{"2001:DB8:1:234::1"};
    string::const_iterator
      i = ipv6_addr_string.cbegin(),
      e = ipv6_addr_string.cend();

    IPv6_Addr ipv6_addr;

    bool const parse_success =
      qi::phrase_parse(i, e, parser_ipv6_addr, qi::ascii::blank, ipv6_addr);

    BOOST_CHECK(parse_success);
    BOOST_CHECK(i == e);

    BOOST_CHECK_EQUAL(ipv6_addr, IPv6_Addr::from_string(ipv6_addr_string));
  }

  // Normal unicast address (doesn't contain "::")
  if (true) {
    string const ipv6_addr_string{"2001:DB8:1:A:B:23:456:789"};
    string::const_iterator
      i = ipv6_addr_string.cbegin(),
      e = ipv6_addr_string.cend();

    IPv6_Addr ipv6_addr;

    bool const parse_success =
      qi::phrase_parse(i, e, parser_ipv6_addr, qi::ascii::blank, ipv6_addr);

    BOOST_CHECK(parse_success);
    BOOST_CHECK(i == e);

    BOOST_CHECK_EQUAL(ipv6_addr, IPv6_Addr::from_string(ipv6_addr_string));
  }
}


BOOST_AUTO_TEST_CASE(test_parser_ipv4_addr_with_cidr)
{
  Parser_IPv4_Addr_with_CIDR<string::const_iterator> const
    parser_ipv4_addr_with_cidr;

  if (true) {
    string const ipv4_addr_string{"192.0.2.1"};
    string const ipv4_addr_with_cidr_string{ipv4_addr_string + "/16"};
    string::const_iterator
      i = ipv4_addr_with_cidr_string.cbegin(),
      e = ipv4_addr_with_cidr_string.cend();

    IPv4_Addr_with_Prefix ipv4_addr_with_prefix;

    bool const parse_success =
      qi::phrase_parse(i, e, parser_ipv4_addr_with_cidr,
                       qi::ascii::blank,
                       ipv4_addr_with_prefix);

    BOOST_CHECK(parse_success);
    BOOST_CHECK(i == e);

    BOOST_CHECK_EQUAL(ipv4_addr_with_prefix.addr(),
                      IPv4_Addr::from_string(ipv4_addr_string));
    BOOST_CHECK_EQUAL(ipv4_addr_with_prefix.cidr(), 16);
  }
}


BOOST_AUTO_TEST_CASE(test_parser_ipv6_addr_with_cidr)
{
  Parser_IPv6_Addr_with_CIDR<string::const_iterator> const
    parser_ipv6_addr_with_cidr;

  if (true) {
    string const ipv6_addr_string{"2001:DB8:1::23:456:789"};
    string const ipv6_addr_with_cidr_string{ipv6_addr_string + "/48"};
    string::const_iterator
      i = ipv6_addr_with_cidr_string.cbegin(),
      e = ipv6_addr_with_cidr_string.cend();

    IPv6_Addr_with_Prefix ipv6_addr_with_prefix;

    bool const parse_success =
      qi::phrase_parse(i, e, parser_ipv6_addr_with_cidr,
                       qi::ascii::blank,
                       ipv6_addr_with_prefix);

    BOOST_CHECK(parse_success);
    BOOST_CHECK(i == e);

    BOOST_CHECK_EQUAL(ipv6_addr_with_prefix.addr(),
                      IPv6_Addr::from_string(ipv6_addr_string));
    BOOST_CHECK_EQUAL(ipv6_addr_with_prefix.cidr(), 48);
  }
}


BOOST_AUTO_TEST_CASE(test_parser_ipv4_addr_with_mask)
{
  Parser_IPv4_Addr_with_Mask<string::const_iterator> const
    parser_ipv4_addr_with_mask;

  if (true) {
    string const ipv4_addr_string{"192.0.2.1"};
    string const ipv4_mask_string{"255.255.255.0"};
    string const ipv4_addr_with_mask_string{ipv4_addr_string + "/" + ipv4_mask_string};
    string::const_iterator
      i = ipv4_addr_with_mask_string.cbegin(),
      e = ipv4_addr_with_mask_string.cend();

    IPv4_Addr_with_Prefix ipv4_addr_with_prefix;

    bool const parse_success =
      qi::phrase_parse(i, e, parser_ipv4_addr_with_mask,
                       qi::ascii::blank,
                       ipv4_addr_with_prefix);

    BOOST_CHECK(parse_success);
    BOOST_CHECK(i == e);

    BOOST_CHECK_EQUAL(ipv4_addr_with_prefix.addr(),
                      IPv4_Addr::from_string(ipv4_addr_string));
    BOOST_CHECK_EQUAL(ipv4_addr_with_prefix.cidr(), 24);
  }
}


BOOST_AUTO_TEST_CASE(test_parser_ipv6_addr_with_mask)
{
  Parser_IPv6_Addr_with_Mask<string::const_iterator> const
    parser_ipv6_addr_with_mask;

  if (true) {
    string const ipv6_addr_string{"2001:DB8:1::23:456:789"};
    string const ipv6_mask_string{"FFFF:FFFF:FFFF:FFFF:0:0:0:0"};
    string const ipv6_addr_with_mask_string{ipv6_addr_string + "/" + ipv6_mask_string};
    string::const_iterator
      i = ipv6_addr_with_mask_string.cbegin(),
      e = ipv6_addr_with_mask_string.cend();

    IPv6_Addr_with_Prefix ipv6_addr_with_prefix;

    bool const parse_success =
      qi::phrase_parse(i, e, parser_ipv6_addr_with_mask,
                       qi::ascii::blank,
                       ipv6_addr_with_prefix);

    BOOST_CHECK(parse_success);
    BOOST_CHECK(i == e);

    BOOST_CHECK_EQUAL(ipv6_addr_with_prefix.addr(),
                      IPv6_Addr::from_string(ipv6_addr_string));
    BOOST_CHECK_EQUAL(ipv6_addr_with_prefix.cidr(), 64);
  }
}

BOOST_AUTO_TEST_CASE(test_parser_ip_addr_with_mask)
{
  Parser_IP_Addr_with_Mask<string::const_iterator> const
    parser_ip_addr_with_mask;

  if (true) { // IPv4
    string const ip_addr_string{"192.0.2.1"};
    string const ip_mask_string{"255.255.255.0"};
    string const ip_addr_with_mask_string{ip_addr_string + "/" + ip_mask_string};
    string::const_iterator
      i = ip_addr_with_mask_string.cbegin(),
      e = ip_addr_with_mask_string.cend();

    IP_Addr_with_Prefix ip_addr_with_prefix;

    bool const parse_success =
      qi::phrase_parse(i, e, parser_ip_addr_with_mask,
                       qi::ascii::blank,
                       ip_addr_with_prefix);

    BOOST_CHECK(parse_success);
    BOOST_CHECK(i == e);

    BOOST_CHECK_EQUAL(ip_addr_with_prefix.addr(),
                      IP_Addr::from_string(ip_addr_string));
    BOOST_CHECK_EQUAL(ip_addr_with_prefix.cidr(), 24);
  }

  if (true) { // IPv6
    string const ip_addr_string{"2001:DB8:1::23:456:789"};
    string const ip_mask_string{"FFFF:FFFF:FFFF:FFFF:0:0:0:0"};
    string const ip_addr_with_mask_string{ip_addr_string + "/" + ip_mask_string};
    string::const_iterator
      i = ip_addr_with_mask_string.cbegin(),
      e = ip_addr_with_mask_string.cend();

    IP_Addr_with_Prefix ip_addr_with_prefix;

    bool const parse_success =
      qi::phrase_parse(i, e, parser_ip_addr_with_mask,
                       qi::ascii::blank,
                       ip_addr_with_prefix);

    BOOST_CHECK(parse_success);
    BOOST_CHECK(i == e);

    BOOST_CHECK_EQUAL(ip_addr_with_prefix.addr(),
                      IP_Addr::from_string(ip_addr_string));
    BOOST_CHECK_EQUAL(ip_addr_with_prefix.cidr(), 64);
  }
}

BOOST_AUTO_TEST_CASE(test_parser_ip_addr_with_cidr)
{
  Parser_IP_Addr_with_CIDR<string::const_iterator> const
    parser_ip_addr_with_cidr;

  if (true) { // IPv4
    string const ip_addr_string{"192.0.2.1"};
    string const ip_addr_with_cidr_string{ip_addr_string + "/16"};
    string::const_iterator
      i = ip_addr_with_cidr_string.cbegin(),
      e = ip_addr_with_cidr_string.cend();

    IP_Addr_with_Prefix ip_addr_with_prefix;

    bool const parse_success =
      qi::phrase_parse(i, e, parser_ip_addr_with_cidr,
                       qi::ascii::blank,
                       ip_addr_with_prefix);

    BOOST_CHECK(parse_success);
    BOOST_CHECK(i == e);

    BOOST_CHECK_EQUAL(ip_addr_with_prefix.addr(),
                      IP_Addr::from_string(ip_addr_string));
    BOOST_CHECK_EQUAL(ip_addr_with_prefix.cidr(), 16);
  }

  if (true) { // IPv6
    string const ip_addr_string{"2001:DB8:1::23:456:789"};
    string const ip_addr_with_cidr_string{ip_addr_string + "/48"};
    string::const_iterator
      i = ip_addr_with_cidr_string.cbegin(),
      e = ip_addr_with_cidr_string.cend();

    IP_Addr_with_Prefix ip_addr_with_prefix;

    bool const parse_success =
      qi::phrase_parse(i, e, parser_ip_addr_with_cidr,
                       qi::ascii::blank,
                       ip_addr_with_prefix);

    BOOST_CHECK(parse_success);
    BOOST_CHECK(i == e);

    BOOST_CHECK_EQUAL(ip_addr_with_prefix.addr(),
                      IP_Addr::from_string(ip_addr_string));
    BOOST_CHECK_EQUAL(ip_addr_with_prefix.cidr(), 48);
  }
}


BOOST_AUTO_TEST_CASE(test_parser_domain_name_wellformed)
{

}


BOOST_AUTO_TEST_CASE(test_parser_domain_name_malformed)
{

}
