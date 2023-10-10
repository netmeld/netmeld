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

#ifndef TIME_HPP
#define TIME_HPP

#include <compare>
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

      std::strong_ordering operator<=>(const Time&) const;
      bool operator==(const Time&) const;

      friend std::ostream& operator<<(std::ostream&, const Time&);
      friend std::istream& operator>>(std::istream&, Time&);
  };
}
#endif // TIME_HPP
