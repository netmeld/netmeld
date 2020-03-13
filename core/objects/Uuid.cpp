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

#include <netmeld/core/objects/Uuid.hpp>


namespace netmeld::core::objects {

  Uuid::Uuid() :
    uuid {uuids::random_generator()()}
  { }

  Uuid::Uuid(const boost::uuids::uuid& bUuid) :
    uuid {uuids::uuid(bUuid)}
  { }

  Uuid::Uuid(const std::string& strUuid) :
    uuid {uuids::string_generator()(strUuid)}
  { }

  void
  Uuid::readUuid(const sfs::path& p)
  {
    std::ifstream f{p};
    f >> uuid;
    f.close();
  }

  bool
  Uuid::isNull() const
  {
    return uuid.is_nil();
  }

  std::string
  Uuid::toString() const
  {
    return uuids::to_string(uuid);
  }

  std::string
  Uuid::toDebugString() const
  {
    std::ostringstream oss;

    oss << "[";
    oss << uuid;
    oss << "]";

    return oss.str();
  }

  bool
  operator==(const Uuid& first, const Uuid& second)
  {
    return first.uuid == second.uuid;
  }

  std::ostream&
  operator<<(std::ostream& os, const Uuid& ao)
  {
    return os << ao.toString();
  }
}


// ----------------------------------------------------------------------
// nmco:Uuid <--> Postgresql UUID
// ----------------------------------------------------------------------
namespace pqxx {
  const char*
  string_traits<nmco::Uuid>::
  name()
  {
    return "nmco::Uuid";
  }


  bool
  string_traits<nmco::Uuid>::
  has_null()
  {
    return true;
  }


  bool
  string_traits<nmco::Uuid>::
  is_null(const nmco::Uuid& obj)
  {
    return obj.isNull();
  }


  nmco::Uuid
  string_traits<nmco::Uuid>::
  null()
  {
    return nmco::Uuid(uuids::nil_uuid());
  }


  void
  string_traits<nmco::Uuid>::
  from_string(const char str[], nmco::Uuid& obj)
  {
    obj = nmco::Uuid(std::string(str));
  }


  std::string
  string_traits<nmco::Uuid>::
  to_string(const nmco::Uuid& obj)
  {
    return obj.toString();
  }
}
