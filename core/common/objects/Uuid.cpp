// =============================================================================
// Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC
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

  Uuid::Uuid(const Uuid& _uuid) :
    uuid {_uuid.uuid}
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
    oss << "uuid: " << uuid;
    oss << "]";

    return oss.str();
  }

  std::strong_ordering
  Uuid::operator<=>(const Uuid& rhs) const
  {
    // boost::uuids::uuid doesn't have operator<=>() yet.
    if (uuid < rhs.uuid) {
      return std::strong_ordering::less;
    }
    if (uuid > rhs.uuid) {
      return std::strong_ordering::greater;
    }
    return std::strong_ordering::equal;
  }

  bool
  Uuid::operator==(const Uuid& rhs) const
  {
    return 0 == operator<=>(rhs);
  }

  std::ostream&
  operator<<(std::ostream& os, const Uuid& ao)
  {
    return os << ao.toString();
  }
}
