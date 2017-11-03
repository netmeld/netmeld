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

#ifndef NETMELD_QUERIES_COMMON_HPP
#define NETMELD_QUERIES_COMMON_HPP


#include <netmeld/common/cve.hpp>
#include <netmeld/common/networking.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/uuid/uuid.hpp>

#include <pqxx/pqxx>


void
db_prepare_common(pqxx::connection& db);


// pqxx mappings between some C++ types and PostgreSQL data types.

namespace pqxx {


template<>
struct PQXX_LIBEXPORT string_traits<CVE>
{
  static const char* name();
  static bool has_null();
  static bool is_null(CVE const&);
  static CVE null();
  static void from_string(const char str[], CVE& obj);
  static PGSTD::string to_string(CVE const& obj);
};


template<>
struct PQXX_LIBEXPORT string_traits<boost::posix_time::ptime>
{
  static const char* name();
  static bool has_null();
  static bool is_null(boost::posix_time::ptime const& obj);
  static boost::posix_time::ptime null();
  static void from_string(const char str[], boost::posix_time::ptime& obj);
  static PGSTD::string to_string(boost::posix_time::ptime const& obj);
};


template<>
struct PQXX_LIBEXPORT string_traits<boost::uuids::uuid>
{
  static const char* name();
  static bool has_null();
  static bool is_null(boost::uuids::uuid const& obj);
  static boost::uuids::uuid null();
  static void from_string(const char str[], boost::uuids::uuid& obj);
  static PGSTD::string to_string(boost::uuids::uuid const& obj);
};


// ----------------------------------------------------------------------

template<>
struct PQXX_LIBEXPORT string_traits<IPv4_Addr>
{
  static const char* name();
  static bool has_null();
  static bool is_null(IPv4_Addr const& obj);
  static IPv4_Addr null();
  static void from_string(const char str[], IPv4_Addr& obj);
  static PGSTD::string to_string(IPv4_Addr const& obj);
};

template<>
struct PQXX_LIBEXPORT string_traits<IPv6_Addr>
{
  static const char* name();
  static bool has_null();
  static bool is_null(IPv6_Addr const& obj);
  static IPv6_Addr null();
  static void from_string(const char str[], IPv6_Addr& obj);
  static PGSTD::string to_string(IPv6_Addr const& obj);
};

template<>
struct PQXX_LIBEXPORT string_traits<IP_Addr>
{
  static const char* name();
  static bool has_null();
  static bool is_null(IP_Addr const& obj);
  static IP_Addr null();
  static void from_string(const char str[], IP_Addr& obj);
  static PGSTD::string to_string(IP_Addr const& obj);
};


// ----------------------------------------------------------------------

template<>
struct PQXX_LIBEXPORT string_traits<IPv4_Addr_with_Prefix>
{
  static const char* name();
  static bool has_null();
  static bool is_null(IPv4_Addr_with_Prefix const& obj);
  static IPv4_Addr_with_Prefix null();
  static void from_string(const char str[], IPv4_Addr_with_Prefix& obj);
  static PGSTD::string to_string(IPv4_Addr_with_Prefix const& obj);
};

template<>
struct PQXX_LIBEXPORT string_traits<IPv6_Addr_with_Prefix>
{
  static const char* name();
  static bool has_null();
  static bool is_null(IPv6_Addr_with_Prefix const& obj);
  static IPv6_Addr_with_Prefix null();
  static void from_string(const char str[], IPv6_Addr_with_Prefix& obj);
  static PGSTD::string to_string(IPv6_Addr_with_Prefix const& obj);
};

template<>
struct PQXX_LIBEXPORT string_traits<IP_Addr_with_Prefix>
{
  static const char* name();
  static bool has_null();
  static bool is_null(IP_Addr_with_Prefix const& obj);
  static IP_Addr_with_Prefix null();
  static void from_string(const char str[], IP_Addr_with_Prefix& obj);
  static PGSTD::string to_string(IP_Addr_with_Prefix const& obj);
};


// ----------------------------------------------------------------------

}  // namespace pqxx


#endif  /* NETMELD_QUERIES_COMMON_HPP */
