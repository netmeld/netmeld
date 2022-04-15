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

#include <netmeld/datastore/objects/Cve.hpp>
#include <netmeld/datastore/parsers/ParserHelper.hpp>
#include <netmeld/datastore/parsers/ParserCve.hpp>

namespace nmdp = netmeld::datastore::parsers;


namespace netmeld::datastore::objects {

  Cve::Cve()
  {}

  Cve::Cve(short _year, int _number) :
    year(_year),
    number(_number)
  {}

  Cve::Cve(const std::string& _cveId)
  {
    auto temp = nmdp::fromString<nmdp::ParserCve, Cve>(_cveId);
    year      = temp.year;
    number    = temp.number;
  }

  void
  Cve::setPort(const Port& _port)
  {
    port = _port;
  }

  void
  Cve::setPluginId(const unsigned int id)
  {
    pluginId = id;
  }

  short
  Cve::getYear() const {
    return year;
  }

  int
  Cve::getNumber() const {
    return number;
  }

  bool
  Cve::isValid() const
  {
    return (year != -1)
        && (number != -1)
        && port.isValid()
        && (pluginId != UINT_MAX)
        ;
  }

  void
  Cve::save(pqxx::transaction_base& t,
            const nmco::Uuid& toolRunId, const std::string& _deviceId)
  {
    if (!isValid()) {
      LOG_DEBUG << "Cve object is not saving: " << toDebugString()
                << std::endl;
      return;
    }

    port.save(t, toolRunId, _deviceId);

    t.exec_prepared("insert_raw_nessus_result_cve",
      toolRunId,
      port.getIpAddress().toString(),
      port.getProtocol(),
      port.getPort(),
      pluginId,
      *this);
  }

  std::string
  Cve::toString() const
  {
    std::ostringstream oss;
    oss << "CVE-" << year << "-" << number;
    return oss.str();
  }

  std::string
  Cve::toDebugString() const
  {
    std::ostringstream oss;

    oss << "[";
    oss << year << ", "
        << number << ", "
        << port.toDebugString() << ", "
        << pluginId;
    oss << "]";

    return oss.str();
  }

  std::partial_ordering
  Cve::operator<=>(const Cve& rhs) const
  {
    if (auto cmp = year <=> rhs.year; 0 != cmp) {
      return cmp;
    }
    if (auto cmp = number <=> rhs.number; 0 != cmp) {
      return cmp;
    }
    if (auto cmp = port <=> rhs.port; 0 != cmp) {
      return cmp;
    }
    return pluginId <=> rhs.pluginId;
  }

  bool
  Cve::operator==(const Cve& rhs) const
  {
    return 0 == operator<=>(rhs);
  }
}


// ----------------------------------------------------------------------
// nmdo::Cve <--> Postgresql CVE
// ----------------------------------------------------------------------
namespace pqxx {
  const char*
  string_traits<nmdo::Cve>::
  name()
  {
    return "netmeld::common::objects::Cve";
  }

  bool
  string_traits<nmdo::Cve>::
  has_null()
  {
    return false;
  }

  bool
  string_traits<nmdo::Cve>::
  is_null(nmdo::Cve const&)
  {
    return false;
  }

  nmdo::Cve
  string_traits<nmdo::Cve>::
  null()
  {
    internal::throw_null_conversion(name());
    return nmdo::Cve(0, 0);
  }

  void
  string_traits<nmdo::Cve>::
  from_string(const char str[], nmdo::Cve& obj)
  {
    obj = nmdo::Cve(str);
  }

  std::string
  string_traits<nmdo::Cve>::
  to_string(nmdo::Cve const& obj)
  {
    std::ostringstream oss;
    oss << '(' << obj.getYear() << ", " << obj.getNumber() << ')';
    return oss.str();
  }
}
