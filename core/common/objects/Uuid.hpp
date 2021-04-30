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

#ifndef UUID_HPP
#define UUID_HPP

#include <compare>
#include <string>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <netmeld/core/utils/FileManager.hpp>

namespace uuids = boost::uuids;


namespace netmeld::core::objects {

  class Uuid
  {
    // =========================================================================
    // Variables
    // =========================================================================
    private:
    protected:
      uuids::uuid uuid;

    public:

    // =========================================================================
    // Constructors
    // =========================================================================
    private:
    protected:
    public:
      Uuid();
      explicit Uuid(const uuids::uuid&);
      explicit Uuid(const std::string&);

    // =========================================================================
    // Methods
    // =========================================================================
    private:
    protected:
    public:
      void readUuid(const sfs::path& p);

      bool isNull() const;

      std::string toString() const;
      std::string toDebugString() const;

      std::strong_ordering operator<=>(const Uuid&) const;
      bool operator==(const Uuid&) const;

      friend std::ostream& operator<<(std::ostream&, const Uuid&);
  };
}

// pqxx mappings between nmco::Uuid and PostgresSQL UUID
#include <pqxx/pqxx>
namespace pqxx {
  namespace nmco = netmeld::core::objects;

  template<>
  struct PQXX_LIBEXPORT string_traits<nmco::Uuid>
  {
    static const char* name();
    static bool has_null();
    static bool is_null(const nmco::Uuid& obj);
    static nmco::Uuid null();
    static void from_string(const char str[], nmco::Uuid& obj);
    static std::string to_string(const nmco::Uuid& obj);
  };
}

#endif // UUID_HPP
