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

#ifndef PARSER_NETWORKING_HPP
#define PARSER_NETWORKING_HPP


#ifndef BOOST_SPIRIT_USE_PHOENIX_V3
#define BOOST_SPIRIT_USE_PHOENIX_V3
#endif

#include <netmeld/common/networking.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/adapted.hpp>

#include <array>
#include <bitset>
#include <string>
#include <tuple>
#include <vector>


// ----------------------------------------------------------------------
// Adapt the IPvX_Addr_with_Prefix types:
// ----------------------------------------------------------------------

IPv4_Addr_with_Prefix
ipv4_addr_with_prefix_ctor(IPv4_Addr const& addr, IPv4_Addr const& mask);

IPv4_Addr_with_Prefix
ipv4_addr_with_prefix_ctor(IPv4_Addr const& addr, uint16_t cidr = 32);

BOOST_PHOENIX_ADAPT_FUNCTION(
  IPv4_Addr_with_Prefix,
  ipv4_addr_with_prefix_ctor_,
  ipv4_addr_with_prefix_ctor,
  2)

BOOST_PHOENIX_ADAPT_FUNCTION(
  IPv4_Addr_with_Prefix,
  ipv4_addr_with_prefix_ctor_,
  ipv4_addr_with_prefix_ctor,
  1)


IPv6_Addr_with_Prefix
ipv6_addr_with_prefix_ctor(IPv6_Addr const& addr, IPv6_Addr const& mask);

IPv6_Addr_with_Prefix
ipv6_addr_with_prefix_ctor(IPv6_Addr const& addr, uint16_t cidr = 128);

BOOST_PHOENIX_ADAPT_FUNCTION(
  IPv6_Addr_with_Prefix,
  ipv6_addr_with_prefix_ctor_,
  ipv6_addr_with_prefix_ctor,
  2)

BOOST_PHOENIX_ADAPT_FUNCTION(
  IPv6_Addr_with_Prefix,
  ipv6_addr_with_prefix_ctor_,
  ipv6_addr_with_prefix_ctor,
  1)


// ----------------------------------------------------------------------
// Parsing MAC Addresses:
// ----------------------------------------------------------------------

template<typename InputIterator_T>
struct Parser_MAC_Addr :
  boost::spirit::qi::grammar<InputIterator_T, MAC_Addr()>
{
  ~Parser_MAC_Addr() = default;

  Parser_MAC_Addr();

  boost::spirit::qi::rule<InputIterator_T, MAC_Addr()>
  start;

  boost::spirit::qi::uint_parser<uint8_t, 16, 2, 2>
  hex_byte;
};


template<typename InputIterator_T>
Parser_MAC_Addr<InputIterator_T>::
Parser_MAC_Addr() :
  Parser_MAC_Addr::base_type(start),
  start(),
  hex_byte()
{
  namespace qi = boost::spirit::qi;

  start
    = qi::hold[hex_byte >> qi::repeat(5)[qi::lit(':') >> hex_byte]]
    | qi::hold[hex_byte >> qi::repeat(5)[qi::lit('-') >> hex_byte]]
    | qi::hold[hex_byte >> hex_byte >>
               qi::repeat(2)[qi::lit('.') >> hex_byte >> hex_byte]]
    | qi::hold[qi::attr(0)]
    ;
}


// ----------------------------------------------------------------------
// Parsing IPv4 Addresses:
// ----------------------------------------------------------------------

BOOST_PHOENIX_ADAPT_FUNCTION(
  IPv4_Addr,
  ipv4_addr_from_string_,
  IPv4_Addr::from_string,
  1)


template<typename InputIterator_T>
struct Parser_IPv4_Addr :
  boost::spirit::qi::grammar<InputIterator_T, IPv4_Addr()>
{
  ~Parser_IPv4_Addr() = default;

  Parser_IPv4_Addr();

  boost::spirit::qi::rule<InputIterator_T, IPv4_Addr()>
  start;

  boost::spirit::qi::rule<InputIterator_T, std::string()>
  ipv4_addr_string;

  boost::spirit::qi::rule<InputIterator_T, std::string()>
  octet;
};


template<typename InputIterator_T>
Parser_IPv4_Addr<InputIterator_T>::
Parser_IPv4_Addr() :
  Parser_IPv4_Addr::base_type(start),
  start(),
  ipv4_addr_string(),
  octet()
{
  namespace qi = boost::spirit::qi;

  start
    = ((ipv4_addr_string)[qi::_val = ipv4_addr_from_string_(qi::_1)])
    ;

  ipv4_addr_string
    = qi::hold[octet >> qi::repeat(3)[qi::hold[qi::char_('.') >> octet]]]
    ;

  // This will match three digit numbers larger than 255 (such as 999).
  // However, it's close enough for basic parsing requirements
  // and ipv4_addr_from_string_ will error on any invalid addresses.
  octet
    = (qi::repeat(1,3)[qi::ascii::digit])
    ;
}


// ----------------------------------------------------------------------

template<typename InputIterator_T>
struct Parser_IPv4_Addr_with_CIDR :
  boost::spirit::qi::grammar<InputIterator_T, IPv4_Addr_with_Prefix()>
{
  ~Parser_IPv4_Addr_with_CIDR() = default;

  Parser_IPv4_Addr_with_CIDR();

  boost::spirit::qi::rule<InputIterator_T, IPv4_Addr_with_Prefix()>
  start;

  Parser_IPv4_Addr<InputIterator_T>
  ipv4_addr;
};


template<typename InputIterator_T>
Parser_IPv4_Addr_with_CIDR<InputIterator_T>::
Parser_IPv4_Addr_with_CIDR() :
  Parser_IPv4_Addr_with_CIDR::base_type(start),
  start(),
  ipv4_addr()
{
  namespace qi = boost::spirit::qi;

  start
    = qi::hold[(ipv4_addr >> qi::lit('/') >> qi::ushort_)
               [qi::_val = ipv4_addr_with_prefix_ctor_(qi::_1, qi::_2)]]
    ;
}


// ----------------------------------------------------------------------

BOOST_PHOENIX_ADAPT_FUNCTION(
  uint16_t,
  ipv4_cidr_from_mask_,
  cidr_from_mask<IPv4_Addr>,
  1)


template<typename InputIterator_T>
struct Parser_IPv4_Addr_with_Mask :
  boost::spirit::qi::grammar<InputIterator_T, IPv4_Addr_with_Prefix()>
{
  ~Parser_IPv4_Addr_with_Mask() = default;

  Parser_IPv4_Addr_with_Mask();

  boost::spirit::qi::rule<InputIterator_T, IPv4_Addr_with_Prefix()>
  start;

  Parser_IPv4_Addr<InputIterator_T>
  ipv4_addr;

  boost::spirit::qi::rule<InputIterator_T, uint16_t()>
  ipv4_mask;
};


template<typename InputIterator_T>
Parser_IPv4_Addr_with_Mask<InputIterator_T>::
Parser_IPv4_Addr_with_Mask() :
  Parser_IPv4_Addr_with_Mask::base_type(start),
  start(),
  ipv4_addr()
{
  namespace qi = boost::spirit::qi;

  start
    = qi::hold[(ipv4_addr >> qi::lit('/') >> ipv4_mask)
               [qi::_val = ipv4_addr_with_prefix_ctor_(qi::_1, qi::_2)]]
    ;

  ipv4_mask
    = ((ipv4_addr)[qi::_val = ipv4_cidr_from_mask_(qi::_1)])
    ;
}


// ----------------------------------------------------------------------
// Parsing IPv6 Addresses:
// ----------------------------------------------------------------------

BOOST_PHOENIX_ADAPT_FUNCTION(
  IPv6_Addr,
  ipv6_addr_from_string_,
  IPv6_Addr::from_string,
  1)


template<typename InputIterator_T>
struct Parser_IPv6_Addr :
  boost::spirit::qi::grammar<InputIterator_T, IPv6_Addr()>
{
  ~Parser_IPv6_Addr() = default;

  Parser_IPv6_Addr();

  boost::spirit::qi::rule<InputIterator_T, IPv6_Addr()>
  start;

  boost::spirit::qi::rule<InputIterator_T, std::string()>
  ipv6_addr_string;

  boost::spirit::qi::rule<InputIterator_T, std::string()>
  h16;
};


template<typename InputIterator_T>
Parser_IPv6_Addr<InputIterator_T>::
Parser_IPv6_Addr() :
  Parser_IPv6_Addr::base_type(start),
  start(),
  ipv6_addr_string(),
  h16()
{
  namespace qi = boost::spirit::qi;

  start
    = ((ipv6_addr_string)[qi::_val = ipv6_addr_from_string_(qi::_1)])
    ;

  // This will match strings that aren't valid IPv6 addresses.
  // However, it's close enough for basic parsing requirements
  // and ipv6_addr_from_string_ will error on any invalid addresses.
  ipv6_addr_string
    = qi::hold[h16 >> qi::repeat(7)[qi::hold[qi::char_(':') >> h16]]]
    | qi::hold[qi::repeat(1,7)[qi::hold[h16 >> qi::char_(':')]] >>
               qi::repeat(1,7)[qi::hold[qi::char_(':') >> h16]]]
    | qi::hold[qi::repeat(1,7)[qi::hold[h16 >> qi::char_(':')]] >>
               qi::char_(':')]
    | qi::hold[qi::char_(':') >>
               qi::repeat(1,7)[qi::hold[qi::char_(':') >> h16]]]
    | qi::hold[qi::string("::")]
    ;

  h16
    = (qi::repeat(1,4)[qi::ascii::xdigit])
    ;
}


// ----------------------------------------------------------------------

BOOST_PHOENIX_ADAPT_FUNCTION(
  uint16_t,
  ipv6_cidr_from_mask_,
  cidr_from_mask<IPv6_Addr>,
  1)


template<typename InputIterator_T>
struct Parser_IPv6_Addr_with_CIDR :
  boost::spirit::qi::grammar<InputIterator_T, IPv6_Addr_with_Prefix()>
{
  ~Parser_IPv6_Addr_with_CIDR() = default;

  Parser_IPv6_Addr_with_CIDR();

  boost::spirit::qi::rule<InputIterator_T, IPv6_Addr_with_Prefix()>
  start;

  Parser_IPv6_Addr<InputIterator_T>
  ipv6_addr;
};


template<typename InputIterator_T>
Parser_IPv6_Addr_with_CIDR<InputIterator_T>::
Parser_IPv6_Addr_with_CIDR() :
  Parser_IPv6_Addr_with_CIDR::base_type(start),
  start(),
  ipv6_addr()
{
  namespace qi = boost::spirit::qi;

  start
    = qi::hold[(ipv6_addr >> qi::lit('/') >> qi::ushort_)
               [qi::_val = ipv6_addr_with_prefix_ctor_(qi::_1, qi::_2)]]
    ;
}


// ----------------------------------------------------------------------

template<typename InputIterator_T>
struct Parser_IPv6_Addr_with_Mask :
  boost::spirit::qi::grammar<InputIterator_T, IPv6_Addr_with_Prefix()>
{
  ~Parser_IPv6_Addr_with_Mask() = default;

  Parser_IPv6_Addr_with_Mask();

  boost::spirit::qi::rule<InputIterator_T, IPv6_Addr_with_Prefix()>
  start;

  Parser_IPv6_Addr<InputIterator_T>
  ipv6_addr;

  boost::spirit::qi::rule<InputIterator_T, uint16_t()>
  ipv6_mask;
};


template<typename InputIterator_T>
Parser_IPv6_Addr_with_Mask<InputIterator_T>::
Parser_IPv6_Addr_with_Mask() :
  Parser_IPv6_Addr_with_Mask::base_type(start),
  start(),
  ipv6_addr()
{
  namespace qi = boost::spirit::qi;

  start
    = qi::hold[(ipv6_addr >> qi::lit('/') >> ipv6_mask)
               [qi::_val = ipv6_addr_with_prefix_ctor_(qi::_1, qi::_2)]]
    ;

  ipv6_mask
    = ((ipv6_addr)[qi::_val = ipv6_cidr_from_mask_(qi::_1)])
    ;
}


// ----------------------------------------------------------------------
// Parsing IP Addresses (both IPv4 and IPv6):
// ----------------------------------------------------------------------

template<typename InputIterator_T>
struct Parser_IP_Addr :
  boost::spirit::qi::grammar<InputIterator_T, IP_Addr()>
{
  ~Parser_IP_Addr() = default;

  Parser_IP_Addr();

  boost::spirit::qi::rule<InputIterator_T, IP_Addr()>
  start;

  Parser_IPv4_Addr<InputIterator_T>
  ipv4_addr;

  Parser_IPv6_Addr<InputIterator_T>
  ipv6_addr;
};


template<typename InputIterator_T>
Parser_IP_Addr<InputIterator_T>::
Parser_IP_Addr() :
  Parser_IP_Addr::base_type(start),
  start(),
  ipv4_addr(),
  ipv6_addr()
{
  namespace qi = boost::spirit::qi;

  start
    = (ipv4_addr)
    | (ipv6_addr)
    ;
}


// ----------------------------------------------------------------------

template<typename InputIterator_T>
struct Parser_IP_Addr_with_CIDR :
  boost::spirit::qi::grammar<InputIterator_T, IP_Addr_with_Prefix()>
{
  ~Parser_IP_Addr_with_CIDR() = default;

  Parser_IP_Addr_with_CIDR();

  boost::spirit::qi::rule<InputIterator_T, IP_Addr_with_Prefix()>
  start;

  Parser_IPv4_Addr_with_CIDR<InputIterator_T>
  ipv4_addr_with_cidr;

  Parser_IPv6_Addr_with_CIDR<InputIterator_T>
  ipv6_addr_with_cidr;
};


template<typename InputIterator_T>
Parser_IP_Addr_with_CIDR<InputIterator_T>::
Parser_IP_Addr_with_CIDR() :
  Parser_IP_Addr_with_CIDR::base_type(start),
  start(),
  ipv4_addr_with_cidr(),
  ipv6_addr_with_cidr()
{
  namespace qi = boost::spirit::qi;

  start
    = (ipv4_addr_with_cidr)
    | (ipv6_addr_with_cidr)
    ;
}


// ----------------------------------------------------------------------

template<typename InputIterator_T>
struct Parser_IP_Addr_with_Mask :
  boost::spirit::qi::grammar<InputIterator_T, IP_Addr_with_Prefix()>
{
  ~Parser_IP_Addr_with_Mask() = default;

  Parser_IP_Addr_with_Mask();

  boost::spirit::qi::rule<InputIterator_T, IP_Addr_with_Prefix()>
  start;

  Parser_IPv4_Addr_with_Mask<InputIterator_T>
  ipv4_addr_with_mask;

  Parser_IPv6_Addr_with_Mask<InputIterator_T>
  ipv6_addr_with_mask;
};


template<typename InputIterator_T>
Parser_IP_Addr_with_Mask<InputIterator_T>::
Parser_IP_Addr_with_Mask() :
  Parser_IP_Addr_with_Mask::base_type(start),
  start(),
  ipv4_addr_with_mask(),
  ipv6_addr_with_mask()
{
  namespace qi = boost::spirit::qi;

  start
    = (ipv4_addr_with_mask)
    | (ipv6_addr_with_mask)
    ;
}


// ----------------------------------------------------------------------
// Parsing Domain Names:
// ----------------------------------------------------------------------

template<typename InputIterator_T>
struct Parser_Domain_Name :
  boost::spirit::qi::grammar<InputIterator_T, std::string()>
{
  ~Parser_Domain_Name() = default;

  Parser_Domain_Name();

  boost::spirit::qi::rule<InputIterator_T, std::string()>
  start;

  boost::spirit::qi::rule<InputIterator_T, std::string()>
  label;
};


template<typename InputIterator_T>
Parser_Domain_Name<InputIterator_T>::
Parser_Domain_Name() :
  Parser_Domain_Name::base_type(start),
  start(),
  label()
{
  namespace qi = boost::spirit::qi;

  // A domain name is between 1 and 127 labels, each separated by a period.
  start
    = (label >> qi::repeat(0,126)[qi::char_('.') >> label])
    ;

  // Each label must be between 1 and 63 characters long.
  // Labels may only contain alphanumeric characters, hyphens, and underscores.
  // Labels must not begin or end with a hyphen (not enforced here).
  label
    = (qi::repeat(1,63)[qi::ascii::alnum | qi::char_('_') | qi::char_('-')])
    ;
}


// ----------------------------------------------------------------------


#endif  /* PARSER_NETWORKING_HPP */
