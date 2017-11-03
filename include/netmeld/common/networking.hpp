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


#ifndef NETWORKING_HPP
#define NETWORKING_HPP


#include <boost/asio/ip/address.hpp>
#include <boost/operators.hpp>

#include <cstdint>
#include <vector>


using MAC_Addr  = std::vector<uint8_t>;

using IPv4_Addr = boost::asio::ip::address_v4;
using IPv6_Addr = boost::asio::ip::address_v6;
using IP_Addr   = boost::asio::ip::address;


// This function template works with IPv4_Addr and IPv6_Addr types.
template<typename T>
uint16_t
cidr_from_mask(T const& mask);

// This function template works with IPv4_Addr and IPv6_Addr types.
template<typename T>
T
mask_from_cidr(uint16_t const cidr);


template<typename T>
class IPvX_Addr_with_Prefix :
  boost::totally_ordered<IPvX_Addr_with_Prefix<T> >
{
public:
  virtual ~IPvX_Addr_with_Prefix() = default;

  IPvX_Addr_with_Prefix();
  IPvX_Addr_with_Prefix(T const& a, T const& m);
  IPvX_Addr_with_Prefix(T const& a, uint16_t c);
  IPvX_Addr_with_Prefix(T const& a);

  std::string to_string() const;

  static
  IPvX_Addr_with_Prefix<T> from_string(std::string const& s);

  T addr() const;
  T mask() const;
  uint16_t cidr() const;

  void set_addr(T const& a);
  void set_mask(T const& m);
  void set_cidr(uint16_t c);

  bool operator==(IPvX_Addr_with_Prefix<T> const& other) const;
  bool operator<(IPvX_Addr_with_Prefix<T> const& other) const;

private:
  T addr_;
  uint16_t cidr_;
};


using IPv4_Addr_with_Prefix = IPvX_Addr_with_Prefix<IPv4_Addr>;
using IPv6_Addr_with_Prefix = IPvX_Addr_with_Prefix<IPv6_Addr>;


class IP_Addr_with_Prefix :
  boost::totally_ordered<IP_Addr_with_Prefix>
{
public:
  virtual ~IP_Addr_with_Prefix() = default;

  IP_Addr_with_Prefix();

  IP_Addr_with_Prefix(IPv4_Addr_with_Prefix const& other);
  IP_Addr_with_Prefix(IPv6_Addr_with_Prefix const& other);

  std::string to_string() const;

  static
  IP_Addr_with_Prefix from_string(std::string const& s);

  IP_Addr addr() const;
  //IP_Addr mask() const;
  uint16_t cidr() const;

  void set_addr(IP_Addr const& a);
  //void set_mask(IP_Addr const& m);
  void set_cidr(uint16_t c);

  bool operator==(IP_Addr_with_Prefix const& other) const;
  bool operator<(IP_Addr_with_Prefix const& other) const;

private:
  IP_Addr addr_;
  uint16_t cidr_;
};


#include <netmeld/common/impl/networking.ipp>


#endif  /* NETWORKING_HPP */
