// =============================================================================
// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
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

#ifndef NETMELD_POSTGRES_CONVERSIONS_HPP
#define NETMELD_POSTGRES_CONVERSIONS_HPP

#include <pqxx/pqxx>


// ======================================================================
// Core type conversions
// ======================================================================
namespace netmeld::core::objects {
  class Uuid;
  class Time;
}

// ----------------------------------------------------------------------
// nmco:Uuid <--> PostgreSQL UUID
// ----------------------------------------------------------------------
namespace pqxx {
  namespace nmco = netmeld::core::objects;

  template<> std::string const type_name<nmco::Uuid>  {"nmco::Uuid"};

  template<> inline constexpr bool is_unquoted_safe<nmco::Uuid> {true};

  template<> struct nullness<nmco::Uuid>
  {
    static constexpr bool has_null    {true};
    static constexpr bool always_null {false};
    static bool is_null(const nmco::Uuid&);
    [[nodiscard]] static nmco::Uuid null();
  };

  template<> struct string_traits<nmco::Uuid>
  {
    static constexpr bool converts_to_string    {true};
    static constexpr bool converts_from_string  {true};
    static zview to_buf(char*, char*, const nmco::Uuid&);
    static char* into_buf(char*, char*, const nmco::Uuid&);
    static std::size_t size_buffer(const nmco::Uuid&) noexcept;
    static nmco::Uuid from_string(std::string_view);
  };
}


// ----------------------------------------------------------------------
// nmco:Time <--> PostgreSQL TIMESTAMP
// ----------------------------------------------------------------------
namespace pqxx {
  namespace nmco = netmeld::core::objects;

  template<> std::string const type_name<nmco::Time>  {"nmco::Time"};

  template<> inline constexpr bool is_unquoted_safe<nmco::Time> {true};

  template<> struct nullness<nmco::Time>
  {
    static constexpr bool has_null    {true};
    static constexpr bool always_null {false};
    static bool is_null(const nmco::Time&);
    [[nodiscard]] static nmco::Time null();
  };

  template<> struct string_traits<nmco::Time>
  {
    static constexpr bool converts_to_string    {true};
    static constexpr bool converts_from_string  {true};
    static zview to_buf(char*, char*, const nmco::Time&);
    static char* into_buf(char*, char*, const nmco::Time&);
    static std::size_t size_buffer(const nmco::Time&) noexcept;
    static nmco::Time from_string(std::string_view);
  };
}


// ======================================================================
// Datastore type conversions
// ======================================================================
namespace netmeld::datastore::objects {
  class Cve;
  class PortRange;
}

// ----------------------------------------------------------------------
// nmdo:Cve <--> PostgreSQL CVE
// ----------------------------------------------------------------------
namespace pqxx {
  namespace nmdo = netmeld::datastore::objects;

  template<> std::string const type_name<nmdo::Cve>  {"nmdo::Cve"};

  template<> inline constexpr bool is_unquoted_safe<nmdo::Cve> {false};

  template<> struct nullness<nmdo::Cve> : pqxx::no_null<nmdo::Cve> {};

  template<> struct string_traits<nmdo::Cve>
  {
    static constexpr bool converts_to_string    {true};
    static constexpr bool converts_from_string  {true};
    static zview to_buf(char*, char*, const nmdo::Cve&);
    static char* into_buf(char*, char*, const nmdo::Cve&);
    static std::size_t size_buffer(const nmdo::Cve&) noexcept;
    static nmdo::Cve from_string(std::string_view);
  };
}


// ----------------------------------------------------------------------
// nmdo:PortRange <--> PostgreSQL PortRange
// ----------------------------------------------------------------------
namespace pqxx {
  namespace nmdo = netmeld::datastore::objects;

  template<> std::string const type_name<nmdo::PortRange>  {"nmdo::PortRange"};

  template<> inline constexpr bool is_unquoted_safe<nmdo::PortRange> {false};

  template<> struct nullness<nmdo::PortRange> : pqxx::no_null<nmdo::PortRange> {};

  template<> struct string_traits<nmdo::PortRange>
  {
    static constexpr bool converts_to_string    {true};
    static constexpr bool converts_from_string  {true};
    static zview to_buf(char*, char*, const nmdo::PortRange&);
    static char* into_buf(char*, char*, const nmdo::PortRange&);
    static std::size_t size_buffer(const nmdo::PortRange&) noexcept;
    static nmdo::PortRange from_string(std::string_view);
  };
}
#endif  /* NETMELD_POSTGRES_CONVERSIONS_HPP */
