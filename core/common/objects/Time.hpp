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

#ifndef TIME_HPP
#define TIME_HPP

#include <string>

#include <boost/date_time/posix_time/posix_time.hpp>

#include <netmeld/core/utils/FileManager.hpp>

namespace pt = boost::posix_time;


namespace netmeld::core::objects {

  class Time
  {
    // =========================================================================
    // Variables
    // =========================================================================
    private:
    protected:
      pt::ptime time;

    public:

    // =========================================================================
    // Constructors
    // =========================================================================
    private:
    protected:
    public:
      Time();
      explicit Time(pt::ptime);
      explicit Time(const std::string&);

    // =========================================================================
    // Methods
    // =========================================================================
    private:
    protected:
    public:
      void readUnixTimestamp(const std::string&);
      void readFormatted(const std::string&, const std::string&);
      void readTime(const sfs::path& p);

      bool isNull() const;

      std::string toString() const;
      std::string toIsoString() const;
      std::string toDebugString() const;

      friend bool operator<(const Time&, const Time&);
      friend std::ostream& operator<<(std::ostream&, const Time&);
      friend std::istream& operator>>(std::istream&, Time&);
  };
}

// pqxx mappings between nmco::Time and PostgresSQL TIMESTAMP
#include <pqxx/pqxx>
namespace pqxx {
  namespace nmco = netmeld::core::objects;

  template<>
  struct PQXX_LIBEXPORT string_traits<nmco::Time>
  {
    static const char* name();
    static bool has_null();
    static bool is_null(nmco::Time const& obj);
    static nmco::Time null();
    static void from_string(const char str[], nmco::Time& obj);
    static std::string to_string(nmco::Time const& obj);
  };
}

#endif // TIME_HPP
