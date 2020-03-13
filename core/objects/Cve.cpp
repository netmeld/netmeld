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

#include <netmeld/core/objects/Cve.hpp>
#include <netmeld/core/parsers/ParserHelper.hpp>
#include <netmeld/core/parsers/ParserCve.hpp>

namespace nmcp = netmeld::core::parsers;


namespace netmeld::core::objects {

  Cve::Cve()
  {}

  Cve::Cve(short _year, int _number) :
    year(_year),
    number(_number)
  {}

  Cve::Cve(const std::string& _cveId)
  {
    auto temp = nmcp::fromString<nmcp::ParserCve, Cve>(_cveId);
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
            const Uuid& toolRunId, const std::string& _deviceId)
  {
    if (!isValid()) {
      LOG_DEBUG << "Cve object is not saving: " << toDebugString()
                << std::endl;
      return;
    }

    port.save(t, toolRunId, _deviceId);

    t.exec_prepared("insert_raw_nessus_result_cve",
      toolRunId,
      port.getIpAddr(),
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

  bool
  operator==(const Cve& first, const Cve& second)
  {
    return first.year == second.year
        && first.number == second.number
        && first.port == second.port
        && first.pluginId == second.pluginId
        ;
  }
}


// ----------------------------------------------------------------------
// nmco::Cve <--> Postgresql CVE
// ----------------------------------------------------------------------
namespace pqxx {
  const char*
  string_traits<nmco::Cve>::
  name()
  {
    return "netmeld::common::objects::Cve";
  }

  bool
  string_traits<nmco::Cve>::
  has_null()
  {
    return false;
  }

  bool
  string_traits<nmco::Cve>::
  is_null(nmco::Cve const&)
  {
    return false;
  }

  nmco::Cve
  string_traits<nmco::Cve>::
  null()
  {
    internal::throw_null_conversion(name());
    return nmco::Cve(0, 0);
  }

  void
  string_traits<nmco::Cve>::
  from_string(const char str[], nmco::Cve& obj)
  {
    obj = nmco::Cve(str);
  }

  std::string
  string_traits<nmco::Cve>::
  to_string(nmco::Cve const& obj)
  {
    std::ostringstream oss;
    oss << '(' << obj.getYear() << ", " << obj.getNumber() << ')';
    return oss.str();
  }
}
