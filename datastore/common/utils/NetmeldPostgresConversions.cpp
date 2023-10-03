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

#include <netmeld/datastore/utils/NetmeldPostgresConversions.hpp>


// ----------------------------------------------------------------------
// nmco:Uuid <--> PostgreSQL UUID
// ----------------------------------------------------------------------
namespace pqxx {
  bool
  nullness<nmco::Uuid>::is_null(const nmco::Uuid& obj)
  {
    return obj.isNull();
  }

  [[nodiscard]] nmco::Uuid
  nullness<nmco::Uuid>::null()
  {
    return nmco::Uuid(uuids::nil_uuid());
  }

  zview
  string_traits<nmco::Uuid>::to_buf(char* begin, char* end, const nmco::Uuid& obj)
  {
    return generic_to_buf(begin, end, obj);
  }
  
  char*
  string_traits<nmco::Uuid>::into_buf(char* begin, char* end, const nmco::Uuid& obj)
  {
    const std::string value {obj.toString()};
    if (internal::cmp_greater_equal(std::size(value), end - begin))
      throw conversion_overrun{
        "Could not convert nmco::Uuid to string: too long for buffer."};
    value.copy(begin, std::size(value));
    begin[std::size(value)] = '\0';
    return begin + std::size(value) + 1;
  }

  std::size_t
  string_traits<nmco::Uuid>::size_buffer(const nmco::Uuid&) noexcept
  {
    return 37 // hex chars
         + 4  // dash
         + 1  // zero-terminator
         ;
  }

  nmco::Uuid
  string_traits<nmco::Uuid>::from_string(std::string_view text)
  {
    return nmco::Uuid(std::string(text));
  }
}


// ----------------------------------------------------------------------
// nmco:Time <--> PostgreSQL TIMESTAMP
// ----------------------------------------------------------------------
namespace pqxx {
  bool
  nullness<nmco::Time>::is_null(const nmco::Time& obj)
  {
    return obj.isNull();
  }

  [[nodiscard]] nmco::Time
  nullness<nmco::Time>::null()
  {
    return nmco::Time(pt::not_a_date_time);
  }

  zview
  string_traits<nmco::Time>::to_buf(char* begin, char* end, const nmco::Time& obj)
  {
    return generic_to_buf(begin, end, obj);
  }

  char*
  string_traits<nmco::Time>::into_buf(char* begin, char* end, const nmco::Time& obj)
  {
    const std::string value {obj.toString()};
    if (internal::cmp_greater_equal(std::size(value), end - begin))
      throw conversion_overrun{
        "Could not convert nmco::Time to string: too long for buffer."};
    value.copy(begin, std::size(value));
    begin[std::size(value)] = '\0';
    return begin + std::size(value) + 1;
  }

  std::size_t
  string_traits<nmco::Time>::size_buffer(const nmco::Time& obj) noexcept
  {
    return std::size(obj.toString())
         + 1 // zero-terminator
         ;
  }

  nmco::Time
  string_traits<nmco::Time>::from_string(std::string_view text)
  {
    return nmco::Time(std::string(text));
  }
}
