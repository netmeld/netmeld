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

#include <netmeld/common/networking.hpp>

#include <string>


using std::string;


BOOST_AUTO_TEST_CASE(test_cidr_from_mask)
{
  BOOST_CHECK_EQUAL( 0, cidr_from_mask(IPv4_Addr::from_string("0.0.0.0")));
  BOOST_CHECK_EQUAL( 8, cidr_from_mask(IPv4_Addr::from_string("255.0.0.0")));
  BOOST_CHECK_EQUAL(16, cidr_from_mask(IPv4_Addr::from_string("255.255.0.0")));
  BOOST_CHECK_EQUAL(24, cidr_from_mask(IPv4_Addr::from_string("255.255.255.0")));
  BOOST_CHECK_EQUAL(32, cidr_from_mask(IPv4_Addr::from_string("255.255.255.255")));

  BOOST_CHECK_EQUAL( 4, cidr_from_mask(IPv4_Addr::from_string("240.0.0.0")));
  BOOST_CHECK_EQUAL( 9, cidr_from_mask(IPv4_Addr::from_string("255.128.0.0")));
  BOOST_CHECK_EQUAL(15, cidr_from_mask(IPv4_Addr::from_string("255.254.0.0")));
  BOOST_CHECK_EQUAL(20, cidr_from_mask(IPv4_Addr::from_string("255.255.240.0")));
  BOOST_CHECK_EQUAL(30, cidr_from_mask(IPv4_Addr::from_string("255.255.255.252")));

  BOOST_CHECK_EQUAL(48, cidr_from_mask(IPv6_Addr::from_string("ffff:ffff:ffff::")));
  BOOST_CHECK_EQUAL(64, cidr_from_mask(IPv6_Addr::from_string("ffff:ffff:ffff:ffff::")));
}


BOOST_AUTO_TEST_CASE(test_mask_from_cidr)
{
  BOOST_CHECK_EQUAL(mask_from_cidr<IPv4_Addr>(0),  IPv4_Addr::from_string("0.0.0.0"));
  BOOST_CHECK_EQUAL(mask_from_cidr<IPv4_Addr>(8),  IPv4_Addr::from_string("255.0.0.0"));
  BOOST_CHECK_EQUAL(mask_from_cidr<IPv4_Addr>(16), IPv4_Addr::from_string("255.255.0.0"));
  BOOST_CHECK_EQUAL(mask_from_cidr<IPv4_Addr>(24), IPv4_Addr::from_string("255.255.255.0"));
  BOOST_CHECK_EQUAL(mask_from_cidr<IPv4_Addr>(32), IPv4_Addr::from_string("255.255.255.255"));

  BOOST_CHECK_EQUAL(mask_from_cidr<IPv4_Addr>(4),  IPv4_Addr::from_string("240.0.0.0"));
  BOOST_CHECK_EQUAL(mask_from_cidr<IPv4_Addr>(9),  IPv4_Addr::from_string("255.128.0.0"));
  BOOST_CHECK_EQUAL(mask_from_cidr<IPv4_Addr>(15), IPv4_Addr::from_string("255.254.0.0"));
  BOOST_CHECK_EQUAL(mask_from_cidr<IPv4_Addr>(20), IPv4_Addr::from_string("255.255.240.0"));
  BOOST_CHECK_EQUAL(mask_from_cidr<IPv4_Addr>(30), IPv4_Addr::from_string("255.255.255.252"));

  BOOST_CHECK_EQUAL(mask_from_cidr<IPv6_Addr>(48), IPv6_Addr::from_string("ffff:ffff:ffff::"));
  BOOST_CHECK_EQUAL(mask_from_cidr<IPv6_Addr>(64), IPv6_Addr::from_string("ffff:ffff:ffff:ffff::"));
}

BOOST_AUTO_TEST_CASE(test_ip_addr_with_prefix_from_string)
{
  if (true) { // IPv4 mask
    string const ip_addr_string{"192.0.2.1"};
    string const ip_mask_string{"255.255.255.0"};
    string const ip_addr_with_mask_string{ip_addr_string + "/" + ip_mask_string};

    IP_Addr_with_Prefix ip_addr_with_prefix =
      IP_Addr_with_Prefix::from_string(ip_addr_with_mask_string);

    BOOST_CHECK_EQUAL(ip_addr_with_prefix.addr(),
                      IP_Addr::from_string(ip_addr_string));
    BOOST_CHECK_EQUAL(ip_addr_with_prefix.cidr(), 24);
  }
  if (true) { // IPv4 cidr
    string const ip_addr_string{"192.0.2.1"};
    string const ip_addr_with_cidr_string{ip_addr_string + "/16"};

    IP_Addr_with_Prefix ip_addr_with_prefix =
      IP_Addr_with_Prefix::from_string(ip_addr_with_cidr_string);

    BOOST_CHECK_EQUAL(ip_addr_with_prefix.addr(),
                      IP_Addr::from_string(ip_addr_string));
    BOOST_CHECK_EQUAL(ip_addr_with_prefix.cidr(), 16);
  }

  if (true) { // IPv6 mask
    string const ip_addr_string{"2001:DB8:1::23:456:789"};
    string const ip_mask_string{"FFFF:FFFF:FFFF:FFFF:0:0:0:0"};
    string const ip_addr_with_mask_string{ip_addr_string + "/" + ip_mask_string};
    
    IP_Addr_with_Prefix ip_addr_with_prefix =
      IP_Addr_with_Prefix::from_string(ip_addr_with_mask_string);

    BOOST_CHECK_EQUAL(ip_addr_with_prefix.addr(),
                      IP_Addr::from_string(ip_addr_string));
    BOOST_CHECK_EQUAL(ip_addr_with_prefix.cidr(), 64);
  }
  if (true) { // IPv6 cidr
    string const ip_addr_string{"2001:DB8:1::23:456:789"};
    string const ip_addr_with_cidr_string{ip_addr_string + "/48"};

    IP_Addr_with_Prefix ip_addr_with_prefix =
      IP_Addr_with_Prefix::from_string(ip_addr_with_cidr_string);

    BOOST_CHECK_EQUAL(ip_addr_with_prefix.addr(),
                      IP_Addr::from_string(ip_addr_string));
    BOOST_CHECK_EQUAL(ip_addr_with_prefix.cidr(), 48);
  }
}
