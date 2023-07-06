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

#include <netmeld/core/objects/Time.hpp>


namespace netmeld::core::objects {
  Time::Time() :
    time {pt::microsec_clock::universal_time()}
  { }

  Time::Time(pt::ptime ptTime) :
    time {pt::ptime(ptTime)}
  { }

  Time::Time(const std::string& strTime)
  {
    if (std::string("-infinity") == strTime) {
      time = pt::ptime(pt::neg_infin);
      return;
    }

    if (std::string("infinity") == strTime) {
      time = pt::ptime(pt::pos_infin);
      return;
    }

    time = pt::time_from_string(strTime);
  }

  void
  Time::readUnixTimestamp(const std::string& toRead)
  {
    time = pt::from_time_t(boost::lexical_cast<time_t>(toRead));
  }

  void
  Time::readFormatted(const std::string& toRead, const std::string& format)
  {
    pt::time_input_facet* facet = new pt::time_input_facet(format);
    std::stringstream ss;
    ss.imbue(std::locale(std::locale(), facet));
    ss.str(toRead);
    ss >> time;
  }

  void
  Time::readTime(const sfs::path& p)
  {
    std::ifstream f{p.string()};
    std::string s;
    getline(f, s);
    f.close();
    time = boost::date_time::parse_delimited_time<pt::ptime>(s, 'T');
  }

  bool
  Time::isNull() const
  {
    return time.is_not_a_date_time();
  }

  std::string
  Time::toString() const
  {
    if (time.is_neg_infinity()) {
      return "-infinity";
    }

    if (time.is_pos_infinity()) {
      return "infinity";
    }

    return pt::to_iso_extended_string(time);
  }

  std::string
  Time::toIsoString() const
  {
    if (time.is_neg_infinity()) {
      return "-infinity";
    }

    if (time.is_pos_infinity()) {
      return "infinity";
    }

    return pt::to_iso_string(time);
  }

  std::string
  Time::toDebugString() const
  {
    std::ostringstream oss;

    oss << "["
        << "time: " << time
        << "]";

    return oss.str();
  }

  std::strong_ordering
  Time::operator<=>(const Time& rhs) const
  {
    // boost::posix_time::ptime doesn't have operator<=>() yet.
    if (time < rhs.time) {
      return std::strong_ordering::less;
    }
    if (time > rhs.time) {
      return std::strong_ordering::greater;
    }
    return std::strong_ordering::equal;
  }

  bool
  Time::operator==(const Time& rhs) const
  {
    return 0 == operator<=>(rhs);
  }

  std::ostream&
  operator<<(std::ostream& os, const Time& t1)
  {
    return os << t1.toString();
  }

  std::istream&
  operator>>(std::istream& is, Time& t1)
  {
    pt::time_input_facet facet;
    facet.set_iso_extended_format();
    is.imbue(std::locale(is.getloc(), &facet));
    is >> t1.time;
    return is;
  }
}


// ----------------------------------------------------------------------
// nmco::Time <--> Postgresql TIMESTAMP
// ----------------------------------------------------------------------
namespace pqxx {
  const char*
  string_traits<nmco::Time>::
  name()
  {
    return "nmco::Time";
  }


  bool
  string_traits<nmco::Time>::
  has_null()
  {
    return true;
  }


  bool
  string_traits<nmco::Time>::
  is_null(nmco::Time const& obj)
  {
    return obj.isNull();
  }


  nmco::Time
  string_traits<nmco::Time>::
  null()
  {
    return nmco::Time(pt::not_a_date_time);
  }


  void
  string_traits<nmco::Time>::
  from_string(const char str[], nmco::Time& obj)
  {
    obj = nmco::Time(std::string(str));
  }


  std::string
  string_traits<nmco::Time>::
  to_string(nmco::Time const& obj)
  {
    return obj.toString();
  }

}
