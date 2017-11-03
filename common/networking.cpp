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

#ifndef BOOST_SPIRIT_USE_PHOENIX_V3
#define BOOST_SPIRIT_USE_PHOENIX_V3
#endif

#include <netmeld/common/networking.hpp>
#include <netmeld/common/parser_networking.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/adapted.hpp>

#include <bitset>
#include <stdexcept>

#include <iostream>
using std::cout;
using std::endl;
using std::cerr;

using std::bitset;
using std::runtime_error;
using std::string;


// ----------------------------------------------------------------------

template<typename T>
uint16_t
cidr_from_mask(T const& mask)
{
  size_t cidr = 0;
  for (uint8_t b : mask.to_bytes()) {
    cidr += bitset<8>(b).count();
  }
  return static_cast<uint16_t>(cidr);
}


// Explicit instantiations
template uint16_t cidr_from_mask<IPv4_Addr>(IPv4_Addr const& mask);
template uint16_t cidr_from_mask<IPv6_Addr>(IPv6_Addr const& mask);


// ----------------------------------------------------------------------

template<typename T>
IPvX_Addr_with_Prefix<T>::
IPvX_Addr_with_Prefix() :
  addr_(),
  cidr_(0)
{

}


template<typename T>
IPvX_Addr_with_Prefix<T>::
IPvX_Addr_with_Prefix(T const& a, T const& m) :
  addr_(a),
  cidr_(cidr_from_mask<T>(m))
{

}


template<typename T>
IPvX_Addr_with_Prefix<T>::
IPvX_Addr_with_Prefix(T const& a, uint16_t c) :
  addr_(a),
  cidr_(c)
{

}


template<>
IPvX_Addr_with_Prefix<IPv4_Addr>::
IPvX_Addr_with_Prefix(IPv4_Addr const& a) :
  addr_(a),
  cidr_(32)
{

}


template<>
IPvX_Addr_with_Prefix<IPv6_Addr>::
IPvX_Addr_with_Prefix(IPv6_Addr const& a) :
  addr_(a),
  cidr_(128)
{

}


template<typename T>
string
IPvX_Addr_with_Prefix<T>::
to_string() const
{
  string result = addr_.to_string();
  result += '/';
  result += std::to_string(static_cast<uint32_t>(cidr_));

  return result;
}


template<>
IPvX_Addr_with_Prefix<IPv4_Addr>
IPvX_Addr_with_Prefix<IPv4_Addr>::
from_string(string const& s)
{
  namespace qi = boost::spirit::qi;

  Parser_IPv4_Addr_with_CIDR<string::const_iterator> const
    parser_ipv4_addr_with_cidr;

  Parser_IPv4_Addr_with_Mask<string::const_iterator> const
    parser_ipv4_addr_with_mask;

  string::const_iterator
    i = s.cbegin(),
    e = s.cend();

  IPv4_Addr_with_Prefix ipv4_addr_with_prefix;

  bool const parse_success =
      qi::phrase_parse(i, e,
                       parser_ipv4_addr_with_cidr |
                       parser_ipv4_addr_with_mask,
                       qi::ascii::blank,
                       ipv4_addr_with_prefix);

  if ((!parse_success) || (i != e)) {
    throw runtime_error("Error parsing IPv4_Addr_with_Prefix");
  }

  return ipv4_addr_with_prefix;
}


template<>
IPvX_Addr_with_Prefix<IPv6_Addr>
IPvX_Addr_with_Prefix<IPv6_Addr>::
from_string(string const& s)
{
  namespace qi = boost::spirit::qi;

  Parser_IPv6_Addr_with_CIDR<string::const_iterator> const
    parser_ipv6_addr_with_cidr;

  Parser_IPv6_Addr_with_Mask<string::const_iterator> const
    parser_ipv6_addr_with_mask;

  string::const_iterator
    i = s.cbegin(),
    e = s.cend();

  IPv6_Addr_with_Prefix ipv6_addr_with_prefix;

  bool const parse_success =
      qi::phrase_parse(i, e,
                       parser_ipv6_addr_with_cidr |
                       parser_ipv6_addr_with_mask,
                       qi::ascii::blank,
                       ipv6_addr_with_prefix);

  if ((!parse_success) || (i != e)) {
    throw runtime_error("Error parsing IPv6_Addr_with_Prefix");
  }

  return ipv6_addr_with_prefix;
}


// Explicit instantiations
template class IPvX_Addr_with_Prefix<IPv4_Addr>;
template class IPvX_Addr_with_Prefix<IPv6_Addr>;


// ----------------------------------------------------------------------

IP_Addr_with_Prefix::
IP_Addr_with_Prefix() :
  addr_(),
  cidr_(0)
{

}


IP_Addr_with_Prefix::
IP_Addr_with_Prefix(IPv4_Addr_with_Prefix const& other) :
  addr_(other.addr()),
  cidr_(other.cidr())
{

}


IP_Addr_with_Prefix::
IP_Addr_with_Prefix(IPv6_Addr_with_Prefix const& other) :
  addr_(other.addr()),
  cidr_(other.cidr())
{

}


string
IP_Addr_with_Prefix::
to_string() const
{
  string result = addr_.to_string();
  result += '/';
  result += std::to_string(static_cast<uint32_t>(cidr_));

  return result;
}


IP_Addr_with_Prefix
IP_Addr_with_Prefix::
from_string(string const& s)
{
  namespace qi = boost::spirit::qi;

  Parser_IP_Addr_with_CIDR<string::const_iterator> const
    parser_ip_addr_with_cidr;

  Parser_IP_Addr_with_Mask<string::const_iterator> const
    parser_ip_addr_with_mask;

  string::const_iterator
    i = s.cbegin(),
    e = s.cend();

  IP_Addr_with_Prefix ip_addr_with_prefix;

  bool const parse_success =
      qi::phrase_parse(i, e,
                       parser_ip_addr_with_mask |
                       parser_ip_addr_with_cidr,
                       qi::ascii::blank,
                       ip_addr_with_prefix);

  if ((!parse_success) || (i != e)) {
    throw runtime_error("Error parsing IP_Addr_with_Prefix");
  }

  return ip_addr_with_prefix;
}


// ----------------------------------------------------------------------
