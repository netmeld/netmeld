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

#ifndef PORTRANGE_HPP
#define PORTRANGE_HPP

#include <cstdint>
#include <string>
#include <tuple>


namespace netmeld::datastore::objects {

  class PortRange : public std::tuple<uint16_t, uint16_t>
  {
    // =========================================================================
    // Variables
    // =========================================================================
    private:
    protected:
    public:

    // =========================================================================
    // Constructors
    // =========================================================================
    private:
    protected:
    public:
      explicit PortRange(uint16_t);
      explicit PortRange(uint16_t, uint16_t);
      explicit PortRange(const std::string&);

    // =========================================================================
    // Methods
    // =========================================================================
    private:
    protected:
    public:
      std::string toString() const;
      std::string toDebugString() const;
      std::string toHumanString() const;

      friend std::ostream& operator<<(std::ostream&, const PortRange&);
  };
}


// ----------------------------------------------------------------------
// nmdo:PortRange <--> PostgreSQL PortRange
// ----------------------------------------------------------------------
#include <pqxx/pqxx>

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
#endif // PORTRANGE_HPP
